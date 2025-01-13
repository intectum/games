#pragma once

#include <ludo/api.h>

namespace astrum
{
  void control_spaceship(ludo::container& container, const ludo::window& window, uint32_t index, ludo::vec3& camera_position, ludo::quat& camera_rotation);
}
