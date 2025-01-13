#pragma once

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{

  float luna_height(const ludo::vec3& position);

  inline terrain_funcs luna_funcs
  {
    .height = luna_height,
    .color = [](float longitude, const std::array<float, 3>& heights, float gradient) { return ludo::vec4_one; }
  };
}
