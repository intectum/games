#ifndef ASTRUM_ENTITIES_TERRA_H
#define ASTRUM_ENTITIES_TERRA_H

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  void add_terra(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity);

  float terra_height(const ludo::vec3& position);

  ludo::vec4 terra_color(float longitude, const std::array<float, 3>& heights, float gradient);

  std::vector<tree> terra_tree(uint32_t patch_index);
}

#endif // ASTRUM_ENTITIES_TERRA_H
