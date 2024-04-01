/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ASSIMP_TEXTURES_H
#define LUDO_ASSIMP_TEXTURES_H

#include <assimp/scene.h>

#include <ludo/rendering.h>

#include "util.h"

namespace ludo
{
  std::vector<ludo::texture*> import_textures(instance& instance, const std::string& folder, const aiScene& assimp_scene, const std::vector<import_object>& mesh_objects, const std::string& partition);
}

#endif // LUDO_ASSIMP_TEXTURES_H
