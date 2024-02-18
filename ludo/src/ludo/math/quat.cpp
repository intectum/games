/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <cmath>

#include "quat.h"
#include "util.h"

namespace ludo
{
  quat exp(const quat& quaternion);
  quat log(const quat& quaternion);
  quat pow(const quat& quaternion, float exponent);

  quat::quat() : std::array<float, 4>()
  {}

  quat::quat(float x, float y, float z, float w) : std::array<float, 4> { x, y, z, w }
  {}

  quat::quat(float x, float y, float z) : std::array<float, 4>()
  {
    auto cosines = vec3
    {
      std::cos(x / 2.0f),
      std::cos(y / 2.0f),
      std::cos(z / 2.0f)
    };

    auto sines = vec3
    {
      std::sin(x / 2.0f),
      std::sin(y / 2.0f),
      std::sin(z / 2.0f)
    };

    *this =
    {
      sines[0] * cosines[1] * cosines[2] - cosines[0] * sines[1] * sines[2],
      cosines[0] * sines[1] * cosines[2] + sines[0] * cosines[1] * sines[2],
      cosines[0] * cosines[1] * sines[2] - sines[0] * sines[1] * cosines[2],
      cosines[0] * cosines[1] * cosines[2] + sines[0] * sines[1] * sines[2]
    };
  }

  quat::quat(const vec3& axis, float angle) : std::array<float, 4>()
  {
    auto half_angle = angle / 2.0f;
    auto scalar = std::sin(half_angle);

    *this = quat
    {
      axis[0] * scalar,
      axis[1] * scalar,
      axis[2] * scalar,
      std::cos(half_angle)
    };
  }

  quat::quat(std::array<float, 9> rotation) : std::array<float, 4>()
  {
    float trace = rotation[0] + rotation[4] + rotation[8];

    if (trace > 0)
    {
      auto w = std::sqrt(1.0f + trace) / 2.0f;
      auto w4 = w * 4.0f;

      *this =
      {
        (rotation[5] - rotation[7]) / w4,
        (rotation[6] - rotation[2]) / w4,
        (rotation[1] - rotation[3]) / w4,
        w
      };
    }
    else if (rotation[0] > rotation[4] && rotation[0] > rotation[8])
    {
      auto x = std::sqrt(1.0f + rotation[0] - rotation[4] - rotation[8]) / 2.0f;
      auto x4 = x * 4.0f;

      *this =
      {
        x,
        (rotation[3] + rotation[1]) / x4,
        (rotation[6] + rotation[2]) / x4,
        (rotation[5] - rotation[7]) / x4,
      };
    }
    else if (rotation[4] > rotation[8])
    {
      auto y = std::sqrt(1.0f + rotation[4] - rotation[0] - rotation[8]) / 2.0f;
      auto y4 = y * 4.0f;

      *this =
      {
        (rotation[3] + rotation[1]) / y4,
        y,
        (rotation[7] + rotation[5]) / y4,
        (rotation[6] - rotation[2]) / y4,
      };
    }
    else
    {
      auto z = std::sqrt(1.0f + rotation[8] - rotation[0] - rotation[4]) / 2.0f;
      auto z4 = z * 4.0f;

      *this =
      {
        (rotation[6] + rotation[2]) / z4,
        (rotation[7] + rotation[5]) / z4,
        z,
        (rotation[1] - rotation[3]) / z4,
      };
    }
  }

  quat::quat(const vec3& from, const vec3& to) : std::array<float, 4>()
  {
    assert(near(length(from), 1.0f) && "'from' must be unit length");
    assert(near(length(to), 1.0f) && "'to' must be unit length");

    auto dot = ludo::dot(from, to);

    if (dot > 0.9999f)
    {
      *this = quat_identity;
      return;
    }

    if (dot < -0.9999f)
    {
      // 180 degrees around the x-axis
      *this = quat { 1.0f, 0.0f, 0.f, 0.0f };
      return;
    }

    auto cross = ludo::cross(from, to);

    *this =
    {
      cross[0],
      cross[1],
      cross[2],
      1.0f + dot
    };

    normalize(*this);
  }

  quat::quat(const quat& from, const quat& to) : std::array<float, 4>()
  {
    auto from_inverse = from;
    invert(from_inverse);

    *this = from_inverse * to;
  }

  quat operator*(const quat& lhs, const quat& rhs)
  {
    auto product = lhs;
    product *= rhs;
    return product;
  }

  quat& operator*=(quat& lhs, const quat& rhs)
  {
    lhs =
    {
      lhs[3] * rhs[0] + lhs[0] * rhs[3] + lhs[1] * rhs[2] - lhs[2] * rhs[1],
      lhs[3] * rhs[1] - lhs[0] * rhs[2] + lhs[1] * rhs[3] + lhs[2] * rhs[0],
      lhs[3] * rhs[2] + lhs[0] * rhs[1] - lhs[1] * rhs[0] + lhs[2] * rhs[3],
      lhs[3] * rhs[3] - lhs[0] * rhs[0] - lhs[1] * rhs[1] - lhs[2] * rhs[2]
    };

    return lhs;
  }

  quat operator*(const quat& quaternion, float scalar)
  {
    auto product = quaternion;
    product *= scalar;
    return product;
  }

  quat& operator*=(quat& quaternion, float scalar)
  {
    quaternion[0] *= scalar;
    quaternion[1] *= scalar;
    quaternion[2] *= scalar;
    quaternion[3] *= scalar;

    return quaternion;
  }

