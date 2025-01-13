#pragma once

#include <ludo/api.h>

namespace astrum
{
  std::tuple<std::vector<ludo::render_program>, std::vector<ludo::buffer>, std::vector<ludo::frame_buffer>> build_bloom(const ludo::window& window, const ludo::frame_buffer& previous_frame_buffer, uint32_t iterations, float final_texture_size);
}
