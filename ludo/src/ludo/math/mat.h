/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MATH_MAT_H_
#define LUDO_MATH_MAT_H_

#include <array>
#include <ostream>

#include "../core.h"
#include "vec.h"

namespace ludo
{
  ///
  /// A column-major 3x3 matrix. Not the kind with Keanu Reeves in it. I'm sorry if you were looking for that one.
  /// This is the mathematical kind for use with 3D rotations.
  struct mat3 : public std::array<float, 9>
  {
    ///
    /// Creates a rotation matrix with un-initialized values.
    mat3();

    ///
    /// Creates a rotation matrix with explicitly initialized values.
    /// \param m00 The [0,0] value (the x component of the x axis).
    /// \param m01 The [0,1] value (the y component of the x axis).
    /// \param m02 The [0,2] value (the z component of the x axis).
    /// \param m10 The [1,0] value (the x component of the y axis).
    /// \param m11 The [1,1] value (the y component of the y axis).
    /// \param m12 The [1,2] value (the z component of the y axis).
    /// \param m20 The [2,0] value (the x component of the z axis).
    /// \param m21 The [2,1] value (the y component of the z axis).
    /// \param m22 The [2,2] value (the z component of the z axis).
    mat3(float m00, float m01, float m02,
         float m10, float m11, float m12,
         float m20, float m21, float m22);

    ///
    /// Creates a rotation matrix from euler angles.
    /// \param x The euler angle around the x-axis.
    /// \param y The euler angle around the y-axis.
    /// \param z The euler angle around the z-axis.
    mat3(float x, float y, float z);

    ///
    /// Creates a rotation matrix from an axis angle.
    /// \param axis The axis of rotation.
    /// \param angle The angle around the axis of rotation.
    mat3(const vec3& axis, float angle);

    ///
    /// Creates a rotation matrix from a quaternion.
    /// \param quat The quaternion.
    explicit mat3(std::array<float, 4> quaternion);

    ///
    /// Creates a rotation matrix from a transformation matrix.
    /// \param transformation The transformation matrix.
    explicit mat3(std::array<float, 16> transformation);

    ///
    /// Creates a rotation matrix that represents the rotation between two unit vectors.
    /// \param from The starting vector (must be unit length).
    /// \param to The ending vector (must be unit length).
    mat3(const vec3& from, const vec3& to);
  };

  ///
  /// A column-major 4x4 matrix. Not the kind with Keanu Reeves in it. I'm sorry if you were looking for that one.
  /// This is the mathematical kind for use with 3D transformations (rotations AND positions).
  struct mat4 : public std::array<float, 16>
  {
    ///
    /// Creates a transformation matrix with un-initialized values.
    mat4();

    ///
    /// Creates a transformation matrix with explicitly initialized values.
    /// \param m00 The [0,0] value (the x component of the x axis).
    /// \param m01 The [0,1] value (the y component of the x axis).
    /// \param m02 The [0,2] value (the z component of the x axis).
    /// \param m03 The [0,3] value.
    /// \param m10 The [1,0] value (the x component of the y axis).
    /// \param m11 The [1,1] value (the y component of the y axis).
    /// \param m12 The [1,2] value (the z component of the y axis).
    /// \param m13 The [1,3] value.
    /// \param m20 The [2,0] value (the x component of the z axis).
    /// \param m21 The [2,1] value (the y component of the z axis).
    /// \param m22 The [2,2] value (the z component of the z axis).
    /// \param m23 The [2,3] value.
    /// \param m30 The [3,0] value (the x component of the position).
    /// \param m31 The [3,1] value (the y component of the position).
    /// \param m32 The [3,2] value (the z component of the position).
    /// \param m33 The [3,3] value.
    mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33);

