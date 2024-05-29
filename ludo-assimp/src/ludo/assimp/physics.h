/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ASSIMP_PHYSICS_H
#define LUDO_ASSIMP_PHYSICS_H

#include <assimp/scene.h>

#include <ludo/importing.h>
#include <ludo/physics.h>

#include "util.h"

namespace ludo
{
  void import_body_shapes(import_results& results, const aiScene& assimp_scene, const std::vector<import_object>& rigid_body_objects);
}

#endif // LUDO_ASSIMP_PHYSICS_H
