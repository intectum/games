/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <vector>

#include "../math/quat.h"
#include "../meshes.h"

// TODO the comments here are probably wrong... maybe we should have tests for these operations...
// TODO also, change 'triangles' to just be a pointer to indices...

namespace ludo
{
  ///
  /// Sets the color of vertices.
  /// \param vertices The vertices to colorize.
  /// \param count The number of vertices to colorize.
  /// \param format The vertex format of the mesh.
  /// \param color The color.
  /// \param debug If true, alternates the color to make it more visible.
  void colorize(buffer& vertices, uint32_t count, const vertex_format& format, const vec4& color, bool debug = false);

  ///
  /// Extrudes triangles.
  /// The additional vertices and indices added by the extrusion will be appended at vertex_counts[0] and vertex_counts[0] will be updated to reflect the new size of the mesh.
  /// The mesh's buffers must have capacity for the additional vertices and indices added by the extrusion.
  /// \param vertices The vertices to extrude.
  /// \param format The vertex format of the mesh.
  /// \param triangles The indices of the triangles to extrude (must form a higher-order polygon).
  /// \param invert Determines if the faces should be inverted (inwards facing).
  /// \param extrusion The vector of extrusion.
  void extrude(buffer& vertices, const vertex_format& format, const std::vector<std::array<uint32_t, 3>>& triangles, const vec3& extrusion, bool invert = false);

  ///
  /// Flips triangles to face the opposite direction (assumes the primitive is a triangle list).
  /// \param vertices The triangles to flip.
  /// \param format The vertex format of the mesh.
  /// \param triangles The indices of the triangles to flip.
  void flip(buffer& vertices, const vertex_format& format, const std::vector<std::array<uint32_t, 3>>& triangles);

  ///
  /// Rotates vertices.
  /// \param vertices The vertices to rotate.
  /// \param count The number of vertices to rotate.
  /// \param format The vertex format of the mesh.
  /// \param rotation The rotation.
  void rotate(buffer& vertices, uint32_t count, const vertex_format& format, const quat& rotation);

  ///
  /// Scales vertices.
  /// \param vertices The vertices to scale.
  /// \param count The number of vertices to scale.
  /// \param format The vertex format of the mesh.
  /// \param scalar The scalar.
  void scale(buffer& vertices, uint32_t count, const vertex_format& format, float scalar);

  ///
  /// Translates vertices.
  /// \param vertices The vertices to translate.
  /// \param count The number of vertices to translate.
  /// \param format The vertex format of the mesh.
  /// \param translation The translation.
  void translate(buffer& vertices, uint32_t count, const vertex_format& format, const vec3& translation);
}
