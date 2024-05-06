/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "mat.h"
#include "util.h"

namespace ludo
{
  mat3::mat3() : std::array<float, 9>()
  {}

  mat3::mat3(float m00, float m01, float m02,
             float m10, float m11, float m12,
             float m20, float m21, float m22) : std::array<float, 9>
    // The matrix is column-major so this is visually transposed as shown here
    {
      m00, m01, m02,
      m10, m11, m12,
      m20, m21, m22
    }
  {}

  // TODO can this have a consistent rotation order with quat please?
  mat3::mat3(float x, float y, float z) : std::array<float, 9>()
  {
    auto cosines = vec3
    {
      std::cos(x),
      std::cos(y),
      std::cos(z)
    };

    auto sines = vec3
    {
      std::sin(x),
      std::sin(y),
      std::sin(z)
    };

    *this =
    {
      cosines[1] * cosines[2], cosines[0] * sines[2] + cosines[2] * sines[0] * sines[1], sines[0] * sines[2] - cosines[0] * cosines[2] * sines[1],
      -cosines[1] * sines[2], cosines[0] * cosines[2] - sines[0] * sines[1] * sines[2], cosines[2] * sines[0] + cosines[0] * sines[1] * sines[2],
      sines[1], -cosines[1] * sines[0], cosines[0] * cosines[1]
    };
  }

  mat3::mat3(const vec3& axis, float angle) : std::array<float, 9>()
  {
    auto cosine = cosf(angle);
    auto sine = sinf(angle);
    auto one_minus_cosine = 1 - cosine;

    auto sine_x = sine * axis[0];
    auto sine_y = sine * axis[1];
    auto sine_z = sine * axis[2];

    auto xy = axis[0] * axis[1];
    auto xz = axis[0] * axis[2];
    auto yz = axis[1] * axis[2];

    *this =
    {
      axis[0] * axis[0] * one_minus_cosine + cosine, xy * one_minus_cosine + sine_z, xz * one_minus_cosine - sine_y,
      xy * one_minus_cosine - sine_z, axis[1] * axis[1] * one_minus_cosine + cosine, yz * one_minus_cosine + sine_x,
      xz * one_minus_cosine + sine_y, yz * one_minus_cosine - sine_x, axis[2] * axis[2] * one_minus_cosine + cosine
    };
  }

