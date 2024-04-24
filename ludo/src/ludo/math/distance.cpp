/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "distance.h"

namespace ludo
{
  /*
   * The equation for the distance from a point to a line:
   *     distance = ||(a - p) - ((a - p).n)n||
   * Where:
   *     a is any point on the line.
   *     p is an arbitrary point.
   *     n is a unit vector in the direction vector of the line.
   * Reference:
   *     http://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
   */
  float distance_point_to_line_segment(const vec3& point, const std::array<vec3, 2>& line_segment)
  {
    auto n = line_segment[1] - line_segment[0];
    auto a_minus_p = line_segment[0] - point;

    // If the point is past either end of the line segment take the distance from the respective end
    auto l_dot_p = dot(n, a_minus_p);
    if (l_dot_p > 0.0f)
    {
      return length(a_minus_p);
    }
    else if (l_dot_p < -1.0f)
    {
      return length(line_segment[1] - point);
    }

    normalize(n);
    return length(a_minus_p - dot(a_minus_p, n) * n);
  }
}
