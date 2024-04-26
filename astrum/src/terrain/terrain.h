#ifndef ASTRUM_TERRAIN_TERRAIN_H
#define ASTRUM_TERRAIN_TERRAIN_H

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void add_terrain(ludo::instance& inst, const terrain& init, const celestial_body& celestial_body, const std::string& partition = "default");

  std::pair<uint32_t, uint32_t> terrain_counts(const std::vector<lod>& lods);

  void stream_terrain(ludo::instance& inst);
}

#endif // ASTRUM_TERRAIN_TERRAIN_H
