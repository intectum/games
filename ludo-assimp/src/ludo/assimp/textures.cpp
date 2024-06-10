/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "textures.h"

namespace ludo
{
  texture import_texture(const std::string& folder, const aiScene& assimp_scene, const import_object& mesh_object)
  {
    auto assimp_material = assimp_scene.mMaterials[assimp_scene.mMeshes[mesh_object.mesh_index]->mMaterialIndex];
    auto texture_path = aiString();
    assimp_material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path);
    if (!texture_path.length)
    {
      return {};
    }

    return ludo::load(folder + texture_path.C_Str());
  }
}
