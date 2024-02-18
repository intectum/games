#ifndef ASTRUM_ENTITIES_PEOPLE_H
#define ASTRUM_ENTITIES_PEOPLE_H

#include <ludo/api.h>

namespace astrum
{
  void add_person(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity);

  void simulate_people(ludo::instance& inst);
}

#endif // ASTRUM_ENTITIES_PEOPLE_H
