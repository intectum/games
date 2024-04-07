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
  /// Possible components are:
  ///   p: position
  ///   n: normal
  ///   c: color
  ///   t: texture coordinate
  ///   b: bone indices and weights
  ///   i: int
  ///   u: unsigned int
  ///   f: float
  /// All components except for i and u represent floats. i and u represent int32_t and uint32_t respectively.
  /// Component counts represent the number of float/int32_t/uint32_t values within the component e.g. the component p3 represents a position consisting of 3 floats.
  struct LUDO_API vertex_format
  {
    std::vector<char> components; ///< The components the vertex is comprised of.
    std::vector<uint8_t> component_counts; ///< The counts of values within the components.
    uint8_t size = 0; ///< The total size of the vertex in bytes.
  };

  ///
  /// A set of options for building a standardized vertex format.
  struct LUDO_API vertex_format_options
  {
    bool normals = false; ///< Determines if normals will be included in the vertex format.
    bool colors = false; ///< Determines if colors will be included in the vertex format.
    bool texture = false; ///< Determines if texture coordinates will be included in the vertex format.
    bool bones = false; ///< Determines if bone indices and weights will be included in the vertex format.
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
    .components = { 'p' },
    .component_counts = { 3 },
    .size = 3 * sizeof(float)
  };

  const auto vertex_format_pc = vertex_format ///< A vertex format containing position and color information
  {
    .components = { 'p', 'c' },
    .component_counts = { 3, 4 },
    .size = 7 * sizeof(float)
  };

  const auto vertex_format_pn = vertex_format ///< A vertex format containing position and normal information
  {
    .components = { 'p', 'n' },
    .component_counts = { 3, 3 },
    .size = 6 * sizeof(float)
  };

  const auto vertex_format_pnc = vertex_format ///< A vertex format containing position, normal and color information
  {
    .components = { 'p', 'n', 'c' },
    .component_counts = { 3, 3, 4 },
    .size = 10 * sizeof(float)
  };

  const auto vertex_format_pnt = vertex_format ///< A vertex format containing position, normal and texture coordinate information
  {
    .components = { 'p', 'n', 't' },
    .component_counts = { 3, 3, 2 },
    .size = 8 * sizeof(float)
  };

  const auto vertex_format_pt = vertex_format ///< A vertex format containing position and texture coordinate information
  {
    .components = { 'p', 't' },
    .component_counts = { 3, 2 },
    .size = 5 * sizeof(float)
  };

  ///
  /// Creates a vertex format based on the options provided.
  /// It will be of the form p3[n3][c4][t2_0...t2_n][u4f4] where the optional components are only included if the relevant options are specified.
  /// The last component [u4f4] is only included if the bone count is greater than 0.
  /// \param options The options used to build the vertex format.
  /// \return A vertex format based on the options provided.
  LUDO_API vertex_format format(const vertex_format_options& options);

  ///
  /// Determines the primitive count of the given component.
  /// \param format The format to find the primitive count within.
  /// \param component The component to find the primitive count of.
  /// \return The primitive count of the given component (or 0 if it is not found).
  uint32_t count(const vertex_format& format, char component);

  ///
  /// Determines the byte offset to the given component.
  /// \param format The format to find the offset within.
  /// \param component The component to find the offset to.
  /// \return The byte offset to the given component.
  LUDO_API uint32_t offset(const vertex_format& format, char component);

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