    ///
    /// Creates a transformation matrix from a rotation matrix and a position vector.
    /// \param position The position vector.
    /// \param rotation The rotation matrix.
    mat4(std::array<float, 3> position, std::array<float, 9> rotation);
  };

  const auto mat3_identity = mat3 ///< The identity matrix.
  {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  };

  const auto mat4_identity = mat4 ///< The identity matrix.
  {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  ///
  /// Adds the right-hand matrix to the left-hand matrix.
  /// \param lhs The left-hand matrix.
  /// \param rhs The right-hand matrix.
  /// \return The product.
  mat3 operator+(const mat3& lhs, const mat3& rhs);
  mat4 operator+(const mat4& lhs, const mat4& rhs);
  mat3& operator+=(mat3& lhs, const mat3& rhs);
  mat4& operator+=(mat4& lhs, const mat4& rhs);

  ///
  /// Subtracts the right-hand matrix from the left-hand matrix.
  /// \param lhs The left-hand matrix.
  /// \param rhs The right-hand matrix.
  /// \return The product.
  mat3 operator-(const mat3& lhs, const mat3& rhs);
  mat4 operator-(const mat4& lhs, const mat4& rhs);
  mat3& operator-=(mat3& lhs, const mat3& rhs);
  mat4& operator-=(mat4& lhs, const mat4& rhs);

  ///
  /// Multiplies the left-hand matrix by the right-hand matrix.
  /// \param lhs The left-hand matrix.
  /// \param rhs The right-hand matrix.
  /// \return The product.
  mat3 operator*(const mat3& lhs, const mat3& rhs);
  mat4 operator*(const mat4& lhs, const mat4& rhs);
  mat3& operator*=(mat3& lhs, const mat3& rhs);
  mat4& operator*=(mat4& lhs, const mat4& rhs);

  ///
  /// Multiplies a matrix with a vector.
  /// \param matrix The matrix.
  /// \param vector The vector.
  /// \return The product.
  vec3 operator*(const mat3& matrix, const vec3& vector);
  vec4 operator*(const mat4& matrix, const vec4& vector);

  ///
  /// Multiplies a vector with a matrix.
  /// \param vector The vector.
  /// \param matrix The matrix.
  /// \return The product.
  vec3 operator*(const vec3& vector, const mat3& matrix);
  vec4 operator*(const vec4& vector, const mat4& matrix);

  ///
  /// Multiplies the matrix by a scalar.
  /// \param matrix The matrix.
  /// \param scalar The scalar to multiply the matrix by.
  /// \return The scaled matrix.
  mat3 operator*(const mat3& matrix, float scalar);
  mat4 operator*(const mat4& matrix, float scalar);
  mat3& operator*=(mat3& matrix, float scalar);
  mat4& operator*=(mat4& matrix, float scalar);

  ///
  /// Multiplies the matrix by a scalar.
  /// \param scalar The scalar to multiply the matrix by.
  /// \param matrix The matrix.
  /// \return The scaled matrix.
  mat3 operator*(float scalar, const mat3& matrix);
  mat4 operator*(float scalar, const mat4& matrix);

  ///
  /// Sends a textual representation of a matrix to an output stream.
  /// \param stream The output stream.
  /// \param matrix The matrix.
  /// \return The output stream.
  std::ostream& operator<<(std::ostream& stream, const mat3& matrix);
  std::ostream& operator<<(std::ostream& stream, const mat4& matrix);

  ///
  /// Retrieves the position of a matrix.
  /// \param matrix The matrix.
  /// \return The position.
  vec3 position(const mat4& matrix);
  vec4 position4(const mat4& matrix);

  ///
  /// Sets the position of a matrix.
  /// \param matrix The matrix.
  /// \param position The position.
  void position(mat4& matrix, const vec3& position);
  void position(mat4& matrix, const vec4& position);

  ///
  /// Retrieves the rightward facing (positive x) axis of a matrix.
  /// \param matrix The matrix.
  /// \return The rightward facing (positive x) axis.
  vec3 right(const mat3& matrix);
  vec3 right(const mat4& matrix);
  vec4 right4(const mat4& matrix);

  ///
  /// Retrieves the upward facing (positive y) axis of a matrix.
  /// \param matrix The matrix.
  /// \return The upward facing (positive y) axis.
  vec3 up(const mat3& matrix);
  vec3 up(const mat4& matrix);
  vec4 up4(const mat4& matrix);

  ///
  /// Retrieves the outward facing (negative z) axis of a matrix.
  /// \param matrix The matrix.
  /// \return The outward facing (negative z) axis.
  vec3 out(const mat3& matrix);
  vec3 out(const mat4& matrix);
  vec4 out4(const mat4& matrix);

  ///
  /// Retrieves the euler angle representation of a matrix.
  /// \param matrix The matrix.
  /// \return The euler angle representation of the matrix.
  vec3 angles(const mat3& matrix);
  vec3 angles(const mat4& matrix);

  /// Determines the determinant... whoah.
  /// \param matrix The matrix.
  /// \return The determined determinant. Not to say the the determinant has a determined personality, just that it has
  /// indeed been determined.
  float determinant(const mat3& matrix);
  float determinant(const mat4& matrix);

  ///
  /// Inverts a matrix.
  /// \param matrix The matrix.
  void invert(mat3& matrix);
  void invert(mat4& matrix);

  ///
  /// Transposes a matrix.
  /// \param matrix The matrix.
  void transpose(mat3& matrix);
  void transpose(mat4& matrix);

  ///
  /// Sets the scale of a transformation matrix.
  /// \param matrix The matrix.
  /// \param scale The scalar.
  void scale_abs(mat4& matrix, const vec3& scale);

  ///
  /// Scales a transformation matrix.
  /// \param matrix The matrix to scale.
  /// \param scale The scalar.
  void scale(mat4& matrix, const vec3& scale);

  ///
  /// Translates a transformation matrix.
  /// \param matrix The matrix to translate.
  /// \param translation The translation.
  void translate(mat4& matrix, const vec3& translation);
  void translate(mat4& matrix, const vec4& translation);

  ///
  /// Determines if all the corresponding values of two matrices are 'near' to each-other. This helps to determine
  /// equality of matrices which have been derived through different calculations and as a result of floating point
  /// inaccuracy are slightly different.
  /// \param a The first matrix.
  /// \param b The second matrix.
  /// \param epsilon The maximum acceptable difference.
  /// \return True if all the corresponding values of the matrices do not differ more than the epsilon, false otherwise.
  bool near(const mat3& a, const mat3& b, float epsilon = 0.0001f);
  bool near(const mat4& a, const mat4& b, float epsilon = 0.0001f);

  ///
  /// Creates a matrix representing an orthogonal projection. Orthogonal projections work well with 2D games.
  /// \param width The width of the camera frame.
  /// \param height The height of the camera frame.
  /// \param near_clipping_distance The distance to the near clipping plane.
  /// \param far_clipping_distance The distance to the far clipping plane.
  /// \return A matrix representing an orthogonal projection.
  mat4 orthogonal(float width, float height, float near_clipping_distance, float far_clipping_distance);

  ///
  /// Creates a matrix representing a perspective projection. Perspective projections work well with 3D games.
  /// \param y_axis_field_of_view The field of view on the Y axis.
  /// \param aspect_ratio The aspect ratio.
  /// \param near_clipping_distance The distance to the near clipping plane.
  /// \param far_clipping_distance The distance to the far clipping plane.
  /// \return A matrix representing a perspective projection.
  mat4 perspective(float y_axis_field_of_view, float aspect_ratio, float near_clipping_distance, float far_clipping_distance);
}

#endif /* LUDO_MATH_MAT_H_ */
