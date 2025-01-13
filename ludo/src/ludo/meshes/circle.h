/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "shapes.h"

namespace ludo
{
  void append_circle(mesh& mesh, buffer& indices, buffer& vertices, const vertex_format& format, const vec3& center, float radius, uint32_t divisions, const vec4& color, bool invert);
}
