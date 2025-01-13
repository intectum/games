#include "tone_mapping.h"
#include "util.h"

namespace astrum
{
  std::tuple<ludo::render_program, ludo::buffer, ludo::frame_buffer> build_tone_mapper(const ludo::window& window, const ludo::frame_buffer& previous_frame_buffer)
  {
    auto render_program = ludo::render_program { .format = ludo::vertex_format_pt };
    ludo::init(render_program, ludo::asset_folder + "/shaders/post.vert",
               ludo::asset_folder + "/shaders/tone_mapping.frag");

    return {
      render_program,
      build_post_processing_data_buffer(previous_frame_buffer.color_texture_ids[0], 0),
      build_post_processing_frame_buffer(window)
    };
  }
}
