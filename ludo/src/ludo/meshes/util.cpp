/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "util.h"

namespace ludo
{
  void write_vertex(mesh& mesh, uint32_t& index_index, uint32_t& vertex_index, uint8_t vertex_size, const vec3& position, uint8_t position_offset, const vec3& normal, bool has_normals, uint8_t normal_offset, const vec2& tex_coord, bool has_tex_coords, uint8_t tex_coord_offset, bool unique_only, bool no_normal_check)
  {
    if (unique_only)
    {
      auto byte_index = 0;
      for (auto existing_vertex_index = 0; existing_vertex_index < vertex_index; existing_vertex_index++)
      {
        if (near(read<vec3>(mesh.vertex_buffer, byte_index + position_offset), position) &&
            (no_normal_check || !has_normals || near(read<vec3>(mesh.vertex_buffer, byte_index + normal_offset), normal)) &&
            (!has_tex_coords || near(read<vec2>(mesh.vertex_buffer, byte_index + tex_coord_offset), tex_coord)))
        {
          write(mesh.index_buffer, index_index * sizeof(uint32_t), existing_vertex_index);

          index_index++;

          return;
        }

        byte_index += vertex_size;
      }
    }

    auto byte_index = vertex_index * vertex_size;
    write(mesh.vertex_buffer, byte_index + position_offset, position);
    if (has_normals) write(mesh.vertex_buffer, byte_index + normal_offset, normal);
    if (has_tex_coords) write(mesh.vertex_buffer, byte_index + tex_coord_offset, tex_coord);

    write(mesh.index_buffer, index_index * sizeof(uint32_t), vertex_index);

    vertex_index++;
    index_index++;
  }
}
