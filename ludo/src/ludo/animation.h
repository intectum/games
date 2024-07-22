/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <vector>

#include "math/mat.h"
#include "math/transform.h"
#include "meshes.h"

namespace ludo
{
  const uint32_t max_bone_weights_per_vertex = 4; ///< The maximum number of bone weights that can be assigned to a vertex.
  const uint32_t max_bones_per_armature = 8; ///< The maximum number of bones that can be included in an armature.

  ///
  /// An armature.
  struct armature
  {
    uint64_t id; ///< The ID of this armature.

    mat4 transform; ///< The transform of this armature.

    int32_t bone_index = -1; ///< The index of the bone represented by this armature.
    mat4 bone_offset; ///< The offset to the bone represented by this armature.

    std::vector<armature> children; ///< The children of this armature.
  };

  ///
  /// A node in an animation.
  struct animation_node
  {
    int32_t bone_index = -1; ///< The index of the corresponding bone.

    std::vector<std::pair<float, vec3>> position_keyframes; ///< The position keyframes.
    std::vector<std::pair<float, quat>> rotation_keyframes; ///< The rotation keyframes.
    std::vector<std::pair<float, vec3>> scale_keyframes; ///< The scale keyframes.
  };

  ///
  /// An animation.
  struct animation
  {
    uint64_t id; ///< The ID of the animation.

    std::string name; ///< The name of the animation.
    float ticks = 0.0f; ///< The duration.
    float ticks_per_second = 0.0f; ///< The speed to play at.

    std::vector<animation_node> nodes; ///< The nodes that make up the animation.
  };

  ///
  /// Initializes an armature.
  /// \param armature The armature.
  void init(armature& armature);

  ///
  /// De-initializes an armature.
  /// \param armature The armature.
  void de_init(armature& armature);

  ///
  /// Initializes an animation.
  /// \param animation The animation.
  void init(animation& animation);

  ///
  /// De-initializes an animation.
  /// \param animation The animation.
  void de_init(animation& animation);

  ///
  /// Interpolates an animation.
  /// \param animation The animation to interpolate.
  /// \param armature The armature to interpolate the animation on.
  /// \param time The time within the animation to interpolate to.
  /// \param final_transforms The result of the interpolation.
  void interpolate(const animation& animation, const armature& armature, float time, mat4* final_transforms);
}
