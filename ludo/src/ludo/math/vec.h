/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MATH_VEC_H_
#define LUDO_MATH_VEC_H_

#include <array>
#include <ostream>

#include "util.h"

namespace ludo
{
  ///
  /// A 2D vector.
  struct vec2 : public std::array<float, 2>
  {
    ///
    /// Creates a 2D vector with un-initialized values.
    vec2();

    ///
    /// Creates a 2D vector with explicitly initialized values.
    /// \param x The x value.
    /// \param y The y value.
    vec2(float x, float y);
  };

  ///
  /// A 3D vector.
  struct vec3 : public std::array<float, 3>
  {
    ///
    /// Creates a 3D vector with un-initialized values.
    vec3();

    ///
    /// Creates a 3D vector with explicitly initialized values.
    /// \param x The x value.
    /// \param y The y value.
    /// \param z The z value.
    vec3(float x, float y, float z);

    /// Creates a 3D vector from a homogeneous 3D vector.
    /// \param vec4 The homogeneous 3D vector.
    explicit vec3(std::array<float, 4> vec4);
  };

  ///
  /// A homogeneous 3D vector.
  struct vec4 : public std::array<float, 4>
  {
    ///
    /// Creates a homogeneous 3D vector with un-initialized values.
    vec4();

    ///
    /// Creates a homogeneous 3D vector with explicitly initialized values.
    /// \param x The x value.
    /// \param y The y value.
    /// \param z The z value.
    /// \param w The w value.
    vec4(float x, float y, float z, float w);

    /// Creates a homogeneous 3D vector from a 3D vector.
    /// \param vec3 The 3D vector.
    explicit vec4(std::array<float, 3> vec3, float w = 1.0f);
  };

  const auto vec2_one = vec2 { 1.0f, 1.0f }; ///< A 2D vector with all values set to 1.
  const auto vec2_unit_x = vec2 { 1.0f, 0.0f }; ///< A 2D unit vector on the positive x-axis.
  const auto vec2_unit_y = vec2 { 0.0f, 1.0f }; ///< A 2D unit vector on the positive y-axis.
  const auto vec2_zero = vec2 { 0.0f, 0.0f }; ///< A 2D vector with all values set to 0.

  const auto vec3_one = vec3 { 1.0f, 1.0f, 1.0f }; ///< A 3D vector with all values set to 1.
  const auto vec3_unit_x = vec3 { 1.0f, 0.0f, 0.0f }; ///< A 3D unit vector on the positive x-axis.
  const auto vec3_unit_y = vec3 { 0.0f, 1.0f, 0.0f }; ///< A 3D unit vector on the positive y-axis.
  const auto vec3_unit_z = vec3 { 0.0f, 0.0f, 1.0f }; ///< A 3D unit vector on the positive z-axis.
  const auto vec3_zero = vec3 { 0.0f, 0.0f, 0.0f }; ///< A 3D vector with all values set to 0.

  const auto vec4_one = vec4 { 1.0f, 1.0f, 1.0f, 1.0f }; ///< A homogeneous 3D vector with all values set to 1.
  const auto vec4_unit_x = vec4 { 1.0f, 0.0f, 0.0f, 1.0f }; ///< A homogeneous 3D unit vector on the positive x-axis.
  const auto vec4_unit_y = vec4 { 0.0f, 1.0f, 0.0f, 1.0f }; ///< A homogeneous 3D unit vector on the positive y-axis.
  const auto vec4_unit_z = vec4 { 0.0f, 0.0f, 1.0f, 1.0f }; ///< A homogeneous 3D unit vector on the positive z-axis.
  const auto vec4_zero = vec4 { 0.0f, 0.0f, 0.0f, 1.0f }; ///< A homogeneous 3D vector with all values set to 0 (except for w which is set to 1).

  ///
  /// Adds the right-hand vector to the left-hand vector.
  /// \param lhs The left-hand vector.
  /// \param rhs The right-hand vector.
  /// \return The product.
  vec2 operator+(const vec2& lhs, const vec2& rhs);
  vec3 operator+(const vec3& lhs, const vec3& rhs);
  vec4 operator+(const vec4& lhs, const vec4& rhs);
  vec2& operator+=(vec2& lhs, const vec2& rhs);
  vec3& operator+=(vec3& lhs, const vec3& rhs);
  vec4& operator+=(vec4& lhs, const vec4& rhs);

  ///
  /// Subtracts the right-hand vector from the left-hand vector.
  /// \param lhs The left-hand vector.
  /// \param rhs The right-hand vector.
  /// \return The product.
  vec2 operator-(const vec2& lhs, const vec2& rhs);
  vec3 operator-(const vec3& lhs, const vec3& rhs);
  vec4 operator-(const vec4& lhs, const vec4& rhs);
  vec2& operator-=(vec2& lhs, const vec2& rhs);
  vec3& operator-=(vec3& lhs, const vec3& rhs);
  vec4& operator-=(vec4& lhs, const vec4& rhs);

  ///
  /// Multiplies the left-hand vector by the right-hand vector.
  /// \param lhs The left-hand vector.
  /// \param rhs The right-hand vector.
  /// \return The product.
  vec2 operator*(const vec2& lhs, const vec2& rhs);
  vec3 operator*(const vec3& lhs, const vec3& rhs);
  vec4 operator*(const vec4& lhs, const vec4& rhs);
  vec2& operator*=(vec2& lhs, const vec2& rhs);
  vec3& operator*=(vec3& lhs, const vec3& rhs);
  vec4& operator*=(vec4& lhs, const vec4& rhs);

