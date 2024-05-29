/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "circle.h"
#include "shapes.h"
#include "util.h"

namespace ludo
{
  void pipe(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center_front, const vec3& center_back, float radius, uint32_t divisions, bool smooth, const vec4& color, bool invert);

  void cylinder(mesh& mesh, const vertex_format& format, uint32_t start_index, uint32_t start_vertex, const shape_options& options)
  {
    assert(options.divisions >= 3 && "must have at-least 3 divisions");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto index_index = start_index;
    auto vertex_index = start_vertex;

    auto radius = options.dimensions[0] / 2.0f;
    auto center_front = options.center + vec3 { 0.0f, 0.0f, options.dimensions[0] * 0.5f };
    auto center_back = options.center + vec3 { 0.0f, 0.0f, options.dimensions[0] * -0.5f };

    if (options.outward_faces)
    {
      circle(mesh, format, index_index, vertex_index, center_front, radius, options.divisions, options.color, false);
      pipe(mesh, format, index_index, vertex_index, center_front, center_back, radius, options.divisions, options.smooth, options.color, false);
      circle(mesh, format, index_index, vertex_index, center_back, radius, options.divisions, options.color, true);
    }

    if (options.inward_faces)
    {
      circle(mesh, format, index_index, vertex_index, center_front, radius, options.divisions, options.color, true);
      pipe(mesh, format, index_index, vertex_index, center_front, center_back, radius, options.divisions, options.smooth, options.color, true);
      circle(mesh, format, index_index, vertex_index, center_back, radius, options.divisions, options.color, false);
    }
  }

  std::pair<uint32_t, uint32_t> cylinder_counts(const vertex_format& format, const shape_options& options)
  {
    assert(options.divisions >= 3 && "must have at-least 3 divisions");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto circle_counts = ludo::circle_counts(format, options);
    auto pipe_total = options.divisions * 6;
    auto pipe_unique = options.divisions * (options.smooth ? 2 : 4); // TODO revise based on presence of normals, texture coordinates etc.
    if (options.outward_faces && options.inward_faces)
    {
      pipe_total *= 2;
      pipe_unique *= 2;
    }
    auto total = circle_counts.first * 2 + pipe_total;
    auto unique = circle_counts.second * 2 + pipe_unique;

    return { total, unique };
  }

  void pipe(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center_front, const vec3& center_back, float radius, uint32_t divisions, bool smooth, const vec4& color, bool invert)
  {
    for (auto division = 0; division < divisions; division++)
    {
      auto angle_0 = -two_pi * static_cast<float>(division) / static_cast<float>(divisions);
      auto angle_1 = -two_pi * static_cast<float>(division + 1) / static_cast<float>(divisions);
      auto circle_position_0 = vec3 { std::sin(angle_0), std::cos(angle_0), 0.0f } * radius;
      auto circle_position_1 = vec3 { std::sin(angle_1), std::cos(angle_1), 0.0f } * radius;

      auto position_bottom_left = center_front + circle_position_0;
      auto position_bottom_right = center_back + circle_position_0;
      auto position_top_left = center_front + circle_position_1;
      auto position_top_right = center_back + circle_position_1;

      auto normal_0 = circle_position_0;
      auto normal_1 = circle_position_1;
      if (!smooth)
      {
        normal_0 = cross(position_bottom_right - position_bottom_left, position_top_left - position_bottom_left);
        normal_1 = normal_0;
      }
      normalize(normal_0);
      normalize(normal_1);

      if (invert)
      {
        std::swap(position_bottom_left, position_bottom_right);
        std::swap(position_top_left, position_top_right);
        normal_0 *= -1.0f;
        normal_1 *= -1.0f;
      }

      write_vertex(mesh, format, index_index, vertex_index, position_bottom_left, normal_0, color, { 0.0f, 0.0f });
      write_vertex(mesh, format, index_index, vertex_index, position_bottom_right, normal_0, color, { 0.0f, 0.0f });
      write_vertex(mesh, format, index_index, vertex_index, position_top_left, normal_1, color, { 0.0f, 0.0f });

      write_vertex(mesh, format, index_index, vertex_index, position_bottom_right, normal_0, color, { 0.0f, 0.0f });
      write_vertex(mesh, format, index_index, vertex_index, position_top_right, normal_1, color, { 0.0f, 0.0f });
      write_vertex(mesh, format, index_index, vertex_index, position_top_left, normal_1, color, { 0.0f, 0.0f });
    }
  }
}
