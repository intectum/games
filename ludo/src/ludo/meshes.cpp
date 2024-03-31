/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "animation.h"
#include "meshes.h"
#include "rendering.h"

namespace ludo
{
  vertex_format format(const vertex_format_options& options)
  {
    auto format = vertex_format();
    format.components.emplace_back('p');
    format.component_counts.emplace_back(3);
    format.size += 3 * sizeof(float);

    if (options.normals)
    {
      format.components.emplace_back('n');
      format.component_counts.emplace_back(3);
      format.size += 3 * sizeof(float);
    }

    if (options.colors)
    {
      format.components.emplace_back('c');
      format.component_counts.emplace_back(4);
      format.size += 4 * sizeof(float);
    }

    if (options.texture)
    {
      format.components.emplace_back('t');
      format.component_counts.emplace_back(2);
      format.size += 2 * sizeof(float);
    }

    if (options.bones)
    {
      format.components.emplace_back('u');
      format.component_counts.emplace_back(max_bone_weights_per_vertex);
      format.size += max_bone_weights_per_vertex * sizeof(uint32_t);
      format.components.emplace_back('f');
      format.component_counts.emplace_back(max_bone_weights_per_vertex);
      format.size += max_bone_weights_per_vertex * sizeof(float);
    }

    return format;
  }

  uint32_t count(const vertex_format& format, char component)
  {
    auto iter = std::find(format.components.begin(), format.components.end(), component);

    if (iter == format.components.end())
    {
      return 0;
    }

    return format.component_counts[iter - format.components.begin()];
  }

  uint32_t offset(const vertex_format& format, char component)
  {
    auto offset = uint32_t(0);

    for (auto component_index = 0; component_index < format.components.size() && format.components[component_index] != component; component_index++)
    {
      auto component_element_size = sizeof(float);
      if (format.components[component_index] == 'i' || format.components[component_index] == 'u')
      {
        component_element_size = sizeof(uint32_t);
      }

      offset += format.component_counts[component_index] * component_element_size;
    }

    return offset;
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