  ///
  /// Multiplies the vector by a scalar.
  /// \param vector The vector.
  /// \param scalar The scalar to multiply the vector by.
  /// \return The scaled vector.
  vec2 operator*(const vec2& vector, float scalar);
  vec3 operator*(const vec3& vector, float scalar);
  vec4 operator*(const vec4& vector, float scalar);
  vec2& operator*=(vec2& vector, float scalar);
  vec3& operator*=(vec3& vector, float scalar);
  vec4& operator*=(vec4& vector, float scalar);

  ///
  /// Multiplies the vector by a scalar.
  /// \param scalar The scalar to multiply the vector by.
  /// \param vector The vector.
  /// \return The scaled vector.
  vec2 operator*(float scalar, const vec2& vector);
  vec3 operator*(float scalar, const vec3& vector);
  vec4 operator*(float scalar, const vec4& vector);

  ///
  /// Divides the left-hand vector by the right-hand vector.
  /// \param lhs The left-hand vector.
  /// \param rhs The right-hand vector.
  /// \return The product.
  vec2 operator/(const vec2& lhs, const vec2& rhs);
  vec3 operator/(const vec3& lhs, const vec3& rhs);
  vec4 operator/(const vec4& lhs, const vec4& rhs);
  vec2& operator/=(vec2& lhs, const vec2& rhs);
  vec3& operator/=(vec3& lhs, const vec3& rhs);
  vec4& operator/=(vec4& lhs, const vec4& rhs);

  ///
  /// Divides the vector by a scalar.
  /// \param vector The vector.
  /// \param scalar The scalar to divide the vector by.
  /// \return The scaled vector.
  vec2 operator/(const vec2& vector, float scalar);
  vec3 operator/(const vec3& vector, float scalar);
  vec4 operator/(const vec4& vector, float scalar);
  vec2& operator/=(vec2& vector, float scalar);
  vec3& operator/=(vec3& vector, float scalar);
  vec4& operator/=(vec4& vector, float scalar);

  ///
  /// Sends a textual representation of a vector to an output stream.
  /// \param stream The output stream.
  /// \param vector The vector.
  /// \return The output stream.
  std::ostream& operator<<(std::ostream& stream, const vec2& vector);
  std::ostream& operator<<(std::ostream& stream, const vec3& vector);
  std::ostream& operator<<(std::ostream& stream, const vec4& vector);

  ///
  /// Retrieves the length (or "magnitude" in fancy talk) of a vector.
  /// \ @param vector The vector.
  /// \return The length of the vector.
  float length(const vec2& vector);
  float length(const vec3& vector);
  float length(const vec4& vector);

  ///
  /// Retrieves the squared length (or "squared magnitude" in fancy talk) of a vector. This is faster than retrieving
  /// the length so it is good for comparing lengths.
  /// \param vector The vector.
  /// \return The squared length of the vector.
  float length2(const vec2& vector);
  float length2(const vec3& vector);
  float length2(const vec4& vector);

  ///
  /// Normalizes a vector.
  /// \param vector The vector.
  void normalize(vec2& vector);
  void normalize(vec3& vector);
  void normalize(vec4& vector);

  ///
  /// Rotates a vector.
  /// \param vector The vector to rotate.
  /// \param angle The angle to rotate the vector by.
  void rotate(vec2& vector, float angle);

  ///
  /// Rotates a vector.
  /// \param vector The vector to rotate.
  /// \param axis The axis of rotation.
  /// \param angle The angle around the axis of rotation.
  void rotate(vec3& vector, vec3& axis, float angle);

  ///
  /// Homogenizes a vector.
  /// \param vector The vector.
  void homogenize(vec4& vector);

  ///
  /// Determines if all the corresponding values of two vectors are 'near' to each-other. This helps to determine
  /// equality of vectors which have been derived through different calculations and as a result of floating point
  /// inaccuracy are slightly different.
  /// \param a The first vector.
  /// \param b The second vector.
  /// \param epsilon The maximum acceptable difference.
  /// \return True if all the corresponding values of the vectors do not differ more than the epsilon, false otherwise.
  bool near(const vec2& a, const vec2& b, float epsilon = 0.0001f);
  bool near(const vec3& a, const vec3& b, float epsilon = 0.0001f);
  bool near(const vec4& a, const vec4& b, float epsilon = 0.0001f);

  ///
  /// Calculates the cross product of two vectors.
  /// \param lhs The left-hand vector.
  /// \param rhs The right-hand vector.
  /// \return The cross product of two vectors.
  float cross(const vec2& lhs, const vec2& rhs);
  vec3 cross(const vec3& lhs, const vec3& rhs);

  ///
  /// Calculates the dot product of two vectors.
  /// \param lhs The left-hand vector.
  /// \param rhs The right-hand vector.
  /// \return The dot product of two vectors.
  float dot(const vec2& lhs, const vec2& rhs);
  float dot(const vec3& lhs, const vec3& rhs);
  float dot(const vec4& lhs, const vec4& rhs);

  ///
  /// Projects one vector onto another.
  /// \param vector The vector to project.
  /// \param target The vector to project onto.
  /// \return The projection.
  vec2 project(const vec2& vector, const vec2& target);
  vec3 project(const vec3& vector, const vec3& target);

  ///
  /// Performs a scalar projection of one unit vector onto another unit vector.
  /// \param vector The vector to project (must be unit length).
  /// \param target The vector to project onto (must be unit length).
  /// \return The scalar projection.
  float scalar_project(const vec2& vector, const vec2& target);
  float scalar_project(const vec3& vector, const vec3& target);

  ///
  /// Calculates the angle between two unit vectors.
  /// \param a The first vector (must be unit length).
  /// \param b The second vector (must be unit length).
  /// \return The angle.
  float angle_between(const vec2& a, const vec2& b);
  float angle_between(const vec3& a, const vec3& b);
}

#endif /* LUDO_MATH_VEC_H_ */
