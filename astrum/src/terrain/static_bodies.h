#pragma once

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void update_terrain_static_bodies(ludo::instance& inst, terrain& terrain, float radius, const ludo::vec3& position, float point_mass_max_distance);
}
