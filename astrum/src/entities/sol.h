#ifndef ASTRUM_ENTITIES_SOL_H
#define ASTRUM_ENTITIES_SOL_H

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  void add_sol(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity);

  void sync_light_with_sol(ludo::instance& inst);
}

#endif // ASTRUM_ENTITIES_SOL_H
