/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "shapes.h"
#include "util.h"

namespace ludo
{
  void polar_cap(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, uint32_t divisions, bool north, bool smooth, bool invert);
  void quads(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, uint32_t divisions, bool smooth, bool invert);
  vec3 point_on_sphere(float radius, uint32_t divisions, uint32_t parallel, uint32_t meridian);

  void sphere_uv(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const shape_options& options)
  {
    assert(options.divisions >= 3 && "must have at-least 3 divisions");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto radius = options.dimensions[0] / 2.0f;

    if (options.outward_faces)
    {
      polar_cap(mesh, format, index_index, vertex_index, options.center, radius, options.divisions, true, options.smooth, false);
      quads(mesh, format, index_index, vertex_index, options.center, radius, options.divisions, options.smooth, false);
      polar_cap(mesh, format, index_index, vertex_index, options.center, radius, options.divisions, false, options.smooth, false);
    }

    if (options.inward_faces)
    {
      polar_cap(mesh, format, index_index, vertex_index, options.center, radius, options.divisions, true, options.smooth, true);
      quads(mesh, format, index_index, vertex_index, options.center, radius, options.divisions, options.smooth, true);
      polar_cap(mesh, format, index_index, vertex_index, options.center, radius, options.divisions, false, options.smooth, true);
    }
  }

  std::pair<uint32_t, uint32_t> sphere_uv_counts(const shape_options& options)
  {
    assert(options.divisions >= 3 && "must have at-least 3 divisions");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto polar_cap_total = options.divisions * 3;
    auto polar_cap_unique = options.smooth ? options.divisions + 1 : polar_cap_total;
    auto quad_total = options.divisions * 6 * (options.divisions - 2);
    auto quad_unique = options.smooth ? options.divisions * (options.divisions - 3) : options.divisions * 4 * (options.divisions - 2);
    auto total = polar_cap_total * 2 + quad_total;
    auto unique = polar_cap_unique * 2 + quad_unique;
    if (options.inward_faces && options.outward_faces)
    {
      total *= 2;
      unique *= 2;
    }

    return { total, unique };
  }

  void polar_cap(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, uint32_t divisions, bool north, bool smooth, bool invert)
  {
    auto parallel = north ? 1 : divisions - 1;
    auto position_0 = vec3 { 0.0f, north ? radius : -radius, 0.0f };

    for (auto meridian = 0; meridian < divisions; meridian++)
    {
      auto position_1 = point_on_sphere(radius, divisions, parallel, meridian);
      auto position_2 = point_on_sphere(radius, divisions, parallel, meridian + 1);

      auto normal_0 = position_0;
      auto normal_1 = position_1;
      auto normal_2 = position_2;
      if (!smooth)
      {
        normal_0 = cross(position_1 - position_0, position_2 - position_0);
        normal_1 = normal_0;
        normal_2 = normal_0;
      }
      normalize(normal_0);
      normalize(normal_1);
      normalize(normal_2);

      if (!smooth && north != invert)
      {
        normal_0 *= -1.0f;
        normal_1 *= -1.0f;
        normal_2 *= -1.0f;
      }

      write_vertex(mesh, format, index_index, vertex_index, center + position_0, normal_0, { 0.0f, 0.0f });

      if (north != invert)
      {
        write_vertex(mesh, format, index_index, vertex_index, center + position_2, normal_2, { 0.0f, 0.0f });
        write_vertex(mesh, format, index_index, vertex_index, center + position_1, normal_1, { 0.0f, 0.0f });
      }
      else
      {
        write_vertex(mesh, format, index_index, vertex_index, center + position_1, normal_1, { 0.0f, 0.0f });
        write_vertex(mesh, format, index_index, vertex_index, center + position_2, normal_2, { 0.0f, 0.0f });
      }
    }
  }

  void quads(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, uint32_t divisions, bool smooth, bool invert)
  {
    for (auto parallel = 1; parallel < divisions - 1; parallel++)
    {
      for (auto meridian = 0; meridian < divisions; meridian++)
      {
        auto meridian_0 = invert ? meridian + 1 : meridian;
        auto meridian_1 = invert ? meridian : meridian + 1;
        auto position_0 = point_on_sphere(radius, divisions, parallel, meridian_0);
        auto position_1 = point_on_sphere(radius, divisions, parallel, meridian_1);
        auto position_2 = point_on_sphere(radius, divisions, parallel + 1, meridian_0);
        auto position_3 = point_on_sphere(radius, divisions, parallel + 1, meridian_1);

        auto normal_0 = position_0;
        auto normal_1 = position_1;
        auto normal_2 = position_2;
        auto normal_3 = position_3;
        if (!smooth)
        {
          normal_0 = cross(position_1 - position_0, position_2 - position_0);
          normal_1 = normal_0;
          normal_2 = normal_0;
          normal_3 = normal_0;
        }
        normalize(normal_0);
        normalize(normal_1);
        normalize(normal_2);
        normalize(normal_3);

        write_vertex(mesh, format, index_index, vertex_index, center + position_0, normal_0, { 0.0f, 0.0f });
        write_vertex(mesh, format, index_index, vertex_index, center + position_1, normal_1, { 0.0f, 0.0f });
        write_vertex(mesh, format, index_index, vertex_index, center + position_2, normal_2, { 0.0f, 0.0f });

        write_vertex(mesh, format, index_index, vertex_index, center + position_1, normal_1, { 0.0f, 0.0f });
        write_vertex(mesh, format, index_index, vertex_index, center + position_3, normal_3, { 0.0f, 0.0f });
        write_vertex(mesh, format, index_index, vertex_index, center + position_2, normal_2, { 0.0f, 0.0f });
      }
    }
  }

  vec3 point_on_sphere(float radius, uint32_t divisions, uint32_t parallel, uint32_t meridian)
  {
    auto a = pi * static_cast<float>(parallel) / static_cast<float>(divisions);
    auto b = two_pi * static_cast<float>(meridian) / static_cast<float>(divisions);

    return { std::sin(a) * std::cos(b) * radius, std::cos(a) * radius, std::sin(a) * std::sin(b) * radius };
  }
}
