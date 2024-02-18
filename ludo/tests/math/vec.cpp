/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/math/vec.h>
#include <ludo/testing.h>

#include "vec.h"

namespace ludo
{
  const auto vec2_test = vec2 { 0.0f, 1.0f };
  const auto vec2_test2 = vec2 { 1.0f, 0.0f };

  const auto vec3_test = vec3 { 0.0f, 1.0f, 2.0f };
  const auto vec3_test2 = vec3 { 2.0f, 1.0f, 0.0f };

  const auto vec4_test = vec4 { 0.0f, 1.0f, 2.0f, 3.0f };
  const auto vec4_test2 = vec4 { 3.0f, 2.0f, 1.0f, 0.0f };

  void test_math_vec()
  {
    test_group("vec");

    auto vec2_constructor_explicit = vec2 { 0.0f, 1.0f };
    test_equal(
      "vec2 explicit constructor",
      vec2_constructor_explicit[0] == 0.0f &&
      vec2_constructor_explicit[1] == 1.0f,
      true
    );

    auto vec3_constructor_explicit = vec3 { 0.0f, 1.0f, 2.0f };
    test_equal(
      "vec3 explicit constructor",
      vec3_constructor_explicit[0] == 0.0f &&
      vec3_constructor_explicit[1] == 1.0f &&
      vec3_constructor_explicit[2] == 2.0f,
      true
    );

    test_equal("vec3 vec4 constructor", vec3(vec4_test), vec3 { 0.0f, 1.0f, 2.0f });

    auto vec4_constructor_explicit = vec4 { 0.0f, 1.0f, 2.0f, 3.0f };
    test_equal(
      "vec4 explicit constructor",
      vec4_constructor_explicit[0] == 0.0f &&
      vec4_constructor_explicit[1] == 1.0f &&
      vec4_constructor_explicit[2] == 2.0f &&
      vec4_constructor_explicit[3] == 3.0f,
      true
    );

    test_equal("vec4 vec3 constructor", vec4(vec3_test), vec4 { 0.0f, 1.0f, 2.0f, 1.0f });

    test_equal("vec2 add vec2", vec2_test + vec2_test, vec2 { 0.0f, 2.0f });
    test_equal("vec3 add vec3", vec3_test + vec3_test, vec3 { 0.0f, 2.0f, 4.0f });
    test_equal("vec4 add vec4", vec4_test + vec4_test, vec4 { 0.0f, 2.0f, 4.0f, 6.0f });

    auto vec2_add_vec2_inplace = vec2_test;
    vec2_add_vec2_inplace += vec2_test;
    test_equal("vec2 add vec2 inplace", vec2_add_vec2_inplace, vec2 { 0.0f, 2.0f });

    auto vec3_add_vec3_inplace = vec3_test;
    vec3_add_vec3_inplace += vec3_test;
    test_equal("vec3 add vec3 inplace", vec3_add_vec3_inplace, vec3 { 0.0f, 2.0f, 4.0f });

    auto vec4_add_vec4_inplace = vec4_test;
    vec4_add_vec4_inplace += vec4_test;
    test_equal("vec4 add vec4 inplace", vec4_add_vec4_inplace, vec4 { 0.0f, 2.0f, 4.0f, 6.0f });

    test_equal("vec2 subtract vec2", vec2_test - vec2_test, vec2 { 0.0f, 0.0f });
    test_equal("vec3 subtract vec3", vec3_test - vec3_test, vec3 { 0.0f, 0.0f, 0.0f });
    test_equal("vec4 subtract vec4", vec4_test - vec4_test, vec4 { 0.0f, 0.0f, 0.0f, 0.0f });

    auto vec2_subtract_vec2_inplace = vec2_test;
    vec2_subtract_vec2_inplace -= vec2_test;
    test_equal("vec2 subtract vec2 inplace", vec2_subtract_vec2_inplace, vec2 { 0.0f, 0.0f });

    auto vec3_subtract_vec3_inplace = vec3_test;
    vec3_subtract_vec3_inplace -= vec3_test;
    test_equal("vec3 subtract vec3 inplace", vec3_subtract_vec3_inplace, vec3 { 0.0f, 0.0f, 0.0f });

    auto vec4_subtract_vec4_inplace = vec4_test;
    vec4_subtract_vec4_inplace -= vec4_test;
    test_equal("vec4 subtract vec4 inplace", vec4_subtract_vec4_inplace, vec4 { 0.0f, 0.0f, 0.0f, 0.0f });

    test_equal("vec2 multiply vec2", vec2_test * vec2_test, vec2 { 0.0f, 1.0f });
    test_equal("vec3 multiply vec3", vec3_test * vec3_test, vec3 { 0.0f, 1.0f, 4.0f });
    test_equal("vec4 multiply vec4", vec4_test * vec4_test, vec4 { 0.0f, 1.0f, 4.0f, 9.0f });

    auto vec2_multiply_vec2_inplace = vec2_test;
    vec2_multiply_vec2_inplace *= vec2_test;
    test_equal("vec2 multiply vec2 inplace", vec2_multiply_vec2_inplace, vec2 { 0.0f, 1.0f });

    auto vec3_multiply_vec3_inplace = vec3_test;
    vec3_multiply_vec3_inplace *= vec3_test;
    test_equal("vec3 multiply vec3 inplace", vec3_multiply_vec3_inplace, vec3 { 0.0f, 1.0f, 4.0f });

    auto vec4_multiply_vec4_inplace = vec4_test;
    vec4_multiply_vec4_inplace *= vec4_test;
    test_equal("vec4 multiply vec4 inplace", vec4_multiply_vec4_inplace, vec4 { 0.0f, 1.0f, 4.0f, 9.0f });

    test_equal("vec2 multiply scalar", vec2_test * 10.0f, vec2 { 0.0f, 10.0f });
    test_equal("vec3 multiply scalar", vec3_test * 10.0f, vec3 { 0.0f, 10.0f, 20.0f });
    test_equal("vec4 multiply scalar", vec4_test * 10.0f, vec4 { 0.0f, 10.0f, 20.0f, 30.0f });

    auto vec2_multiply_scalar_inplace = vec2_test;
    vec2_multiply_scalar_inplace *= 10.0f;
    test_equal("vec2 multiply scalar inplace", vec2_multiply_scalar_inplace, vec2 { 0.0f, 10.0f });

    auto vec3_multiply_scalar_inplace = vec3_test;
    vec3_multiply_scalar_inplace *= 10.0f;
    test_equal("vec3 multiply scalar inplace", vec3_multiply_scalar_inplace, vec3 { 0.0f, 10.0f, 20.0f });

    auto vec4_multiply_scalar_inplace = vec4_test;
    vec4_multiply_scalar_inplace *= 10.0f;
    test_equal("vec4 multiply scalar inplace", vec4_multiply_scalar_inplace, vec4 { 0.0f, 10.0f, 20.0f, 30.0f });

    test_equal("vec2 multiply scalar (scalar on left)", 10.0f * vec2_test, vec2 { 0.0f, 10.0f });
    test_equal("vec3 multiply scalar (scalar on left)", 10.0f * vec3_test, vec3 { 0.0f, 10.0f, 20.0f });
    test_equal("vec4 multiply scalar (scalar on left)", 10.0f * vec4_test, vec4 { 0.0f, 10.0f, 20.0f, 30.0f });

    auto vec2_test_nozero = vec2_test;
    vec2_test_nozero[0] = 0.1f;
    auto vec3_test_nozero = vec3_test;
    vec3_test_nozero[0] = 0.1f;
    auto vec4_test_nozero = vec4_test;
    vec4_test_nozero[0] = 0.1f;

    test_equal("vec2 divide vec2", vec2_test_nozero / vec2_test_nozero, vec2 { 1.0f, 1.0f });
    test_equal("vec3 divide vec3", vec3_test_nozero / vec3_test_nozero, vec3 { 1.0f, 1.0f, 1.0f });
    test_equal("vec4 divide vec4", vec4_test_nozero / vec4_test_nozero, vec4 { 1.0f, 1.0f, 1.0f, 1.0f });

    auto vec2_divide_vec2_inplace = vec2_test_nozero;
    vec2_divide_vec2_inplace /= vec2_test_nozero;
    test_equal("vec2 divide vec2 inplace", vec2_divide_vec2_inplace, vec2 { 1.0f, 1.0f });

    auto vec3_divide_vec3_inplace = vec3_test_nozero;
    vec3_divide_vec3_inplace /= vec3_test_nozero;
    test_equal("vec3 divide vec3 inplace", vec3_divide_vec3_inplace, vec3 { 1.0f, 1.0f, 1.0f });

    auto vec4_divide_vec4_inplace = vec4_test_nozero;
    vec4_divide_vec4_inplace /= vec4_test_nozero;
    test_equal("vec4 divide vec4 inplace", vec4_divide_vec4_inplace, vec4 { 1.0f, 1.0f, 1.0f, 1.0f });

    test_equal("vec2 divide scalar", vec2_test / 10.0f, vec2 { 0.0f, 0.1f });
    test_equal("vec3 divide scalar", vec3_test / 10.0f, vec3 { 0.0f, 0.1f, 0.2f });
    test_equal("vec4 divide scalar", vec4_test / 10.0f, vec4 { 0.0f, 0.1f, 0.2f, 0.3f });

    auto vec2_divide_scalar_inplace = vec2_test;
    vec2_divide_scalar_inplace /= 10.0f;
    test_equal("vec2 divide scalar inplace", vec2_divide_scalar_inplace, vec2 { 0.0f, 0.1f });

    auto vec3_divide_scalar_inplace = vec3_test;
    vec3_divide_scalar_inplace /= 10.0f;
    test_equal("vec3 divide scalar inplace", vec3_divide_scalar_inplace, vec3 { 0.0f, 0.1f, 0.2f });

    auto vec4_divide_scalar_inplace = vec4_test;
    vec4_divide_scalar_inplace /= 10.0f;
    test_equal("vec4 divide scalar inplace", vec4_divide_scalar_inplace, vec4 { 0.0f, 0.1f, 0.2f, 0.3f });

    auto vec2_stream = std::stringstream();
    vec2_stream << vec2_test;
    test_equal("vec2 to stream", vec2_stream.str(), std::string("[0,1]"));

    auto vec3_stream = std::stringstream();
    vec3_stream << vec3_test;
    test_equal("vec3 to stream", vec3_stream.str(), std::string("[0,1,2]"));

    auto vec4_stream = std::stringstream();
    vec4_stream << vec4_test;
    test_equal("vec4 to stream", vec4_stream.str(), std::string("[0,1,2,3]"));

    test_near("vec2 length", length(vec2_test), 1.0f);
    test_near("vec3 length", length(vec3_test), 2.2361f);
    test_near("vec4 length", length(vec4_test), 6.7082f);

    test_equal("vec2 length squared", length2(vec2_test), 1.0f);
    test_equal("vec3 length squared", length2(vec3_test), 5.0f);
    test_equal("vec4 length squared", length2(vec4_test), 45.0f);

    auto vec2_normalized = vec2_test;
    normalize(vec2_normalized);
    test_near("vec2 normalize", length(vec2_normalized), 1.0f);

    auto vec2_normalized_zero_length = vec2_zero;
    normalize(vec2_normalized_zero_length);
    test_equal("vec2 normalize (zero length)", vec2_normalized_zero_length, vec2_zero);

    auto vec3_normalized = vec3_test;
    normalize(vec3_normalized);
    test_near("vec3 normalize", length(vec3_normalized), 1.0f);

    auto vec3_normalized_zero_length = vec3_zero;
    normalize(vec3_normalized_zero_length);
    test_equal("vec3 normalize (zero length)", vec3_normalized_zero_length, vec3_zero);

    auto vec4_normalized = vec4_test;
    normalize(vec4_normalized);
    test_near("vec4 normalize", length(vec4_normalized), 0.1491f);

    auto vec4_normalized_zero_length = vec4_zero;
    normalize(vec4_normalized_zero_length);
    test_equal("vec4 normalize (zero length)", vec4_normalized_zero_length, vec4_zero);

    auto vec2_rotate = vec2_unit_x;
    rotate(vec2_rotate, pi / 2.0f);
    test_near("vec2 rotate", vec2_rotate, vec2_unit_y);

    auto vec4_homogenize = vec4_test;
    homogenize(vec4_homogenize);
    test_near("vec4 homogenize", vec4_homogenize, vec4 { 0.0f, 0.3333f, 0.6667f, 1.0f });

    test_equal("vec2 near (all far)", near(vec2_test, vec2_test2), false);

    auto vec2_all_near = vec2_test;
    vec2_all_near[0] += 0.00005f;
    vec2_all_near[1] += 0.00005f;
    test_equal("vec2 near (all near)", near(vec2_test, vec2_all_near), true);

    auto vec2_some_near = vec2_test2;
    vec2_some_near[0] = 0.0002f;
    vec2_some_near[1] = 1.0002f;
    test_equal("vec2 near (some near)", near(vec2_test, vec2_some_near), false);

    test_equal("vec2 near (all equal)", near(vec2_test, vec2_test), true);

    auto vec2_some_equal = vec2_test2;
    vec2_some_equal[0] = 0.0f;
    test_equal("vec2 near (some equal)", near(vec2_test, vec2_some_equal), false);

    test_equal("vec3 near (all far)", near(vec3_test, vec3_test2), false);

    auto vec3_all_near = vec3_test;
    vec3_all_near[0] += 0.00005f;
    vec3_all_near[1] += 0.00005f;
    vec3_all_near[2] += 0.00005f;
    test_equal("vec3 near (all near)", near(vec3_test, vec3_all_near), true);

    auto vec3_some_near = vec3_test2;
    vec3_some_near[0] = 0.0002f;
    vec3_some_near[1] = 1.0002f;
    test_equal("vec3 near (some near)", near(vec3_test, vec3_some_near), false);

    test_equal("vec3 near (all equal)", near(vec3_test, vec3_test), true);

    auto vec3_some_equal = vec3_test2;
    vec3_some_equal[0] = 0.0f;
    vec3_some_equal[1] = 1.0f;
    test_equal("vec3 near (some equal)", near(vec3_test, vec3_some_equal), false);

    test_equal("vec4 near (all far)", near(vec4_test, vec4_test2), false);

    auto vec4_all_near = vec4_test;
    vec4_all_near[0] += 0.000005f;
    vec4_all_near[1] += 0.000005f;
    vec4_all_near[2] += 0.000005f;
    vec4_all_near[3] += 0.000005f;
    test_equal("vec4 near (all near)", near(vec4_test, vec4_all_near), true);

    auto vec4_some_near = vec4_test2;
    vec4_some_near[0] = 0.0002f;
    vec4_some_near[1] = 1.0002f;
    test_equal("vec4 near (some near)", near(vec4_test, vec4_some_near), false);

    test_equal("vec4 near (all equal)", near(vec4_test, vec4_test), true);

    auto vec4_some_equal = vec4_test2;
    vec4_some_equal[0] = 0.0f;
    vec4_some_equal[1] = 1.0f;
    test_equal("vec4 near (some equal)", near(vec4_test, vec4_some_equal), false);

    test_equal("vec2 cross", cross(vec2_test, vec2_test2), -1.0f);
    test_equal("vec3 cross", cross(vec3_test, vec3_test2), vec3 { -2.0f, 4.0f, -2.0f });

    test_equal("vec2 dot", dot(vec2_test, vec2_test2), 0.0f);
    test_equal("vec3 dot", dot(vec3_test, vec3_test2), 1.0f);

    auto vec2_normalized2 = vec2_test2;
    normalize(vec2_normalized2);
    test_near("vec2 project", project(vec2_test, vec2_normalized2), vec2 { 0.0f, 0.0f });

    auto vec3_normalized2 = vec3_test2;
    normalize(vec3_normalized2);
    test_near("vec3 project", project(vec3_test, vec3_normalized2), vec3 { 2.0f / 5.0f, 1.0f / 5.0f, 0.0f });

    test_near("vec2 scalar project", scalar_project(vec2_test, vec2_normalized2), 0.0f);
    test_near("vec3 scalar project", scalar_project(vec3_test, vec3_normalized2), 0.4472f);

    test_equal("vec2 angle between", angle_between(vec2_unit_x, vec2_unit_y), pi / 2.0f);
    test_equal("vec3 angle between", angle_between(vec3_unit_x, vec3_unit_y), pi / 2.0f);

    test_near("vec3 project onto plane", project_onto_plane(vec3 { 1.0f, 1.0f, 1.0f }, vec3_unit_y), vec3 { 1.0f, 0.0f, 1.0f });
  }
}
