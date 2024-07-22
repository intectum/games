/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

namespace ludo
{
  const float pi = 3.14159265358979323f; ///< The ratio of a circle's circumference to its diameter
  const float two_pi = 2.0f * pi;

  ///
  /// Determines if two floating point values are 'near' to each-other. This helps to determine equality of values which
  /// have been derived through different calculations and as a result of floating point inaccuracy are slightly
  /// different.
  /// \param a The first floating point value.
  /// \param b The second floating point value.
  /// \param epsilon The maximum acceptable difference.
  /// \return True if the floating point values do not differ more than the epsilon, false otherwise.
  bool near(float a, float b, float epsilon = 0.0001f);

  ///
  /// Determines the sign of the given value.
  /// \param value The value.
  /// \return -1.0f if the given value is negative, 1.0f otherwise.
  float sign(float value);

  ///
  /// Reduce an arbitrary angle to the shortest equivalent angle.
  /// \param angle The angle to reduce.
  /// \return The shortest equivalent angle.
  float shortest_angle(float angle);
}
