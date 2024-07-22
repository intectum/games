/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <assimp/scene.h>

#include <ludo/importing.h>
#include <ludo/rendering.h>

#include "util.h"

namespace ludo
{
  void import_meshes(import_results& results, heap& indices, heap& vertices, const std::string& folder, const aiScene& assimp_scene, const std::vector<import_object>& mesh_objects, const import_options& options);
}
