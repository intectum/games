#pragma once

#include <ludo/api.h>

namespace astrum
{
  void control_map(const ludo::instance& inst, const std::vector<ludo::container>& containers, const ludo::window& window, map_controls& map_controls, ludo::vec3& camera_position, ludo::quat& camera_rotation);
}
