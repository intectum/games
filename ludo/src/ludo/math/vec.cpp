/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <cmath>

#include "util.h"
#include "vec.h"

namespace ludo
{
  vec2::vec2() : std::array<float, 2>()
  {}

  vec2::vec2(float x, float y) : std::array<float, 2> { x, y }
  {}

  vec3::vec3() : std::array<float, 3>()
  {}

  vec3::vec3(float x, float y, float z) : std::array<float, 3> { x, y, z }
  {}

  vec3::vec3(std::array<float, 4> vec4) : std::array<float, 3> { vec4[0], vec4[1], vec4[2] }
  {}

  vec4::vec4() : std::array<float, 4>()
  {}

  vec4::vec4(float x, float y, float z, float w) : std::array<float, 4> { x, y, z, w }
  {}

  vec4::vec4(std::array<float, 3> vec3, float w) : std::array<float, 4> { vec3[0], vec3[1], vec3[2], w }
  {}

  vec2 operator+(const vec2& lhs, const vec2& rhs)
  {
    auto sum = lhs;
    sum += rhs;
    return sum;
  }

  vec3 operator+(const vec3& lhs, const vec3& rhs)
  {
    auto sum = lhs;
    sum += rhs;
    return sum;
  }

  vec4 operator+(const vec4& lhs, const vec4& rhs)
  {
    auto sum = lhs;
    sum += rhs;
    return sum;
  }

  vec2& operator+=(vec2& lhs, const vec2& rhs)
  {
    lhs[0] += rhs[0];
    lhs[1] += rhs[1];

    return lhs;
  }

  vec3& operator+=(vec3& lhs, const vec3& rhs)
  {
    lhs[0] += rhs[0];
    lhs[1] += rhs[1];
    lhs[2] += rhs[2];

    return lhs;
  }

  vec4& operator+=(vec4& lhs, const vec4& rhs)
  {
    lhs[0] += rhs[0];
    lhs[1] += rhs[1];
    lhs[2] += rhs[2];
    lhs[3] += rhs[3];

    return lhs;
  }

  vec2 operator-(const vec2& lhs, const vec2& rhs)
  {
    auto sum = lhs;
    sum -= rhs;
    return sum;
  }

  vec3 operator-(const vec3& lhs, const vec3& rhs)
  {
    auto sum = lhs;
    sum -= rhs;
    return sum;
  }

  vec4 operator-(const vec4& lhs, const vec4& rhs)
  {
    auto sum = lhs;
    sum -= rhs;
    return sum;
  }

  vec2& operator-=(vec2& lhs, const vec2& rhs)
  {
    lhs[0] -= rhs[0];
    lhs[1] -= rhs[1];

    return lhs;
  }

  vec3& operator-=(vec3& lhs, const vec3& rhs)
  {
    lhs[0] -= rhs[0];
    lhs[1] -= rhs[1];
    lhs[2] -= rhs[2];

    return lhs;
  }

  vec4& operator-=(vec4& lhs, const vec4& rhs)
  {
    lhs[0] -= rhs[0];
    lhs[1] -= rhs[1];
    lhs[2] -= rhs[2];
    lhs[3] -= rhs[3];

    return lhs;
  }

  vec2 operator*(const vec2& lhs, const vec2& rhs)
  {
    auto product = lhs;
    product *= rhs;
    return product;
  }

  vec3 operator*(const vec3& lhs, const vec3& rhs)
  {
    auto product = lhs;
    product *= rhs;
    return product;
  }

  vec4 operator*(const vec4& lhs, const vec4& rhs)
  {
    auto product = lhs;
    product *= rhs;
    return product;
  }

  vec2& operator*=(vec2& lhs, const vec2& rhs)
  {
    lhs[0] *= rhs[0];
    lhs[1] *= rhs[1];

    return lhs;
  }

  vec3& operator*=(vec3& lhs, const vec3& rhs)
  {
    lhs[0] *= rhs[0];
    lhs[1] *= rhs[1];
    lhs[2] *= rhs[2];

    return lhs;
  }

  vec4& operator*=(vec4& lhs, const vec4& rhs)
  {
    lhs[0] *= rhs[0];
    lhs[1] *= rhs[1];
    lhs[2] *= rhs[2];
    lhs[3] *= rhs[3];

    return lhs;
  }

