#pragma once

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void load_terrain_chunk(const terrain& terrain, float radius, uint32_t chunk_index, uint32_t lod_index, ludo::mesh& mesh);
}
