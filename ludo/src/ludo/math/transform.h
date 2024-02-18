/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MATH_TRANSFORM_H_
#define LUDO_MATH_TRANSFORM_H_

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

#endif /* LUDO_MATH_TRANSFORM_H_ */
