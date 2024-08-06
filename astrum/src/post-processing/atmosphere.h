#pragma once

#include <ludo/api.h>

namespace astrum
{
  ludo::frame_buffer add_atmosphere(ludo::instance& inst, float planet_radius, float atmosphere_radius);

  void commit_atmosphere_render_commands(ludo::instance& inst, const ludo::frame_buffer& frame_buffer, const ludo::render_mesh& render_mesh, uint32_t celestial_body_index);

  void write_atmosphere_textures(float atmosphere_radius);
}
