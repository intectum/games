#ifndef ASTRUM_ENTITIES_LUNA_H
#define ASTRUM_ENTITIES_LUNA_H

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  float luna_height(const ludo::vec3& position);

  ludo::vec4 luna_color(float longitude, const std::array<float, 3>& heights, float gradient);

  std::vector<tree> luna_tree(uint64_t patch_id);
}

#endif // ASTRUM_ENTITIES_LUNA_H
