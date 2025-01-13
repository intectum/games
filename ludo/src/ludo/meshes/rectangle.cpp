/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "rectangle.h"
#include "shapes.h"
#include "util.h"

namespace ludo
{
  void append_rectangle(mesh& mesh, buffer& indices, buffer& vertices, const vertex_format& format, const shape_options& options)
  {
    assert(options.divisions >= 1 && "must have at-least 1 division");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    if (options.outward_faces)
    {
      append_rectangle(
        mesh,
        indices,
        vertices,
        format,
        options.center + vec3 { options.dimensions[0] * -0.5f, options.dimensions[1] * -0.5f, 0.0f },
        vec3 { options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2_zero,
        vec2_one,
        true,
        false,
        options.color,
        options.divisions
      );
    }

    if (options.inward_faces)
    {
      append_rectangle(
        mesh,
        indices,
        vertices,
        format,
        options.center + vec3 { options.dimensions[0] * 0.5f, options.dimensions[1] * -0.5f, 0.0f },
        vec3 { -options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2_zero,
        vec2_one,
        true,
        false,
        options.color,
        options.divisions
      );
    }
  }

  std::pair<uint32_t, uint32_t> rectangle_counts(const vertex_format& format, const shape_options& options)
  {
    assert(options.divisions >= 1 && "must have at-least 1 division");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto total = options.divisions * options.divisions * 6;
    auto unique = (options.divisions + 1) * (options.divisions + 1);
    if (options.outward_faces && options.inward_faces) // TODO revise based on presence of normals, texture coordinates etc.
    {
      total *= 2;
      unique *= 2;
    }

    return { total, unique };
  }

  void append_rectangle(mesh& mesh, buffer& indices, buffer& vertices, const vertex_format& format, const vec3& position_bottom_left, const vec3& position_delta_right, const vec3& position_delta_top, const vec2& tex_coord_min, const vec2& tex_coord_delta, bool unique_only, bool no_normal_check, const vec4& color, uint32_t divisions)
  {
    auto cell_position_delta_right = position_delta_right / static_cast<float>(divisions);
    auto cell_position_delta_top = position_delta_top / static_cast<float>(divisions);
    auto cell_tex_coord_delta = tex_coord_delta / static_cast<float>(divisions);

    for (auto row = 0; row < divisions; row++)
    {
      auto cell_position_bottom_left = position_bottom_left + cell_position_delta_top * static_cast<float>(row);
      auto cell_tex_coord_min = tex_coord_min + vec2 { 0.0f, cell_tex_coord_delta[1] } * static_cast<float>(row);

      for (auto column = 0; column < divisions; column++)
      {
        append_rectangle(mesh, indices, vertices, format, cell_position_bottom_left, cell_position_delta_right, cell_position_delta_top, cell_tex_coord_min, cell_tex_coord_delta, unique_only, no_normal_check, color);

        cell_position_bottom_left += cell_position_delta_right;
        cell_tex_coord_min += vec2 { cell_tex_coord_delta[0], 0.0f };
      }
    }
  }

  void append_rectangle(mesh& mesh, buffer& indices, buffer& vertices, const vertex_format& format, const vec3& position_bottom_left, const vec3& position_delta_right, const vec3& position_delta_top, const vec2& tex_coord_min, const vec2& tex_coord_delta, bool unique_only, bool no_normal_check, const vec4& color)
  {
    auto normal = cross(position_delta_right, position_delta_top);
    normalize(normal);

    auto tex_coord_bottom_right = tex_coord_min + vec2 { tex_coord_delta[0], 0.0f };
    auto tex_coord_top_left = tex_coord_min + vec2 { 0.0f, tex_coord_delta[1] };
    auto tex_coord_top_right = tex_coord_min + tex_coord_delta;

    append_vertex(mesh, indices, vertices, format, position_bottom_left, normal, color, tex_coord_min, unique_only, no_normal_check);
    append_vertex(mesh, indices, vertices, format, position_bottom_left + position_delta_right, normal, color, tex_coord_bottom_right, unique_only, no_normal_check);
    append_vertex(mesh, indices, vertices, format, position_bottom_left + position_delta_top, normal, color, tex_coord_top_left, unique_only, no_normal_check);

    append_vertex(mesh, indices, vertices, format, position_bottom_left + position_delta_right, normal, color, tex_coord_bottom_right, unique_only, no_normal_check);
    append_vertex(mesh, indices, vertices, format, position_bottom_left + position_delta_right + position_delta_top, normal, color, tex_coord_top_right, unique_only, no_normal_check);
    append_vertex(mesh, indices, vertices, format, position_bottom_left + position_delta_top, normal, color, tex_coord_top_left, unique_only, no_normal_check);
  }
}