  mat3::mat3(std::array<float, 4> quaternion) : std::array<float, 9>()
  {
    auto xx = quaternion[0] * quaternion[0];
    auto xy = quaternion[0] * quaternion[1];
    auto xz = quaternion[0] * quaternion[2];
    auto xw = quaternion[0] * quaternion[3];

    auto yy = quaternion[1] * quaternion[1];
    auto yz = quaternion[1] * quaternion[2];
    auto yw = quaternion[1] * quaternion[3];

    auto zz = quaternion[2] * quaternion[2];
    auto zw = quaternion[2] * quaternion[3];

    // The matrix is column-major so this is visually transposed as shown here
    *this = mat3
    {
      1.0f - 2.0f * (yy + zz), 2.0f * (xy + zw), 2.0f * (xz - yw),
      2.0f * (xy - zw), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + xw),
      2.0f * (xz + yw), 2.0f * (yz - xw), 1.0f - 2.0f * (xx + yy)
    };
  }

  mat3::mat3(std::array<float, 16> transformation) : std::array<float, 9>
    // The matrix is column-major so this is visually transposed as shown here
    {
      transformation[0], transformation[1], transformation[2], // transformation[3]
      transformation[4], transformation[5], transformation[6], // transformation[7]
      transformation[8], transformation[9], transformation[10] // transformation[11]
      // transformation[12], transformation[13], transformation[14], transformation[15]
    }
  {}

  mat3::mat3(const vec3& from, const vec3& to) : std::array<float, 9>()
  {
    // Formula taken from https://www.theochem.ru.nl/%7Epwormer/Knowino/knowino.org/wiki/Rotation_matrix.html#Vector_rotation
    // TODO handle the case where from and to are anti-parallel!!!

    auto u = cross(from, to);
    auto c = dot(from, to);
    auto h = (1 - c) / (1 - c * c);

    // The matrix is column-major so this is visually transposed as shown here
    *this = mat3
    {
      c + h * powf(u[0], 2), h * u[0] * u[1] + u[2], h * u[0] * u[2] - u[1],
      h * u[0] * u[1] - u[2], c + h * powf(u[1], 2), h * u[1] * u[2] + u[0],
      h * u[0] * u[2] + u[1], h * u[1] * u[2] - u[0], c + h * powf(u[2], 2)
    };
  }

  mat4::mat4() : std::array<float, 16>()
  {}

  mat4::mat4(float m00, float m01, float m02, float m03,
             float m10, float m11, float m12, float m13,
             float m20, float m21, float m22, float m23,
             float m30, float m31, float m32, float m33) : std::array<float, 16>
    // The matrix is column-major so this is visually transposed as shown here
    {
      m00, m01, m02, m03,
      m10, m11, m12, m13,
      m20, m21, m22, m23,
      m30, m31, m32, m33
    }
  {}

  mat4::mat4(std::array<float, 3> position, std::array<float, 9> rotation) : std::array<float, 16>
    // The matrix is column-major so this is visually transposed as shown here
  {
    rotation[0], rotation[1], rotation[2], 0.0f,
    rotation[3], rotation[4], rotation[5], 0.0f,
    rotation[6], rotation[7], rotation[8], 0.0f,
    position[0], position[1], position[2], 1.0f
  }
  {}

  mat3 operator+(const mat3& lhs, const mat3& rhs)
  {
    auto sum = lhs;
    sum += rhs;
    return sum;
  }

  mat4 operator+(const mat4& lhs, const mat4& rhs)
  {
    auto sum = lhs;
    sum += rhs;
    return sum;
  }

  mat3& operator+=(mat3& lhs, const mat3& rhs)
  {
    lhs[0] += rhs[0];
    lhs[1] += rhs[1];
    lhs[2] += rhs[2];

    lhs[3] += rhs[3];
    lhs[4] += rhs[4];
    lhs[5] += rhs[5];

    lhs[6] += rhs[6];
    lhs[7] += rhs[7];
    lhs[8] += rhs[8];

    return lhs;
  }

  mat4& operator+=(mat4& lhs, const mat4& rhs)
  {
    lhs[0] += rhs[0];
    lhs[1] += rhs[1];
    lhs[2] += rhs[2];
    lhs[3] += rhs[3];

    lhs[4] += rhs[4];
    lhs[5] += rhs[5];
    lhs[6] += rhs[6];
    lhs[7] += rhs[7];

    lhs[8] += rhs[8];
    lhs[9] += rhs[9];
    lhs[10] += rhs[10];
    lhs[11] += rhs[11];

    lhs[12] += rhs[12];
    lhs[13] += rhs[13];
    lhs[14] += rhs[14];
    lhs[15] += rhs[15];

    return lhs;
  }

  mat3 operator-(const mat3& lhs, const mat3& rhs)
  {
    auto sum = lhs;
    sum -= rhs;
    return sum;
  }

  mat4 operator-(const mat4& lhs, const mat4& rhs)
  {
    auto sum = lhs;
    sum -= rhs;
    return sum;
  }

  mat3& operator-=(mat3& lhs, const mat3& rhs)
  {
    lhs[0] -= rhs[0];
    lhs[1] -= rhs[1];
    lhs[2] -= rhs[2];

    lhs[3] -= rhs[3];
    lhs[4] -= rhs[4];
    lhs[5] -= rhs[5];

    lhs[6] -= rhs[6];
    lhs[7] -= rhs[7];
    lhs[8] -= rhs[8];

    return lhs;
  }

  mat4& operator-=(mat4& lhs, const mat4& rhs)
  {
    lhs[0] -= rhs[0];
    lhs[1] -= rhs[1];
    lhs[2] -= rhs[2];
    lhs[3] -= rhs[3];

    lhs[4] -= rhs[4];
    lhs[5] -= rhs[5];
    lhs[6] -= rhs[6];
    lhs[7] -= rhs[7];

    lhs[8] -= rhs[8];
    lhs[9] -= rhs[9];
    lhs[10] -= rhs[10];
    lhs[11] -= rhs[11];

    lhs[12] -= rhs[12];
    lhs[13] -= rhs[13];
    lhs[14] -= rhs[14];
    lhs[15] -= rhs[15];

    return lhs;
  }

  mat3 operator*(const mat3& lhs, const mat3& rhs)
  {
    auto product = lhs;
    product *= rhs;
    return product;
  }

  mat4 operator*(const mat4& lhs, const mat4& rhs)
  {
    auto product = lhs;
    product *= rhs;
    return product;
  }

  mat3& operator*=(mat3& lhs, const mat3& rhs)
  {
    lhs =
    {
      lhs[0] * rhs[0] + lhs[3] * rhs[1] + lhs[6] * rhs[2],
      lhs[1] * rhs[0] + lhs[4] * rhs[1] + lhs[7] * rhs[2],
      lhs[2] * rhs[0] + lhs[5] * rhs[1] + lhs[8] * rhs[2],

      lhs[0] * rhs[3] + lhs[3] * rhs[4] + lhs[6] * rhs[5],
      lhs[1] * rhs[3] + lhs[4] * rhs[4] + lhs[7] * rhs[5],
      lhs[2] * rhs[3] + lhs[5] * rhs[4] + lhs[8] * rhs[5],

      lhs[0] * rhs[6] + lhs[3] * rhs[7] + lhs[6] * rhs[8],
      lhs[1] * rhs[6] + lhs[4] * rhs[7] + lhs[7] * rhs[8],
      lhs[2] * rhs[6] + lhs[5] * rhs[7] + lhs[8] * rhs[8]
    };

    return lhs;
  }

  mat4& operator*=(mat4& lhs, const mat4& rhs)
  {
    lhs =
    {
      lhs[0] * rhs[0] + lhs[4] * rhs[1] + lhs[8] * rhs[2] + lhs[12] * rhs[3],
      lhs[1] * rhs[0] + lhs[5] * rhs[1] + lhs[9] * rhs[2] + lhs[13] * rhs[3],
      lhs[2] * rhs[0] + lhs[6] * rhs[1] + lhs[10] * rhs[2] + lhs[14] * rhs[3],
      lhs[3] * rhs[0] + lhs[7] * rhs[1] + lhs[11] * rhs[2] + lhs[15] * rhs[3],

      lhs[0] * rhs[4] + lhs[4] * rhs[5] + lhs[8] * rhs[6] + lhs[12] * rhs[7],
      lhs[1] * rhs[4] + lhs[5] * rhs[5] + lhs[9] * rhs[6] + lhs[13] * rhs[7],
      lhs[2] * rhs[4] + lhs[6] * rhs[5] + lhs[10] * rhs[6] + lhs[14] * rhs[7],
      lhs[3] * rhs[4] + lhs[7] * rhs[5] + lhs[11] * rhs[6] + lhs[15] * rhs[7],

      lhs[0] * rhs[8] + lhs[4] * rhs[9] + lhs[8] * rhs[10] + lhs[12] * rhs[11],
      lhs[1] * rhs[8] + lhs[5] * rhs[9] + lhs[9] * rhs[10] + lhs[13] * rhs[11],
      lhs[2] * rhs[8] + lhs[6] * rhs[9] + lhs[10] * rhs[10] + lhs[14] * rhs[11],
      lhs[3] * rhs[8] + lhs[7] * rhs[9] + lhs[11] * rhs[10] + lhs[15] * rhs[11],

      lhs[0] * rhs[12] + lhs[4] * rhs[13] + lhs[8] * rhs[14] + lhs[12] * rhs[15],
      lhs[1] * rhs[12] + lhs[5] * rhs[13] + lhs[9] * rhs[14] + lhs[13] * rhs[15],
      lhs[2] * rhs[12] + lhs[6] * rhs[13] + lhs[10] * rhs[14] + lhs[14] * rhs[15],
      lhs[3] * rhs[12] + lhs[7] * rhs[13] + lhs[11] * rhs[14] + lhs[15] * rhs[15]
    };

    return lhs;
  }

  vec3 operator*(const mat3& matrix, const vec3& vector)
  {
    return
    {
      matrix[0] * vector[0] + matrix[3] * vector[1] + matrix[6] * vector[2],
      matrix[1] * vector[0] + matrix[4] * vector[1] + matrix[7] * vector[2],
      matrix[2] * vector[0] + matrix[5] * vector[1] + matrix[8] * vector[2]
    };
  }

  vec4 operator*(const mat4& matrix, const vec4& vector)
  {
    return
    {
      matrix[0] * vector[0] + matrix[4] * vector[1] + matrix[8] * vector[2] + matrix[12] * vector[3],
      matrix[1] * vector[0] + matrix[5] * vector[1] + matrix[9] * vector[2] + matrix[13] * vector[3],
      matrix[2] * vector[0] + matrix[6] * vector[1] + matrix[10] * vector[2] + matrix[14] * vector[3],
      matrix[3] * vector[0] + matrix[7] * vector[1] + matrix[11] * vector[2] + matrix[15] * vector[3]
    };
  }

  vec3 operator*(const vec3& vector, const mat3& matrix)
  {
    return
    {
      vector[0] * matrix[0] + vector[1] * matrix[1] + vector[2] * matrix[2],
      vector[0] * matrix[3] + vector[1] * matrix[4] + vector[2] * matrix[5],
      vector[0] * matrix[6] + vector[1] * matrix[7] + vector[2] * matrix[8]
    };
  }

  vec4 operator*(const vec4& vector, const mat4& matrix)
  {
    return
    {
      vector[0] * matrix[0] + vector[1] * matrix[1] + vector[2] * matrix[2] + vector[3] * matrix[3],
      vector[0] * matrix[4] + vector[1] * matrix[5] + vector[2] * matrix[6] + vector[3] * matrix[7],
      vector[0] * matrix[8] + vector[1] * matrix[9] + vector[2] * matrix[10] + vector[3] * matrix[11],
      vector[0] * matrix[12] + vector[1] * matrix[13] + vector[2] * matrix[14] + vector[3] * matrix[15]
    };
  }

  mat3 operator*(const mat3& matrix, float scalar)
  {
    auto product = matrix;
    product *= scalar;
    return product;
  }

  mat4 operator*(const mat4& matrix, float scalar)
  {
    auto product = matrix;
    product *= scalar;
    return product;
  }

  mat3& operator*=(mat3& matrix, float scalar)
  {
    matrix[0] *= scalar;
    matrix[1] *= scalar;
    matrix[2] *= scalar;

    matrix[3] *= scalar;
    matrix[4] *= scalar;
    matrix[5] *= scalar;

    matrix[6] *= scalar;
    matrix[7] *= scalar;
    matrix[8] *= scalar;

    return matrix;
  }

  mat4& operator*=(mat4& matrix, float scalar)
  {
    matrix[0] *= scalar;
    matrix[1] *= scalar;
    matrix[2] *= scalar;
    matrix[3] *= scalar;

    matrix[4] *= scalar;
    matrix[5] *= scalar;
    matrix[6] *= scalar;
    matrix[7] *= scalar;

    matrix[8] *= scalar;
    matrix[9] *= scalar;
    matrix[10] *= scalar;
    matrix[11] *= scalar;

    matrix[12] *= scalar;
    matrix[13] *= scalar;
    matrix[14] *= scalar;
    matrix[15] *= scalar;

    return matrix;
  }

  mat3 operator*(float scalar, const mat3& matrix)
  {
    return matrix * scalar;
  }

  mat4 operator*(float scalar, const mat4& matrix)
  {
    return matrix * scalar;
  }

  std::ostream& operator<<(std::ostream& stream, const mat3& matrix)
  {
    stream << "[" << matrix[0] << "," << matrix[3] << "," << matrix[6] << "]\n";
    stream << "[" << matrix[1] << "," << matrix[4] << "," << matrix[7] << "]\n";
    stream << "[" << matrix[2] << "," << matrix[5] << "," << matrix[8] << "]";

    return stream;
  }

  std::ostream& operator<<(std::ostream& stream, const mat4& matrix)
  {
    stream << "[" << matrix[0] << "," << matrix[4] << "," << matrix[8] << "," << matrix[12] << "]\n";
    stream << "[" << matrix[1] << "," << matrix[5] << "," << matrix[9] << "," << matrix[13] << "]\n";
    stream << "[" << matrix[2] << "," << matrix[6] << "," << matrix[10] << "," << matrix[14] << "]\n";
    stream << "[" << matrix[3] << "," << matrix[7] << "," << matrix[11] << "," << matrix[15] << "]";

    return stream;
  }

  vec3 position(const mat4& matrix)
  {
    return { matrix[12], matrix[13], matrix[14] };
  }

  vec4 position4(const mat4& matrix)
  {
    return { matrix[12], matrix[13], matrix[14], matrix[15] };
  }

  void position(mat4& matrix, const vec3& position)
  {
    matrix[12] = position[0];
    matrix[13] = position[1];
    matrix[14] = position[2];
    matrix[15] = 1.0f;
  }

  void position(mat4& matrix, const vec4& position)
  {
    matrix[12] = position[0];
    matrix[13] = position[1];
    matrix[14] = position[2];
    matrix[15] = position[3];
  }

  vec3 right(const mat3& matrix)
  {
    return { matrix[0], matrix[1], matrix[2] };
  }

  vec3 right(const mat4& matrix)
  {
    return { matrix[0], matrix[1], matrix[2] };
  }

  vec4 right4(const mat4& matrix)
  {
    return { matrix[0], matrix[1], matrix[2], matrix[3] };
  }

  vec3 up(const mat3& matrix)
  {
    return { matrix[3], matrix[4], matrix[5] };
  }

  vec3 up(const mat4& matrix)
  {
    return { matrix[4], matrix[5], matrix[6] };
  }

  vec4 up4(const mat4& matrix)
  {
    return { matrix[4], matrix[5], matrix[6], matrix[7] };
  }

  vec3 out(const mat3& matrix)
  {
    return { matrix[6], matrix[7], matrix[8] };
  }

  vec3 out(const mat4& matrix)
  {
    return { matrix[8], matrix[9], matrix[10] };
  }

  vec4 out4(const mat4& matrix)
  {
    return { matrix[8], matrix[9], matrix[10], matrix[11] };
  }

  vec3 angles(const mat3& matrix)
  {
    return
    {
      std::atan2(matrix[5], matrix[8]),
      std::atan2(-matrix[2], std::sqrt(powf(matrix[5], 2) + powf(matrix[8], 2))),
      std::atan2(matrix[1], matrix[0])
    };
  }

  vec3 angles(const mat4& matrix)
  {
    return
    {
      std::atan2(matrix[6], matrix[10]),
      std::atan2(-matrix[2], std::sqrt(powf(matrix[6], 2) + powf(matrix[10], 2))),
      std::atan2(matrix[1], matrix[0])
    };
  }

  float determinant(const mat3& matrix)
  {
    return matrix[0] * (matrix[4] * matrix[8] - matrix[7] * matrix[5]) - matrix[1] * (matrix[3] * matrix[8] - matrix[6] * matrix[5]) + matrix[2] * (matrix[3] * matrix[7] - matrix[6] * matrix[4]);
  }

  float determinant(const mat4& matrix)
  {
    auto determinant = matrix[0] *
    (
      (matrix[5] * matrix[10] * matrix[15] + matrix[6] * matrix[11] * matrix[13] + matrix[7] * matrix[9] * matrix[14]) -
      matrix[7] * matrix[10] * matrix[13] - matrix[5] * matrix[11] * matrix[14] - matrix[6] * matrix[9] * matrix[15]
    );

    determinant -= matrix[1] *
    (
      (matrix[4] * matrix[10] * matrix[15] + matrix[6] * matrix[11] * matrix[12] + matrix[7] * matrix[8] * matrix[14]) -
      matrix[7] * matrix[10] * matrix[12] - matrix[4] * matrix[11] * matrix[14] - matrix[6] * matrix[8] * matrix[15]
    );

    determinant += matrix[2] *
    (
      (matrix[4] * matrix[9] * matrix[15] + matrix[5] * matrix[11] * matrix[12] + matrix[7] * matrix[8] * matrix[13]) -
      matrix[7] * matrix[9] * matrix[12] - matrix[4] * matrix[11] * matrix[13] - matrix[5] * matrix[8] * matrix[15]
    );

    determinant -= matrix[3] *
    (
      (matrix[4] * matrix[9] * matrix[14] + matrix[5] * matrix[10] * matrix[12] + matrix[6] * matrix[8] * matrix[13]) -
      matrix[6] * matrix[9] * matrix[12] - matrix[4] * matrix[10] * matrix[13] - matrix[5] * matrix[8] * matrix[14]
    );

    return determinant;
  }

  void invert(mat3& matrix)
  {
    auto determinant = ludo::determinant(matrix);
    auto determinant_inverse = 1.0f / determinant; // TODO what if the determinant is zero?

    matrix =
    {
      -(matrix[4] * matrix[8] - matrix[7] * matrix[5]),
      (matrix[3] * matrix[8] - matrix[5] * matrix[6]),
      -(matrix[3] * matrix[7] - matrix[6] * matrix[4]),

      (matrix[1] * matrix[8] - matrix[2] * matrix[7]),
      -(matrix[0] * matrix[8] - matrix[2] * matrix[6]),
      (matrix[0] * matrix[7] - matrix[6] * matrix[1]),

      -(matrix[1] * matrix[5] - matrix[2] * matrix[4]),
      (matrix[0] * matrix[5] - matrix[3] * matrix[2]),
      -(matrix[0] * matrix[4] - matrix[3] * matrix[1])
    };

    transpose(matrix);
    matrix *= determinant_inverse;
  }

  void invert(mat4& matrix)
  {
    auto determinant = ludo::determinant(matrix);
    auto determinant_inverse = 1.0f / determinant; // TODO what if the determinant is zero?

    matrix =
    {
      ludo::determinant({ matrix[5], matrix[6], matrix[7], matrix[9], matrix[10], matrix[11], matrix[13], matrix[14], matrix[15] }),
      -ludo::determinant({ matrix[4], matrix[6], matrix[7], matrix[8], matrix[10], matrix[11], matrix[12], matrix[14], matrix[15] }),
      ludo::determinant({ matrix[4], matrix[5], matrix[7], matrix[8], matrix[9], matrix[11], matrix[12], matrix[13], matrix[15] }),
      -ludo::determinant({ matrix[4], matrix[5], matrix[6], matrix[8], matrix[9], matrix[10], matrix[12], matrix[13], matrix[14] }),

      -ludo::determinant({ matrix[1], matrix[2], matrix[3], matrix[9], matrix[10], matrix[11], matrix[13], matrix[14], matrix[15] }),
      ludo::determinant({ matrix[0], matrix[2], matrix[3], matrix[8], matrix[10], matrix[11], matrix[12], matrix[14], matrix[15] }),
      -ludo::determinant({ matrix[0], matrix[1], matrix[3], matrix[8], matrix[9], matrix[11], matrix[12], matrix[13], matrix[15] }),
      ludo::determinant({ matrix[0], matrix[1], matrix[2], matrix[8], matrix[9], matrix[10], matrix[12], matrix[13], matrix[14] }),

      ludo::determinant({ matrix[1], matrix[2], matrix[3], matrix[5], matrix[6], matrix[7], matrix[13], matrix[14], matrix[15] }),
      -ludo::determinant({ matrix[0], matrix[2], matrix[3], matrix[4], matrix[6], matrix[7], matrix[12], matrix[14], matrix[15] }),
      ludo::determinant({ matrix[0], matrix[1], matrix[3], matrix[4], matrix[5], matrix[7], matrix[12], matrix[13], matrix[15] }),
      -ludo::determinant({ matrix[0], matrix[1], matrix[2], matrix[4], matrix[5], matrix[6], matrix[12], matrix[13], matrix[14] }),

      -ludo::determinant({ matrix[1], matrix[2], matrix[3], matrix[5], matrix[6], matrix[7], matrix[9], matrix[10], matrix[11] }),
      ludo::determinant({ matrix[0], matrix[2], matrix[3], matrix[4], matrix[6], matrix[7], matrix[8], matrix[10], matrix[11] }),
      -ludo::determinant({ matrix[0], matrix[1], matrix[3], matrix[4], matrix[5], matrix[7], matrix[8], matrix[9], matrix[11] }),
      ludo::determinant({ matrix[0], matrix[1], matrix[2], matrix[4], matrix[5], matrix[6], matrix[8], matrix[9], matrix[10] })
    };

    transpose(matrix);
    matrix *= determinant_inverse;
  }

  void transpose(mat3& matrix)
  {
    float temp;

    temp = matrix[1];
    matrix[1] = matrix[3];
    matrix[3] = temp;

    temp = matrix[2];
    matrix[2] = matrix[6];
    matrix[6] = temp;

    temp = matrix[5];
    matrix[5] = matrix[7];
    matrix[7] = temp;
  }

  void transpose(mat4& matrix)
  {
    float temp;

    temp = matrix[1];
    matrix[1] = matrix[4];
    matrix[4] = temp;

    temp = matrix[2];
    matrix[2] = matrix[8];
    matrix[8] = temp;

    temp = matrix[3];
    matrix[3] = matrix[12];
    matrix[12] = temp;

    temp = matrix[6];
    matrix[6] = matrix[9];
    matrix[9] = temp;

    temp = matrix[7];
    matrix[7] = matrix[13];
    matrix[13] = temp;

    temp = matrix[11];
    matrix[11] = matrix[14];
    matrix[14] = temp;
  }

  void scale_abs(mat4& matrix, const vec3& scale)
  {
    matrix[0] = scale[0];
    matrix[5] = scale[1];
    matrix[10] = scale[2];
  }

  void scale(mat4& matrix, const vec3& scale)
  {
    matrix[0] *= scale[0];
    matrix[5] *= scale[1];
    matrix[10] *= scale[2];
  }

  void translate(mat4& matrix, const vec3& translation)
  {
    matrix[12] += matrix[0] * translation[0] + matrix[4] * translation[1] + matrix[8] * translation[2];
    matrix[13] += matrix[1] * translation[0] + matrix[5] * translation[1] + matrix[9] * translation[2];
    matrix[14] += matrix[2] * translation[0] + matrix[6] * translation[1] + matrix[10] * translation[2];
  }

  void translate(mat4& matrix, const vec4& translation)
  {
    matrix[12] += matrix[0] * translation[0] + matrix[4] * translation[1] + matrix[8] * translation[2];
    matrix[13] += matrix[1] * translation[0] + matrix[5] * translation[1] + matrix[9] * translation[2];
    matrix[14] += matrix[2] * translation[0] + matrix[6] * translation[1] + matrix[10] * translation[2];
    matrix[15] += matrix[3] * translation[0] + matrix[7] * translation[1] + matrix[11] * translation[2];
  }

  mat4 orthogonal(float width, float height, float near_clipping_distance, float far_clipping_distance)
  {
    // The matrix is column-major so this is visually transposed as shown here
    return
    {
      2.0f / width, 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / height, 0.0f, 0.0f,
      0.0f, 0.0f, -2.0f / (far_clipping_distance - near_clipping_distance), 0.0f,
      0.0f, 0.0f, (far_clipping_distance + near_clipping_distance) / (far_clipping_distance - near_clipping_distance), 1.0f
    };
  }

  mat4 perspective(float y_axis_field_of_view, float aspect_ratio, float near_clipping_distance, float far_clipping_distance)
  {
    auto half_edge_y = near_clipping_distance * tanf(y_axis_field_of_view * pi / 360.0f);
    auto half_edge_x = half_edge_y * aspect_ratio;

    auto frame_height = half_edge_y * 2.0f;
    auto frame_width = half_edge_x * 2.0f;

    auto depth = far_clipping_distance - near_clipping_distance;
    auto two_near_clipping_distance = 2.0f * near_clipping_distance;

    // The matrix is column-major so this is visually transposed as shown here
    return
    {
      two_near_clipping_distance / frame_width, 0.0f, 0.0f, 0.0f,
      0.0f, two_near_clipping_distance / frame_height, 0.0f, 0.0f,
      0.0f, 0.0f, (near_clipping_distance + far_clipping_distance) * -1.0f / depth, -1.0f,
      0.0f, 0.0f, -two_near_clipping_distance * far_clipping_distance / depth, 0.0f
    };
  }

  bool near(const mat3& a, const mat3& b, float epsilon)
  {
    return
      near(a[0], b[0], epsilon) && near(a[1], b[1], epsilon) && near(a[2], b[2], epsilon) &&
      near(a[3], b[3], epsilon) && near(a[4], b[4], epsilon) && near(a[5], b[5], epsilon) &&
      near(a[6], b[6], epsilon) && near(a[7], b[7], epsilon) && near(a[8], b[8], epsilon);
  }

  bool near(const mat4& a, const mat4& b, float epsilon)
  {
    return
      near(a[0], b[0], epsilon) && near(a[1], b[1], epsilon) && near(a[2], b[2], epsilon) && near(a[3], b[3], epsilon) &&
      near(a[4], b[4], epsilon) && near(a[5], b[5], epsilon) && near(a[6], b[6], epsilon) && near(a[7], b[7], epsilon) &&
      near(a[8], b[8], epsilon) && near(a[9], b[9], epsilon) && near(a[10], b[10], epsilon) && near(a[11], b[11], epsilon) &&
      near(a[12], b[12], epsilon) && near(a[13], b[13], epsilon) && near(a[14], b[14], epsilon) && near(a[15], b[15], epsilon);
  }
}
