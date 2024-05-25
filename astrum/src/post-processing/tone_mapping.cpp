#include <fstream>

#include <ludo/opengl/util.h>

#include "tone_mapping.h"
#include "util.h"

namespace astrum
{
  void add_tone_mapping(ludo::instance& inst, uint64_t vertex_shader_id, const ludo::mesh_instance& mesh_instance)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto& previous_frame_buffer = frame_buffers[frame_buffers.length - 1];

    auto& draw_commands = data_heap(inst, "ludo::vram_draw_commands");

    auto fragment_stream = std::ifstream("assets/shaders/tone_mapping.frag");
    auto fragment_shader = ludo::add(inst, ludo::shader(), ludo::shader_type::FRAGMENT, fragment_stream);
    auto render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .vertex_shader_id = vertex_shader_id,
        .fragment_shader_id = fragment_shader->id,
        .format = ludo::vertex_format_pt,
        .command_buffer = ludo::allocate(draw_commands, sizeof(ludo::draw_command))
      }
    );

    auto frame_buffer = add_post_processing_frame_buffer(inst);
    auto shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer.color_texture_ids[0], 0);

    ludo::add<ludo::script>(inst, [=](ludo::instance& inst)
    {
      ludo::add_draw_command(*render_program, mesh_instance);
      ludo::render(inst,
      {
        .frame_buffer_id = frame_buffer->id,
        .shader_buffer = shader_buffer
      });
    });
  }
}
