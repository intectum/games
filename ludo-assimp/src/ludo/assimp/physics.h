/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <assimp/scene.h>

#include <ludo/importing.h>
#include <ludo/physics.h>

#include "util.h"

namespace ludo
{
  void import_body_shapes(import_results& results, const aiScene& assimp_scene, const std::vector<import_object>& rigid_body_objects);
}
