#pragma once

#include <ludo/api.h>

namespace astrum
{
  std::tuple<ludo::render_program, ludo::buffer, ludo::frame_buffer> build_tone_mapper(const ludo::window& window, const ludo::frame_buffer& previous_frame_buffer);
}
