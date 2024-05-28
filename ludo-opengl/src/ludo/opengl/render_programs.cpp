/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <fstream>
#include <iostream>

#include <ludo/animation.h>
#include <ludo/physics.h>

#include "default_shaders.h"
#include "shaders.h"

namespace ludo
{
  void init(render_program& render_program, const vertex_format& format, heap& render_commands, uint32_t instance_capacity)
  {
    render_program.format = format;

    if (!render_program.instance_size)
    {
      render_program.instance_size = sizeof(mat4);
      if (format.has_texture_coordinate)
      {
        render_program.instance_size += 16;
      }
      if (format.has_bone_weights)
      {
        render_program.instance_size += max_bones_per_armature * sizeof(mat4);
      }
    }

    auto vertex_shader_code = default_vertex_shader_code(format);
    auto fragment_shader_code = default_fragment_shader_code(format);
    init(render_program, vertex_shader_code, fragment_shader_code, render_commands, instance_capacity);
  }

  void init(render_program& render_program, const std::string& vertex_shader_file_name, const std::string& fragment_shader_file_name, heap& render_commands, uint32_t instance_capacity)
  {
    auto vertex_shader_code = std::ifstream(vertex_shader_file_name);
    auto fragment_shader_code = std::ifstream(fragment_shader_file_name);

    init(render_program, vertex_shader_code, fragment_shader_code, render_commands, instance_capacity);
  }

  void init(render_program& render_program, std::istream& vertex_shader_code, std::istream& fragment_shader_code, heap& render_commands, uint32_t instance_capacity)
  {
    init(render_program, vertex_shader_code, fragment_shader_code);

    render_program.command_buffer = allocate(render_commands, instance_capacity * sizeof(render_command));

    if (render_program.instance_size)
    {
      render_program.instance_buffer_front = allocate_vram(instance_capacity * render_program.instance_size);
      render_program.instance_buffer_back = allocate_heap(instance_capacity * render_program.instance_size);
    }
  }

  void init(render_program& render_program, std::istream& vertex_shader_code, std::istream& fragment_shader_code)
  {
    render_program.id = glCreateProgram(); check_opengl_error();

    auto vertex_shader = compile(vertex_shader_code, GL_VERTEX_SHADER);
    auto fragment_shader = compile(fragment_shader_code, GL_FRAGMENT_SHADER);

    glAttachShader(render_program.id, vertex_shader); check_opengl_error();
    glAttachShader(render_program.id, fragment_shader); check_opengl_error();
    glLinkProgram(render_program.id); check_opengl_error();

    auto link_status = GLint();
    glGetProgramiv(render_program.id, GL_LINK_STATUS, &link_status); check_opengl_error();

    GLchar info_log[1024];
    glGetProgramInfoLog(render_program.id, sizeof(info_log), nullptr, info_log); check_opengl_error();

    if (info_log[0])
    {
      std::cout << "render program link log: " << info_log << std::endl;
    }
    assert(link_status && "failed to link render program");

    glDeleteShader(vertex_shader); check_opengl_error();
    glDeleteShader(fragment_shader); check_opengl_error();
  }

  void de_init(render_program& render_program, heap& render_commands)
  {
    glDeleteProgram(render_program.id); check_opengl_error();
    render_program.id = 0;

    if (render_program.command_buffer.data)
    {
      deallocate(render_commands, render_program.command_buffer);
    }

    if (render_program.shader_buffer.back.data)
    {
      deallocate_dual(render_program.shader_buffer);
    }

    if (render_program.instance_buffer_front.data)
    {
      deallocate_vram(render_program.instance_buffer_front);
    }

    if (render_program.instance_buffer_back.data)
    {
      deallocate(render_program.instance_buffer_back);
    }
  }

  void commit(render_program& render_program)
  {
    commit(render_program.shader_buffer);
    std::memcpy(render_program.instance_buffer_front.data, render_program.instance_buffer_back.data, render_program.instance_buffer_front.size);
  }

  void use(render_program& render_program)
  {
    glValidateProgram(render_program.id); check_opengl_error();

    auto validate_status = GLint();
    glGetProgramiv(render_program.id, GL_VALIDATE_STATUS, &validate_status); check_opengl_error();

    GLchar info_log[1024];
    glGetProgramInfoLog(render_program.id, sizeof(info_log), nullptr, info_log); check_opengl_error();

    if (info_log[0])
    {
      std::cout << "render program validation log: " << info_log << std::endl;
    }
    assert(validate_status && "failed to validate render program");

    glUseProgram(render_program.id); check_opengl_error();

    if (render_program.push_on_bind)
    {
      commit(render_program);
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, render_program.shader_buffer.front.id); check_opengl_error();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, render_program.instance_buffer_front.id); check_opengl_error();

    // Convert b4 to u4f4
    auto format = render_program.format;
    for (auto index = 0; index < format.components.size(); index++)
    {
      if (format.components[index].first == 'b')
      {
        format.components[index].first = 'u';
        index++;
        format.components.insert(format.components.begin() + index, { 'f', max_bone_weights_per_vertex });
      }
    }

    auto offset = uint32_t(0);
    for (auto index = 0; index < format.components.size(); index++)
    {
      glEnableVertexAttribArray(index); check_opengl_error();

      if (format.components[index].first == 'i' || format.components[index].first == 'u')
      {
        glVertexAttribIPointer(
          index,
          static_cast<GLint>(format.components[index].second),
          format.components[index].first == 'i' ? GL_INT : GL_UNSIGNED_INT,
          static_cast<GLsizei>(format.size),
          reinterpret_cast<void*>(offset)
        ); check_opengl_error();

        offset += format.components[index].second * sizeof(uint32_t);
      }
      else
      {
        glVertexAttribPointer(
          index,
          static_cast<GLint>(format.components[index].second),
          GL_FLOAT,
          GL_FALSE,
          static_cast<GLsizei>(format.size),
          reinterpret_cast<void*>(offset)
        ); check_opengl_error();

        offset += format.components[index].second * sizeof(float);
      }
    }
  }

  void add_render_command(render_program& render_program, const render_mesh& render_mesh)
  {
    auto position = (render_program.active_commands.start + render_program.active_commands.count++) * sizeof(render_command);
    cast<render_command>(render_program.command_buffer, position) =
      {
        .index_count = render_mesh.indices.count,
        .instance_count = render_mesh.instances.count,
        .index_start = render_mesh.indices.start,
        .vertex_start = render_mesh.vertices.start,
        .instance_start = render_mesh.instances.start
      };
  }
}
