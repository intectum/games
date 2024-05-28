/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_GEOMETRY_H
#define LUDO_GEOMETRY_H

#include <istream>
#include <ostream>

#include "data/buffers.h"
#include "data/data.h"
#include "math/mat.h"
#include "math/vec.h"
#include "util.h"

namespace ludo
{
  ///
  /// A mesh primitive.
  enum class mesh_primitive
  {
    POINT_LIST, ///< A list of points.
    LINE_LIST, ///< A list of lines.
    LINE_STRIP, ///< A strip of lines (the last vertex of one line is also the first vertex of the next line).
    TRIANGLE_LIST, ///< A list of triangles.
    TRIANGLE_STRIP ///< A strip of triangles (the last edge of one triangle is also the first edge of the next triangle).
  };

  ///
  /// A vertex format
  /// Possible component types are:
  ///   p: position
  ///   n: normal
  ///   c: color
  ///   t: texture coordinate
  ///   b: bone weights
  ///   i: int
  ///   u: unsigned int
  ///   f: float
  /// All components except for i and u represent floats. i and u represent int32_t and uint32_t respectively.
  /// Component counts represent the number of float/int32_t/uint32_t values within the component e.g. the component p3 represents a position consisting of 3 floats.
  struct LUDO_API vertex_format // TODO split into vertex_format and vertex_options?
  {
    std::vector<std::pair<char, uint32_t>> components; ///< The components. They are of the form { <type>, <count> }.
    uint32_t size = 0; ///< The total size of the vertex in bytes.

    bool has_normal = false; ///< Determines if a normal is included.
    bool has_color = false; ///< Determines if a color is included.
    bool has_texture_coordinate = false; ///< Determines ifa texture coordinate is included.
    bool has_bone_weights = false; ///< Determines if bone weights are included.

    uint32_t position_offset = 0; ///< The offset in bytes to the position.
    uint32_t normal_offset = 0; ///< The offset in bytes to the normal.
    uint32_t color_offset = 0; ///< The offset in bytes to the color.
    uint32_t texture_coordinate_offset = 0; ///< The offset in bytes to the texture coordinate.
    uint32_t bone_weights_offset = 0; ///< The offset in bytes to the bone weights.
  };

  ///
  /// A mesh.
  struct LUDO_API mesh
  {
    uint64_t id = 0; ///< A unique identifier.
    uint64_t texture_id = 0; ///< The texture to apply to the mesh. TODO remove some of these?
    uint64_t armature_id = 0; ///< The armature to apply to the mesh. TODO remove some of these?
    std::vector<uint64_t> animation_ids; ///< The animations to apply to the mesh. TODO remove some of these?

    buffer index_buffer; ///< A buffer containing the indices.
    buffer vertex_buffer; ///< A buffer containing the vertices.
    uint32_t vertex_size = 0; ///< The size in bytes of a vertex within this mesh.
  };

  const auto vertex_format_p = vertex_format ///< A vertex format containing only position information
  {
    .components = { { 'p', 3 } },
    .size = 3 * sizeof(float),
  };

  const auto vertex_format_pc = vertex_format ///< A vertex format containing position and color information
  {
    .components = { { 'p', 3 }, { 'c', 4 } },
    .size = 7 * sizeof(float),
    .has_color = true,
    .color_offset = 3 * sizeof(float)
  };

  const auto vertex_format_pn = vertex_format ///< A vertex format containing position and normal information
  {
    .components = { { 'p', 3 }, { 'n', 3 } },
    .size = 6 * sizeof(float),
    .has_normal = true,
    .normal_offset = 3 * sizeof(float)
  };

  const auto vertex_format_pnc = vertex_format ///< A vertex format containing position, normal and color information
  {
    .components = { { 'p', 3 }, { 'n', 3 }, { 'c', 4 } },
    .size = 10 * sizeof(float),
    .has_normal = true,
    .has_color = true,
    .normal_offset = 3 * sizeof(float),
    .color_offset = 6 * sizeof(float)
  };

