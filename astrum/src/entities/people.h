#pragma once

#include <ludo/api.h>

namespace astrum
{
  void simulate_people(const ludo::instance& inst, std::vector<ludo::container>& containers, const ludo::animation& animation, const ludo::armature& armature, uint32_t relative_celestial_body_index);
}
