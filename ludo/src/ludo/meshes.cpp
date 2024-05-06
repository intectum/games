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

    auto& indices = data_heap(instance, "ludo::vram_indices");
    mesh->index_buffer = allocate(indices, index_count * sizeof(uint32_t));

    auto& vertices = data_heap(instance, "ludo::vram_vertices");
    mesh->vertex_buffer = allocate(vertices, vertex_count * vertex_size, vertex_size);

    mesh->vertex_size = vertex_size;

    return mesh;
  }

  template<>
  void remove<mesh>(instance& instance, mesh* element, const std::string& partition)
  {
    if (element->index_buffer.data)
    {
      auto& indices = data_heap(instance, "ludo::vram_indices");
      deallocate(indices, element->index_buffer);
    }

    if (element->vertex_buffer.data)
    {
      auto& vertices = data_heap(instance, "ludo::vram_vertices");
      deallocate(vertices, element->vertex_buffer);
    }

    remove(data<mesh>(instance), element, partition);
  }

  mesh_instance* add(instance& instance, const mesh_instance& init, const mesh& mesh, const std::string& partition)
  {
    auto mesh_instance = add(instance, init, partition);

    mesh_instance->mesh_id = mesh.id; // TODO remove mesh_id from mesh_instance?
    mesh_instance->render_program_id = init.render_program_id ? init.render_program_id : mesh.render_program_id; // TODO remove render_program_id from mesh?

    auto& indices = data_heap(instance, "ludo::vram_indices");
    mesh_instance->indices.start = (mesh.index_buffer.data - indices.data) / sizeof(uint32_t);
    mesh_instance->indices.count = mesh.index_buffer.size / sizeof(uint32_t);

    auto& vertices = data_heap(instance, "ludo::vram_vertices");
    mesh_instance->vertices.start = (mesh.vertex_buffer.data - vertices.data) / mesh.vertex_size;
    mesh_instance->vertices.count = mesh.vertex_buffer.size / mesh.vertex_size;

    if (mesh_instance->render_program_id)
    {
      auto render_program = get<ludo::render_program>(instance, mesh_instance->render_program_id);
      mesh_instance->instance_buffer = allocate(render_program->instance_buffer_back, render_program->instance_size);

      instance_transform(*mesh_instance) = mat4_identity;

      if (mesh.texture_id)
      {
        set_texture(*mesh_instance, texture { .id = mesh.texture_id });
      }

      if (mesh.armature_id)
      {
        std::array<mat4, max_bones_per_armature> bone_transforms;
        bone_transforms.fill(mat4_identity);

        set_bone_transforms(*mesh_instance, bone_transforms);
      }
    }

    return mesh_instance;
  }

  template<>
  void remove<mesh_instance>(instance& instance, mesh_instance* element, const std::string& partition)
  {
    if (element->render_program_id && element->instance_buffer.data)
    {
      auto render_program = get<ludo::render_program>(instance, element->render_program_id);
      deallocate(render_program->instance_buffer_back, element->instance_buffer);
    }

    remove(data<mesh_instance>(instance), element, partition);
  }

  mat4& instance_transform(mesh_instance& mesh_instance)
  {
    return cast<mat4>(mesh_instance.instance_buffer, 0);
  }

  const mat4& instance_transform(const mesh_instance& mesh_instance)
  {
    return cast<const mat4>(mesh_instance.instance_buffer, 0);
  }
}
