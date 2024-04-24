/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "animation.h"
#include "meshes.h"
#include "rendering.h"

namespace ludo
{
  vertex_format format(bool normal, bool color, bool texture_coordinate, bool bone_weights)
  {
    auto format = vertex_format();

    format.components.emplace_back(std::pair { 'p', 3 });
    format.size += 3 * sizeof(float);

    if (normal)
    {
      format.has_normal = true;
      format.normal_offset = format.size;

      format.components.emplace_back(std::pair { 'n', 3 });
      format.size += 3 * sizeof(float);
    }

    if (color)
    {
      format.has_color = true;
      format.color_offset = format.size;

      format.components.emplace_back(std::pair { 'c', 4 });
      format.size += 4 * sizeof(float);
    }

    if (texture_coordinate)
    {
      format.has_texture_coordinate = true;
      format.texture_coordinate_offset = format.size;

      format.components.emplace_back(std::pair { 't', 2 });
      format.size += 2 * sizeof(float);
    }

    if (bone_weights)
    {
      format.has_bone_weights = true;

      format.components.emplace_back(std::pair { 'b', max_bone_weights_per_vertex });
      format.size += max_bone_weights_per_vertex * (sizeof(uint32_t) + sizeof(float));
    }

    return format;
  }

  mesh* add(instance& instance, const mesh& init, uint32_t index_count, uint32_t vertex_count, uint8_t vertex_size, const std::string& partition)
  {
    auto mesh = add(instance, init, partition);

    auto& indices = data_heap<index_t>(instance);
    mesh->index_buffer = allocate(indices, index_count * sizeof(uint32_t));

    auto& vertices = data_heap<vertex_t>(instance);
    mesh->vertex_buffer = allocate(vertices, vertex_count * vertex_size, vertex_size);

    return mesh;
  }

  template<>
  void remove<mesh>(instance& instance, mesh* element, const std::string& partition)
  {
    if (element->index_buffer.data)
    {
      auto& indices = data_heap<index_t>(instance);
      deallocate(indices, element->index_buffer);
    }

    if (element->vertex_buffer.data)
    {
      auto& vertices = data_heap<vertex_t>(instance);
      deallocate(vertices, element->vertex_buffer);
    }

    remove(data<mesh>(instance), element, partition);
  }

  mesh_instance* add(instance& instance, const mesh_instance& init, const mesh& mesh, const std::string& partition)
  {
    auto mesh_instance = add(instance, init, partition);

    mesh_instance->mesh_id = mesh.id;
    mesh_instance->render_program_id = init.render_program_id ? init.render_program_id : mesh.render_program_id;
    mesh_instance->texture_id = init.texture_id ? init.texture_id : mesh.texture_id;
    mesh_instance->index_buffer = mesh.index_buffer;
    mesh_instance->vertex_buffer = mesh.vertex_buffer;

    if (mesh.armature_id && !mesh_instance->armature_instance_id)
    {
      mesh_instance->armature_instance_id = add(instance, armature_instance(), partition)->id;
    }

    return mesh_instance;
  }
}
