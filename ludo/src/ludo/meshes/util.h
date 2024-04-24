/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MESHES_UTIL_H
#define LUDO_MESHES_UTIL_H

#include "../meshes.h"

namespace ludo
{
  ///
  /// Writes an index and vertex at the given indices within the given mesh.
  /// If it is writing unique vertices only it may only write an index and not a vertex (if the vertex already exists).
  /// It will only search the vertices before the given vertex index for matching vertices.
  /// Vertex format information is passed individually (instead of being calculated in this function) to improve performance where this function is called many times.
  /// \param mesh The mesh to write the index and vertex to.
  /// \param format The vertex format of the mesh.
  /// \param index_index The index at which to write the index. NOTE: The value passed will be incremented if an index was written.
  /// \param vertex_index The index at which to write the vertex. NOTE: The value passed will be incremented if a vertex was written.
  /// \param position The position to write to the vertex.
  /// \param normal The normal to write to the vertex.
  /// \param texture_coordinate The texture coordinate to write to the vertex.
  /// \param unique_only Determines if only unique vertices should be written.
  /// \param no_normal_check Determines if normals should be taken into account when searching for matching vertices.
  void write_vertex(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& position, const vec3& normal, const vec2& texture_coordinate, bool unique_only = true, bool no_normal_check = false);
}

#endif // LUDO_MESHES_UTIL_H
