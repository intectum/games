/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "rectangle.h"
#include "shapes.h"
#include "util.h"

namespace ludo
{
  void rectangle(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const shape_options& options)
  {
    assert(options.divisions >= 1 && "must have at-least 1 division");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    if (options.outward_faces)
    {
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * -0.5f, options.dimensions[1] * -0.5f, 0.0f },
        vec3 { options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2_zero,
        vec2_one,
        true,
        false,
        options.divisions
      );
    }

    if (options.inward_faces)
    {
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * 0.5f, options.dimensions[1] * -0.5f, 0.0f },
        vec3 { -options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2_zero,
        vec2_one,
        true,
        false,
        options.divisions
      );
    }
  }

  std::pair<uint32_t, uint32_t> rectangle_counts(const shape_options& options)
  {
    assert(options.divisions >= 1 && "must have at-least 1 division");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto total = options.divisions * options.divisions * 6;
    auto unique = (options.divisions + 1) * (options.divisions + 1);
    if (options.outward_faces && options.inward_faces)
    {
      total *= 2;
      unique *= 2;
    }

    return { total, unique };
  }

  void rectangle(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& position_bottom_left, const vec3& position_delta_right, const vec3& position_delta_top, const vec2& tex_coord_min, const vec2& tex_coord_delta, bool unique_only, bool no_normal_check, uint32_t divisions)
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
        rectangle(mesh, format, index_index, vertex_index, cell_position_bottom_left, cell_position_delta_right, cell_position_delta_top, cell_tex_coord_min, cell_tex_coord_delta, unique_only, no_normal_check);

        cell_position_bottom_left += cell_position_delta_right;
        cell_tex_coord_min += vec2 { cell_tex_coord_delta[0], 0.0f };
      }
    }
  }

  void rectangle(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& position_bottom_left, const vec3& position_delta_right, const vec3& position_delta_top, const vec2& tex_coord_min, const vec2& tex_coord_delta, bool unique_only, bool no_normal_check)
  {
    auto position_offset = offset(format, 'p');
    auto normal_offset = offset(format, 'n');
    auto tex_coord_offset = offset(format, 't');
    auto has_normals = std::any_of(format.components.begin(), format.components.end(), [](const char& component) { return component == 'n'; });
    auto has_tex_coords = std::any_of(format.components.begin(), format.components.end(), [](const char& component) { return component == 't'; });

    auto normal = cross(position_delta_right, position_delta_top);
    normalize(normal);

    auto tex_coord_bottom_right = tex_coord_min + vec2 { tex_coord_delta[0], 0.0f };
    auto tex_coord_top_left = tex_coord_min + vec2 { 0.0f, tex_coord_delta[1] };
    auto tex_coord_top_right = tex_coord_min + tex_coord_delta;

    write_vertex(mesh, index_index, vertex_index, format.size, position_bottom_left, position_offset, normal, has_normals, normal_offset, tex_coord_min, has_tex_coords, tex_coord_offset, unique_only, no_normal_check);
    write_vertex(mesh, index_index, vertex_index, format.size, position_bottom_left + position_delta_right, position_offset, normal, has_normals, normal_offset, tex_coord_bottom_right, has_tex_coords, tex_coord_offset, unique_only, no_normal_check);
    write_vertex(mesh, index_index, vertex_index, format.size, position_bottom_left + position_delta_top, position_offset, normal, has_normals, normal_offset, tex_coord_top_left, has_tex_coords, tex_coord_offset, unique_only, no_normal_check);

    write_vertex(mesh, index_index, vertex_index, format.size, position_bottom_left + position_delta_right, position_offset, normal, has_normals, normal_offset, tex_coord_bottom_right, has_tex_coords, tex_coord_offset, unique_only, no_normal_check);
    write_vertex(mesh, index_index, vertex_index, format.size, position_bottom_left + position_delta_right + position_delta_top, position_offset, normal, has_normals, normal_offset, tex_coord_top_right, has_tex_coords, tex_coord_offset, unique_only, no_normal_check);
    write_vertex(mesh, index_index, vertex_index, format.size, position_bottom_left + position_delta_top, position_offset, normal, has_normals, normal_offset, tex_coord_top_left, has_tex_coords, tex_coord_offset, unique_only, no_normal_check);
  }
}
