#pragma once

#include <ludo/api.h>

namespace astrum
{
  void enter_spaceship(ludo::container& container, uint32_t person_index, uint32_t spaceship_index);

  void exit_spaceship(ludo::container& container, uint32_t person_index, uint32_t spaceship_index);

  void simulate_spaceships(const ludo::instance& inst, std::vector<ludo::container>& containers);
}
