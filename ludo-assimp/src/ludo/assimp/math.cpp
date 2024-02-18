/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "math.h"

namespace ludo
{
  mat4 to_mat4(const aiMatrix4x4& original)
  {
    auto matrix = *reinterpret_cast<const mat4*>(&original);
    ludo::transpose(matrix); // Assimp's matrices are row-major.

    return matrix;
  }

  quat to_quat(const aiQuaternion& original)
  {
    // We cannot use reinterpret_cast here since Assimp stores the w dimension first.
    return quat { original.x, original.y, original.z, original.w };
  }

  vec2 to_vec2(const aiVector3D& original)
  {
    return *reinterpret_cast<const vec2*>(&original);
  }

  vec3 to_vec3(const aiVector3D& original)
  {
    return *reinterpret_cast<const vec3*>(&original);
  }

  vec4 to_vec4(const aiColor4D& original)
  {
    return *reinterpret_cast<const vec4*>(&original);
  }
}
