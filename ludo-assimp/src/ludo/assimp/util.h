/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <assimp/scene.h>

#include <ludo/math/mat.h>

namespace ludo
{
  struct import_object
  {
    uint32_t mesh_index = 0;
    mat4 transform;
  };

  std::pair<uint32_t, uint32_t> import_counts(const aiScene& assimp_scene, const std::vector<import_object>& mesh_objects);
}
