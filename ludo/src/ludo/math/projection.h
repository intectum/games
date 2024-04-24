/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MATH_PROJECTION_H
#define LUDO_MATH_PROJECTION_H

#include "vec.h"

namespace ludo
{
  ///
  /// Projects a point onto a line.
  /// \param point The point to project.
  /// \param target_point A point on the target line segment.
  /// \param target_direction A vector along the target line segment.
  /// \return The projection.
  vec3 project_point_onto_line_segment(const vec3& point, const vec3& target_point, const vec3& target_direction);

  ///
  /// Projects a point onto a plane.
  /// \param point The point to project.
  /// \param target_point A point on the target plane.
  /// \param target_normal The normal to the target plane (must be unit length).
  /// \return The projection.
  vec3 project_point_onto_plane(const vec3& point, const vec3& target_point, const vec3& target_normal);
}

#endif // LUDO_MATH_PROJECTION_H
