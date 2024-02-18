/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/math/mat.h>

#include "math.h"

namespace ludo
{
  btTransform to_btTransform(const transform& original)
  {
    auto matrix = mat4(original.position, mat3(original.rotation));

    btTransform transform;
    transform.setFromOpenGLMatrix(matrix.begin());

    return transform;
  }

  btVector3 to_btVector3(const vec3& original)
  {
    return btVector3(original[0], original[1], original[2]);
  }

  transform to_transform(const btTransform& original)
  {
    auto matrix = mat4();
    original.getOpenGLMatrix(matrix.begin());

    return
    {
      .position = position(matrix),
      .rotation = quat(mat3(matrix))
    };
  }

  vec3 to_vec3(const btVector3& original)
  {
    return { original.getX(), original.getY(), original.getZ() };
  }
}
