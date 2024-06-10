#pragma once

#include <ludo/api.h>

namespace astrum
{
  void add_atmosphere(ludo::instance& inst, const ludo::render_mesh& render_mesh, uint32_t celestial_body_index, float planet_radius, float atmosphere_radius);

  void write_atmosphere_textures(float atmosphere_radius);
}
