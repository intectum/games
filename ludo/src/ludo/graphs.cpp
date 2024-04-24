/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <limits>

#include "graphs.h"

namespace ludo
{
  // TODO this will need to be recomputed when the mesh rotates! Can we have a variation that works for any orientation?
  aabb bounds(const mesh& mesh, const vertex_format& format)
  {
    auto bounds = aabb
    {
      .min = ludo::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
      .max = ludo::vec3(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest())
    };

    for (auto vertex_position = 0; vertex_position < mesh.vertex_buffer.size; vertex_position += format.size)
    {
      auto position = ludo::read<ludo::vec3>(mesh.vertex_buffer, vertex_position + format.position_offset);

      bounds.min =
      {
        std::min(bounds.min[0], position[0]),
        std::min(bounds.min[1], position[1]),
        std::min(bounds.min[2], position[2])
      };

      bounds.max =
      {
        std::max(bounds.max[0], position[0]),
        std::max(bounds.max[1], position[1]),
        std::max(bounds.max[2], position[2])
      };
    }

    return bounds;
  }

  bool contains(const aabb& container, const aabb& containee)
  {
    return
      containee.min[0] >= container.min[0] && containee.min[1] >= container.min[1] && containee.min[2] >= container.min[2] &&
      containee.max[0] <= container.max[0] && containee.max[1] <= container.max[1] && containee.max[2] <= container.max[2];
  }

  bool contains(const aabb& container, const vec3& position)
  {
    return
      position[0] >= container.min[0] && position[1] >= container.min[1] && position[2] >= container.min[2] &&
      position[0] <= container.max[0] && position[1] <= container.max[1] && position[2] <= container.max[2];
  }

  // TODO I think this is good for culling etc. but not totally accurate?
  bool intersect(const aabb& a, const aabb& b)
  {
    auto half_dimensions_a = (a.max - a.min) / 2.0f;
    auto half_dimensions_b = (b.max - b.min) / 2.0f;

    auto center_a = a.min + half_dimensions_a;
    auto center_b = b.min + half_dimensions_b;

    return
      std::abs(center_a[0] - center_b[0]) < (half_dimensions_a[0] + half_dimensions_b[0]) &&
      std::abs(center_a[1] - center_b[1]) < (half_dimensions_a[1] + half_dimensions_b[1]) &&
      std::abs(center_a[2] - center_b[2]) < (half_dimensions_a[2] + half_dimensions_b[2]);
  }
}
