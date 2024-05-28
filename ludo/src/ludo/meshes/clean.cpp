/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <limits>
#include <map>

#include "clean.h"
#include "util.h"

namespace ludo
{
  std::pair<uint32_t, uint32_t> clean(mesh& destination, const mesh& source, const vertex_format& destination_format, const vertex_format& source_format, bool dry_run)
  {
    auto counts = std::pair<uint32_t, uint32_t> { 0, 0 };

    auto to_mesh = destination;
    if (dry_run)
    {
      to_mesh = ludo::mesh();
      to_mesh.index_buffer = allocate(destination.index_buffer.size);
      to_mesh.vertex_buffer = allocate(destination.vertex_buffer.size);
    }

    auto index_stream = stream(source.index_buffer);
    while (!ended(index_stream))
    {
      auto indices = std::array<uint32_t, 3>
      {
        read<uint32_t>(index_stream),
        read<uint32_t>(index_stream),
        read<uint32_t>(index_stream)
      };

      auto positions = std::array<vec3, 3>
      {
        cast<vec3>(source.vertex_buffer, indices[0] * source_format.size + source_format.position_offset),
        cast<vec3>(source.vertex_buffer, indices[1] * source_format.size + source_format.position_offset),
        cast<vec3>(source.vertex_buffer, indices[2] * source_format.size + source_format.position_offset)
      };

      auto perpendicular = cross(positions[1] - positions[0], positions[2] - positions[0]);
      auto area = length(perpendicular) / 2.0f;
      if (!near(area, 0.0f))
      {
        for (auto index : indices)
        {
          auto& position = cast<vec3>(source.vertex_buffer, index * source_format.size + source_format.position_offset);
          auto normal = source_format.has_normal ? cast<vec3>(source.vertex_buffer, index * source_format.size + source_format.normal_offset) : vec3();
          auto color = source_format.has_color ? cast<vec4>(source.vertex_buffer, index * source_format.size + source_format.color_offset) : vec4();
          auto texture_coordinate = source_format.has_texture_coordinate ? cast<vec2>(source.vertex_buffer, index * source_format.size + source_format.texture_coordinate_offset): vec2();

          write_vertex(to_mesh, destination_format, counts.first, counts.second, position, normal, color, texture_coordinate);
        }
      }
    }

    if (dry_run)
    {
      deallocate(to_mesh.index_buffer);
      deallocate(to_mesh.vertex_buffer);
    }

    return counts;
  }
}
