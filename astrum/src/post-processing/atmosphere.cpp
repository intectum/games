#include <fstream>

#include <FreeImagePlus.h>

#include <ludo/opengl/textures.h>
#include <ludo/opengl/util.h>

#include "../constants.h"
#include "../physics/point_masses.h"
#include "atmosphere.h"
#include "util.h"

namespace astrum
{
  ludo::vec2 ray_sphere_intersections(const ludo::vec3& ray_origin, const ludo::vec3& ray_direction, const ludo::vec3& sphere_center, float sphere_radius);
  float optical_depth(uint32_t sample_count, float scale_height, float planet_radius, float atmosphere_radius, const ludo::vec3& ray_origin, const ludo::vec3& ray_direction, float ray_length);
  float atmospheric_density(float scale_height, float altitude);
  float normalized_altitude(float planet_radius, float atmosphere_radius, const ludo::vec3& position);

  void add_atmosphere(ludo::instance& inst, uint64_t vertex_shader_id, uint64_t mesh_instance_id, uint32_t celestial_body_index, float planet_radius, float atmosphere_radius)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto& previous_frame_buffer = frame_buffers[frame_buffers.array_size - 1];

    auto& vram_draw_commands = data_heap<ludo::draw_command>(inst);

    auto fragment_stream = std::ifstream("assets/shaders/atmosphere.frag");
    auto fragment_shader = ludo::add(inst, ludo::shader(), ludo::shader_type::FRAGMENT, fragment_stream);
    auto render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .vertex_shader_id = vertex_shader_id,
        .fragment_shader_id = fragment_shader->id,
        .format = ludo::vertex_format_pt,
        .command_buffer = ludo::allocate(vram_draw_commands, sizeof(ludo::draw_command)),
        .shader_buffer = ludo::allocate_vram(16 * 3)
      },
      "atmosphere"
    );
    auto byte_index = 0;

    auto atmosphere_image = fipImage();
    atmosphere_image.load("assets/effects/atmosphere.tiff");

    auto atmosphere_texture = ludo::add(inst, ludo::texture { .datatype = ludo::pixel_datatype::FLOAT32, .width = atmosphere_image.getWidth(), .height = atmosphere_image.getHeight() }, { .clamp = true });

    ludo::write(*atmosphere_texture, reinterpret_cast<std::byte*>(atmosphere_image.accessPixels()));
    ludo::write(render_program->shader_buffer, byte_index, ludo::handle(*atmosphere_texture));
    byte_index += 8;

    auto blue_noise_image = fipImage();
    blue_noise_image.load("assets/effects/blue-noise.png");

    auto blue_noise_texture = ludo::add(inst, ludo::texture { .components = ludo::pixel_components::RGBA, .width = blue_noise_image.getWidth(), .height = blue_noise_image.getHeight() });

    ludo::write(*blue_noise_texture, reinterpret_cast<std::byte*>(blue_noise_image.accessPixels()));
    ludo::write(render_program->shader_buffer, byte_index, ludo::handle(*blue_noise_texture));
    byte_index += 8;

    byte_index += 12; // skip planet_t.position
    ludo::write(render_program->shader_buffer, byte_index, planet_radius);
    byte_index += 4;
    ludo::write(render_program->shader_buffer, byte_index, atmosphere_radius);

    ludo::add<ludo::script>(inst, [celestial_body_index](ludo::instance& inst)
    {
      auto render_program = ludo::first<ludo::render_program>(inst, "atmosphere");
      auto& celestial_body_point_masses = ludo::data<point_mass>(inst, "celestial-bodies");

      ludo::write(render_program->shader_buffer, 16, celestial_body_point_masses[celestial_body_index].transform.position);
    });

    ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
    {
      .frame_buffer_id = add_post_processing_frame_buffer(inst)->id,
      .render_program_id = render_program->id,
      .mesh_instance_ids = { mesh_instance_id },
      .shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer.color_texture_ids[0], previous_frame_buffer.depth_texture_id)
    });
  }

  void write_atmosphere_texture(uint32_t sample_count, float scale_height, float planet_radius, float atmosphere_radius, const std::string& texture_filename, uint32_t texture_size)
  {
    auto image = fipImage(FIT_RGBF, texture_size, texture_size, 96);

    for (auto row = 0; row < texture_size; row++)
    {
      auto image_data = reinterpret_cast<float*>(image.getScanLine(row));
      auto altitude = 1.0f / static_cast<float>(texture_size) * static_cast<float>(row);
      auto scaled_altitude = (atmosphere_radius - planet_radius) * altitude;

      auto ray_origin = ludo::vec3 { 0.0f, planet_radius + scaled_altitude, 0.0f };

      for (auto column = 0; column < texture_size; column++)
      {
        auto ray_angle = ludo::pi / static_cast<float>(texture_size) * static_cast<float>(column);

        auto ray_direction2 = ludo::vec2(0.0f, 1.0f);
        ludo::rotate(ray_direction2, ray_angle);
        auto ray_direction = ludo::vec3(ray_direction2[0], ray_direction2[1], 0.0f);

        auto ray_intersections = ray_sphere_intersections(ray_origin, ray_direction, ludo::vec3_zero, atmosphere_radius);
        auto ray_length = ray_intersections[1] - ray_intersections[0];

        // NOTE: Red and blue are flipped when the texture is saved...
        // A bug in FreeImage when saving to TIFF?
        image_data[FI_RGBA_BLUE] = atmospheric_density(scale_height, altitude);
        image_data[FI_RGBA_GREEN] = optical_depth(sample_count, scale_height, planet_radius, atmosphere_radius, ray_origin, ray_direction, ray_length) / 512.0f;
        image_data[FI_RGBA_RED] = 0.0f;
        image_data += 3;
      }
    }

    image.save(texture_filename.c_str());
    image.clear();
  }

  ludo::vec2 ray_sphere_intersections(const ludo::vec3& ray_origin, const ludo::vec3& ray_direction, const ludo::vec3& sphere_center, float sphere_radius)
  {
    auto offset = ray_origin - sphere_center;
    auto a = 1.0f; // set to dot(ray_direction, ray_direction) if ray_direction might not be normalized.
    auto b = 2.0f * dot(offset, ray_direction);
    auto c = dot(offset, offset) - sphere_radius * sphere_radius;
    auto d = b * b - 4.0f * a * c; // Discriminant from quadratic formula.

    // Number of intersections: 0 when d < 0; 1 when d = 0; 2 when d > 0.
    if (d > 0.0f)
    {
      auto s = sqrt(d);
      auto distance_to_sphere_far = (-b + s) / (2.0f * a);

      if (distance_to_sphere_far >= 0)
      {
        float distance_to_sphere_near = std::max(0.0f, (-b - s) / (2.0f * a));
        return { distance_to_sphere_near, distance_to_sphere_far };
      }
    }

    return { 0.0f, 0.0f };
  }

  float optical_depth(uint32_t sample_count, float scale_height, float planet_radius, float atmosphere_radius, const ludo::vec3& ray_origin, const ludo::vec3& ray_direction, float ray_length)
  {
    auto step_size = ray_length / static_cast<float>(sample_count + 1);

    auto depth = 0.0f;
    for (auto sample_index = 1; sample_index <= sample_count; sample_index++)
    {
      auto sample_position = ray_origin + ray_direction * static_cast<float>(sample_index) * step_size;
      auto sample_altitude = normalized_altitude(planet_radius, atmosphere_radius, sample_position);
      depth += atmospheric_density(scale_height, sample_altitude) * step_size;
    }

    return depth;
  }

  float atmospheric_density(float scale_height, float altitude)
  {
    auto density = std::exp(-altitude / scale_height);

    // Ensure the density is 0.0 at the atmosphere radius.
    density *= 1.0f - altitude;

    return density;
  }

  float normalized_altitude(float planet_radius, float atmosphere_radius, const ludo::vec3& position)
  {
    auto altitude = length(position) - planet_radius;
    return altitude / (atmosphere_radius - planet_radius);
  }
}
