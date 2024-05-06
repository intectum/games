/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <ludo/animation.h>

#include "math.h"
#include "meshes.h"
#include "physics.h"
#include "textures.h"
#include "util.h"

namespace ludo
{
  void find_objects(const aiNode& assimp_node, std::vector<import_object>& mesh_objects, std::vector<import_object>& rigid_body_objects, const mat4& parent_transform, bool rigid_body_shapes);
  void validate(const aiScene& assimp_scene, const std::vector<import_object>& mesh_objects);

  std::vector<mesh> import(instance& instance, const std::string& file_name, const import_options& options, const std::string& partition)
  {
    Assimp::Importer importer;
    auto assimp_scene = importer.ReadFile(file_name, aiProcessPreset_TargetRealtime_MaxQuality);
    if (assimp_scene == nullptr)
    {
      std::cout << "Assimp error: " << importer.GetErrorString() << std::endl;
      assert(false && "Assimp error");
      return {};
    }

    auto mesh_objects = std::vector<import_object>();
    auto rigid_body_objects = std::vector<import_object>();
    find_objects(*assimp_scene->mRootNode, mesh_objects, rigid_body_objects, mat4_identity, false);
    validate(*assimp_scene, mesh_objects);

    for (auto& rigid_body_object : rigid_body_objects)
    {
      import_body_shape(instance, *assimp_scene, rigid_body_object, partition);
    }

    if (!mesh_objects.empty())
    {
      auto folder = file_name.substr(0, file_name.find_last_of('/') + 1);
      auto textures = import_textures(instance, folder, *assimp_scene, mesh_objects, partition);
      return import_meshes(instance, *assimp_scene, mesh_objects, textures, options, partition);
    }

    return {};
  }

  std::pair<uint32_t, uint32_t> import_counts(const std::string& file_name)
  {
    Assimp::Importer importer;
    auto assimp_scene = importer.ReadFile(file_name, aiProcessPreset_TargetRealtime_MaxQuality);
    if (assimp_scene == nullptr)
    {
      std::cout << "Assimp error: " << importer.GetErrorString() << std::endl;
      assert(false && "Assimp error");
      return { 0, 0 };
    }

    auto mesh_objects = std::vector<import_object>();
    auto rigid_body_objects = std::vector<import_object>();
    find_objects(*assimp_scene->mRootNode, mesh_objects, rigid_body_objects, mat4_identity, false);
    validate(*assimp_scene, mesh_objects);

    return import_counts(*assimp_scene, mesh_objects);
  }

  void find_objects(const aiNode& assimp_node, std::vector<import_object>& mesh_objects, std::vector<import_object>& rigid_body_objects, const mat4& parent_transform, bool rigid_body_shapes)
  {
    auto transform = parent_transform * to_mat4(assimp_node.mTransformation);
    rigid_body_shapes = rigid_body_shapes || std::string(assimp_node.mName.C_Str()).find("RigidBody") != std::string::npos;

    for (auto index = uint32_t(0); index < assimp_node.mNumMeshes; index++)
    {
      if (rigid_body_shapes)
      {
        rigid_body_objects.emplace_back(import_object { .mesh_index = assimp_node.mMeshes[index], .transform = transform });
      }
      else
      {
        mesh_objects.emplace_back(import_object { .mesh_index = assimp_node.mMeshes[index], .transform = transform });
      }
    }

    for (auto index = 0; index < assimp_node.mNumChildren; index++)
    {
      find_objects(*assimp_node.mChildren[index], mesh_objects, rigid_body_objects, transform, rigid_body_shapes);
    }
  }

  void validate(const aiScene& assimp_scene, const std::vector<import_object>& mesh_objects)
  {
    auto& first_assimp_mesh = *assimp_scene.mMeshes[mesh_objects[0].mesh_index];
    auto assimp_primitive = first_assimp_mesh.mPrimitiveTypes; // We use aiProcess_SortByPType so all primitives should be the same.
    auto assimp_material = assimp_scene.mMaterials[first_assimp_mesh.mMaterialIndex];

    for (auto& mesh_object : mesh_objects)
    {
      auto& assimp_mesh = *assimp_scene.mMeshes[mesh_object.mesh_index];

      assert(assimp_mesh.mPrimitiveTypes != aiPrimitiveType_POLYGON && "primitives must be points, lines or triangles");
      assert(assimp_mesh.mPrimitiveTypes == assimp_primitive && "meshes must have the same primitive");
      assert(assimp_scene.mMaterials[assimp_mesh.mMaterialIndex] == assimp_material && "meshes must have the same material");
    }
  }
}
