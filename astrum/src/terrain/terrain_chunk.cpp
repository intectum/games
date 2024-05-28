#include "mesh.h"
#include "terrain_chunk.h"

namespace astrum
{
  void load_terrain_chunk(const terrain& terrain, float radius, uint32_t chunk_index, uint32_t lod_index, ludo::mesh& mesh)
  {
    auto& lowest_detail_lod = terrain.lods[0];

    auto& low_detail_lod = terrain.lods[lod_index > 0 ? lod_index - 1 : 0];
    auto& high_detail_lod = terrain.lods[lod_index];

    auto low_detail_format = terrain.format;
    low_detail_format.position_offset += terrain.format.size / 2;
    low_detail_format.normal_offset += terrain.format.size / 2;
    low_detail_format.color_offset += terrain.format.size / 2;

    terrain_mesh(terrain, radius, mesh, low_detail_format, terrain.format, true, chunk_index, lowest_detail_lod.level, low_detail_lod.level, high_detail_lod.level);
  }
}
