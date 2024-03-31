#ifndef ASTRUM_ENTITIES_CELESTIAL_BODIES_H
#define ASTRUM_ENTITIES_CELESTIAL_BODIES_H

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void relativize_to_nearest_celestial_body(ludo::instance& inst);

  void add_celestial_body(ludo::instance& inst, const celestial_body& init, const std::vector<lod>& lods, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity);

  void update_celestial_bodies(ludo::instance& inst, const std::vector<std::vector<lod>>& lods);

  std::pair<uint32_t, uint32_t> celestial_body_counts(const std::vector<lod>& lods);
}

#endif // ASTRUM_ENTITIES_CELESTIAL_BODIES_H
