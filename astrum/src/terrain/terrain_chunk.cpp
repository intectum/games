#include "mesh.h"
#include "terrain_chunk.h"

namespace astrum
{
  void load_terrain_chunk(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, float radius, const std::vector<lod>& lods, const ludo::vertex_format& format, uint32_t chunk_index, uint32_t lod_index)
  {
    auto& lowest_detail_lod = lods[0];

    auto& low_detail_lod = lods[lod_index > 0 ? lod_index - 1 : 0];
    auto& high_detail_lod = lods[lod_index];

    auto low_detail_format = format;
    low_detail_format.position_offset += format.size / 2;
    low_detail_format.normal_offset += format.size / 2;
    low_detail_format.color_offset += format.size / 2;

    terrain_mesh(mesh, indices, vertices, terrain_funcs, radius, low_detail_format, format, true, chunk_index, lowest_detail_lod.level, low_detail_lod.level, high_detail_lod.level);
  }
}
