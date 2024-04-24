/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "box.h"
#include "shapes.h"

namespace ludo
{
  void sphere_cube(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const shape_options& options, bool spherified)
  {
    assert(options.divisions >= 2 && "must have at-least 2 divisions");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto [ vertex_count, index_count ] = sphere_cube_counts(options);
    auto radius = options.dimensions[0] / 2.0f;

    // I couldn't figure out how to adapt the 'spherifying' code to different cube sizes, so we're using the 2x2x2 cube and multiplying the result by the radius.
    auto box_index_index = index_index;
    auto box_vertex_index = vertex_index;
    auto box_options = options;
    box_options.dimensions = vec3 { 2.0f, 2.0f, 2.0f };
    box(mesh, format, box_index_index, box_vertex_index, box_options, options.smooth, options.smooth);

    auto byte_index = vertex_index * format.size;
    for (auto existing_vertex_index = vertex_index; existing_vertex_index < vertex_index + vertex_count; existing_vertex_index++)
    {
      auto position = read<vec3>(mesh.vertex_buffer, byte_index + format.position_offset) - options.center;

      if (spherified)
      {
        auto position2 = position * position;
        position =
        {
          position[0] * std::sqrt(1.0f - 0.5f * (position2[1] + position2[2]) + position2[1] * position2[2] / 3.0f),
          position[1] * std::sqrt(1.0f - 0.5f * (position2[2] + position2[0]) + position2[2] * position2[0] / 3.0f),
          position[2] * std::sqrt(1.0f - 0.5f * (position2[0] + position2[1]) + position2[0] * position2[1] / 3.0f)
        };
      }
      else
      {
        normalize(position);
      }

      write(mesh.vertex_buffer, byte_index + format.position_offset, options.center + position * radius);
      byte_index += format.size;
    }

    if (format.has_normal)
    {
      byte_index = vertex_index * format.size;

      if (options.smooth)
      {
        for (auto existing_vertex_index = vertex_index; existing_vertex_index < vertex_index + vertex_count; existing_vertex_index++)
        {
          auto position = read<vec3>(mesh.vertex_buffer, byte_index + format.position_offset);
          auto normal = position - options.center;
          normalize(normal);

          write(mesh.vertex_buffer, byte_index + format.normal_offset, normal);
          byte_index += format.size;
        }
      }
      else
      {
        for (auto existing_index_index = index_index; existing_index_index < index_index + index_count; existing_index_index += 3)
        {
          auto index_0 = read<uint32_t>(mesh.index_buffer, existing_index_index * sizeof(uint32_t));
          auto index_1 = read<uint32_t>(mesh.index_buffer, (existing_index_index + 1) * sizeof(uint32_t));
          auto index_2 = read<uint32_t>(mesh.index_buffer, (existing_index_index + 2) * sizeof(uint32_t));

          auto position_0 = read<vec3>(mesh.vertex_buffer, index_0 * format.size + format.position_offset);
          auto position_1 = read<vec3>(mesh.vertex_buffer, index_1 * format.size + format.position_offset);
          auto position_2 = read<vec3>(mesh.vertex_buffer, index_2 * format.size + format.position_offset);
          auto normal = cross(position_1 - position_0, position_2 - position_0);
          normalize(normal);

          write(mesh.vertex_buffer, index_0 * format.size + format.normal_offset, normal);
          write(mesh.vertex_buffer, index_1 * format.size + format.normal_offset, normal);
          write(mesh.vertex_buffer, index_2 * format.size + format.normal_offset, normal);
        }
      }
    }
  }

  std::pair<uint32_t, uint32_t> sphere_cube_counts(const shape_options& options)
  {
    assert(options.divisions >= 2 && "must have at-least 2 divisions");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto counts = rectangle_counts(options);
    counts.first *= 6;
    if (options.smooth)
    {
      counts.second = counts.second * 2 + (counts.second - (options.divisions - 1) * (options.divisions - 1)) * (options.divisions - 1);
    }
    else
    {
      counts.second = counts.first;
    }

    return counts;
  }
}
