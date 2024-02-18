/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/rendering.h>

#include "built_ins.h"
#include "meshes.h"
#include "util.h"

namespace ludo
{
  template<>
  mesh_buffer* add(instance& instance, const mesh_buffer& init, const std::string& partition)
  {
    auto mesh = add(data<ludo::mesh_buffer>(instance), init, partition);

    auto name = GLuint();
    glGenVertexArrays(1, &name); check_opengl_error();
    mesh->id = name;

    return mesh;
  }

  mesh_buffer* add(instance& instance, const mesh_buffer& init, const mesh_buffer_options& options, const std::string& partition)
  {
    auto mesh_buffer = add(instance, init, partition);
    mesh_buffer->format = ludo::format(options);

    bind(*mesh_buffer);

    auto attribute_index = uint32_t(0);

    if (!mesh_buffer->render_program_id)
    {
      mesh_buffer->render_program_id = built_in_render_program(instance, options);
    }

    if (options.instance_count)
    {
      mesh_buffer->command_buffer = allocate_vram(options.instance_count * sizeof(draw_command));

      mesh_buffer->data_buffers.emplace_back(allocate_vram(options.instance_count * sizeof(mat4)));
      glBindBuffer(GL_ARRAY_BUFFER, mesh_buffer->data_buffers[0].id); check_opengl_error();

      auto offset = uint32_t(0);
      for (auto column_index = 0; column_index < 4; column_index++)
      {
        glEnableVertexAttribArray(attribute_index); check_opengl_error();
        glVertexAttribPointer(
          attribute_index,
          4,
          GL_FLOAT,
          GL_FALSE,
          16 * sizeof(float),
          reinterpret_cast<const void*>(offset)
        ); check_opengl_error();
        glVertexAttribDivisor(attribute_index, 1); check_opengl_error();

        offset += 4 * sizeof(float);
        attribute_index++;
      }

      auto byte_index = 0;
      for (auto instance_index = 0; instance_index < options.instance_count; instance_index++)
      {
        write(mesh_buffer->data_buffers[0], byte_index, mat4_identity);
        byte_index += sizeof(mat4);
      }
    }
    else
    {
      mesh_buffer->data_buffers.emplace_back();
    }

    if (options.index_count)
    {
      mesh_buffer->index_buffer = allocate_vram(options.index_count * sizeof(uint32_t));
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_buffer->index_buffer.id); check_opengl_error();
    }

    if (options.vertex_count)
    {
      mesh_buffer->vertex_buffer = allocate_vram(options.vertex_count * mesh_buffer->format.size);
      glBindBuffer(GL_ARRAY_BUFFER, mesh_buffer->vertex_buffer.id); check_opengl_error();
    }

    auto offset = uint32_t(0);
    for (auto component_index = 0; component_index < mesh_buffer->format.components.size(); component_index++)
    {
      glEnableVertexAttribArray(attribute_index); check_opengl_error();

      if (mesh_buffer->format.components[component_index] == 'i' || mesh_buffer->format.components[component_index] == 'u')
      {
        glVertexAttribIPointer(
          attribute_index,
          mesh_buffer->format.component_counts[component_index],
          mesh_buffer->format.components[component_index] == 'i' ? GL_INT : GL_UNSIGNED_INT,
          mesh_buffer->format.size,
          reinterpret_cast<const void*>(offset)
        ); check_opengl_error();

        offset += mesh_buffer->format.component_counts[component_index] * sizeof(uint32_t);
      }
      else
      {
        glVertexAttribPointer(
          attribute_index,
          mesh_buffer->format.component_counts[component_index],
          GL_FLOAT,
          GL_FALSE,
          mesh_buffer->format.size,
          reinterpret_cast<const void*>(offset)
        ); check_opengl_error();

        offset += mesh_buffer->format.component_counts[component_index] * sizeof(float);
      }

      attribute_index++;
    }

    if (options.texture_count)
    {
      mesh_buffer->data_buffers.emplace_back(allocate_vram(options.texture_count * sizeof(uint64_t)));
    }
    else
    {
      mesh_buffer->data_buffers.emplace_back();
    }

    if (options.bone_count)
    {
      mesh_buffer->data_buffers.emplace_back(allocate_vram(options.bone_count * sizeof(mat4)));

      auto byte_index = 0;
      for (auto bone_index = 0; bone_index < options.bone_count; bone_index++)
      {
        write(mesh_buffer->data_buffers[2], byte_index, mat4_identity);
        byte_index += sizeof(mat4);
      }
    }
    else
    {
      mesh_buffer->data_buffers.emplace_back();
    }

    // Unbind the vertex array so no funny business gets included in its state
    glBindVertexArray(0); check_opengl_error();

    return mesh_buffer;
  }

  template<>
  void remove<mesh_buffer>(instance& instance, mesh_buffer* element, const std::string& partition)
  {
    auto name = static_cast<GLuint>(element->id);
    glDeleteVertexArrays(1, &name); check_opengl_error();

    if (element->command_buffer.size)
    {
      deallocate_vram(element->command_buffer);
    }

    if (element->index_buffer.size)
    {
      deallocate_vram(element->index_buffer);
    }

    if (element->vertex_buffer.size)
    {
      deallocate_vram(element->vertex_buffer);
    }

    for (auto& data_buffer : element->data_buffers)
    {
      deallocate_vram(data_buffer);
    }

    remove(data<mesh_buffer>(instance), element, partition);
  }

  mat4 get_transform(const mesh_buffer& mesh_buffer, uint32_t instance_index)
  {
    return *reinterpret_cast<mat4*>(mesh_buffer.data_buffers[0].data + instance_index * sizeof(mat4));
  }

  void set_transform(mesh_buffer& mesh_buffer, uint32_t instance_index, const mat4& transform)
  {
    write(mesh_buffer.data_buffers[0], instance_index * sizeof(mat4), transform);
  }

  void set_texture(mesh_buffer& mesh_buffer, const texture& texture, uint8_t index)
  {
    write(mesh_buffer.data_buffers[1], index * sizeof(uint64_t), texture);
  }

  void bind(const mesh_buffer& mesh_buffer)
  {
    // The command buffer is not saved as part of the vertex array, so we need to bind it manually
    if (mesh_buffer.command_buffer.size)
    {
      glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mesh_buffer.command_buffer.id); check_opengl_error();
    }

    glBindVertexArray(mesh_buffer.id); check_opengl_error();

    // The data buffers are not saved as part of the vertex array, so we need to bind them manually
    for (auto index = 0; index < mesh_buffer.data_buffers.size(); index++)
    {
      if (index == 0)
      {
        continue;
      }

      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3 + index, mesh_buffer.data_buffers[index].id); check_opengl_error();
    }
  }
}
