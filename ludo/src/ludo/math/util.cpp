/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "util.h"

namespace ludo
{
  bool near(float a, float b, float epsilon)
  {
    return std::abs(a - b) < epsilon;
  }

  float sign(float value)
  {
    return value < 0.0f ? -1.0f : 1.0f;
  }

  float shortest_angle(float angle)
  {
    // Reduce to the range [-2pi,2pi]
    angle -= std::floor(angle / two_pi) * two_pi;

    // Switch an angle >pi to an angle <pi in the opposite direction
    if (std::abs(angle) > pi)
    {
      angle -= two_pi * sign(angle);
    }

    return angle;
  }
}
