/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MATH_DISTANCE_H
#define LUDO_MATH_DISTANCE_H

#include "vec.h"

namespace ludo
{
  float distance_point_to_line_segment(const vec3& point, const std::array<vec3, 2>& line_segment);
}

#endif // LUDO_MATH_DISTANCE_H
