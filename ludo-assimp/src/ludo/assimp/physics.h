/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ASSIMP_PHYSICS_H
#define LUDO_ASSIMP_PHYSICS_H

#include <assimp/scene.h>

#include <ludo/core.h>

#include "util.h"

namespace ludo
{
  void import_body_shape(instance& instance, const aiScene& assimp_scene, const import_object& rigid_body_object, const std::string& partition);
}

#endif // LUDO_ASSIMP_PHYSICS_H
