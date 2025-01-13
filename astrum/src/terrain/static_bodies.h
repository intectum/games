#pragma once

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void update_terrain_static_bodies(ludo::container& container, terrain& terrain, float radius, const ludo::vec3& position, float point_mass_max_distance);
}
