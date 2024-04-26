#ifndef ASTRUM_ENTITIES_SOL_H
#define ASTRUM_ENTITIES_SOL_H

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  void add_sol(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity);

  float sol_height(const ludo::vec3& position);

  ludo::vec4 sol_color(float longitude, const std::array<float, 3>& heights, float gradient);

  std::vector<tree> sol_tree(uint64_t patch_id);

  void sync_light_with_sol(ludo::instance& inst);
}

#endif // ASTRUM_ENTITIES_SOL_H