  vec2 operator*(const vec2& vector, float scalar)
  {
    auto product = vector;
    product *= scalar;
    return product;
  }

  vec3 operator*(const vec3& vector, float scalar)
  {
    auto product = vector;
    product *= scalar;
    return product;
  }

  vec4 operator*(const vec4& vector, float scalar)
  {
    auto product = vector;
    product *= scalar;
    return product;
  }

  vec2& operator*=(vec2& vector, float scalar)
  {
    vector[0] *= scalar;
    vector[1] *= scalar;

    return vector;
  }

  vec3& operator*=(vec3& vector, float scalar)
  {
    vector[0] *= scalar;
    vector[1] *= scalar;
    vector[2] *= scalar;

    return vector;
  }

  vec4& operator*=(vec4& vector, float scalar)
  {
    vector[0] *= scalar;
    vector[1] *= scalar;
    vector[2] *= scalar;
    vector[3] *= scalar;

    return vector;
  }

  vec2 operator*(float scalar, const vec2& vector)
  {
    return vector * scalar;
  }

  vec3 operator*(float scalar, const vec3& vector)
  {
    return vector * scalar;
  }

  vec4 operator*(float scalar, const vec4& vector)
  {
    return vector * scalar;
  }

  vec2 operator/(const vec2& lhs, const vec2& rhs)
  {
    auto product = lhs;
    product /= rhs;
    return product;
  }

  vec3 operator/(const vec3& lhs, const vec3& rhs)
  {
    auto product = lhs;
    product /= rhs;
    return product;
  }

  vec4 operator/(const vec4& lhs, const vec4& rhs)
  {
    auto product = lhs;
    product /= rhs;
    return product;
  }

  vec2& operator/=(vec2& lhs, const vec2& rhs)
  {
    lhs[0] /= rhs[0];
    lhs[1] /= rhs[1];

    return lhs;
  }

  vec3& operator/=(vec3& lhs, const vec3& rhs)
  {
    lhs[0] /= rhs[0];
    lhs[1] /= rhs[1];
    lhs[2] /= rhs[2];

    return lhs;
  }

  vec4& operator/=(vec4& lhs, const vec4& rhs)
  {
    lhs[0] /= rhs[0];
    lhs[1] /= rhs[1];
    lhs[2] /= rhs[2];
    lhs[3] /= rhs[3];

    return lhs;
  }

  vec2 operator/(const vec2& vector, float scalar)
  {
    auto product = vector;
    product /= scalar;
    return product;
  }

  vec3 operator/(const vec3& vector, float scalar)
  {
    auto product = vector;
    product /= scalar;
    return product;
  }

  vec4 operator/(const vec4& vector, float scalar)
  {
    auto product = vector;
    product /= scalar;
    return product;
  }

  vec2& operator/=(vec2& vector, float scalar)
  {
    vector[0] /= scalar;
    vector[1] /= scalar;

    return vector;
  }

  vec3& operator/=(vec3& vector, float scalar)
  {
    vector[0] /= scalar;
    vector[1] /= scalar;
    vector[2] /= scalar;

    return vector;
  }

  vec4& operator/=(vec4& vector, float scalar)
  {
    vector[0] /= scalar;
    vector[1] /= scalar;
    vector[2] /= scalar;
    vector[3] /= scalar;

    return vector;
  }

  std::ostream& operator<<(std::ostream& stream, const vec2& vector)
  {
    stream << "[" << vector[0] << "," << vector[1] << "]";

    return stream;
  }

  std::ostream& operator<<(std::ostream& stream, const vec3& vector)
  {
    stream << "[" << vector[0] << "," << vector[1] << "," << vector[2] << "]";

    return stream;
  }

  std::ostream& operator<<(std::ostream& stream, const vec4& vector)
  {
    stream << "[" << vector[0] << "," << vector[1] << "," << vector[2] << "," << vector[3] << "]";

    return stream;
  }

  float length(const vec2& vector)
  {
    return std::sqrt(length2(vector));
  }

  float length(const vec3& vector)
  {
    return std::sqrt(length2(vector));
  }

  float length(const vec4& vector)
  {
    return std::sqrt(length2(vector));
  }

