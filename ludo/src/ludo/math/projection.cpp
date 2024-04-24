/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>

#include "projection.h"

namespace ludo
{
  vec3 project_point_onto_line_segment(const vec3& point, const vec3& target_point, const vec3& target_direction)
  {
    auto to_point = point - target_point;
    return target_point + (target_direction * ludo::dot(target_direction, to_point) / ludo::dot(target_direction, target_direction));
  }

  vec3 project_point_onto_plane(const vec3& point, const vec3& target_point, const vec3& target_normal)
  {
    assert(near(length(target_normal), 1.0f) && "'target_normal' must be unit length");

    auto to_plane = point - target_point;
    auto perpendicular_distance_to_plane = ludo::dot(to_plane, target_normal);

    auto point_normalized = point;
    normalize(point_normalized);

    return point - target_normal * perpendicular_distance_to_plane;
  }
}
