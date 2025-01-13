#pragma once

#include <ludo/api.h>

#include "../constants.h"
#include "../types.h"

namespace astrum
{
  void load_terrain_chunk(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, float radius, const std::vector<lod>& lods, const ludo::vertex_format& format, uint32_t chunk_index, uint32_t lod_index);
}
