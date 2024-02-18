#ifndef ASTRUM_POST_PROCESSING_ATMOSPHERE_H
#define ASTRUM_POST_PROCESSING_ATMOSPHERE_H

#include <ludo/api.h>

namespace astrum
{
  void add_atmosphere(ludo::instance& inst, uint64_t mesh_id, uint32_t celestial_body_index, float planet_radius, float atmosphere_radius);
}

#endif // ASTRUM_POST_PROCESSING_ATMOSPHERE_H
