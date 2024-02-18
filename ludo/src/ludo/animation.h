/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ANIMATION_H
#define LUDO_ANIMATION_H

#include <vector>

#include "math/mat.h"
#include "math/transform.h"

namespace ludo
{
  struct armature
  {
    mat4 transform;

    int32_t bone_index = -1;
    mat4 bone_offset;

    std::vector<armature> children;
  };

  struct animation_node
  {
    int32_t bone_index = -1;

    std::vector<std::pair<float, vec3>> position_keyframes;
    std::vector<std::pair<float, quat>> rotation_keyframes;
    std::vector<std::pair<float, vec3>> scale_keyframes;
  };

  struct animation
  {
    std::string name;
    float ticks = 0.0f;
    float ticks_per_second = 0.0f;

    std::vector<animation_node> nodes;
  };

  const uint8_t max_bone_weights_per_vertex = 4;

  void interpolate(const animation& animation, const armature& armature, float time, mat4* final_transform_matrices);
}

#endif // LUDO_ANIMATION_H