  const auto vertex_format_pnt = vertex_format ///< A vertex format containing position, normal and texture coordinate information
  {
    .components = { { 'p', 3 }, { 'n', 3 }, { 't', 2 } },
    .size = 8 * sizeof(float),
    .has_normal = true,
    .has_texture_coordinate = true,
    .normal_offset = 3 * sizeof(float),
    .texture_coordinate_offset = 6 * sizeof(float)
  };

  const auto vertex_format_pt = vertex_format ///< A vertex format containing position and texture coordinate information
  {
    .components = { { 'p', 3 }, { 't', 2 } },
    .size = 5 * sizeof(float),
    .has_texture_coordinate = true,
    .texture_coordinate_offset = 3 * sizeof(float)
  };

  ///
  /// Creates a vertex format based on the options provided.
  /// It will be of the form p3[n3][c4][t2_0...t2_n][u4f4] where the optional components are only included if specified.
  /// \param normal Determines if a normal should be included.
  /// \param color Determines if a color should be included.
  /// \param texture_coordinate Determines if a texture coordinate should be included.
  /// \param bone_weights Determines if bone weights should be included.
  /// \return A vertex format based on the options provided.
  LUDO_API vertex_format format(bool normal = false, bool color = false, bool texture_coordinate = false, bool bone_weights = false);

  ///
  /// Initializes a mesh with index and vertex buffers.
  /// \param mesh The mesh.
  /// \param indices The indices to allocate from.
  /// \param vertices The vertices to allocate from.
  /// \param index_count The number of indices to allocate.
  /// \param vertex_count The number of vertices to allocate.
  /// \param vertex_size The size (in bytes) of a vertex.
  LUDO_API void init(mesh& mesh, heap& indices, heap& vertices, uint32_t index_count, uint32_t vertex_count, uint8_t vertex_size);

  ///
  /// De-initializes a mesh and reclaims the index and vertex buffers.
  /// \param mesh The mesh.
  /// \param indices The indices to reclaim to.
  /// \param vertices The vertices to reclaim to.
  LUDO_API void de_init(mesh& mesh, heap& indices, heap& vertices);

  ///
  /// Loads a mesh from a ludo mesh file.
  /// \param file_name The name of the file containing the mesh data.
  /// \param indices The indices to allocate from.
  /// \param vertices The vertices to allocate from.
  /// \return The mesh.
  LUDO_API mesh load(const std::string& file_name, heap& indices, heap& vertices);

  ///
  /// Loads a mesh from a stream.
  /// \param stream The mesh data.
  /// \param indices The indices to allocate from.
  /// \param vertices The vertices to allocate from.
  /// \return The mesh.
  LUDO_API mesh load(std::istream& stream, heap& indices, heap& vertices);

  ///
  /// Saves a mesh to a ludo mesh file.
  /// \param mesh The mesh.
  /// \param file_name The name of the file to save to.
  LUDO_API void save(const mesh& mesh, const std::string& file_name);

  ///
  /// Saves a mesh to a stream.
  /// \param mesh The mesh.
  /// \param stream The mesh data.
  LUDO_API void save(const mesh& mesh, std::ostream& stream);

  ///
  /// Reads mesh counts from a ludo mesh file.
  /// \param file_name The name of the file containing the mesh data.
  /// \param indices The indices to allocate from.
  /// \param vertices The vertices to allocate from.
  /// \return The mesh counts.
  LUDO_API std::pair<uint32_t, uint32_t> mesh_counts(const std::string& file_name);

  ///
  /// Reads mesh counts mesh from a stream.
  /// \param stream The mesh data.
  /// \param indices The indices to allocate from.
  /// \param vertices The vertices to allocate from.
  /// \return The mesh counts.
  LUDO_API std::pair<uint32_t, uint32_t> mesh_counts(std::istream& stream);
}

#endif // LUDO_GEOMETRY_H
