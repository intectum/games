#pragma once

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  void add_luna(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity);
}
