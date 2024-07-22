#pragma once

#include <ludo/api.h>

namespace astrum
{
  std::vector<ludo::contact> deepest_contacts(const std::vector<ludo::contact>& contacts);

  float orbital_speed(float orbit_radius, float mass_of_larger_body);
}
