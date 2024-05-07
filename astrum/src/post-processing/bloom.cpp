#include <fstream>

#include <ludo/opengl/util.h>

#include "bloom.h"
#include "util.h"

namespace astrum
{
  void add_bloom(ludo::instance& inst, uint64_t vertex_shader_id, uint64_t mesh_instance_id, uint32_t iterations, float final_texture_size)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto original_frame_buffer = &frame_buffers[frame_buffers.length - 1];

    auto& draw_commands = data_heap(inst, "ludo::vram_draw_commands");

    auto brightness_fragment_stream = std::ifstream("assets/shaders/brightness.frag");
    auto brightness_fragment_shader = ludo::add(inst, ludo::shader(), ludo::shader_type::FRAGMENT, brightness_fragment_stream);
    auto brightness_render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .vertex_shader_id = vertex_shader_id,
        .fragment_shader_id = brightness_fragment_shader->id,
        .format = ludo::vertex_format_pt,
        .command_buffer = ludo::allocate(draw_commands, sizeof(ludo::draw_command))
      }
    );

    auto gaussian_fragment_stream = std::ifstream("assets/shaders/gaussian.frag");
    auto gaussian_fragment_shader = ludo::add(inst, ludo::shader(), ludo::shader_type::FRAGMENT, gaussian_fragment_stream);
    auto gaussian_render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .vertex_shader_id = vertex_shader_id,
        .fragment_shader_id = gaussian_fragment_shader->id,
        .format = ludo::vertex_format_pt,
        .command_buffer = ludo::allocate(draw_commands, 2 * iterations * sizeof(ludo::draw_command))
      }
    );

    auto additive_fragment_stream = std::ifstream("assets/shaders/additive.frag");
    auto additive_fragment_shader = ludo::add(inst, ludo::shader(), ludo::shader_type::FRAGMENT, additive_fragment_stream);
    auto additive_render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .vertex_shader_id = vertex_shader_id,
        .fragment_shader_id = additive_fragment_shader->id,
        .format = ludo::vertex_format_pt,
        .command_buffer = ludo::allocate(draw_commands, sizeof(ludo::draw_command))
      }
    );

    auto previous_frame_buffer = original_frame_buffer;
    auto current_frame_buffer = add_post_processing_frame_buffer(inst);

    ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
    {
      .frame_buffer_id = current_frame_buffer->id,
      .render_program_id = brightness_render_program->id,
      .mesh_instance_ids = { mesh_instance_id },
      .shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer->color_texture_ids[0], 0)
    });

    for (auto iteration = 0; iteration < iterations; iteration++)
    {
      auto texture_size = 1.0f - static_cast<float>(iteration) / static_cast<float>(iterations) * (1.0f - final_texture_size);

      previous_frame_buffer = current_frame_buffer;
      current_frame_buffer = add_post_processing_frame_buffer(inst, false, texture_size);

      auto horizontal_shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer->color_texture_ids[0], 0);
      ludo::cast<bool>(horizontal_shader_buffer.back, 8) = true;

      ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
      {
        .frame_buffer_id = current_frame_buffer->id,
        .render_program_id = gaussian_render_program->id,
        .mesh_instance_ids = { mesh_instance_id },
        .shader_buffer = horizontal_shader_buffer
      });

      previous_frame_buffer = current_frame_buffer;
      current_frame_buffer = add_post_processing_frame_buffer(inst, false, texture_size);

      auto vertical_shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer->color_texture_ids[0], 0);
      ludo::cast<bool>(horizontal_shader_buffer.back, 8) = false;

      ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
      {
        .frame_buffer_id = current_frame_buffer->id,
        .render_program_id = gaussian_render_program->id,
        .mesh_instance_ids = { mesh_instance_id },
        .shader_buffer = vertical_shader_buffer
      });
    }

    previous_frame_buffer = current_frame_buffer;
    current_frame_buffer = add_post_processing_frame_buffer(inst);

    ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
    {
      .frame_buffer_id = current_frame_buffer->id,
      .render_program_id = additive_render_program->id,
      .mesh_instance_ids = { mesh_instance_id },
      .shader_buffer = create_post_processing_shader_buffer(original_frame_buffer->color_texture_ids[0], previous_frame_buffer->color_texture_ids[0])
    });
  }
}
