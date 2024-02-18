/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <algorithm>
#include <cassert>
#include <cmath>

#include "animation.h"

namespace ludo
{
  void interpolate(const animation& animation, const armature& armature, float tick_time, const mat4& parent_transform_matrix, mat4* final_transform_matrices);
  vec3 interpolate_position(float tick_time, const animation_node& animation_node);
  quat interpolate_rotation(float tick_time, const animation_node& animation_node);
  vec3 interpolate_scale(float tick_time, const animation_node& animation_node);

  // This function is based on this tutorial for Assimp animation: https://ogldev.org/www/tutorial38/tutorial38.html
  void interpolate(const animation& animation, const armature& armature, float time, mat4* final_transform_matrices)
  {
    auto tick_time = std::fmod(time * animation.ticks_per_second, animation.ticks);

    interpolate(animation, armature, tick_time, mat4_identity, final_transform_matrices);
  }

  void interpolate(const animation& animation, const armature& armature, float tick_time, const mat4& parent_transform_matrix, mat4* final_transform_matrices)
  {
    auto transform_matrix = parent_transform_matrix;

    auto animation_node_iter = std::find_if(animation.nodes.begin(), animation.nodes.end(), [&armature](const ludo::animation_node& animation_node)
    {
      return animation_node.bone_index == armature.bone_index;
    });

    if (animation_node_iter != animation.nodes.end())
    {
      auto position_matrix = mat4(interpolate_position(tick_time, *animation_node_iter), mat3_identity);
      auto rotation_matrix = mat4(vec3_zero, mat3(interpolate_rotation(tick_time, *animation_node_iter)));
      auto scale_matrix = mat4_identity;
      scale_abs(scale_matrix, interpolate_scale(tick_time, *animation_node_iter));

      transform_matrix *= position_matrix * rotation_matrix * scale_matrix;

      final_transform_matrices[armature.bone_index] = transform_matrix * armature.bone_offset;
    }
    else
    {
      transform_matrix *= armature.transform;
    }

    for (auto& child : armature.children)
    {
      interpolate(animation, child, tick_time, transform_matrix, final_transform_matrices);
    }
  }

  vec3 interpolate_position(float tick_time, const animation_node& animation_node)
  {
    assert(!animation_node.position_keyframes.empty() && "the node does not have any position keyframes");

    if (animation_node.position_keyframes.size() == 1)
    {
      return animation_node.position_keyframes[0].second;
    }

    auto position_keyframe_index = 0;
    while (position_keyframe_index < animation_node.position_keyframes.size() - 1)
    {
      if (tick_time < animation_node.position_keyframes[position_keyframe_index + 1].first)
      {
        break;
      }

      position_keyframe_index++;
    }

    auto next_position_keyframe_index = position_keyframe_index + 1;
    assert(next_position_keyframe_index < animation_node.position_keyframes.size() && "the node did not contain any position keyframes before the current time");

    auto& position_keyframe = animation_node.position_keyframes[position_keyframe_index];
    auto& next_position_keyframe = animation_node.position_keyframes[next_position_keyframe_index];

    auto delta_time = next_position_keyframe.first - position_keyframe.first;
    auto keyframe_time = (tick_time - position_keyframe.first) / delta_time;

    return position_keyframe.second + (next_position_keyframe.second - position_keyframe.second) * keyframe_time;
  }

  quat interpolate_rotation(float tick_time, const animation_node& animation_node)
  {
    assert(!animation_node.rotation_keyframes.empty() && "the node does not have any rotation keyframes");

    if (animation_node.rotation_keyframes.size() == 1)
    {
      return animation_node.rotation_keyframes[0].second;
    }

    auto rotation_keyframe_index = 0;
    while (rotation_keyframe_index < animation_node.rotation_keyframes.size() - 1)
    {
      if (tick_time < animation_node.rotation_keyframes[rotation_keyframe_index + 1].first)
      {
        break;
      }

      rotation_keyframe_index++;
    }

    auto next_rotation_keyframe_index = rotation_keyframe_index + 1;
    assert(next_rotation_keyframe_index < animation_node.rotation_keyframes.size() && "the node did not contain any rotation keyframes before the current time");

    auto& rotation_keyframe = animation_node.rotation_keyframes[rotation_keyframe_index];
    auto& next_rotation_keyframe = animation_node.rotation_keyframes[next_rotation_keyframe_index];

    auto delta_time = next_rotation_keyframe.first - rotation_keyframe.first;
    auto keyframe_time = (tick_time - rotation_keyframe.first) / delta_time;

    return slerp(rotation_keyframe.second, next_rotation_keyframe.second, keyframe_time);
  }

  vec3 interpolate_scale(float tick_time, const animation_node& animation_node)
  {
    assert(!animation_node.scale_keyframes.empty() && "the node does not have any scale keyframes");

    if (animation_node.scale_keyframes.size() == 1)
    {
      return animation_node.scale_keyframes[0].second;
    }

    auto scale_keyframe_index = 0;
    while (scale_keyframe_index < animation_node.scale_keyframes.size() - 1)
    {
      if (tick_time < animation_node.scale_keyframes[scale_keyframe_index + 1].first)
      {
        break;
      }

      scale_keyframe_index++;
    }

    auto next_scale_keyframe_index = scale_keyframe_index + 1;
    assert(next_scale_keyframe_index < animation_node.scale_keyframes.size() && "the node did not contain any scale keyframes before the current time");

    auto& scale_keyframe = animation_node.scale_keyframes[scale_keyframe_index];
    auto& next_scale_keyframe = animation_node.scale_keyframes[next_scale_keyframe_index];

    auto delta_time = next_scale_keyframe.first - scale_keyframe.first;
    auto keyframe_time = (tick_time - scale_keyframe.first) / delta_time;

    return scale_keyframe.second + (next_scale_keyframe.second - scale_keyframe.second) * keyframe_time;
  }
}
