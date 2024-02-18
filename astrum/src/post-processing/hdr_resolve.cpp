#include <fstream>

#include <ludo/opengl/built_ins.h>

#include "hdr_resolve.h"
#include "util.h"

namespace astrum
{
  void add_hdr_resolve(ludo::instance& inst, uint64_t mesh_id)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto& previous_frame_buffer = frame_buffers[frame_buffers.array_size - 1];

    auto fragment_stream = std::ifstream("assets/shaders/hdr.frag");
    auto fragment_shader = ludo::add(inst, ludo::shader { .type = ludo::shader_type::FRAGMENT }, fragment_stream);
    auto render_program = ludo::add(inst, ludo::render_program { .vertex_shader_id = ludo::post_processing_vertex_shader(inst), .fragment_shader_id = fragment_shader->id });

    ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
    {
      .frame_buffer_id = add_post_processing_frame_buffer(inst)->id,
      .mesh_ids = { mesh_id },
      .render_program_id = render_program->id,
      .data_buffer = create_post_processing_data_buffer(previous_frame_buffer.color_texture_ids[0], 0)
    });
  }
}
