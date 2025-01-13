/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "util.h"

namespace ludo
{
  void append_vertex(mesh& mesh, buffer& indices, buffer& vertices, const vertex_format& format, const vec3& position, const vec3& normal, const vec4& color, const vec2& texture_coordinate, bool unique_only, bool no_normal_check)
  {
    if (unique_only)
    {
      auto byte_index = mesh.vertices.start * format.size;
      for (auto existing_vertex_index = mesh.vertices.start; existing_vertex_index < mesh.vertices.start + mesh.vertices.count; existing_vertex_index++)
      {
        if (near(cast<vec3>(vertices, byte_index + format.position_offset), position) &&
            (no_normal_check || !format.has_normal || near(cast<vec3>(vertices, byte_index + format.normal_offset), normal)) &&
            (!format.has_color || near(cast<vec4>(vertices, byte_index + format.color_offset), color)) &&
            (!format.has_texture_coordinate || near(cast<vec2>(vertices, byte_index + format.texture_coordinate_offset), texture_coordinate)))
        {
          cast<uint32_t>(indices, (mesh.indices.start + mesh.indices.count) * sizeof(uint32_t)) = existing_vertex_index;

          mesh.indices.count++;

          return;
        }

        byte_index += format.size;
      }
    }

    auto byte_index = (mesh.vertices.start + mesh.vertices.count) * format.size;
    cast<vec3>(vertices, byte_index + format.position_offset) = position;
    if (format.has_normal) cast<vec3>(vertices, byte_index + format.normal_offset) = normal;
    if (format.has_color) cast<vec4>(vertices, byte_index + format.color_offset) = color;
    if (format.has_texture_coordinate) cast<vec2>(vertices, byte_index + format.texture_coordinate_offset) = texture_coordinate;

    cast<uint32_t>(indices, (mesh.indices.start + mesh.indices.count) * sizeof(uint32_t)) = mesh.vertices.start + mesh.vertices.count;

    mesh.indices.count++;
    mesh.vertices.count++;
  }
}
