#pragma once

#include <ludo/api.h>

namespace astrum
{
  ludo::frame_buffer add_tone_mapping(ludo::instance& inst);

  void commit_tone_mapping_render_commands(ludo::instance& inst, const ludo::frame_buffer& frame_buffer, const ludo::render_mesh& render_mesh);
}
