/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/animation.h>

#include "animation.h"
#include "math.h"

namespace ludo
{
  void find_bone_path(const aiNode& assimp_node, const aiMesh& assimp_mesh, std::unordered_map<const aiNode*, bool>& bone_path);
  int32_t find_bone_index(const aiMesh& assimp_mesh, const std::string& name);
  animation_node to_animation_node(const aiNodeAnim& original, int32_t bone_index);
  armature to_armature(const aiNode& assimp_node, const aiMesh& assimp_mesh, std::unordered_map<const aiNode*, bool>& bone_path, const mat4& root_matrix = mat4_identity, bool root_found = false);

  armature import_armature(const aiScene& assimp_scene, const aiMesh& assimp_mesh)
  {
    auto bone_path = std::unordered_map<const aiNode*, bool>();
    find_bone_path(*assimp_scene.mRootNode, assimp_mesh, bone_path);

    auto bone_path_iter = std::find_if(bone_path.begin(), bone_path.end(), [](const std::pair<const aiNode*, bool>& entry)
    {
      return entry.second;
    });

    if (bone_path_iter == bone_path.end())
    {
      return {};
    }

    return to_armature(*assimp_scene.mRootNode, assimp_mesh, bone_path);
  }

  std::vector<animation> import_animations(const aiScene& assimp_scene, const aiMesh& assimp_mesh)
  {
    auto animations = std::vector<animation>();

    for (auto index = 0; index < assimp_scene.mNumAnimations; index++)
    {
      auto assimp_animation = assimp_scene.mAnimations[index];
      auto nodes = std::vector<animation_node>();

      for (auto node_anim_index = 0; node_anim_index < assimp_animation->mNumChannels; node_anim_index++)
      {
        auto assimp_node_anim = assimp_animation->mChannels[node_anim_index];
        auto node = animation_node();

        nodes.emplace_back(to_animation_node(*assimp_node_anim, find_bone_index(assimp_mesh, std::string(assimp_node_anim->mNodeName.C_Str()))));
      }

      auto animation = ludo::animation
      {
        .name = assimp_animation->mName.C_Str(),
        .ticks = static_cast<float>(assimp_animation->mDuration),
        // Sometimes Assimp cannot determine the ticks per second, we default to 24 in this case (the default from Blender)
        .ticks_per_second = static_cast<float>(assimp_animation->mTicksPerSecond == 0.0 ? 24.0 : assimp_animation->mTicksPerSecond),
        .nodes = nodes
      };
      init(animation);

      animations.push_back(animation);
    }

    return animations;
  }

  void find_bone_path(const aiNode& assimp_node, const aiMesh& assimp_mesh, std::unordered_map<const aiNode*, bool>& bone_path)
  {
    for (auto index = 0; index < assimp_node.mNumChildren; index++)
    {
      auto& assimp_child_node = *assimp_node.mChildren[index];
      find_bone_path(assimp_child_node, assimp_mesh, bone_path);
      if (bone_path[&assimp_child_node])
      {
        bone_path[&assimp_node] = true;
      }
    }

    if (bone_path[&assimp_node])
    {
      return;
    }

    bone_path[&assimp_node] = find_bone_index(assimp_mesh, std::string(assimp_node.mName.C_Str())) != -1;
  }

  int32_t find_bone_index(const aiMesh& assimp_mesh, const std::string& name)
  {
    for (auto index = 0; index < assimp_mesh.mNumBones; index++)
    {
      auto assimp_bone = assimp_mesh.mBones[index];
      if (assimp_bone->mName.C_Str() == name)
      {
        return index;
      }
    }

    return -1;
  }

  armature to_armature(const aiNode& assimp_node, const aiMesh& assimp_mesh, std::unordered_map<const aiNode*, bool>& bone_path, const mat4& root_matrix, bool root_found)
  {
    auto bone_index = find_bone_index(assimp_mesh, std::string(assimp_node.mName.C_Str()));

    if (bone_index == -1 && !root_found)
    {
      auto root_bone_node = static_cast<aiNode*>(nullptr);
      for (auto index = 0; index < assimp_node.mNumChildren; index++)
      {
        auto child = assimp_node.mChildren[index];
        if (bone_path[child])
        {
          assert(!root_bone_node && "multiple root bones found");
          root_bone_node = child;
        }
      }

      return to_armature(*root_bone_node, assimp_mesh, bone_path, root_matrix * to_mat4(assimp_node.mTransformation));
    }

    auto node = armature { .transform = to_mat4(assimp_node.mTransformation) };
    init(node);

    if (!root_found)
    {
      // We need to apply the combined ancestor matrices as well to get the root of the armature in global space.
      // That way we can discard the ancestor nodes/matrices.
      node.transform = root_matrix * node.transform;
    }

    if (bone_index != -1)
    {
      node.bone_index = bone_index;

      // The bone offset matrices are in global space.
      // Since we transformed our mesh from mesh space -> global space already, we need to undo that transformation here.
      // If we don't it will result in a 'double application' of the transformation from mesh space -> global space.
      // Here I am actually transforming from global space -> armature space, which is the same in my example file.
      // I don't think this is actually correct since the bones are actually part of the mesh...
      auto root_matrix_inverse = root_matrix;
      invert(root_matrix_inverse);
      node.bone_offset = to_mat4(assimp_mesh.mBones[bone_index]->mOffsetMatrix) * root_matrix_inverse;
    }

    for (auto index = 0; index < assimp_node.mNumChildren; index++)
    {
      auto child = assimp_node.mChildren[index];
      if (bone_path[child])
      {
        node.children.emplace_back(to_armature(*child, assimp_mesh, bone_path, root_matrix, true));
      }
    }

    return node;
  }

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
