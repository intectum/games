#include <fstream>

#include "hdr_resolve.h"
#include "util.h"

namespace astrum
{
  void add_hdr_resolve(ludo::instance& inst, uint64_t vertex_shader_id, uint64_t mesh_id)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto& previous_frame_buffer = frame_buffers[frame_buffers.array_size - 1];

    auto fragment_stream = std::ifstream("assets/shaders/hdr.frag");
    auto fragment_shader = ludo::add(inst, ludo::shader(), ludo::shader_type::FRAGMENT, fragment_stream);
    auto render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .vertex_shader_id = vertex_shader_id,
        .fragment_shader_id = fragment_shader->id,
        .format = ludo::vertex_format_pt
      }
    );

    ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
    {
      .frame_buffer_id = add_post_processing_frame_buffer(inst)->id,
      .render_program_id = render_program->id,
      .mesh_ids = { mesh_id },
      .shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer.color_texture_ids[0], 0)
    });
  }
}
