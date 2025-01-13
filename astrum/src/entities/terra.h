#pragma once

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  float terra_height(const ludo::vec3& position);

  ludo::vec4 terra_color(float longitude, const std::array<float, 3>& heights, float gradient);

  //std::array<std::vector<tree>, tree_type_count> terra_tree(float radius, uint32_t chunk_index); TODO

  inline terrain_funcs terra_funcs
  {
    .height = terra_height,
    .color = terra_color
  };
}
