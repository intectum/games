/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <sstream>

#include <ludo/math/mat.h>
#include <ludo/math/quat.h>
#include <ludo/math/util.h>
#include <ludo/testing.h>

#include "mat.h"

namespace ludo
{
  const auto mat3_test = mat3
  {
    0.0f, 1.0f, 2.0f,
    3.0f, 4.0f, 5.0f,
    6.0f, 7.0f, 8.0f
  };
  const auto mat3_x20deg = mat3
  {
    1.0f, 0.0f, 0.0f,
    0.0f, 0.939693f, 0.342020f,
    0.0f, -0.342020f, 0.939693f
  };

  const auto mat4_test = mat4
  {
    0.0f, 1.0f, 2.0f, 3.0f,
    4.0f, 5.0f, 6.0f, 7.0f,
    8.0f, 9.0f, 10.0f, 11.0f,
    12.0f, 13.0f, 14.0f, 15.0f
  };
  const auto mat4_x20deg = mat4
  {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.939693f, 0.342020f, 0.0f,
    0.0f, -0.342020f, 0.939693f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  const auto quat_x20deg = quat { 0.173648f, 0.0f, 0.0f, 0.984808f };

  const auto vec3_test = vec3 { 0.0f, 1.0f, 2.0f };

  const auto vec4_test = vec4 { 0.0f, 1.0f, 2.0f, 3.0f };

  void add_to_all(mat3& matrix, float value);
  void add_to_all(mat4& matrix, float value);

  void test_math_mat()
  {
    test_group("mat");

    auto mat3_constructor_explicit = mat3
    {
      0.0f, 1.0f, 2.0f,
      3.0f, 4.0f, 5.0f,
      6.0f, 7.0f, 8.0f
    };
    test_equal(
      "mat3 explicit constructor",
      mat3_constructor_explicit[0] == 0.0f &&
      mat3_constructor_explicit[1] == 1.0f &&
      mat3_constructor_explicit[2] == 2.0f &&
      mat3_constructor_explicit[3] == 3.0f &&
      mat3_constructor_explicit[4] == 4.0f &&
      mat3_constructor_explicit[5] == 5.0f &&
      mat3_constructor_explicit[6] == 6.0f &&
      mat3_constructor_explicit[7] == 7.0f &&
      mat3_constructor_explicit[8] == 8.0f,
      true
    );

    test_near("mat3 euler constructor", mat3(pi / 180.0f * 20.0f, 0.0f, 0.0f), mat3_x20deg);

    test_near("mat3 axis angle constructor", mat3(vec3_unit_x, pi / 180.0f * 20.0f), mat3_x20deg);

    test_equal("mat3 quat constructor (identity)", mat3(quat_identity), mat3_identity);
    test_near("mat3 quat constructor (rotated)", mat3(quat_x20deg), mat3_x20deg);

    test_equal("mat3 mat4 constructor", mat3(mat4_test), mat3
    {
      0.0f, 1.0f, 2.0f,
      4.0f, 5.0f, 6.0f,
      8.0f, 9.0f, 10.0f
    });

    test_near("mat3 from to constructor", mat3(vec3_unit_y, up(mat3_x20deg)), mat3_x20deg);

    auto mat4_constructor_explicit = mat4 { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f };
    test_equal(
      "mat4 explicit constructor",
      mat4_constructor_explicit[0] == 0.0f &&
      mat4_constructor_explicit[1] == 1.0f &&
      mat4_constructor_explicit[2] == 2.0f &&
      mat4_constructor_explicit[3] == 3.0f &&
      mat4_constructor_explicit[4] == 4.0f &&
      mat4_constructor_explicit[5] == 5.0f &&
      mat4_constructor_explicit[6] == 6.0f &&
      mat4_constructor_explicit[7] == 7.0f &&
      mat4_constructor_explicit[8] == 8.0f &&
      mat4_constructor_explicit[9] == 9.0f &&
      mat4_constructor_explicit[10] == 10.0f &&
      mat4_constructor_explicit[11] == 11.0f &&
      mat4_constructor_explicit[12] == 12.0f &&
      mat4_constructor_explicit[13] == 13.0f &&
      mat4_constructor_explicit[14] == 14.0f &&
      mat4_constructor_explicit[15] == 15.0f,
      true
    );

    test_equal("mat4 vec3 mat3 constructor", mat4(vec3_test, mat3_test), mat4
    {
      0.0f, 1.0f, 2.0f, 0.0f,
      3.0f, 4.0f, 5.0f, 0.0f,
      6.0f, 7.0f, 8.0f, 0.0f,
      0.0f, 1.0f, 2.0f, 1.0f
    });

    test_equal("mat3 add mat3", mat3_test + mat3_test, mat3
    {
      0.0f, 2.0f, 4.0f,
      6.0f, 8.0f, 10.0f,
      12.0f, 14.0f, 16.0f
    });

    test_equal("mat4 add mat4", mat4_test + mat4_test, mat4
    {
      0.0f, 2.0f, 4.0f, 6.0f,
      8.0f, 10.0f, 12.0f, 14.0f,
      16.0f, 18.0f, 20.0f, 22.0f,
      24.0f, 26.0f, 28.0f, 30.0f
    });

    auto mat3_add_mat3_inplace = mat3_test;
    mat3_add_mat3_inplace += mat3_test;
    test_equal("mat3 add mat3 inplace", mat3_add_mat3_inplace, mat3
    {
      0.0f, 2.0f, 4.0f,
      6.0f, 8.0f, 10.0f,
      12.0f, 14.0f, 16.0f
    });

    auto mat4_add_mat4_inplace = mat4_test;
    mat4_add_mat4_inplace += mat4_test;
    test_equal("mat4 add mat4 inplace", mat4_add_mat4_inplace, mat4
    {
      0.0f, 2.0f, 4.0f, 6.0f,
      8.0f, 10.0f, 12.0f, 14.0f,
      16.0f, 18.0f, 20.0f, 22.0f,
      24.0f, 26.0f, 28.0f, 30.0f
    });

    test_equal("mat3 subtract mat3", mat3_test - mat3_test, mat3
    {
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f
    });

    test_equal("mat4 subtract mat4", mat4_test - mat4_test, mat4
    {
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f
    });

    auto mat3_subtract_mat3_inplace = mat3_test;
    mat3_subtract_mat3_inplace -= mat3_test;
    test_equal("mat3 subtract mat3 inplace", mat3_subtract_mat3_inplace, mat3
    {
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f
    });

    auto mat4_subtract_mat4_inplace = mat4_test;
    mat4_subtract_mat4_inplace -= mat4_test;
    test_equal("mat4 subtract mat4 inplace", mat4_subtract_mat4_inplace, mat4
    {
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f
    });

    test_equal("mat3 multiply mat3", mat3_test * mat3_test, mat3
    {
      15.0f, 18.0f, 21.0f,
      42.0f, 54.0f, 66.0f,
      69.0f, 90.0f, 111.0f
    });

    test_equal("mat4 multiply mat4", mat4_test * mat4_test, mat4
    {
      56.0f, 62.0f, 68.0f, 74.0f,
      152.0f, 174.0f, 196.0f, 218.0f,
      248.0f, 286.0f, 324.0f, 362.0f,
      344.0f, 398.0f, 452.0f, 506.0f
    });

    auto mat3_multiply_mat3_inplace = mat3_test;
    mat3_multiply_mat3_inplace *= mat3_test;
    test_equal("mat3 multiply mat3 inplace", mat3_multiply_mat3_inplace, mat3
    {
      15.0f, 18.0f, 21.0f,
      42.0f, 54.0f, 66.0f,
      69.0f, 90.0f, 111.0f
    });

    auto mat4_multiply_mat4_inplace = mat4_test;
    mat4_multiply_mat4_inplace *= mat4_test;
    test_equal("mat4 multiply mat4 inplace", mat4_multiply_mat4_inplace, mat4
    {
      56.0f, 62.0f, 68.0f, 74.0f,
      152.0f, 174.0f, 196.0f, 218.0f,
      248.0f, 286.0f, 324.0f, 362.0f,
      344.0f, 398.0f, 452.0f, 506.0f
    });

    test_equal("mat3 multiply vector", mat3_test * vec3_test, vec3 { 15.0f, 18.0f, 21.0f });
    test_equal("mat3 multiply vector (vector on left)", vec3_test * mat3_test, vec3 { 5.0f, 14.0f, 23.0f });

    test_equal("mat4 multiply vector", mat4_test * vec4_test, vec4 { 56.0f, 62.0f, 68.0f, 74.0f });
    test_equal("mat4 multiply vector (vector on left)", vec4_test * mat4_test, vec4 { 14.0f, 38.0f, 62.0f, 86.0f });

    test_equal("mat3 multiply scalar", mat3_test * 10.0f, mat3
    {
      0.0f, 10.0f, 20.0f,
      30.0f, 40.0f, 50.0f,
      60.0f, 70.0f, 80.0f
    });

    test_equal("mat4 multiply scalar", mat4_test * 10.0f, mat4
    {
      0.0f, 10.0f, 20.0f, 30.0f,
      40.0f, 50.0f,60.0f, 70.0f,
      80.0f, 90.0f, 100.0f, 110.0f,
      120.0f, 130.0f, 140.0f, 150.0f
    });

    auto mat3_multiply_scalar_inplace = mat3_test;
    mat3_multiply_scalar_inplace *= 10.0f;
    test_equal("mat3 multiply scalar inplace", mat3_multiply_scalar_inplace, mat3
    {
      0.0f, 10.0f, 20.0f,
      30.0f, 40.0f, 50.0f,
      60.0f, 70.0f, 80.0f
    });

    auto mat4_multiply_scalar_inplace = mat4_test;
    mat4_multiply_scalar_inplace *= 10.0f;
    test_equal("mat4 multiply scalar inplace", mat4_multiply_scalar_inplace, mat4
    {
      0.0f, 10.0f, 20.0f, 30.0f,
      40.0f, 50.0f,60.0f, 70.0f,
      80.0f, 90.0f, 100.0f, 110.0f,
      120.0f, 130.0f, 140.0f, 150.0f
    });

    test_equal("mat3 multiply scalar (scalar on left)", 10.0f * mat3_test, mat3
    {
      0.0f, 10.0f, 20.0f,
      30.0f, 40.0f, 50.0f,
      60.0f, 70.0f, 80.0f
    });

    test_equal("mat4 multiply scalar (scalar on left)", 10.0f * mat4_test, mat4
    {
      0.0f, 10.0f, 20.0f, 30.0f,
      40.0f, 50.0f,60.0f, 70.0f,
      80.0f, 90.0f, 100.0f, 110.0f,
      120.0f, 130.0f, 140.0f, 150.0f
    });

    auto mat3_stream = std::stringstream();
    mat3_stream << mat3_test;
    test_equal("mat3 to stream", mat3_stream.str(), std::string("[0,3,6]\n[1,4,7]\n[2,5,8]"));

    auto mat4_stream = std::stringstream();
    mat4_stream << mat4_test;
    test_equal("mat4 to stream", mat4_stream.str(), std::string("[0,4,8,12]\n[1,5,9,13]\n[2,6,10,14]\n[3,7,11,15]"));

    test_equal("mat4 get position", position(mat4_test), vec3 { 12.0f, 13.0f, 14.0f });
    test_equal("mat4 get position (vec4)", position4(mat4_test), vec4 { 12.0f, 13.0f, 14.0f, 15.0f });

    auto mat4_position = mat4_test;
    position(mat4_position, vec3 {0.1f, 0.2f, 0.3f });
    test_equal(
      "mat4 set position",
      mat4_position[12] == 0.1f &&
      mat4_position[13] == 0.2f &&
      mat4_position[14] == 0.3f &&
      mat4_position[15] == 1.0f,
      true
    );

    auto mat4_position_vec4 = mat4_test;
    position(mat4_position_vec4, vec4 {0.1f, 0.2f, 0.3f, 0.4f });
    test_equal(
      "mat4 set position (vec4)",
      mat4_position_vec4[12] == 0.1f &&
      mat4_position_vec4[13] == 0.2f &&
      mat4_position_vec4[14] == 0.3f &&
      mat4_position_vec4[15] == 0.4f,
      true
    );

    test_equal("mat3 get right", right(mat3_test), vec3 { 0.0f, 1.0f, 2.0f });
    test_equal("mat4 get right", right(mat4_test), vec3 { 0.0f, 1.0f, 2.0f });
    test_equal("mat4 get right (vec4)", right4(mat4_test), vec4 { 0.0f, 1.0f, 2.0f, 3.0f });

    test_equal("mat3 get up", up(mat3_test), vec3 { 3.0f, 4.0f, 5.0f });
    test_equal("mat4 get up", up(mat4_test), vec3 { 4.0f, 5.0f, 6.0f });
    test_equal("mat4 get up (vec4)", up4(mat4_test), vec4 { 4.0f, 5.0f, 6.0f, 7.0f });

    test_equal("mat3 get out", out(mat3_test), vec3 { 6.0f, 7.0f, 8.0f });
    test_equal("mat4 get out", out(mat4_test), vec3 { 8.0f, 9.0f, 10.0f });
    test_equal("mat4 get out (vec4)", out4(mat4_test), vec4 { 8.0f, 9.0f, 10.0f, 11.0f });

    test_near("mat3 angles", angles(mat3_x20deg), vec3 {pi / 180.0f * 20.0f, 0.0f, 0.0f });
    test_near("mat4 angles", angles(mat4_x20deg), vec3 {pi / 180.0f * 20.0f, 0.0f, 0.0f });

    auto mat3_determinant = determinant(mat3_x20deg);
    test_equal("mat3 determinant", mat3_determinant, 1.000000614649f);

    auto mat4_determinant = determinant(mat4_x20deg);
    test_equal("mat4 determinant", mat4_determinant, 1.000000614649f);

    auto mat3_inverse = mat3 {2.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 2.0f };
    invert(mat3_inverse);
    test_near(
      "mat3 invert",
      mat3_inverse,
      mat3
      {
        -1.0f,1.0f / 3.0f, 2.0f / 3.0f,
        1.0f, -1.0f, -1.0f,
        0.0f, 1.0f / 3.0f, -1.0f / 3.0f
      }
    );

    auto mat4_inverse = mat4 {1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
    invert(mat4_inverse);
    test_near(
      "mat4 invert",
      mat4_inverse,
      mat4
      {
        1.0f / 4.0f, 1.0f / 4.0f, 1.0f / 4.0f, -1.0f / 4.0f,
        1.0f / 4.0f, 1.0f / 4.0f, -1.0f / 4.0f, 1.0f / 4.0f,
        1.0f / 4.0f, -1.0f / 4.0f, 1.0f / 4.0f, 1.0f / 4.0f,
        -1.0f / 4.0f, 1.0f / 4.0f, 1.0f / 4.0f, 1.0f / 4.0f
      }
    );

    auto mat3_transpose = mat3_test;
    transpose(mat3_transpose);
    test_equal("mat3 transpose", mat3_transpose, mat3
    {
      0.0f, 3.0f, 6.0f,
      1.0f, 4.0f, 7.0f,
      2.0f, 5.0f, 8.0f
    });

    auto mat4_transpose = mat4_test;
    transpose(mat4_transpose);
    test_equal("mat4 transpose", mat4_transpose, mat4
    {
      0.0f, 4.0f, 8.0f, 12.0f,
      1.0f, 5.0f, 9.0f, 13.0f,
      2.0f, 6.0f, 10.0f, 14.0f,
      3.0f, 7.0f, 11.0f, 15.0f
    });

    auto mat4_scale_abs = mat4_identity * 2.0f;
    scale_abs(mat4_scale_abs, vec3 { 0.1f, 0.2f, 0.3f });
    test_equal(
      "mat4 set scale",
      mat4_scale_abs[0] == 0.1f &&
      mat4_scale_abs[5] == 0.2f &&
      mat4_scale_abs[10] == 0.3f,
      true
    );

    auto mat4_scale = mat4_identity * 2.0f;
    scale(mat4_scale, vec3 { 0.1f, 0.2f, 0.3f });
    test_equal(
      "mat4 scale",
      near(mat4_scale[0], 0.2f) &&
      near(mat4_scale[5], 0.4f) &&
      near(mat4_scale[10], 0.6f),
      true
    );

    auto mat4_translate = mat4_identity;
    translate(mat4_translate, vec3 { 0.1f, 0.2f, 0.3f });
    test_equal(
      "mat4 translate",
      mat4_translate[12] == 0.1f &&
      mat4_translate[13] == 0.2f &&
      mat4_translate[14] == 0.3f &&
      mat4_translate[15] == 1.0f,
      true
    );

    auto mat4_translate_vec4 = mat4_identity;
    translate(mat4_translate_vec4, vec4 { 0.1f, 0.2f, 0.3f, 1.0f });
    test_equal(
      "mat4 translate (vec4)",
      mat4_translate_vec4[12] == 0.1f &&
      mat4_translate_vec4[13] == 0.2f &&
      mat4_translate_vec4[14] == 0.3f &&
      mat4_translate_vec4[15] == 1.0f,
      true
    );

    // TODO test orthogonal and perspective!

    test_equal("mat3 near (all far)", near(mat3_test, mat3_x20deg), false);

    auto mat3_all_near = mat3_test;
    add_to_all(mat3_all_near, 0.00005f);
    test_equal("mat3 near (all near)", near(mat3_test, mat3_all_near), true);

    auto mat3_some_near = mat3_x20deg;
    mat3_some_near[0] = 0.0002f;
    mat3_some_near[1] = 1.0002f;
    test_equal("mat3 near (some near)", near(mat3_test, mat3_some_near), false);

    test_equal("mat3 near (all equal)", near(mat3_test, mat3_test), true);

    auto mat3_some_equal = mat3_x20deg;
    mat3_some_equal[0] = 0.0f;
    mat3_some_equal[1] = 1.0f;
    test_equal("mat3 near (some equal)", near(mat3_test, mat3_some_equal), false);

    test_equal("mat4 near (all far)", near(mat4_test, mat4_x20deg), false);

    auto mat4_all_near = mat4_test;
    add_to_all(mat4_all_near, 0.00005f);
    test_equal("mat4 near (all near)", near(mat4_test, mat4_all_near), true);

    auto mat4_some_near = mat4_x20deg;
    mat4_some_near[0] = 0.0002f;
    mat4_some_near[1] = 1.0002f;
    test_equal("mat4 near (some near)", near(mat4_test, mat4_some_near), false);

    test_equal("mat4 near (all equal)", near(mat4_test, mat4_test), true);

    auto mat4_some_equal = mat4_x20deg;
    mat4_some_equal[0] = 0.0f;
    mat4_some_equal[1] = 1.0f;
    test_equal("mat4 near (some equal)", near(mat4_test, mat4_some_equal), false);
  }

  void add_to_all(mat3& matrix, float value)
  {
    for (auto index = 0; index < 9; index++)
    {
      matrix[index] += value;
    }
  }

  void add_to_all(mat4& matrix, float value)
  {
    for (auto index = 0; index < 16; index++)
    {
      matrix[index] += value;
    }
  }
}
