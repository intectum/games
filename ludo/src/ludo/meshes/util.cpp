/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "util.h"

namespace ludo
{
  void write_vertex(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& position, const vec3& normal, const vec2& tex_coord, bool unique_only, bool no_normal_check)
  {
    if (unique_only)
    {
      auto byte_index = 0;
      for (auto existing_vertex_index = 0; existing_vertex_index < vertex_index; existing_vertex_index++)
      {
        if (near(cast<vec3>(mesh.vertex_buffer, byte_index + format.position_offset), position) &&
            (no_normal_check || !format.has_normal || near(cast<vec3>(mesh.vertex_buffer, byte_index + format.normal_offset), normal)) &&
            (!format.has_texture_coordinate || near(cast<vec2>(mesh.vertex_buffer, byte_index + format.texture_coordinate_offset), tex_coord)))
        {
          cast<uint32_t>(mesh.index_buffer, index_index * sizeof(uint32_t)) = existing_vertex_index;

          index_index++;

          return;
        }

        byte_index += format.size;
      }
    }

    auto byte_index = vertex_index * format.size;
    cast<vec3>(mesh.vertex_buffer, byte_index + format.position_offset) = position;
    if (format.has_normal) cast<vec3>(mesh.vertex_buffer, byte_index + format.normal_offset) = normal;
    if (format.has_texture_coordinate) cast<vec2>(mesh.vertex_buffer, byte_index + format.texture_coordinate_offset) = tex_coord;

    cast<uint32_t>(mesh.index_buffer, index_index * sizeof(uint32_t)) = vertex_index;

    vertex_index++;
    index_index++;
  }
}
