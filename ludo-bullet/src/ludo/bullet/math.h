/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <btBulletDynamicsCommon.h>

#include <ludo/math/mat.h>
#include <ludo/math/quat.h>

namespace ludo
{
  btTransform to_btTransform(const vec3& position, const quat& rotation);

  btVector3 to_btVector3(const vec3& original);

  mat4 to_mat4(const btTransform& original);

  vec3 to_vec3(const btVector3& original);
}
