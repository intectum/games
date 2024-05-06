/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "edit.h"
#include "shapes.h"

namespace ludo
{
  void colorize(mesh& mesh, const vertex_format& format, uint32_t vertex_start, uint32_t vertex_count, const vec4& color, bool debug)
  {
    if (!format.has_color)
    {
      return;
    }

    auto byte_index = vertex_start * format.size + format.color_offset;

    for (auto vertex_index = 0; vertex_index < vertex_count; vertex_index++)
    {
      if (debug && vertex_index % 2)
      {
        cast<ludo::vec4>(mesh.vertex_buffer, byte_index) = vec4 { 1.0f - color[0], 1.0f - color[1], 1.0f - color[2], color[3] };
      }
      else
      {
        cast<ludo::vec4>(mesh.vertex_buffer, byte_index) = color;
      }

      byte_index += format.size;
    }
  }

  void extrude(mesh& mesh, const vertex_format& format, const std::vector<std::array<uint32_t, 3>>& triangles, const vec3& extrusion, bool invert)
  {
    auto position_counts = std::vector<std::pair<vec3, uint32_t>>();
    for (auto& triangle : triangles)
    {
      for (auto vertex_index = 0; vertex_index < 3; vertex_index++)
      {
        auto byte_index = triangle[vertex_index] * format.size + format.position_offset;
        auto position = cast<vec3>(mesh.vertex_buffer, byte_index);
        auto position_count_iter = std::find_if(position_counts.begin(), position_counts.end(), [&position](const std::pair<vec3, uint32_t>& position_count)
        {
          return position_count.first == position;
        });

        if (position_count_iter == position_counts.end())
        {
          position_counts.emplace_back(std::pair { position, 1 });
        }
        else
        {
          position_count_iter->second++;
        }
      }
    }

    auto union_polygon = std::vector<vec3>();
    for (auto& position_count : position_counts)
    {
      // Positions shared by two or fewer triangles must be part of the outer edge of the higher-order polygon
      if (position_count.second <= 2)
      {
        union_polygon.emplace_back(position_count.first);
      }
    }

    // TODO fix this sorting! It does not work for a cylinder.
    std::sort(union_polygon.begin(), union_polygon.end(), [&extrusion](const vec3& position_a, const vec3& position_b)
    {
      // TODO project these onto a plane whose normal is the extrusion vector when comparing
      return dot(extrusion, cross(position_a, position_b)) > 0.0f;
    });

    auto extrusion_strip_positions = std::vector<vec3>();
    for (auto& union_position : union_polygon)
    {
      extrusion_strip_positions.emplace_back(union_position);
      extrusion_strip_positions.emplace_back(union_position + extrusion);
    }
    extrusion_strip_positions.emplace_back(union_polygon[0]);
    extrusion_strip_positions.emplace_back(union_polygon[0] + extrusion);

    // Create extrusion triangles
    auto extrusion_strip_options = shape_options { .outward_faces = !invert, .inward_faces = invert };
    //triangle_strip(mesh, format, extrusion_strip_positions, extrusion_strip_options); TODO

    // Move original triangles
    for (auto& triangle : triangles)
    {
      for (auto vertex_index = 0; vertex_index < 3; vertex_index++)
      {
        auto byte_index = triangle[vertex_index] * format.size + format.position_offset;
        cast<vec3>(mesh.vertex_buffer, byte_index) += extrusion;
      }
    }
  }

  void flip(mesh& mesh, const vertex_format& format, const std::vector<std::array<uint32_t, 3>>& triangles)
  {
    for (auto& triangle : triangles)
    {
      auto vertex_0_data = mesh.vertex_buffer.data + triangle[0] * format.size;
      auto vertex_1_data = mesh.vertex_buffer.data + triangle[1] * format.size;
      auto vertex_2_data = mesh.vertex_buffer.data + triangle[2] * format.size;

      std::swap(*reinterpret_cast<vec3*>(vertex_1_data + format.position_offset), *reinterpret_cast<vec3*>(vertex_2_data + format.position_offset));

      if (format.has_normal)
      {
        *reinterpret_cast<vec3*>(vertex_0_data + format.normal_offset) *= -1.0f;
        *reinterpret_cast<vec3*>(vertex_1_data + format.normal_offset) *= -1.0f;
        *reinterpret_cast<vec3*>(vertex_2_data + format.normal_offset) *= -1.0f;
      }
    }
  }

  void rotate(mesh& mesh, const vertex_format& format, uint32_t vertex_start, uint32_t vertex_count, const quat& rotation)
  {
    auto data = mesh.vertex_buffer.data + vertex_start * format.size;

    auto rotation_matrix = mat3(rotation);

    for (auto vertex_index = 0; vertex_index < vertex_count; vertex_index++)
    {
      auto& position = *reinterpret_cast<vec3*>(data + format.position_offset);
      position = rotation_matrix * position;

      /*auto& normal = *reinterpret_cast<vec3*>(data + format.normal_offset);
      normal = rotation_matrix * normal;*/

      data += format.size;
    }
  }

  void scale(mesh& mesh, const vertex_format& format, uint32_t vertex_start, uint32_t vertex_count, float scalar)
  {
    auto data = mesh.vertex_buffer.data + vertex_start * format.size + format.position_offset;

    for (auto vertex_index = 0; vertex_index < vertex_count; vertex_index++)
    {
      *reinterpret_cast<vec3*>(data) *= scalar;
      data += format.size;
    }
  }

  void translate(mesh& mesh, const vertex_format& format, uint32_t vertex_start, uint32_t vertex_count, const vec3& translation)
  {
    auto data = mesh.vertex_buffer.data + vertex_start * format.size + format.position_offset;

    for (auto vertex_index = 0; vertex_index < vertex_count; vertex_index++)
    {
      *reinterpret_cast<vec3*>(data) += translation;
      data += format.size;
    }
  }
}
