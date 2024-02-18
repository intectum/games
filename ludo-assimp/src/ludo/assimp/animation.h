/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ASSIMP_ANIMATION_H
#define LUDO_ASSIMP_ANIMATION_H

#include <assimp/anim.h>

#include <ludo/animation.h>

namespace ludo
{
  animation_node to_animation_node(const aiNodeAnim& original, int32_t bone_index);
}

#endif // LUDO_ASSIMP_ANIMATION_H
