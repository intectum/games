#pragma once

#include <ludo/api.h>

namespace astrum
{
  void add_atmosphere(ludo::instance& inst, const ludo::render_mesh& render_mesh, uint32_t celestial_body_index, float planet_radius, float atmosphere_radius);

  void write_atmosphere_texture(uint32_t sample_count, float scale_height, float planet_radius, float atmosphere_radius, const std::string& texture_filename, uint32_t texture_size);
}
