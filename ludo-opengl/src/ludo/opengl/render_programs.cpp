/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <fstream>
#include <iostream>

#include <ludo/animation.h>

#include "render_programs.h"
#include "util.h"

namespace ludo
{
  template<>
  render_program* add(instance& instance, const render_program& init, const std::string& partition)
  {
    auto render_program = add(data<ludo::render_program>(instance), init, partition);
    render_program->id = glCreateProgram(); check_opengl_error();

    glAttachShader(render_program->id, render_program->vertex_shader_id); check_opengl_error();

    if (render_program->geometry_shader_id)
    {
      glAttachShader(render_program->id, render_program->geometry_shader_id); check_opengl_error();
    }

    glAttachShader(render_program->id, render_program->fragment_shader_id); check_opengl_error();

    glLinkProgram(render_program->id); check_opengl_error();

    auto link_status = GLint();
    glGetProgramiv(render_program->id, GL_LINK_STATUS, &link_status); check_opengl_error();

    GLchar info_log[1024];
    glGetProgramInfoLog(render_program->id, sizeof(info_log), nullptr, info_log); check_opengl_error();

    if (info_log[0])
    {
      std::cout << "render program link log: " << info_log << std::endl;
    }
    assert(link_status && "failed to link render program");

    return render_program;
  }

  render_program* add(instance& instance, const render_program& init, const std::string& vertex_shader_file_name, const std::string& fragment_shader_file_name, uint32_t capacity, const std::string& partition)
  {
    auto render_program = init;

    auto vertex_stream = std::ifstream(vertex_shader_file_name);
    auto vertex_shader = ludo::add(instance, ludo::shader(), ludo::shader_type::VERTEX, vertex_stream, partition);
    render_program.vertex_shader_id = vertex_shader->id;

    auto fragment_stream = std::ifstream(fragment_shader_file_name);
    auto fragment_shader = ludo::add(instance, ludo::shader(), ludo::shader_type::FRAGMENT, fragment_stream, partition);
    render_program.fragment_shader_id = fragment_shader->id;

    auto& draw_commands = data_heap<draw_command>(instance);
    render_program.command_buffer = allocate(draw_commands, capacity * sizeof(draw_command));

    return add(instance, render_program, partition);
  }

  render_program* add(instance& instance, const render_program& init, const vertex_format& format, uint32_t capacity, const std::string& partition)
  {
    auto render_program = init;
    render_program.format = format;

    if (!render_program.vertex_shader_id)
    {
      auto vertex_shader = ludo::add(instance, ludo::shader(), ludo::shader_type::VERTEX, format, partition);
      render_program.vertex_shader_id = vertex_shader->id;
    }

    if (!render_program.fragment_shader_id)
    {
      auto fragment_shader = ludo::add(instance, ludo::shader(), ludo::shader_type::FRAGMENT, format, partition);
      render_program.fragment_shader_id = fragment_shader->id;
    }

    auto& draw_commands = data_heap<draw_command>(instance);
    render_program.command_buffer = allocate(draw_commands, capacity * sizeof(draw_command));

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

    render_program.instance_buffer_front = allocate_vram(capacity * render_program.instance_size);
    render_program.instance_buffer_back = allocate_heap(capacity * render_program.instance_size);

    return add(instance, render_program, partition);
  }

  template<>
  void remove<render_program>(instance& instance, render_program* element, const std::string& partition)
  {
    glDeleteProgram(element->id); check_opengl_error();

    if (element->command_buffer.data)
    {
      auto& draw_commands = data_heap<draw_command>(instance);
      deallocate(draw_commands, element->command_buffer);
    }

    if (element->shader_buffer.data)
    {
      deallocate_vram(element->shader_buffer);
    }

    if (element->instance_buffer_front.data)
    {
      deallocate_vram(element->instance_buffer_front);
    }

    if (element->instance_buffer_back.data)
    {
      deallocate(element->instance_buffer_back);
    }

    remove(data<render_program>(instance), element, partition);
  }

  void bind(const render_program& render_program)
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
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, render_program.shader_buffer.id); check_opengl_error();

    // TODO do this during 'pre_render' step?
    std::memcpy(render_program.instance_buffer_front.data, render_program.instance_buffer_back.data, render_program.instance_buffer_front.size);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, render_program.instance_buffer_front.id); check_opengl_error();

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
}
