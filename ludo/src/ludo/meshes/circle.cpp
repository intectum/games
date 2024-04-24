/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "circle.h"
#include "shapes.h"
#include "util.h"

namespace ludo
{
  void circle(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const shape_options& options)
  {
    assert(options.divisions >= 3 && "must have at-least 3 divisions");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto radius = options.dimensions[0] / 2.0f;

    if (options.outward_faces)
    {
      circle(mesh, format, index_index, vertex_index, options.center, radius, options.divisions, false);
    }

    if (options.inward_faces)
    {
      circle(mesh, format, index_index, vertex_index, options.center, radius, options.divisions, true);
    }
  }

  std::pair<uint32_t, uint32_t> circle_counts(const shape_options& options)
  {
    assert(options.divisions >= 3 && "must have at-least 3 divisions");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto total =  options.divisions * 3;
    auto unique =  options.divisions + 2;
    if (options.outward_faces && options.inward_faces)
    {
      total *= 2;
      unique *= 2;
    }

    return { total, unique };
  }

  void circle(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, uint32_t divisions, bool invert)
  {
    auto normal = vec3 { 0.0f, 0.0f, 1.0f };
    if (invert)
    {
      normal *= -1.0f;
    }

    for (auto division = 0; division < divisions; division++)
    {
      auto angle_0 = -two_pi * static_cast<float>(division) / static_cast<float>(divisions);
      auto angle_1 = -two_pi * static_cast<float>(division + 1) / static_cast<float>(divisions);

      write_vertex(mesh, format, index_index, vertex_index, center, normal, { 0.0f, 0.0f });

      if (invert)
      {
        write_vertex(mesh, format, index_index, vertex_index, center + vec3 { std::sin(angle_1), std::cos(angle_1), 0.0f } * radius, normal, { 0.0f, 0.0f });
        write_vertex(mesh, format, index_index, vertex_index, center + vec3 { std::sin(angle_0), std::cos(angle_0), 0.0f } * radius, normal, { 0.0f, 0.0f });
      }
      else
      {
        write_vertex(mesh, format, index_index, vertex_index, center + vec3 { std::sin(angle_0), std::cos(angle_0), 0.0f } * radius, normal, { 0.0f, 0.0f });
        write_vertex(mesh, format, index_index, vertex_index, center + vec3 { std::sin(angle_1), std::cos(angle_1), 0.0f } * radius, normal, { 0.0f, 0.0f });
      }
    }
  }
}
