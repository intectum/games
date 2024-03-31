/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ANIMATION_H
#define LUDO_ANIMATION_H

#include <vector>

#include "data.h"
#include "math/mat.h"
#include "math/transform.h"

namespace ludo
{
  const uint32_t max_bone_weights_per_vertex = 4;
  const uint32_t max_bones_per_armature = 8;

  struct LUDO_API armature
  {
    uint64_t id; ///< The ID of the armature.

    mat4 transform;

    int32_t bone_index = -1;
    mat4 bone_offset;

    std::vector<armature> children;
  };

  struct LUDO_API armature_instance
  {
    uint64_t id; ///< The ID of the armature instance.

    mat4 transforms[max_bones_per_armature];
  };

  struct LUDO_API animation_node
  {
    int32_t bone_index = -1;

    std::vector<std::pair<float, vec3>> position_keyframes;
    std::vector<std::pair<float, quat>> rotation_keyframes;
    std::vector<std::pair<float, vec3>> scale_keyframes;
  };

  struct LUDO_API animation
  {
    uint64_t id; ///< The ID of the animation.

    std::string name;
    float ticks = 0.0f;
    float ticks_per_second = 0.0f;

    std::vector<animation_node> nodes;
  };

  LUDO_API void interpolate(const animation& animation, const armature& armature, float time, mat4* final_transforms);

  template<>
  LUDO_API armature_instance* add(instance& instance, const armature_instance& init, const std::string& partition);
}

#endif // LUDO_ANIMATION_H