  float length2(const vec2& vector)
  {
    return powf(vector[0], 2) + powf(vector[1], 2);
  }

  float length2(const vec3& vector)
  {
    return powf(vector[0], 2) + powf(vector[1], 2) + powf(vector[2], 2);
  }

  float length2(const vec4& vector)
  {
    return powf(vector[0] * vector[3], 2) + powf(vector[1] * vector[3], 2) + powf(vector[2] * vector[3], 2);
  }

  void normalize(vec2& vector)
  {
    auto length = ludo::length(vector);
    if (length == 0.0f)
    {
      return;
    }

    vector /= length;
  }

  void normalize(vec3& vector)
  {
    auto length = ludo::length(vector);
    if (length == 0.0f)
    {
      return;
    }

    vector /= length;
  }

  void normalize(vec4& vector)
  {
    auto length = ludo::length(vector);
    if (length == 0.0f)
    {
      return;
    }

    vector /= length;
  }

  void rotate(vec2& vector, float angle)
  {
    auto cosine = std::cos(angle);
    auto sine = std::sin(angle);

    vector =
    {
      vector[0] * cosine - vector[1] * sine,
      vector[0] * sine + vector[1] * cosine
    };
  }

  void homogenize(vec4& vector)
  {
    vector /= vector[3];
  }

  bool near(const vec2& a, const vec2& b, float epsilon)
  {
    return near(a[0], b[0], epsilon) && near(a[1], b[1], epsilon);
  }

  bool near(const vec3& a, const vec3& b, float epsilon)
  {
    return near(a[0], b[0], epsilon) && near(a[1], b[1], epsilon) && near(a[2], b[2], epsilon);
  }

  bool near(const vec4& a, const vec4& b, float epsilon)
  {
    return near(a[0] * a[3], b[0] * b[3], epsilon) && near(a[1] * a[3], b[1] * b[3], epsilon) && near(a[2] * a[3], b[2] * b[3], epsilon);
  }

  float cross(const vec2& lhs, const vec2& rhs)
  {
    return lhs[0] * rhs[1] - rhs[0] * lhs[1];
  }

  vec3 cross(const vec3& lhs, const vec3& rhs)
  {
    return
    {
      lhs[1] * rhs[2] - lhs[2] * rhs[1],
      lhs[2] * rhs[0] - lhs[0] * rhs[2],
      lhs[0] * rhs[1] - lhs[1] * rhs[0]
    };
  }

  float dot(const vec2& lhs, const vec2& rhs)
  {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1];
  }

  float dot(const vec3& lhs, const vec3& rhs)
  {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
  }

  float dot(const vec4& lhs, const vec4& rhs)
  {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3] * rhs[3];
  }

  vec2 project(const vec2& a, const vec2& b)
  {
    assert(near(length(b), 1.0f) && "'b' must be unit length");

    return dot(a, b) * b;
  }

  vec3 project(const vec3& a, const vec3& b)
  {
    assert(near(length(b), 1.0f) && "'b' must be unit length");

    return dot(a, b) * b;
  }

  float scalar_project(const vec2& a, const vec2& b)
  {
    assert(near(length(b), 1.0f) && "'b' must be unit length");

    return dot(a, b);
  }

  float scalar_project(const vec3& a, const vec3& b)
  {
    assert(near(length(b), 1.0f) && "'b' must be unit length");

    return dot(a, b);
  }

  float angle_between(const vec2& a, const vec2& b)
  {
    assert(near(length(a), 1.0f) && "'a' must be unit length");
    assert(near(length(b), 1.0f) && "'b' must be unit length");

    float dot = ludo::dot(a, b);
    // Clamp to the range [-1,1] in case floating point inaccuracy manages to put it out of that range...
    float clamped_dot = std::max(std::min(dot, 1.0f), -1.0f);
    return std::acos(clamped_dot);
  }

  float angle_between(const vec3& a, const vec3& b)
  {
    assert(near(length(a), 1.0f) && "'a' must be unit length");
    assert(near(length(b), 1.0f) && "'b' must be unit length");

    float dot = ludo::dot(a, b);
    // Clamp to the range [-1,1] in case floating point inaccuracy manages to put it out of that range...
    float clamped_dot = std::max(std::min(dot, 1.0f), -1.0f);
    return std::acos(clamped_dot);
  }
}
