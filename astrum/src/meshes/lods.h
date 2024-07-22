#pragma once

#include <ludo/api.h>

namespace astrum
{
  struct lod
  {
    uint32_t level;
    float max_distance;
  };

  std::vector<ludo::mesh> build_lod_meshes(const ludo::mesh& source, const ludo::vertex_format& format, ludo::heap& indices, ludo::heap& vertices, const std::vector<uint32_t>& iterations);

  uint32_t find_lod_index(const std::vector<lod>& lods, const ludo::vec3& camera_position, const ludo::vec3& target_position, const ludo::vec3& target_normal);
}
