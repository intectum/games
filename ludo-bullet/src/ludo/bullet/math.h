/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <btBulletDynamicsCommon.h>

#include <ludo/math/transform.h>
#include <ludo/math/vec.h>

namespace ludo
{
  btTransform to_btTransform(const transform& original);

  btVector3 to_btVector3(const vec3& original);

  transform to_transform(const btTransform& original);

  vec3 to_vec3(const btVector3& original);
}
