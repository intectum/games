/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ASSIMP_MATH_H
#define LUDO_ASSIMP_MATH_H

#include <assimp/color4.h>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <assimp/vector3.h>

#include <ludo/math/mat.h>
#include <ludo/math/transform.h>
#include <ludo/math/vec.h>

namespace ludo
{
  mat4 to_mat4(const aiMatrix4x4& original);

  quat to_quat(const aiQuaternion& original);

  vec2 to_vec2(const aiVector3D& original);

  vec3 to_vec3(const aiVector3D& original);

  vec4 to_vec4(const aiColor4D& original);
}

#endif // LUDO_ASSIMP_MATH_H
