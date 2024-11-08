/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "shapes.h"
#include "util.h"

namespace ludo
{
  void sphere_ico(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, const std::array<vec3, 20>& positions, bool smooth, const vec4& color, bool invert, uint32_t divisions);
  void face(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, const std::array<vec3, 3>& positions, bool smooth, const vec4& color, bool invert, uint32_t divisions);
  void face(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, const std::array<vec3, 3>& positions, bool smooth, const vec4& color, bool invert);

  void sphere_ico(mesh& mesh, const vertex_format& format, uint32_t start_index, uint32_t start_vertex, const shape_options& options)
  {
    assert(options.divisions >= 1 && "must have at-least 1 division");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto index_index = start_index;
    auto vertex_index = start_vertex;

    auto radius = options.dimensions[0] / 2.0f;
    auto t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    auto positions = std::array<vec3, 20>
    {
      vec3 { -1.0f, t, 0.0f },
      vec3 { 1.0f, t, 0.0f },
      vec3 { -1.0f, -t, 0.0f },
      vec3 { 1.0f, -t, 0.0f },

      vec3 { 0.0f, -1.0f, t },
      vec3 { 0.0f, 1.0f, t },
      vec3 { 0.0f, -1.0f, -t },
      vec3 { 0.0f, 1.0f, -t },

      vec3 { t, 0.0f, -1.0f },
      vec3 { t, 0.0f, 1.0f },
      vec3 { -t, 0.0f, -1.0f },
      vec3 { -t, 0.0f, 1.0f },
    };

    for (auto& position : positions)
    {
      normalize(position);
    }

    if (options.outward_faces)
    {
      sphere_ico(mesh, format, index_index, vertex_index, options.center, radius, positions, options.smooth, options.color, false, options.divisions);
    }

    if (options.inward_faces)
    {
      sphere_ico(mesh, format, index_index, vertex_index, options.center, radius, positions, options.smooth, options.color, true, options.divisions);
    }
  }

  std::pair<uint32_t, uint32_t> sphere_ico_counts(const vertex_format& format, const shape_options& options)
  {
    assert(options.divisions >= 1 && "must have at-least 1 division");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto total = 20 * 3 * static_cast<uint32_t>(std::pow(4, options.divisions - 1));
    auto unique = total;

    if (options.smooth) // TODO revise based on presence of normals, texture coordinates etc.
    {
      auto triangles = 20;
      unique = 12;
      for (auto division = 1; division < options.divisions; division++)
      {
        unique += 3 * triangles / 2;
        triangles *= 4;
      }
    }

    if (options.inward_faces && options.outward_faces)
    {
      total *= 2;
      unique *= 2;
    }

    return { total, unique };
  }

  void sphere_ico(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, const std::array<vec3, 20>& positions, bool smooth, const vec4& color, bool invert, uint32_t divisions)
  {
    // 5 faces around point 0.
    face(mesh, format, index_index, vertex_index, center, radius, { positions[0], positions[11], positions[5] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[0], positions[5], positions[1] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[0], positions[1], positions[7] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[0], positions[7], positions[10] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[0], positions[10], positions[11] }, smooth, color, invert, divisions);

    // 5 adjacent faces.
    face(mesh, format, index_index, vertex_index, center, radius, { positions[1], positions[5], positions[9] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[5], positions[11], positions[4] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[11], positions[10], positions[2] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[10], positions[7], positions[6] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[7], positions[1], positions[8] }, smooth, color, invert, divisions);

    // 5 faces around point 3.
    face(mesh, format, index_index, vertex_index, center, radius, { positions[3], positions[9], positions[4] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[3], positions[4], positions[2] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[3], positions[2], positions[6] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[3], positions[6], positions[8] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[3], positions[8], positions[9] }, smooth, color, invert, divisions);

    // 5 adjacent faces.
    face(mesh, format, index_index, vertex_index, center, radius, { positions[4], positions[9], positions[5] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[2], positions[4], positions[11] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[6], positions[2], positions[10] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[8], positions[6], positions[7] }, smooth, color, invert, divisions);
    face(mesh, format, index_index, vertex_index, center, radius, { positions[9], positions[8], positions[1] }, smooth, color, invert, divisions);
  }

  void face(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, const std::array<vec3, 3>& positions, bool smooth, const vec4& color, bool invert, uint32_t divisions)
  {
    if (divisions == 1)
    {
      face(mesh, format, index_index, vertex_index, center, radius, positions, smooth, color, invert);
      return;
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    face(mesh, format, index_index, vertex_index, center, radius, { positions[0], position_01, position_02 }, smooth, color, invert, divisions - 1);
    face(mesh, format, index_index, vertex_index, center, radius, { position_01, positions[1], position_12 }, smooth, color, invert, divisions - 1);
    face(mesh, format, index_index, vertex_index, center, radius, { position_02, position_12, positions[2] }, smooth, color, invert, divisions - 1);
    face(mesh, format, index_index, vertex_index, center, radius, { position_01, position_12, position_02 }, smooth, color, invert, divisions - 1);
  }

  void face(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, const std::array<vec3, 3>& positions, bool smooth, const vec4& color, bool invert)
  {
    auto normal_0 = positions[0];
    auto normal_1 = positions[1];
    auto normal_2 = positions[2];

    if (!smooth)
    {
      normal_0 = cross(positions[1] - positions[0], positions[2] - positions[0]);
      normalize(normal_0);
      normal_1 = normal_0;
      normal_2 = normal_0;
    }

    if (invert)
    {
      normal_0 *= -1.0f;
      normal_1 *= -1.0f;
      normal_2 *= -1.0f;
    }

    write_vertex(mesh, format, index_index, vertex_index, center + positions[0] * radius, normal_0, color, { 0.0f, 0.0f });

    if (invert)
    {
      write_vertex(mesh, format, index_index, vertex_index, center + positions[2] * radius, normal_2, color, { 0.0f, 0.0f });
      write_vertex(mesh, format, index_index, vertex_index, center + positions[1] * radius, normal_1, color, { 0.0f, 0.0f });
    }
    else
    {
      write_vertex(mesh, format, index_index, vertex_index, center + positions[1] * radius, normal_1, color, { 0.0f, 0.0f });
      write_vertex(mesh, format, index_index, vertex_index, center + positions[2] * radius, normal_2, color, { 0.0f, 0.0f });
    }
  }
}
