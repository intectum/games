/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "animation.h"
#include "meshes.h"

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
      format.bone_weights_offset = format.size;

      format.components.emplace_back(std::pair { 'b', max_bone_weights_per_vertex });
      format.size += max_bone_weights_per_vertex * (sizeof(uint32_t) + sizeof(float));
    }

    return format;
  }

  void init(mesh& mesh, heap& indices, heap& vertices, uint32_t index_count, uint32_t vertex_count, uint8_t vertex_size)
  {
    mesh.id = next_id++;

    mesh.index_buffer = allocate(indices, index_count * sizeof(uint32_t));
    mesh.vertex_buffer = allocate(vertices, vertex_count * vertex_size, vertex_size);
    mesh.vertex_size = vertex_size;
  }

  void de_init(mesh& mesh, heap& indices, heap& vertices)
  {
    mesh.id = 0;

    if (mesh.index_buffer.data)
    {
      deallocate(indices, mesh.index_buffer);
    }

    if (mesh.vertex_buffer.data)
    {
      deallocate(vertices, mesh.vertex_buffer);
    }
  }
}
