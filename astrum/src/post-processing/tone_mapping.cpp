#include <ludo/opengl/util.h>

#include "tone_mapping.h"
#include "util.h"

namespace astrum
{
  void add_tone_mapping(ludo::instance& inst, const ludo::render_mesh& render_mesh)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto& previous_frame_buffer = frame_buffers[frame_buffers.length - 1];

    auto& render_commands = data_heap(inst, "ludo::vram_render_commands");

    auto render_program = ludo::add(inst, ludo::render_program { .format = ludo::vertex_format_pt });
    ludo::init(*render_program, ludo::asset_folder + "/shaders/post.vert", ludo::asset_folder + "/shaders/tone_mapping.frag", render_commands, 1);
    render_program->shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer.color_texture_ids[0], 0);

    auto frame_buffer = add_post_processing_frame_buffer(inst);

    ludo::add<ludo::script>(inst, [=](ludo::instance& inst)
    {
      auto rendering_context = ludo::first<ludo::rendering_context>(inst);
      auto& render_programs = ludo::data<ludo::render_program>(inst);

      auto& render_commands = ludo::data_heap(inst, "ludo::vram_render_commands");
      auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
      auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

      ludo::use_and_clear(*frame_buffer);
      ludo::add_render_command(*render_program, render_mesh);
      ludo::commit_render_commands(*rendering_context, render_programs, render_commands, indices, vertices);
    });
  }
}
