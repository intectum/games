#include <fstream>

#include <FreeImagePlus.h>

#include <ludo/opengl/built_ins.h>

#include "../types.h"
#include "atmosphere.h"
#include "util.h"

namespace astrum
{
  void add_atmosphere(ludo::instance& inst, uint64_t mesh_id, uint32_t celestial_body_index, float planet_radius, float atmosphere_radius)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto& previous_frame_buffer = frame_buffers[frame_buffers.array_size - 1];

    auto fragment_stream = std::ifstream("assets/shaders/atmosphere.frag");
    auto fragment_shader = ludo::add(inst, ludo::shader { .type = ludo::shader_type::FRAGMENT }, fragment_stream);
    auto render_program = ludo::add(inst, ludo::render_program { .vertex_shader_id = ludo::post_processing_vertex_shader(inst), .fragment_shader_id = fragment_shader->id }, "atmosphere");
    render_program->data_buffer = ludo::allocate_vram(16 * 3);

    auto image = fipImage();
    image.load("assets/effects/blue-noise.png");

    auto blue_noise_texture = ludo::add(inst, ludo::texture { .format = ludo::pixel_format::RGBA, .width = image.getWidth(), .height = image.getHeight() });

    ludo::write(*blue_noise_texture, reinterpret_cast<std::byte*>(image.accessPixels()));

    auto byte_index = 0;
    ludo::write(render_program->data_buffer, 0, *blue_noise_texture);
    byte_index += 16;
    byte_index += 12; // skip planet_t.position
    ludo::write(render_program->data_buffer, byte_index, planet_radius);
    byte_index += 4;
    ludo::write(render_program->data_buffer, byte_index, atmosphere_radius);

    ludo::add<ludo::script>(inst, [celestial_body_index](ludo::instance& inst)
    {
      auto render_program = ludo::first<ludo::render_program>(inst, "atmosphere");
      auto& celestial_body_point_masses = ludo::data<point_mass>(inst, "celestial-bodies");

      auto byte_index = 16;
      ludo::write(render_program->data_buffer, byte_index, celestial_body_point_masses[celestial_body_index].transform.position);
    });

    ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
    {
      .frame_buffer_id = add_post_processing_frame_buffer(inst)->id,
      .mesh_ids = { mesh_id },
      .render_program_id = render_program->id,
      .data_buffer = create_post_processing_data_buffer(previous_frame_buffer.color_texture_ids[0], previous_frame_buffer.depth_texture_id)
    });
  }
}
