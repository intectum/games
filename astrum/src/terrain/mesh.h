#ifndef ASTRUM_TERRAIN_MESH_H
#define ASTRUM_TERRAIN_MESH_H

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  void terrain_mesh(const terrain& terrain, float radius, ludo::mesh& mesh, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t index, uint32_t chunk_divisions, uint32_t low_detail_divisions, uint32_t high_detail_divisions);

  void terrain_mesh(const terrain& terrain, float radius, ludo::mesh& mesh, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t index, uint32_t chunk_divisions, uint32_t low_detail_divisions, uint32_t high_detail_divisions, const std::array<ludo::vec3, 3>& positions);
}

#endif // ASTRUM_TERRAIN_MESH_H
