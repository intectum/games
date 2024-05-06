/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/animation.h>
#include <ludo/importing.h>
#include <ludo/meshes.h>

#include "animation.h"
#include "math.h"
#include "meshes.h"

namespace ludo
{
  void write_mesh_data(mesh& mesh, const aiScene& assimp_scene, const aiMesh& assimp_mesh, const mat4& transform, uint32_t index_start, uint32_t vertex_start);

  const auto bone_data_size = max_bone_weights_per_vertex * sizeof(uint32_t) + max_bone_weights_per_vertex * sizeof(float);

  std::vector<mesh> import_meshes(instance& instance, const render_program& render_program, const aiScene& assimp_scene, const std::vector<import_object>& mesh_objects, const std::vector<ludo::texture*>& textures, const import_options& options, const std::string& partition)
  {
    auto meshes = std::vector<mesh>();

    if (options.merge_meshes)
    {
      auto mesh_counts = import_counts(assimp_scene, mesh_objects);
      auto texture_count = std::count_if(textures.begin(), textures.end(), [](const ludo::texture* texture) { return texture != nullptr; });

      auto mesh = add(
        instance,
        ludo::mesh { .render_program_id = render_program.id },
        mesh_counts.first,
        mesh_counts.second,
        render_program.format.size,
        partition
      );

      meshes.push_back(*mesh);
    }

    auto index_start = uint32_t(0);
    auto vertex_start = uint32_t(0);
    for (auto index = 0; index < mesh_objects.size(); index++)
    {
      auto& mesh_object = mesh_objects[index];
      auto& assimp_mesh = *assimp_scene.mMeshes[mesh_object.mesh_index];

      auto index_count = assimp_mesh.mNumFaces * assimp_mesh.mFaces[0].mNumIndices; // We use aiProcess_SortByPType so all faces should have the same number of indices.
      auto vertex_count = assimp_mesh.mNumVertices;

      if (options.merge_meshes)
      {
        write_mesh_data(meshes[0], assimp_scene, assimp_mesh, mesh_object.transform, index_start, vertex_start);

        index_start += index_count;
        vertex_start += vertex_count;
      }
      else
      {
        auto mesh = add(
          instance,
          ludo::mesh { .render_program_id = render_program.id, .texture_id = textures[index] ? textures[index]->id : 0 },
          index_count,
          vertex_count,
          render_program.format.size,
          partition
        );

        write_mesh_data(*mesh, assimp_scene, assimp_mesh, mesh_object.transform, 0, 0);

        if (assimp_mesh.mNumBones)
        {
          mesh->armature_id = import_armature(instance, assimp_scene, assimp_mesh, partition);
          mesh->animation_ids = import_animations(instance, assimp_scene, assimp_mesh, partition);
        }

        meshes.push_back(*mesh);
      }
    }

    return meshes;
  }

  void write_mesh_data(mesh& mesh, const aiScene& assimp_scene, const aiMesh& assimp_mesh, const mat4& transform, uint32_t index_start, uint32_t vertex_start)
  {
    auto has_colors = assimp_mesh.GetNumColorChannels() > 0;
    auto has_bones = assimp_mesh.mNumBones > 0;
    auto texture_path = aiString();
    assimp_scene.mMaterials[assimp_mesh.mMaterialIndex]->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path);
    auto has_texture = texture_path.length > 0;

    auto bones_offset = sizeof(vec3) + sizeof(vec3) + (has_colors ? sizeof(vec4) : 0) + (has_texture ? sizeof(vec2) : 0);
    auto vertex_size = bones_offset;
    vertex_size += has_bones ? bone_data_size : 0;

    auto index_stream = ludo::stream(mesh.index_buffer, index_start * sizeof(uint32_t));
    for (auto face_index = 0; face_index < assimp_mesh.mNumFaces; face_index++)
    {
      auto& assimp_face = assimp_mesh.mFaces[face_index];
      for (auto index_index = 0; index_index < assimp_face.mNumIndices; index_index++)
      {
        write(index_stream, vertex_start + assimp_face.mIndices[index_index]);
      }
    }

    auto vertex_stream = ludo::stream(mesh.vertex_buffer, vertex_start * vertex_size);
    for (auto vertex_index = 0; vertex_index < assimp_mesh.mNumVertices; vertex_index++)
    {
      auto position = vec3(transform * vec4(to_vec3(assimp_mesh.mVertices[vertex_index])));
      write(vertex_stream, position);

      auto normal = mat3(transform) * to_vec3(assimp_mesh.mNormals[vertex_index]);
      write(vertex_stream, normal);

      if (has_colors)
      {
        write(vertex_stream, to_vec4(assimp_mesh.mColors[0][vertex_index]));
      }

      if (has_texture)
      {
        write(vertex_stream, to_vec2(assimp_mesh.mTextureCoords[0][vertex_index]));
      }

      if (has_bones)
      {
        // Initialize bone indices and weights to 0
        std::memset(mesh.vertex_buffer.data + vertex_stream.position, 0, bone_data_size);
        vertex_stream.position += bone_data_size;
      }
    }

    if (!has_bones)
    {
      return;
    }

    for (auto bone_index = 0u; bone_index < assimp_mesh.mNumBones; bone_index++)
    {
      auto assimp_bone = assimp_mesh.mBones[bone_index];
      for (auto weight_index = 0 ; weight_index < assimp_bone->mNumWeights ; weight_index++)
      {
        auto& assimp_vertex_weight = assimp_bone->mWeights[weight_index];

        auto first_bone_index_byte_index = (vertex_start + assimp_vertex_weight.mVertexId) * vertex_size + bones_offset;
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
