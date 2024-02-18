/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MESHES_EDIT_H
#define LUDO_MESHES_EDIT_H

#include "../math/quat.h"
#include "../meshes.h"

namespace ludo
{
  ///
  /// Sets the color of vertices.
  /// \param mesh The mesh to colorize vertices within.
  /// \param format The vertex format of the mesh.
  /// \param vertex_start The first vertex to colorize.
  /// \param vertex_count The number of vertices to colorize.
  /// \param color The color.
  /// \param debug If true, alternates the color to make it more visible.
  LUDO_API void colorize(mesh& mesh, const vertex_format& format, uint32_t vertex_start, uint32_t vertex_count, const vec4& color, bool debug = false);

  ///
  /// Extrudes triangles.
  /// The additional vertices and indices added by the extrusion will be appended at vertex_counts[0] and vertex_counts[0] will be updated to reflect the new size of the mesh.
  /// The mesh's buffers must have capacity for the additional vertices and indices added by the extrusion.
  /// \param mesh The mesh to extrude vertices within.
  /// \param format The vertex format of the mesh.
  /// \param triangles The indices of the triangles to extrude (must form a higher-order polygon).
  /// \param invert Determines if the faces should be inverted (inwards facing).
  /// \param extrusion The vector of extrusion.
  LUDO_API void extrude(mesh& mesh, const vertex_format& format, const std::vector<std::array<uint32_t, 3>>& triangles, const vec3& extrusion, bool invert = false);

  ///
  /// Flips triangles to face the opposite direction (assumes the primitive is a triangle list).
  /// \param mesh The mesh to flip triangles within.
  /// \param format The vertex format of the mesh.
  /// \param triangles The indices of the triangles to flip.
  LUDO_API void flip(mesh& mesh, const vertex_format& format, const std::vector<std::array<uint32_t, 3>>& triangles);

  ///
  /// Rotates vertices.
  /// \param mesh The mesh to rotate vertices within.
  /// \param format The vertex format of the mesh.
  /// \param vertex_start The first vertex to rotate.
  /// \param vertex_count The number of vertices to rotate.
  /// \param rotation The rotation.
  LUDO_API void rotate(mesh& mesh, const vertex_format& format, uint32_t vertex_start, uint32_t vertex_count, const quat& rotation);

  ///
  /// Scales vertices.
  /// \param mesh The mesh to scale vertices within.
  /// \param format The vertex format of the mesh.
  /// \param vertex_start The first vertex to scale.
  /// \param vertex_count The number of vertices to scale.
  /// \param scalar The scalar.
  LUDO_API void scale(mesh& mesh, const vertex_format& format, uint32_t vertex_start, uint32_t vertex_count, float scalar);

  ///
  /// Translates vertices.
  /// \param mesh The mesh to translate vertices within.
  /// \param format The vertex format of the mesh.
  /// \param vertex_start The first vertex to translate.
  /// \param vertex_count The number of vertices to translate.
  /// \param translation The translation.
  LUDO_API void translate(mesh& mesh, const vertex_format& format, uint32_t vertex_start, uint32_t vertex_count, const vec3& translation);
}

#endif // LUDO_MESHES_EDIT_H
