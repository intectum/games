#ifndef ASTRUM_TERRAIN_TERRAIN_CHUNK_H
#define ASTRUM_TERRAIN_TERRAIN_CHUNK_H

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void load_terrain_chunk(const terrain& terrain, float radius, uint32_t chunk_index, uint32_t lod_index, ludo::mesh& mesh);

  void init_terrain_chunk(const terrain& terrain, uint32_t chunk_index, ludo::mesh_instance& mesh_instance);

  uint32_t terrain_chunk_lod_index(const terrain& terrain, uint32_t chunk_index, const std::vector<lod>& lods, const ludo::vec3& camera_position, const ludo::vec3& position);
}

#endif // ASTRUM_TERRAIN_TERRAIN_CHUNK_H