  quat operator*(float scalar, const quat& quaternion)
  {
    return quaternion * scalar;
  }

  quat operator/(const quat& quaternion, float scalar)
  {
    auto product = quaternion;
    product /= scalar;
    return product;
  }

  quat& operator/=(quat& quaternion, float scalar)
  {
    quaternion[0] /= scalar;
    quaternion[1] /= scalar;
    quaternion[2] /= scalar;
    quaternion[3] /= scalar;

    return quaternion;
  }

  std::ostream& operator<<(std::ostream& stream, const quat& quaternion)
  {
    stream << "[" << quaternion[0] << "," << quaternion[1] << "," << quaternion[2] << "," << quaternion[3] << "]";

    return stream;
  }

  float length(const quat& quaternion)
  {
    return std::sqrt(powf(quaternion[0], 2) + powf(quaternion[1], 2) + powf(quaternion[2], 2) + powf(quaternion[3], 2));
  }

  vec3 angles(const quat& quaternion)
  {
    auto test = quaternion[0] * quaternion[1] + quaternion[2] * quaternion[3];

    // North Pole
    if (test > 0.4999)
    {
      return vec3
      {
        0.0f,
        pi / 2.0f,
        2.0f * std::atan2(quaternion[0], quaternion[3])
      };
    }

    // South Pole
    if (test < -0.4999)
    {
      return vec3
      {
        0.0f,
        -pi / 2.0f,
        -2.0f * std::atan2(quaternion[0], quaternion[3])
      };
    }

    return vec3
    {
      std::atan2(2.0f * quaternion[0] * quaternion[3] - 2 * quaternion[1] * quaternion[2] , 1.0f - 2.0f * powf(quaternion[0], 2) - 2.0f * powf(quaternion[2], 2)),
      std::asin(2.0f * test),
      std::atan2(2.0f * quaternion[1] * quaternion[3] - 2 * quaternion[0] * quaternion[2] , 1.0f - 2.0f * powf(quaternion[1], 2) - 2.0f * powf(quaternion[2], 2))
    };
  }

  std::pair<vec3, float> axis_angle(const quat& quaternion)
  {
    auto local_quaternion = quaternion;
    if (local_quaternion[3] < -1.0f || local_quaternion[3] > 1.0f) // TODO is less than -1 a real thing that should be possible? Just want to double check that... I managed to get it but might be there is something going wrong...
    {
      normalize(local_quaternion);
    }

    auto scalar = std::sqrt(1.0f - local_quaternion[3] * local_quaternion[3]);

    // If the scalar is close to zero then the direction of the axis is not important.
    auto axis = vec3_unit_x;
    if (!near(scalar, 0.0f))
    {
      axis = vec3
      {
        local_quaternion[0] / scalar,
        local_quaternion[1] / scalar,
        local_quaternion[2] / scalar
      };
    }

    auto angle = std::acos(local_quaternion[3]) * 2.0f;

    return { axis, angle };
  }

  void normalize(quat& quaternion)
  {
    auto length = ludo::length(quaternion);
    if (length == 0.0f)
    {
      return;
    }

    quaternion /= length;
  }

  void invert(quat& quaternion)
  {
    auto length = ludo::length(quaternion);
    if (length == 0)
    {
      quaternion = { 0.0f, 0.0f, 0.0f, 0.0f };
      return;
    }

    length *= length;

    quaternion = { -quaternion[0] / length, -quaternion[1] / length, -quaternion[2] / length, quaternion[3] / length };
  }

  bool near(const quat& a, const quat& b, float epsilon)
  {
    return near(a[0], b[0], epsilon) && near(a[1], b[1], epsilon) && near(a[2], b[2], epsilon) && near(a[3], b[3], epsilon);
  }

  float dot(const quat& lhs, const quat& rhs)
  {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3] * rhs[3];
  }

  quat slerp(const quat& from, const quat& to, float time)
  {
    // Clamp to the range [0,1]
    time = std::max(std::min(time, 1.0f), 0.0f);

    auto from_inverse = from;
    invert(from_inverse);

    auto to_final = to;
    if (dot(from, to) < 0.0f)
    {
      to_final *= -1.0f;
    }

    return from * pow(from_inverse * to_final, time);
  }

  quat exp(const quat& quaternion)
  {
    auto vector = vec3 { quaternion[0], quaternion[1], quaternion[2] };
    auto vector_length = length(vector);
    normalize(vector);

    auto sin_vector_length = std::sin(vector_length);
    auto exp_w = std::exp(quaternion[3]);

    return
    {
      vector[0] * sin_vector_length * exp_w,
      vector[1] * sin_vector_length * exp_w,
      vector[2] * sin_vector_length * exp_w,
      std::cos(vector_length) * exp_w
    };
  }

  quat log(const quat& quaternion)
  {
    auto vector = vec3 { quaternion[0], quaternion[1], quaternion[2] };
    normalize(vector);

    auto length = ludo::length(quaternion);
    auto scalar = std::acos(quaternion[3] / length);

    return
    {
      vector[0] * scalar,
      vector[1] * scalar,
      vector[2] * scalar,
      std::log(length)
    };
  }

  quat pow(const quat& quaternion, float exponent)
  {
    return exp(log(quaternion) * exponent);
  }
}
