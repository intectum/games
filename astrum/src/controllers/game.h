#pragma once

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  void control_game(const ludo::instance& inst, std::vector<ludo::container>& containers, const ludo::window& window, game_controls& game_controls, map_controls& map_controls, ludo::vec3& camera_position, ludo::quat& camera_rotation);
}
