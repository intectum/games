/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "vec.h"

namespace ludo
{
  float distance_point_to_line_segment(const vec3& point, const std::array<vec3, 2>& line_segment);
}
