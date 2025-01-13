#pragma once

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void init_terrain(ludo::container& container, ludo::buffer& indices, ludo::buffer& vertices, ludo::pool* index_pools, ludo::pool* vertex_pools, const ludo::vec3& camera_position, const terrain_funcs& terrain_funcs, const ludo::vertex_format& format, const ludo::vec3& position, float radius, const std::vector<lod>& lods);

  std::pair<uint32_t, uint32_t> terrain_counts(const std::vector<lod>& lods);

  void stream_terrain(ludo::container& container, ludo::buffer& indices, ludo::buffer& vertices, ludo::pool* index_pools, ludo::pool* vertex_pools, const ludo::vec3& camera_position, const terrain_funcs& terrain_funcs, const ludo::vertex_format& format, float radius, const std::vector<lod>& lods);
}
