/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "util.h"

namespace ludo
{
  std::pair<uint32_t, uint32_t> import_counts(const aiScene& assimp_scene, const std::vector<import_object>& mesh_objects)
  {
    auto index_count = uint32_t(0);
    auto vertex_count = uint32_t(0);

    for (auto& mesh_object : mesh_objects)
    {
      auto& assimp_mesh = *assimp_scene.mMeshes[mesh_object.mesh_index];

      index_count += assimp_mesh.mNumFaces * assimp_mesh.mFaces[0].mNumIndices; // We use aiProcess_SortByPType so all faces should have the same number of indices.
      vertex_count += assimp_mesh.mNumVertices;
    }

    return { index_count, vertex_count };
  }
}
