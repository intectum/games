/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/physics.h>

#include "math.h"
#include "physics.h"

namespace ludo
{
  void import_body_shape(instance& instance, const aiScene& assimp_scene, const import_object& rigid_body_object, const std::string& partition)
  {
    auto& assimp_mesh = *assimp_scene.mMeshes[rigid_body_object.mesh_index];

    auto positions = std::vector<vec3>();

    for (auto index = 0; index < assimp_mesh.mNumVertices; index++)
    {
      auto position = vec3(rigid_body_object.transform * vec4(to_vec3(assimp_mesh.mVertices[index])));

      auto position_exists = false;
      for (auto& existing_position : positions)
      {
        if (near(existing_position, position))
        {
          position_exists = true;
          break;
        }
      }

      if (position_exists)
      {
        continue;
      }

      positions.emplace_back(position);
    }

    add(instance, ludo::body_shape { .positions = positions }, partition);
  }
}
