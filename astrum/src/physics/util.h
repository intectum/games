#ifndef ASTRUM_PHYSICS_UTIL_H
#define ASTRUM_PHYSICS_UTIL_H

#include <ludo/api.h>

namespace astrum
{
  ludo::contact* deepest_contact(std::vector<ludo::contact>& contacts, const ludo::body* other_body);

  std::vector<ludo::contact*> deepest_contacts(std::vector<ludo::contact>& contacts);

  float orbital_speed(float orbit_radius, float mass_of_larger_body);
}

#endif // ASTRUM_PHYSICS_UTIL_H
