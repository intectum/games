/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <array>
#include <ostream>

#include "vec.h"

namespace ludo
{
  ///
  /// A quaternion for use with 3D rotations.
  struct quat : public std::array<float, 4>
  {
    ///
    /// Creates a quaternion with un-initialized values.
    quat();

    ///
    /// Creates a quaternion with explicitly initialized values.
    /// \param x The x value.
    /// \param y The y value.
    /// \param z The z value.
    /// \param w The w value.
    quat(float x, float y, float z, float w);

    ///
    /// Creates a quaternion from euler angles.
    /// \param x The euler angle around the x-axis.
    /// \param y The euler angle around the y-axis.
    /// \param z The euler angle around the z-axis.
    quat(float x, float y, float z);

    ///
    /// Creates a quaternion from an axis angle.
    /// \param axis The axis of rotation.
    /// \param angle The angle around the axis of rotation.
    quat(const vec3& axis, float angle);

    ///
    /// Creates a quaternion from a column-major 3x3 matrix.
    /// \param mat3 The matrix.
    explicit quat(std::array<float, 9> rotation);

    ///
    /// Creates a quaternion that represents the rotation between two unit vectors.
    /// \param from The starting vector (must be unit length).
    /// \param to The ending vector (must be unit length).
    quat(const vec3& from, const vec3& to);

    ///
    /// Creates a quaternion that represents the difference between two quaternions.
    /// \param from The starting quaternion.
    /// \param to The ending quaternion.
    quat(const quat& from, const quat& to);
  };

  const auto quat_identity = quat { 0.0f, 0.0f, 0.0f, 1.0f }; ///< The identity quaternion.

  ///
  /// Multiplies the left-hand quaternion by the right-hand quaternion.
  /// \param lhs The left-hand quaternion.
  /// \param rhs The right-hand quaternion.
  /// \return The product.
  quat operator*(const quat& lhs, const quat& rhs);
  quat& operator*=(quat& lhs, const quat& rhs);

  ///
  /// Multiplies the quaternion by a scalar.
  /// \param quaternion The quaternion.
  /// \param scalar The scalar to multiply the quaternion by.
  /// \return The scaled quaternion.
  quat operator*(const quat& quaternion, float scalar);
  quat& operator*=(quat& quaternion, float scalar);

  ///
  /// Multiplies the quaternion by a scalar.
  /// \param scalar The scalar to multiply the quaternion by.
  /// \param quaternion The quaternion.
  /// \return The scaled quaternion.
  quat operator*(float scalar, const quat& quaternion);

  ///
  /// Divides the quaternion by a scalar.
  /// \param quaternion The quaternion.
  /// \param scalar The scalar to divide the quaternion by.
  /// \return The scaled quaternion.
  quat operator/(const quat& quaternion, float scalar);
  quat& operator/=(quat& quaternion, float scalar);

  ///
  /// Sends a textual representation of a quaternion to an output stream.
  /// \param stream The output stream.
  /// \param quaternion The quaternion.
  /// \return The output stream.
  std::ostream& operator<<(std::ostream& stream, const quat& quaternion);

  ///
  /// Retrieves the length (or "magnitude" in fancy talk) of a quaternion.
  /// \param quaternion The quaternion.
  /// \return The length of the quaternion.
  float length(const quat& quaternion);

  ///
  /// Retrieves the euler angle representation of a quaternion.
  /// \param quaternion The quaternion.
  /// \return The euler angle representation of the quaternion.
  vec3 angles(const quat& quaternion);

  ///
  /// Retrieves the axis angle representation of a quaternion.
  /// \param quaternion The quaternion.
  /// \return The axis angle representation of the quaternion.
  std::pair<vec3, float> axis_angle(const quat& quaternion);

  ///
  /// Normalizes a quaternion.
  /// \param quaternion The quaternion.
  void normalize(quat& quaternion);

  ///
  /// Inverts a quaternion.
  /// \param quaternion The quaternion.
  void invert(quat& quaternion);

  ///
  /// Determines if all the corresponding values of two quaternions are 'near' to each-other. This helps to determine
  /// equality of quaternions which have been derived through different calculations and as a result of floating point
  /// inaccuracy are slightly different.
  /// \param a The first quaternion.
  /// \param b The second quaternion.
  /// \param epsilon The maximum acceptable difference.
  /// \return True if all the corresponding values of the quaternions do not differ more than the epsilon, false otherwise.
  bool near(const quat& a, const quat& b, float epsilon = 0.0001f);

  ///
  /// Calculates the dot product of two quaternions.
  /// \param lhs The left-hand quaternion.
  /// \param rhs The right-hand quaternion.
  /// \return The dot product of two quaternions.
  float dot(const quat& lhs, const quat& rhs);

  ///
  /// Performs "spherical linear interpolation" between two quaternions.
  /// \param from The starting quaternion.
  /// \param to The ending quaternion.
  /// \param time The time of the interpolation in the range [0,1].
  /// \return The spherical linear interpolation between two quaternions.
  quat slerp(const quat& from, const quat& to, float time);
}
