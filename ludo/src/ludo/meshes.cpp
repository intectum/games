/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "animation.h"
#include "meshes.h"

namespace ludo
{
  bool operator<(const mesh& lhs, const mesh& rhs)
  {
    if (lhs.mesh_buffer_id < rhs.mesh_buffer_id)
    {
      return true;
    }

    if (lhs.mesh_buffer_id == rhs.mesh_buffer_id)
    {
      if (lhs.id < rhs.id)
      {
        return true;
      }
    }

    if (lhs.id == rhs.id)
    {
      if (lhs.instance_start < rhs.instance_start)
      {
        return true;
      }
    }

    return false;
  }

  template<>
  mesh* add(instance& instance, const mesh& init, const std::string& partition)
  {
    auto mesh = add(data<ludo::mesh>(instance), init, partition);
    mesh->id = next_id++;

    return mesh;
  }

  vertex_format format(const mesh_buffer_options& options)
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

    for (auto index = 0; index < options.texture_count; index++)
    {
      format.components.emplace_back('t');
      format.component_counts.emplace_back(2);
      format.size += 2 * sizeof(float);
    }

    if (options.bone_count)
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

  uint8_t offset(const vertex_format& format, char component)
  {
    auto offset = uint8_t(0);

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
}
