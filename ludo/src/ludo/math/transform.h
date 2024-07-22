/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "quat.h"
#include "vec.h"

namespace ludo
{
  ///
  /// A 3D transformation.
  struct transform
  {
    vec3 position = vec3_zero;
    quat rotation = quat_identity;
  };
}
