/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/math/projection.h>
#include <ludo/testing.h>

#include "projection.h"

namespace ludo
{
  void test_math_projection()
  {
    test_group("projection");

    test_near("project point onto plane", project_point_onto_plane(vec3 { 1.0f, 1.0f, 1.0f }, vec3 { 0.0f, 2.0f, 0.0f }, vec3_unit_y), vec3 { 1.0f, 2.0f, 1.0f });
  }
}
