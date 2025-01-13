/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "../math/vec.h"
#include "../meshes.h"

namespace ludo
{
  ///
  /// Writes an index and vertex at the given indices within the given mesh.
  /// If it is writing unique vertices only it may only write an index and not a vertex (if the vertex already exists).
  /// It will only search the vertices before the given vertex index for matching vertices.
  /// Vertex format information is passed individually (instead of being calculated in this function) to improve performance where this function is called many times.
  /// \param mesh The mesh to write the metadata to.
  /// \param indices The indices to write to.
  /// \param vertices The vertices to write to.
  /// \param format The vertex format of the mesh.
  /// \param position The position to write to the vertex.
  /// \param normal The normal to write to the vertex.
  /// \param color The color to write to the vertex.
  /// \param texture_coordinate The texture coordinate to write to the vertex.
  /// \param unique_only Determines if only unique vertices should be written.
  /// \param no_normal_check Determines if normals should be taken into account when searching for matching vertices.
  void append_vertex(mesh& mesh, buffer& indices, buffer& vertices, const vertex_format& format, const vec3& position, const vec3& normal, const vec4& color, const vec2& texture_coordinate, bool unique_only = true, bool no_normal_check = false);
}
