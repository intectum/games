#ifndef ASTRUM_ENTITIES_TERRA_H
#define ASTRUM_ENTITIES_TERRA_H

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  void add_terra(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity);
}

#endif // ASTRUM_ENTITIES_TERRA_H
