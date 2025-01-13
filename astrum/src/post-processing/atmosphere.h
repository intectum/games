#pragma once

#include <ludo/api.h>

namespace astrum
{
  std::tuple<ludo::render_program, ludo::buffer, ludo::frame_buffer> build_atmosphere(const ludo::window& window, const ludo::frame_buffer& previous_frame_buffer, float planet_radius, float atmosphere_radius);

  void write_atmosphere_textures(float atmosphere_radius);
}
