#include <fstream>

#include <ludo/opengl/built_ins.h>

#include "bloom.h"
#include "util.h"

namespace astrum
{
  void add_bloom(ludo::instance& inst, uint64_t mesh_id, uint32_t iterations, float final_texture_size)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto previous_frame_buffer = &frame_buffers[frame_buffers.array_size - 1];
    auto original_frame_buffer = previous_frame_buffer;

    auto vertex_shader_id = ludo::post_processing_vertex_shader(inst);

    auto brightness_fragment_stream = std::ifstream("assets/shaders/brightness.frag");
    auto brightness_fragment_shader = ludo::add(inst, ludo::shader { .type = ludo::shader_type::FRAGMENT }, brightness_fragment_stream);
    auto brightness_render_program = ludo::add(inst, ludo::render_program { .vertex_shader_id = vertex_shader_id, .fragment_shader_id = brightness_fragment_shader->id });

    auto gaussian_fragment_stream = std::ifstream("assets/shaders/gaussian.frag");
    auto gaussian_fragment_shader = ludo::add(inst, ludo::shader { .type = ludo::shader_type::FRAGMENT }, gaussian_fragment_stream);
    auto gaussian_render_program = ludo::add(inst, ludo::render_program { .vertex_shader_id = vertex_shader_id, .fragment_shader_id = gaussian_fragment_shader->id });

    auto additive_fragment_stream = std::ifstream("assets/shaders/additive.frag");
    auto additive_fragment_shader = ludo::add(inst, ludo::shader { .type = ludo::shader_type::FRAGMENT }, additive_fragment_stream);
    auto additive_render_program = ludo::add(inst, ludo::render_program { .vertex_shader_id = vertex_shader_id, .fragment_shader_id = additive_fragment_shader->id });

    previous_frame_buffer = add_post_processing_frame_buffer(inst);

    ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
    {
      .frame_buffer_id = previous_frame_buffer->id,
      .mesh_ids = { mesh_id },
      .render_program_id = brightness_render_program->id,
      .data_buffer = create_post_processing_data_buffer(previous_frame_buffer->color_texture_ids[0], 0)
    });

    for (auto iteration = 0; iteration < iterations; iteration++)
    {
      auto texture_size = 1.0f - static_cast<float>(iteration) / static_cast<float>(iterations) * (1.0f - final_texture_size);

      auto horizontal_data_buffer = create_post_processing_data_buffer(previous_frame_buffer->color_texture_ids[0], 0);
      ludo::write(horizontal_data_buffer, 8, true);
      previous_frame_buffer = add_post_processing_frame_buffer(inst, false, texture_size);

      ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
      {
        .frame_buffer_id = previous_frame_buffer->id,
        .mesh_ids = { mesh_id },
        .render_program_id = gaussian_render_program->id,
        .data_buffer = horizontal_data_buffer
      });

      auto vertical_data_buffer = create_post_processing_data_buffer(previous_frame_buffer->color_texture_ids[0], 0);
      ludo::write(vertical_data_buffer, 8, false);
      previous_frame_buffer = add_post_processing_frame_buffer(inst, false, texture_size);

      ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
      {
        .frame_buffer_id = previous_frame_buffer->id,
        .mesh_ids = { mesh_id },
        .render_program_id = gaussian_render_program->id,
        .data_buffer = vertical_data_buffer
      });
    }

    ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
    {
      .frame_buffer_id = add_post_processing_frame_buffer(inst)->id,
      .mesh_ids = { mesh_id },
      .render_program_id = additive_render_program->id,
      .data_buffer = create_post_processing_data_buffer(original_frame_buffer->color_texture_ids[0], previous_frame_buffer->color_texture_ids[0])
    });
  }
}
