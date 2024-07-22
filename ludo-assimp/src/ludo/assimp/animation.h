/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <assimp/scene.h>

#include <ludo/animation.h>

namespace ludo
{
  armature import_armature(const aiScene& assimp_scene, const aiMesh& assimp_mesh);

  std::vector<animation> import_animations(const aiScene& assimp_scene, const aiMesh& assimp_mesh);
}
