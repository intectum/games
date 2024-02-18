/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "animation.h"
#include "math.h"

namespace ludo
{
  animation_node to_animation_node(const aiNodeAnim& original, int32_t bone_index)
  {
    auto node = animation_node { .bone_index = bone_index };

    for (auto position_key_index = 0; position_key_index < original.mNumPositionKeys; position_key_index++)
    {
      auto& assimp_position_key = original.mPositionKeys[position_key_index];
      node.position_keyframes.emplace_back(std::pair<float, vec3>
      {
        static_cast<float>(assimp_position_key.mTime),
        to_vec3(assimp_position_key.mValue)
      });
    }

    for (auto rotation_key_index = 0; rotation_key_index < original.mNumRotationKeys; rotation_key_index++)
    {
      auto& assimp_rotation_key = original.mRotationKeys[rotation_key_index];
      node.rotation_keyframes.emplace_back(std::pair<float, quat>
      {
        static_cast<float>(assimp_rotation_key.mTime),
        to_quat(assimp_rotation_key.mValue)
      });
    }

    for (auto scale_key_index = 0; scale_key_index < original.mNumScalingKeys; scale_key_index++)
    {
      auto& assimp_scale_key = original.mScalingKeys[scale_key_index];
      node.scale_keyframes.emplace_back(std::pair<float, vec3>
      {
        static_cast<float>(assimp_scale_key.mTime),
        to_vec3(assimp_scale_key.mValue)
      });
    }

    return node;
  }
}
