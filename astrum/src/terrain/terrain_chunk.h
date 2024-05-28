#ifndef ASTRUM_TERRAIN_TERRAIN_CHUNK_H
#define ASTRUM_TERRAIN_TERRAIN_CHUNK_H

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void load_terrain_chunk(const terrain& terrain, float radius, uint32_t chunk_index, uint32_t lod_index, ludo::mesh& mesh);
}

#endif // ASTRUM_TERRAIN_TERRAIN_CHUNK_H
