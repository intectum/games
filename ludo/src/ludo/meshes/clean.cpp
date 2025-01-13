/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "clean.h"
#include "util.h"

namespace ludo
{
  void clean(mesh& dest_mesh, buffer& dest_indices, buffer& dest_vertices, const buffer& src_indices, const buffer& src_vertices, uint32_t count, const vertex_format& dest_format, const vertex_format& src_format, bool dry_run)
  {
    if (dry_run)
    {
      dest_indices = allocate_buffer(count * sizeof(uint32_t));
      dest_vertices = allocate_buffer(count * dest_format.size);
    }

    for (auto index = 0; index < count; index += 3)
    {
      auto index_0 = cast<uint32_t>(src_indices, index * sizeof(uint32_t));
      auto index_1 = cast<uint32_t>(src_indices, (index + 1) * sizeof(uint32_t));
      auto index_2 = cast<uint32_t>(src_indices, (index + 2) * sizeof(uint32_t));

      auto& position_0 = cast<vec3>(src_vertices, index_0 * src_format.size + src_format.position_offset);
      auto& position_1 = cast<vec3>(src_vertices, index_1 * src_format.size + src_format.position_offset);
      auto& position_2 = cast<vec3>(src_vertices, index_2 * src_format.size + src_format.position_offset);

      auto perpendicular = cross(position_1 - position_0, position_2 - position_0);
      auto area = length(perpendicular) / 2.0f;
      if (!near(area, 0.0f))
      {
        auto normal_0 = src_format.has_normal ? cast<vec3>(src_vertices, index_0 * src_format.size + src_format.normal_offset) : vec3();
        auto normal_1 = src_format.has_normal ? cast<vec3>(src_vertices, index_1 * src_format.size + src_format.normal_offset) : vec3();
        auto normal_2 = src_format.has_normal ? cast<vec3>(src_vertices, index_2 * src_format.size + src_format.normal_offset) : vec3();

        auto color_0 = src_format.has_color ? cast<vec4>(src_vertices, index_0 * src_format.size + src_format.color_offset) : vec4();
        auto color_1 = src_format.has_color ? cast<vec4>(src_vertices, index_1 * src_format.size + src_format.color_offset) : vec4();
        auto color_2 = src_format.has_color ? cast<vec4>(src_vertices, index_2 * src_format.size + src_format.color_offset) : vec4();

        auto texture_coordinate_0 = src_format.has_texture_coordinate ? cast<vec2>(src_vertices, index_0 * src_format.size + src_format.texture_coordinate_offset): vec2();
        auto texture_coordinate_1 = src_format.has_texture_coordinate ? cast<vec2>(src_vertices, index_1 * src_format.size + src_format.texture_coordinate_offset): vec2();
        auto texture_coordinate_2 = src_format.has_texture_coordinate ? cast<vec2>(src_vertices, index_2 * src_format.size + src_format.texture_coordinate_offset): vec2();

        append_vertex(dest_mesh, dest_indices, dest_vertices, dest_format, position_0, normal_0, color_0, texture_coordinate_0);
        append_vertex(dest_mesh, dest_indices, dest_vertices, dest_format, position_1, normal_1, color_1, texture_coordinate_1);
        append_vertex(dest_mesh, dest_indices, dest_vertices, dest_format, position_2, normal_2, color_2, texture_coordinate_2);
      }
    }

    if (dry_run)
    {
      free_buffer(dest_indices);
      free_buffer(dest_vertices);
    }
  }
}
