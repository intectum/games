/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <sstream>

#include <ludo/math/mat.h>
#include <ludo/math/quat.h>
#include <ludo/math/util.h>
#include <ludo/testing.h>

#include "quat.h"

namespace ludo
{
  const auto mat3_x20deg = mat3
  {
    1.0f, 0.0f, 0.0f,
    0.0f, 0.939693f, 0.342020f,
    0.0f, -0.342020f, 0.939693f
  };

  const auto quat_zero = quat { 0.0f, 0.0f, 0.0f, 0.0f };
  const auto quat_test = quat { 0.0f, 1.0f, 2.0f, 3.0f };
  const auto quat_x10deg = quat { 0.087156f, 0.0f, 0.0f, 0.996195f };
  const auto quat_x20deg = quat { 0.173648f, 0.0f, 0.0f, 0.984808f };
  const auto quat_x20deg_inverted = quat { -0.173648f, 0.0f, 0.0f, 0.984808f };
  const auto quat_x180deg = quat { 1.0f, 0.0f, 0.0f, 0.0f };
  const auto quat_y90deg = quat { 0.0f, 0.0f, 0.707107f, 0.707107f }; // North Pole
  const auto quat_y90deg_neg = quat { 0.0f, 0.0f, -0.707107f, 0.707107f }; // South Pole

  void test_math_quat()
  {
    test_group("quat");

    auto constructor_explicit = quat { 0.0f, 1.0f, 2.0f, 3.0f };
    test_equal(
      "explicit constructor",
      constructor_explicit[0] == 0.0f &&
      constructor_explicit[1] == 1.0f &&
      constructor_explicit[2] == 2.0f &&
      constructor_explicit[3] == 3.0f,
      true
    );

    test_near("euler constructor", quat(pi / 180.0f * 20.0f, 0.0f, 0.0f), quat_x20deg);

    test_near("axis angle constructor", quat(vec3_unit_x, pi / 180.0f * 20.0f), quat_x20deg);

    // TODO test all four paths in code
    test_equal("mat3 constructor (identity)", quat(mat3_identity), quat_identity);
    test_near("mat3 constructor (rotated)", quat(mat3_x20deg), quat_x20deg);

    test_near("from to constructor (vec3)", quat(vec3_unit_y, up(mat3_x20deg)), quat_x20deg);
    test_equal("from to constructor (vec3) parallel", quat(vec3_unit_y, vec3_unit_y), quat_identity);
    test_equal("from to constructor (vec3) anti-parallel", quat(vec3_unit_y, vec3_unit_y * -1.0f), quat_x180deg);

    test_near("from to constructor (quat)", quat(quat_x10deg, quat_x20deg), quat_x10deg);

    test_near("multiply", quat_x10deg * quat_x10deg, quat_x20deg);

    auto multiply_inplace = quat_identity;
    multiply_inplace *= quat_x20deg;
    test_near("multiply inplace", multiply_inplace, quat_x20deg);

    test_equal("multiply scalar", quat_test * 10.0f, quat { 0.0f, 10.0f, 20.0f, 30.0f });

    auto multiply_scalar_inplace = quat_test;
    multiply_scalar_inplace *= 10.0f;
    test_equal("multiply scalar inplace", multiply_scalar_inplace, quat { 0.0f, 10.0f, 20.0f, 30.0f });

    test_equal("multiply scalar (scalar on left)", 10.0f * quat_test, quat { 0.0f, 10.0f, 20.0f, 30.0f });

    test_equal("divide scalar", quat_test / 10.0f, quat { 0.0f, 0.1f, 0.2f, 0.3f });

    auto divide_scalar_inplace = quat_test;
    divide_scalar_inplace /= 10.0f;
    test_equal("divide scalar inplace", divide_scalar_inplace, quat { 0.0f, 0.1f, 0.2f, 0.3f });

    auto stream = std::stringstream();
    stream << quat_test;
    test_equal("to stream", stream.str(), std::string("[0,1,2,3]"));

    test_near("length", length(quat_test), 3.7417f);

    test_near("angles", angles(quat_x20deg), vec3 { pi / 180.0f * 20.0f, 0.0f, 0.0f });
    test_near("angles (north pole)", angles(quat_y90deg), vec3 { 0.0f, pi / 180.0f * 90.0f, 0.0f });
    test_near("angles (south pole)", angles(quat_y90deg_neg), vec3 { 0.0f, pi / 180.0f * -90.0f, 0.0f });

    test_near("axis angle axis", axis_angle(quat_x20deg).first, vec3 { 1.0f, 0.0f, 0.0f });
    test_near("axis angle angle", axis_angle(quat_x20deg).second, pi / 180.0f * 20.0f);
    test_near("axis angle axis (north pole)", axis_angle(quat_y90deg).first, vec3 { 0.0f, 0.0f, 1.0f });
    test_near("axis angle angle (north pole)", axis_angle(quat_y90deg).second, pi / 180.0f * 90.0f);
    test_near("axis angle axis (south pole)", axis_angle(quat_y90deg_neg).first, vec3 { 0.0f, 0.0f, -1.0f });
    test_near("axis angle angle (south pole)", axis_angle(quat_y90deg_neg).second, pi / 180.0f * 90.0f);

    auto normalized = quat_test;
    normalize(normalized);
    test_near("normalize", length(normalized), 1.0f);

    auto normalized_zero_length = quat_zero;
    normalize(normalized_zero_length);
    test_equal("normalize (zero length)", normalized_zero_length, quat_zero);

    auto inverse = quat_x20deg;
    invert(inverse);
    test_near("invert", inverse, quat_x20deg_inverted);

    auto inverse_zero_length = quat_zero;
    invert(inverse_zero_length);
    test_equal("invert (zero length)", inverse_zero_length, quat_zero);

    test_equal("near (all far)", near(quat_test, quat_x20deg), false);

    auto all_near = quat_test;
    all_near[0] += 0.00005f;
    all_near[1] += 0.00005f;
    all_near[2] += 0.00005f;
    all_near[3] += 0.00005f;
    test_equal("near (all near)", near(quat_test, all_near), true);

    auto some_near = quat_x20deg;
    some_near[0] = 0.0002f;
    some_near[1] = 1.0002f;
    test_equal("near (some near)", near(quat_test, some_near), false);

    test_equal("near (all equal)", near(quat_test, quat_test), true);

    auto some_equal = quat_x20deg;
    some_equal[0] = 0.0f;
    some_equal[1] = 1.0f;
    test_equal("near (some equal)", near(quat_test, some_equal), false);

    test_near("dot", dot(quat_identity, quat_x20deg), 0.9848f);

    // TODO test negative dot path in code
    test_near("slerp (0)", slerp(quat_identity, quat_x20deg, 0.0f), quat_identity);
    test_near("slerp (0.5)", slerp(quat_identity, quat_x20deg, 0.5f), quat_x10deg);
    test_near("slerp (1)", slerp(quat_identity, quat_x20deg, 1.0f), quat_x20deg);
  }
}
