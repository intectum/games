/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "math.h"

namespace ludo
{
  btTransform to_btTransform(const vec3& position, const quat& rotation)
  {
    auto matrix = mat4(position, mat3(rotation));

    btTransform transform;
    transform.setFromOpenGLMatrix(matrix.begin());

    return transform;
  }

  btVector3 to_btVector3(const vec3& original)
  {
    return btVector3(original[0], original[1], original[2]);
  }

  mat4 to_mat4(const btTransform& original)
  {
    auto matrix = mat4();
    original.getOpenGLMatrix(matrix.begin());

    return matrix;
  }

  vec3 to_vec3(const btVector3& original)
  {
    return { original.getX(), original.getY(), original.getZ() };
  }
}
