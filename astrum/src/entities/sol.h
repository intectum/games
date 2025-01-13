#pragma once

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  inline terrain_funcs sol_funcs
  {
    .height = [](const ludo::vec3& position) { return 1.0f; },
    .color = [](float longitude, const std::array<float, 3>& heights, float gradient) { return ludo::vec4_one; }
  };

  void sync_light_with_sol(ludo::container& container);
}
