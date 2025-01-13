/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <istream>
#include <vector>

#include "memory.h"

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
  struct vertex_format // TODO split into vertex_format and vertex_options?
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
  struct mesh
  {
    range indices; ///< The indices.
    range vertices; ///< The vertices.
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
  vertex_format format(bool normal = false, bool color = false, bool texture_coordinate = false, bool bone_weights = false);

  ///
  /// Loads a mesh from a ludo mesh file.
  /// \param mesh The mesh to load the metadata into.
  /// \param indices The memory to load the indices into.
  /// \param vertices The memory to load the vertices into.
  /// \param file_name The name of the file containing the mesh data.
  void load(mesh& mesh, buffer& indices, buffer& vertices, const std::string& file_name);

  ///
  /// Loads a mesh from a ludo mesh stream.
/// \param mesh The mesh to load the data into.
/// \param indices The memory to load the indices into.
/// \param vertices The memory to load the vertices into.
  /// \param stream The mesh data.
  void load(mesh& mesh, buffer& indices, buffer& vertices, std::istream& stream);

  ///
  /// Loads metadata from a ludo mesh file.
  /// \param mesh The mesh to load the metadata into.
  /// \param file_name The name of the file containing the mesh data.
  void load_metadata(mesh& mesh, const std::string& file_name);

  ///
  /// Loads metadata from a ludo mesh stream.
  /// \param mesh The mesh to load the metadata into.
  /// \param stream The mesh data.
  void load_metadata(mesh& mesh, std::istream& stream);

  ///
  /// Saves a mesh to a ludo mesh file.
  /// \param mesh The mesh.
  /// \param indices The indices.
  /// \param vertices The vertices.
  /// \param file_name The name of the file to save to.
  void save(const mesh& mesh, const buffer& indices, const buffer& vertices, const std::string& file_name);

  ///
  /// Saves a mesh to a ludo mesh stream.
  /// \param mesh The mesh.
  /// \param indices The indices.
  /// \param vertices The vertices.
  /// \param stream The mesh data.
  void save(const mesh& mesh, const buffer& indices, const buffer& vertices, std::ostream& stream);
}
