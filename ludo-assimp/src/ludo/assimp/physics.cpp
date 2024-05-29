/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "math.h"
#include "physics.h"

namespace ludo
{
  dynamic_body_shape import_body_shape(const aiScene& assimp_scene, const import_object& rigid_body_object);

  void import_body_shapes(import_results& results, const aiScene& assimp_scene, const std::vector<import_object>& rigid_body_objects)
  {
    for (auto& rigid_body_object : rigid_body_objects)
    {
      results.dynamic_body_shapes.push_back(import_body_shape(assimp_scene, rigid_body_object));
    }
  }

  dynamic_body_shape import_body_shape(const aiScene& assimp_scene, const import_object& rigid_body_object)
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

    auto dynamic_body_shape = ludo::dynamic_body_shape { .convex_hulls = { positions } };
    ludo::init(dynamic_body_shape);

    return dynamic_body_shape;
  }
}
