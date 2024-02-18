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
  ///   p: position,
  ///   n: normal,
  ///   c: color,
  ///   t: texture coordinate,
  ///   i: int,
  ///   u: unsigned int,
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
  /// A mesh.
  struct LUDO_API mesh
  {
    uint64_t id = 0; ///< The ID of the mesh.
    uint64_t mesh_buffer_id = 0; ///< The ID of the mesh buffer this mesh is within.

    uint32_t instance_start = 0; ///< The starting index of the instances.
    uint32_t instance_count = 1; ///< The number of instances.

    buffer index_buffer; ///< A buffer containing the index data.
    buffer vertex_buffer; ///< A buffer containing the vertex data.
  };

  ///
  /// A mesh buffer.
  struct LUDO_API mesh_buffer
  {
    uint64_t id = 0; ///< The ID of the mesh buffer.
    uint64_t render_program_id = 0; ///< The ID of the render program used to draw this mesh buffer.

    mesh_primitive primitive = mesh_primitive::TRIANGLE_LIST; ///< The primitive the mesh is comprised of.
    vertex_format format;

    buffer command_buffer; ///< A buffer containing the draw commands.
    buffer index_buffer; ///< A buffer containing the index data.
    buffer vertex_buffer; ///< A buffer containing the vertex data.
    std::vector<buffer> data_buffers; ///< Buffers containing data available to the render program while rendering the mesh buffer.
  };

  ///
  /// A set of options for building a standard set of mesh buffer data.
  struct mesh_buffer_options
  {
    uint32_t instance_count = 1; ///< The number of instances in the mesh buffer.

    uint32_t index_count = 0; ///< The number of indices to include in the mesh buffer.
    uint32_t vertex_count = 0; ///< The number of vertices to include in the mesh buffer.

    bool normals = false; ///< Determines if normals will be included in the mesh buffer.
    bool colors = false; ///< Determines if colors will be included in the mesh buffer.

    uint32_t texture_count = 0; ///< The number of textures to include in the mesh buffer.
    uint32_t bone_count = 0; ///< The number of bones to include in the mesh buffer.
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
  /// Determines if the left-hand mesh is less than the right-hand mesh.
  /// A mesh is considered less than another mesh based on the following properties which appear in order of precedence:
  /// - mesh_buffer_id
  /// - id
  /// - instance_start
  /// This is to allow sorting for efficient rendering.
  /// \param lhs The left-hand mesh.
  /// \param rhs The right-hand mesh.
  /// \return True if the left-hand mesh is less than the right-hand mesh, false otherwise.
  bool operator<(const mesh& lhs, const mesh& rhs);

  template<>
  LUDO_API mesh* add(instance& instance, const mesh& init, const std::string& partition);

  template<>
  LUDO_API mesh_buffer* add(instance& instance, const mesh_buffer& init, const std::string& partition);

  ///
  /// Adds a mesh buffer to the data of an instance.
  /// Allocates data buffers (and selects a built-in render program if none is provided) based on the options provided.
  /// The mesh buffer will be created via the add(instance& instance, const mesh_buffer& init, const vertex_format& format, uint32_t vertex_count, uint32_t index_count = 0, const std::string& partition) function.
  /// The vertex format will be created via the format(const mesh_buffer_options& options) function.
  /// The data buffers will be of the form:
  ///   0: <texture_0>...<texture_n>
  ///   1: <transform>
  ///   2: <bone_transform_0>...<bone_transform_n>
  /// \param instance The instance to add the mesh buffer to.
  /// \param init The initial state of the new mesh buffer.
  /// \param options The options used to initialize the mesh buffer.
  /// \param partition The name of the partition.
  /// \return A pointer to the new mesh buffer. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  LUDO_API mesh_buffer* add(instance& instance, const mesh_buffer& init, const mesh_buffer_options& options, const std::string& partition = "default");

  template<>
  LUDO_API void remove<mesh_buffer>(instance& instance, mesh_buffer* element, const std::string& partition);

  ///
  /// Creates a vertex format based on the options provided.
  /// It will be of the form p3[n3][c4][t2_0...t2_n][u4f4] where the optional components are only included if the relevant options are specified.
  /// The last component [u4f4] is only included if the bone count is greater than 0.
  /// \param options The options used to initialize a mesh buffer.
  /// \return A vertex format based on the options provided.
  LUDO_API vertex_format format(const mesh_buffer_options& options);

  ///
  /// Determines the byte offset to the given component.
  /// \param format The format to find the offset within.
  /// \param component The component to find the offset to.
  /// \return The byte offset to the given component.
  LUDO_API uint8_t offset(const vertex_format& format, char component);

  ///
  /// Retrieves a transform from a mesh buffer.
  /// The data buffers must be of the form of those created via the add(context& context, const mesh_buffer& init, const mesh_options& options, const std::string& partition) function.
  /// \param mesh_buffer The mesh buffer to retrieve the transform from.
  /// \param instance_index The index of the instance within the mesh buffer.
  /// \return The transform.
  LUDO_API mat4 get_transform(const mesh_buffer& mesh_buffer, uint32_t instance_index);

  ///
  /// Sets a transform of a mesh buffer.
  /// The data buffers must be of the form of those created via the add(context& context, const mesh_buffer& init, const mesh_options& options, const std::string& partition) function.
  /// \param mesh_buffer The mesh buffer to set the transform of.
  /// \param instance_index The index of the instance within the mesh buffer.
  /// \param transform The transform.
  LUDO_API void set_transform(mesh_buffer& mesh_buffer, uint32_t instance_index, const mat4& transform);

  ///
  /// Sets a texture of a mesh buffer.
  /// The data buffers must be of the form of those created via the add(context& context, const mesh_buffer& init, const mesh_options& options, const std::string& partition) function.
  /// \param mesh_buffer The mesh buffer to set the texture of.
  /// \param texture The texture.
  /// \param index The index of the texture within the mesh buffer.
  LUDO_API void set_texture(mesh_buffer& mesh_buffer, const struct texture& texture, uint8_t index);
}

#endif // LUDO_GEOMETRY_H
