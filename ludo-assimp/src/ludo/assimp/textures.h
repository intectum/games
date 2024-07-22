/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <assimp/scene.h>

#include <ludo/rendering.h>

#include "util.h"

namespace ludo
{
  texture import_texture(const std::string& folder, const aiScene& assimp_scene, const import_object& mesh_object);
}
