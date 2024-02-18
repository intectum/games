/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "box.h"
#include "rectangle.h"

namespace ludo
{
  void box(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const shape_options& options)
  {
    box(mesh, format, index_index, vertex_index, options, true, false);
  }

  void box(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const shape_options& options, bool unique_only, bool no_normal_check)
  {
    assert(options.divisions >= 1 && "must have at-least 1 division");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto tex_coord_delta = vec2 { 1.0f / 4.0f, 1.0f / 3.0f };

    if (options.outward_faces)
    {
      // Front
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * -0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * 0.5f },
        vec3 { options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2 { 1.0f / 4.0f, 1.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Back
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * 0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * -0.5f },
        vec3 { -options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2 { 3.0f / 4.0f, 1.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Left
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * -0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * -0.5f },
        vec3 { 0.0f, 0.0f, options.dimensions[0] },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2 { 0.0f, 1.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Right
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * 0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * 0.5f },
        vec3 { 0.0f, 0.0f, -options.dimensions[0] },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2 { 2.0f / 4.0f, 1.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Top
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * -0.5f, options.dimensions[1] * 0.5f, options.dimensions[2] * 0.5f },
        vec3 { options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, 0.0f, -options.dimensions[1] },
        vec2 { 1.0f / 4.0f, 2.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Bottom
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * -0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * -0.5f },
        vec3 { options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, 0.0f, options.dimensions[1] },
        vec2 { 1.0f / 4.0f, 0.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );
    }

    if (options.inward_faces)
    {
      // Front
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * 0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * 0.5f },
        vec3 { -options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2 { 1.0f / 4.0f, 1.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Back
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { -options.dimensions[0] * 0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * -0.5f },
        vec3 { options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2 { 3.0f / 4.0f, 1.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Left
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * -0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * 0.5f },
        vec3 { 0.0f, 0.0f, -options.dimensions[0] },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2 { 2.0f / 4.0f, 1.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Right
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * 0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * -0.5f },
        vec3 { 0.0f, 0.0f, options.dimensions[0] },
        vec3 { 0.0f, options.dimensions[1], 0.0f },
        vec2 { 0.0f, 1.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Top
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * 0.5f, options.dimensions[1] * 0.5f, options.dimensions[2] * 0.5f },
        vec3 { -options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, 0.0f, -options.dimensions[1] },
        vec2 { 1.0f / 4.0f, 2.0f / 3.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );

      // Bottom
      rectangle(
        mesh,
        format,
        index_index,
        vertex_index,
        options.center + vec3 { options.dimensions[0] * 0.5f, options.dimensions[1] * -0.5f, options.dimensions[2] * -0.5f },
        vec3 { -options.dimensions[0], 0.0f, 0.0f },
        vec3 { 0.0f, 0.0f, options.dimensions[1] },
        vec2 { 1.0f / 4.0f, 0.0f },
        tex_coord_delta,
        unique_only,
        no_normal_check,
        options.divisions
      );
    }
  }

  std::pair<uint32_t, uint32_t> box_counts(const shape_options& options)
  {
    assert(options.divisions >= 1 && "must have at-least 1 division");
    assert(options.outward_faces || options.inward_faces && "outward and/or inward faces must be specified");

    auto counts = rectangle_counts(options);
    counts.first *= 6;
    counts.second *= 6;

    return counts;
  }
}
