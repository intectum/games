/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "shapes.h"

namespace ludo
{
  void circle(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const vec3& center, float radius, uint32_t divisions, const vec4& color, bool invert);
}
