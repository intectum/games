/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_GEOMETRY_H
#define LUDO_GEOMETRY_H

#include "buffers.h"
#include "data.h"
#include "math/mat.h"
#include "math/vec.h"

namespace ludo
{
  using instance_t = std::byte;
  using index_t = uint32_t;
  using vertex_t = std::byte;

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
  struct LUDO_API vertex_format
  {
    std::vector<std::pair<char, uint32_t>> components; ///< The components within the vertex. Components are of the form { <type>, <count> }.
    uint32_t size = 0; ///< The total size of the vertex in bytes.

    bool has_normal = false; ///< Determines if the vertex format includes a normal.
    bool has_color = false; ///< Determines if the vertex format includes a color.
    bool has_texture_coordinate = false; ///< Determines if the vertex format includes a texture coordinate.
    bool has_bone_weights = false; ///< Determines if the vertex format includes bone weights.

    uint32_t position_offset = 0; ///< The offset in bytes to the position.
    uint32_t normal_offset = 0; ///< The offset in bytes to the normal.
    uint32_t color_offset = 0; ///< The offset in bytes to the color.
    uint32_t texture_coordinate_offset = 0; ///< The offset in bytes to the texture coordinate.
  };

  ///
  /// A mesh.
  struct LUDO_API mesh
  {
    uint64_t id = 0; ///< The ID of the mesh.
    uint64_t render_program_id = 0; ///< The ID of the render program used to draw this mesh.
    uint64_t texture_id = 0; ///< The ID of the texture used to render this mesh.
    uint64_t armature_id = 0; ///< The ID of the armature instance used to animate this mesh.
    std::vector<uint64_t> animation_ids; ///< The ID of the armature instance used to animate this mesh.

    buffer index_buffer; ///< A buffer containing the index data.
    buffer vertex_buffer; ///< A buffer containing the vertex data.
  };

  ///
  /// An instance of a mesh.
  struct LUDO_API mesh_instance
  {
    uint64_t id = 0; ///< The ID of the mesh instance.
    uint64_t mesh_id = 0; ///< The ID of the mesh this instance is based on.
    uint64_t render_program_id = 0; ///< The ID of the render program used to draw this mesh instance.
    uint64_t texture_id = 0; ///< The ID of the texture used to render this mesh.
    uint64_t armature_instance_id = 0; ///< The ID of the armature instance used to animate this mesh.

    buffer index_buffer; ///< A buffer containing the index data.
    buffer vertex_buffer; ///< A buffer containing the vertex data.

    mat4 transform = ludo::mat4_identity; ///< The transform of this mesh.
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
  /// \param normal Determines if a normal should be included in the vertex format.
  /// \param color Determines if a color should be included in the vertex format.
  /// \param texture_coordinate Determines if a texture coordinate should be included in the vertex format.
  /// \param bone_weights Determines if bone weights should be included in the vertex format.
  /// \return A vertex format based on the options provided.
  LUDO_API vertex_format format(bool normal = false, bool color = false, bool texture_coordinate = false, bool bone_weights = false);

  ///
  /// Adds a mesh to the data of an instance.
  /// Allocates index and vertex buffers based on the options provided.
  /// \param instance The instance to add the mesh to.
  /// \param init The initial state of the new mesh.
  /// \param index_count The number of indices to allocate.
  /// \param vertex_count The number of vertices to allocate.
  /// \param vertex_size The size of each vertex in bytes.
  /// \param partition The name of the partition.
  /// \return A pointer to the new mesh. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  LUDO_API mesh* add(instance& instance, const mesh& init, uint32_t index_count, uint32_t vertex_count, uint8_t vertex_size, const std::string& partition = "default");

  template<>
  LUDO_API void remove<mesh>(instance& instance, mesh* element, const std::string& partition);

  ///
  /// Adds a mesh instance based on the given mesh.
  /// An armature instance will also be added and attached to the mesh instance if one is not already added.
  /// \param instance The instance to add the mesh instance to.
  /// \param init The initial state of the new mesh instance.
  /// \param mesh The mesh to base the mesh instance on.
  /// \param partition The name of the partition.
  /// \return A pointer to the new mesh instance. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  LUDO_API mesh_instance* add(instance& instance, const mesh_instance& init, const mesh& mesh, const std::string& partition = "default");
}

#endif // LUDO_GEOMETRY_H
