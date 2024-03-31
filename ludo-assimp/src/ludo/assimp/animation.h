/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ASSIMP_ANIMATION_H
#define LUDO_ASSIMP_ANIMATION_H

#include <assimp/scene.h>

namespace ludo
{
  uint64_t import_armature(instance& instance, const aiScene& assimp_scene, const aiMesh& assimp_mesh, const std::string& partition);

  std::vector<uint64_t> import_animations(instance& instance, const aiScene& assimp_scene, const aiMesh& assimp_mesh, const std::string& partition);
}

#endif // LUDO_ASSIMP_ANIMATION_H
