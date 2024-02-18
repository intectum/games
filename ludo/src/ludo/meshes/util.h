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
  /// \param index_index The index at which to write the index. NOTE: The value passed will be incremented if an index was written.
  /// \param vertex_index The index at which to write the vertex. NOTE: The value passed will be incremented if a vertex was written.
  /// \param vertex_size The total size of the vertex in bytes.
  /// \param position The position to write to the vertex.
  /// \param position_offset The offset of the position within the vertex.
  /// \param normal The normal to write to the vertex.
  /// \param has_normals Determines if a normal should be written to the vertex.
  /// \param normal_offset The offset of the normal within the vertex.
  /// \param tex_coord The texture coordinate to write to the vertex.
  /// \param has_tex_coords Determines if a texture coordinate should be written to the vertex.
  /// \param tex_coord_offset The offset of the texture coordinate within the vertex.
  /// \param unique_only Determines if only unique vertices should be written.
  /// \param no_normal_check Determines if normals should be taken into account when searching for matching vertices.
  void write_vertex(mesh& mesh, uint32_t& index_index, uint32_t& vertex_index, uint8_t vertex_size, const vec3& position, uint8_t position_offset, const vec3& normal, bool has_normals, uint8_t normal_offset, const vec2& tex_coord, bool has_tex_coords, uint8_t tex_coord_offset, bool unique_only = true, bool no_normal_check = false);
}

#endif // LUDO_MESHES_UTIL_H
