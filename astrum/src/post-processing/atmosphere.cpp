#include <cmath>
#include <fstream>

#include <ludo/opengl/textures.h>
#include <ludo/opengl/util.h>

#include "atmosphere.h"
#include "util.h"

namespace astrum
{
  void write_atmospheric_density_texture();
  void write_optical_depth_texture(float atmosphere_radius);
  ludo::vec2 ray_sphere_intersections(const ludo::vec3& ray_origin, const ludo::vec3& ray_direction, const ludo::vec3& sphere_center, float sphere_radius);
  float optical_depth(float atmosphere_radius, const ludo::vec3& ray_origin, const ludo::vec3& ray_direction, float ray_length);
  float atmospheric_density(float altitude);
  float normalized_altitude(float atmosphere_radius, const ludo::vec3& position);

  const auto map_size = uint32_t(1024);
  const auto atmospheric_density_scale_height = 0.25f;
  const auto optical_depth_samples = uint32_t(50);

  const auto data_buffer_size = 5 * sizeof(uint64_t) + 8 /* align 16 */ + sizeof(ludo::vec3) + 2 * sizeof(float);

  std::tuple<ludo::render_program, ludo::buffer, ludo::frame_buffer> build_atmosphere(const ludo::window& window, const ludo::frame_buffer& previous_frame_buffer, float planet_radius, float atmosphere_radius)
  {
    auto render_program = ludo::render_program{ .format = ludo::vertex_format_pt };
    ludo::init(render_program, ludo::asset_folder + "/shaders/post.vert", ludo::asset_folder + "/shaders/atmosphere.frag");

    auto data_buffer = build_post_processing_data_buffer(previous_frame_buffer.color_texture_ids[0], previous_frame_buffer.depth_texture_id, data_buffer_size);
    auto data_stream = ludo::stream(data_buffer, 16);

    auto map_data_size = uint32_t(map_size * map_size * sizeof(float));

    auto atmospheric_density_stream = std::ifstream(ludo::asset_folder + "/effects/atmospheric-density.map", std::ios::binary);
    auto atmospheric_density_data = new std::byte[map_data_size];
    atmospheric_density_stream.read(reinterpret_cast<char *>(atmospheric_density_data), map_data_size);

    auto atmospheric_density_texture = ludo::texture{.components = ludo::pixel_components::R, .datatype = ludo::pixel_datatype::FLOAT32, .width = map_size, .height = map_size};
    ludo::init(atmospheric_density_texture, {.clamp = true});
    ludo::write(atmospheric_density_texture, atmospheric_density_data);
    ludo::write(data_stream, ludo::handle(atmospheric_density_texture));

    delete[] atmospheric_density_data;

    auto optical_depth_stream = std::ifstream(ludo::asset_folder + "/effects/optical-depth.map", std::ios::binary);
    auto optical_depth_data = new std::byte[map_data_size];
    optical_depth_stream.read(reinterpret_cast<char *>(optical_depth_data), map_data_size);

    auto optical_depth_texture = ludo::texture{.components = ludo::pixel_components::R, .datatype = ludo::pixel_datatype::FLOAT32, .width = map_size, .height = map_size};
    ludo::init(optical_depth_texture, {.clamp = true});
    ludo::write(optical_depth_texture, optical_depth_data);
    ludo::write(data_stream, ludo::handle(optical_depth_texture));

    delete[] optical_depth_data;

    auto blue_noise_texture = ludo::load(ludo::asset_folder + "/effects/blue-noise.png");
    ludo::write(data_stream, ludo::handle(blue_noise_texture));

    data_stream.position += 8; // align 16
    data_stream.position += 12; // skip planet_t.position

    ludo::write(data_stream, planet_radius);
    ludo::write(data_stream, atmosphere_radius);

    return {
      render_program,
      data_buffer,
      build_post_processing_frame_buffer(window)
    };
  }

  void write_atmosphere_textures(float atmosphere_radius)
  {
    write_atmospheric_density_texture();
    write_optical_depth_texture(atmosphere_radius);
  }

  void write_atmospheric_density_texture()
  {
    auto stream = std::ofstream(ludo::asset_folder + "/effects/atmospheric-density.map", std::ios::binary);

    for (auto row = 0; row < map_size; row++)
    {
      auto altitude = 1.0f / static_cast<float>(map_size) * static_cast<float>(row);

      for (auto column = 0; column < map_size; column++)
      {
        auto value = atmospheric_density(altitude);
        stream.write(reinterpret_cast<const char*>(&value), sizeof(float));
      }
    }
  }

  void write_optical_depth_texture(float atmosphere_radius)
  {
    auto stream = std::ofstream(ludo::asset_folder + "/effects/optical-depth.map", std::ios::binary);

    for (auto row = 0; row < map_size; row++)
    {
      auto altitude = 1.0f / static_cast<float>(map_size) * static_cast<float>(row);
      auto scaled_altitude = (atmosphere_radius - 1.0f) * altitude;

      auto ray_origin = ludo::vec3 { 0.0f, 1.0f + scaled_altitude, 0.0f };

      for (auto column = 0; column < map_size; column++)
      {
        auto ray_angle = ludo::pi / static_cast<float>(map_size) * static_cast<float>(column);

        auto ray_direction2 = ludo::vec2(0.0f, 1.0f);
        ludo::rotate(ray_direction2, ray_angle);
        auto ray_direction = ludo::vec3(ray_direction2[0], ray_direction2[1], 0.0f);

        auto ray_intersections = ray_sphere_intersections(ray_origin, ray_direction, ludo::vec3_zero, atmosphere_radius);
        auto ray_length = ray_intersections[1] - ray_intersections[0];

        auto value = optical_depth(atmosphere_radius, ray_origin, ray_direction, ray_length) / 512.0f;
        stream.write(reinterpret_cast<const char*>(&value), sizeof(float));
      }
    }
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
      auto s = std::sqrt(d);
      auto distance_to_sphere_far = (-b + s) / (2.0f * a);

      if (distance_to_sphere_far >= 0)
      {
        float distance_to_sphere_near = std::max(0.0f, (-b - s) / (2.0f * a));
        return { distance_to_sphere_near, distance_to_sphere_far };
      }
    }

    return { 0.0f, 0.0f };
  }

  float optical_depth(float atmosphere_radius, const ludo::vec3& ray_origin, const ludo::vec3& ray_direction, float ray_length)
  {
    auto step_size = ray_length / static_cast<float>(optical_depth_samples + 1);

    auto depth = 0.0f;
    for (auto sample_index = 1; sample_index <= optical_depth_samples; sample_index++)
    {
      auto sample_position = ray_origin + ray_direction * static_cast<float>(sample_index) * step_size;
      auto sample_altitude = normalized_altitude(atmosphere_radius, sample_position);
      depth += atmospheric_density(sample_altitude) * step_size;
    }

    return depth;
  }

  float atmospheric_density(float altitude)
  {
    auto density = std::exp(-altitude / atmospheric_density_scale_height);

    // Ensure the density is 0.0 at the atmosphere radius.
    density *= 1.0f - altitude;

    return density;
  }

  float normalized_altitude(float atmosphere_radius, const ludo::vec3& position)
  {
    auto altitude = length(position) - 1.0f;
    return altitude / (atmosphere_radius - 1.0f);
  }
}
