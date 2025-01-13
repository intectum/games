#pragma once

#include <ludo/api.h>

namespace astrum
{
  uint32_t relativize_universe(std::vector<ludo::container>& containers, uint32_t relative_celestial_body_index, const ludo::vec3& camera_position);
}
