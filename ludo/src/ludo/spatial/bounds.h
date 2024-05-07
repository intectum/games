/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_SPATIAL_BOUNDS_H
#define LUDO_SPATIAL_BOUNDS_H

#include "../meshes.h"

namespace ludo
{
  ///
  /// A 2D axis-aligned bounding box.
  struct LUDO_API aabb2
  {
    vec2 min = vec2_zero;
    vec2 max = vec2_zero;
  };

  ///
  /// A 3D axis-aligned bounding box.
  struct LUDO_API aabb3
  {
    vec3 min = vec3_zero;
    vec3 max = vec3_zero;
  };

  ///
  /// Calculates the bounds of the given mesh.
  /// \param mesh The mesh.
  /// \param format The vertex format of the mesh.
  /// \return The bounds of the given mesh.
  LUDO_API aabb3 bounds(const mesh& mesh, const vertex_format& format);

  ///
  /// Determines whether an AABB is contained within another AABB.
  /// \param container The containing AABB.
  /// \param containee The AABB to check against the containing AABB.
  /// \return True if the containee AABB is wholly within the container AABB, false otherwise.
  LUDO_API bool contains(const aabb2& container, const aabb2& containee);
  LUDO_API bool contains(const aabb3& container, const aabb3& containee);

  ///
  /// Determines whether a position is contained within another AABB.
  /// \param container The containing AABB.
  /// \param position The position to check against the containing AABB.
  /// \return True if the position is within the container AABB, false otherwise.
  bool contains(const aabb2& container, const vec2& position);
  bool contains(const aabb3& container, const vec3& position);

  ///
  /// Determines whether two AABBs intersect.
  /// \param a The first AABB.
  /// \param b The second AABB.
  /// \return True if the AABBs intersect, false otherwise.
  LUDO_API bool intersect(const aabb2& a, const aabb2& b);
  LUDO_API bool intersect(const aabb3& a, const aabb3& b);
}

#endif // LUDO_SPATIAL_BOUNDS_H
