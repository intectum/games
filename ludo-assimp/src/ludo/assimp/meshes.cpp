/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/animation.h>
#include <ludo/importing.h>
#include <ludo/meshes.h>

#include "animation.h"
#include "math.h"
#include "meshes.h"
#include "textures.h"

namespace ludo
{
  vertex_format format(const aiScene& assimp_scene, const aiMesh& assimp_mesh);
  void write_mesh_data(mesh& mesh, const aiMesh& assimp_mesh, const vertex_format& format, const mat4& transform, uint32_t index_start, uint32_t vertex_start);

  const auto bone_data_size = max_bone_weights_per_vertex * sizeof(uint32_t) + max_bone_weights_per_vertex * sizeof(float);

  void import_meshes(import_results& results, heap& indices, heap& vertices, const std::string& folder, const aiScene& assimp_scene, const std::vector<import_object>& mesh_objects, const import_options& options)
  {
    if (options.merge_meshes && !mesh_objects.empty())
    {
      auto mesh_counts = import_counts(assimp_scene, mesh_objects);
      // TODO could maybe combine from all meshes? Not really sure what the best thing to do here is...
      auto format = ludo::format(assimp_scene, *assimp_scene.mMeshes[0]);

      auto mesh = ludo::mesh();
      init(mesh, indices, vertices, mesh_counts.first, mesh_counts.second, format.size);

      results.meshes.push_back(mesh);
    }

    auto index_start = uint32_t(0);
    auto vertex_start = uint32_t(0);
    for (auto index = 0; index < mesh_objects.size(); index++)
    {
      auto& mesh_object = mesh_objects[index];
      auto& assimp_mesh = *assimp_scene.mMeshes[mesh_object.mesh_index];
      auto format = ludo::format(assimp_scene, assimp_mesh);

      auto index_count = assimp_mesh.mNumFaces * assimp_mesh.mFaces[0].mNumIndices; // We use aiProcess_SortByPType so all faces should have the same number of indices.
      auto vertex_count = assimp_mesh.mNumVertices;

      if (options.merge_meshes)
      {
        write_mesh_data(results.meshes[0], assimp_mesh, format, mesh_object.transform, index_start, vertex_start);

        index_start += index_count;
        vertex_start += vertex_count;
      }
      else
      {
        auto mesh = ludo::mesh();
        init(mesh, indices, vertices, index_count, vertex_count, format.size);
        write_mesh_data(mesh, assimp_mesh, format, mesh_object.transform, 0, 0);

        auto texture = import_texture(folder, assimp_scene, mesh_object);
        if (texture.id)
        {
          results.textures.push_back(texture);
          mesh.texture_id = texture.id;
        }

        if (assimp_mesh.mNumBones)
        {
          auto armature = import_armature(assimp_scene, assimp_mesh);
          if (armature.id)
          {
            results.armatures.push_back(armature);
            mesh.armature_id = armature.id;
          }

          auto animations = import_animations(assimp_scene, assimp_mesh);
          if (!animations.empty())
          {
            results.animations.insert(results.animations.end(), animations.begin(), animations.end());
            for (auto& animation : animations)
            {
              mesh.animation_ids.push_back(animation.id);
            }
          }
        }

        results.meshes.push_back(mesh);
      }
    }
  }

  vertex_format format(const aiScene& assimp_scene, const aiMesh& assimp_mesh)
  {
    assert(assimp_mesh.mNumBones <= max_bones_per_armature && "max bone count exceeded");

    auto texture_path = aiString();
    assimp_scene.mMaterials[assimp_mesh.mMaterialIndex]->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path);

    return format(
      true,
      assimp_mesh.GetNumColorChannels() > 0,
      texture_path.length > 0,
      assimp_mesh.mNumBones > 0
    );
  }

  void write_mesh_data(mesh& mesh, const aiMesh& assimp_mesh, const vertex_format& format, const mat4& transform, uint32_t index_start, uint32_t vertex_start)
  {
    auto index_stream = stream(mesh.index_buffer, index_start * sizeof(uint32_t));
    for (auto face_index = 0; face_index < assimp_mesh.mNumFaces; face_index++)
    {
      auto& assimp_face = assimp_mesh.mFaces[face_index];
      for (auto index_index = 0; index_index < assimp_face.mNumIndices; index_index++)
      {
        write(index_stream, vertex_start + assimp_face.mIndices[index_index]);
      }
    }

    auto vertex_stream = stream(mesh.vertex_buffer, vertex_start * format.size);
    for (auto vertex_index = 0; vertex_index < assimp_mesh.mNumVertices; vertex_index++)
    {
      auto position = vec3(transform * vec4(to_vec3(assimp_mesh.mVertices[vertex_index])));
      write(vertex_stream, position);

      auto normal = mat3(transform) * to_vec3(assimp_mesh.mNormals[vertex_index]);
      write(vertex_stream, normal);

      if (format.has_color)
      {
        write(vertex_stream, to_vec4(assimp_mesh.mColors[0][vertex_index]));
      }

      if (format.has_texture_coordinate)
      {
        write(vertex_stream, to_vec2(assimp_mesh.mTextureCoords[0][vertex_index]));
      }

      if (format.has_bone_weights)
      {
        // Initialize bone indices and weights to 0
        std::memset(mesh.vertex_buffer.data + vertex_stream.position, 0, bone_data_size);
        vertex_stream.position += bone_data_size;
      }
    }

    if (!format.has_bone_weights)
    {
      return;
    }

    for (auto bone_index = 0u; bone_index < assimp_mesh.mNumBones; bone_index++)
    {
      auto assimp_bone = assimp_mesh.mBones[bone_index];
      for (auto weight_index = 0 ; weight_index < assimp_bone->mNumWeights ; weight_index++)
      {
        auto& assimp_vertex_weight = assimp_bone->mWeights[weight_index];

        auto first_bone_index_byte_index = (vertex_start + assimp_vertex_weight.mVertexId) * format.size + format.bone_weights_offset;
        auto bone_index_byte_index = first_bone_index_byte_index;
        auto first_bone_weight_byte_index = first_bone_index_byte_index + max_bone_weights_per_vertex * sizeof(uint32_t);
        auto bone_weight_byte_index = first_bone_weight_byte_index;
        while (cast<float>(mesh.vertex_buffer, bone_weight_byte_index) != 0.0f)
        {
          bone_index_byte_index += sizeof(uint32_t);
          bone_weight_byte_index += sizeof(float);

          assert(bone_index_byte_index < first_bone_weight_byte_index && "the maximum bone weights per vertex has been exceeded");
        }

        cast<uint32_t>(mesh.vertex_buffer, bone_index_byte_index) = bone_index;
        cast<float>(mesh.vertex_buffer, bone_weight_byte_index) = assimp_vertex_weight.mWeight;
      }
    }
  }
}
