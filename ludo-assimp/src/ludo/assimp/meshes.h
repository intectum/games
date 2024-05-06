/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ASSIMP_MESHES_H
#define LUDO_ASSIMP_MESHES_H

#include <assimp/scene.h>

#include <ludo/importing.h>
#include <ludo/rendering.h>

#include "util.h"

namespace ludo
{
  std::vector<mesh> import_meshes(instance& instance, const aiScene& assimp_scene, const std::vector<import_object>& mesh_objects, const std::vector<ludo::texture*>& textures, const import_options& options, const std::string& partition);
}

#endif // LUDO_ASSIMP_MESHES_H
