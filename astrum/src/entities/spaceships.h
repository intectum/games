#pragma once

#include <ludo/api.h>

namespace astrum
{
  void add_spaceship(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity);

  void enter_spaceship(ludo::instance& inst, uint32_t person_index, uint32_t spaceship_index);

  void exit_spaceship(ludo::instance& inst, uint32_t person_index, uint32_t spaceship_index);

  void simulate_spaceships(ludo::instance& inst);
}
