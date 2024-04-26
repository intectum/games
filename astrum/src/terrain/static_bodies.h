#ifndef ASTRUM_TERRAIN_STATIC_BODIES_H
#define ASTRUM_TERRAIN_STATIC_BODIES_H

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void update_terrain_static_bodies(ludo::instance& inst, terrain& terrain, float radius, const ludo::vec3& position, float point_mass_max_distance);
}

#endif // ASTRUM_TERRAIN_STATIC_BODIES_H
