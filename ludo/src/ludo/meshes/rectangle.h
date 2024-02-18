/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MESHES_RECTANGLE_H
#define LUDO_MESHES_RECTANGLE_H

#include "../meshes.h"

namespace ludo
{
  void rectangle(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& position_bottom_left, const vec3& position_delta_right, const vec3& position_delta_top, const vec2& tex_coord_min, const vec2& tex_coord_delta, bool unique_only, bool no_normal_check, uint32_t divisions);
  void rectangle(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& position_bottom_left, const vec3& position_delta_right, const vec3& position_delta_top, const vec2& tex_coord_min, const vec2& tex_coord_delta, bool unique_only, bool no_normal_check);
}

#endif // LUDO_MESHES_RECTANGLE_H
