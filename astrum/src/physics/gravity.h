#pragma once

#include <ludo/api.h>

namespace astrum
{
  void simulate_gravity(ludo::instance& inst, std::vector<ludo::container>& containers, uint32_t relative_celestial_body_index);
}
