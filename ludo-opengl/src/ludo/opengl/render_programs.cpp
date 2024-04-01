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

  render_program* add(instance& instance, const render_program& init, const std::string& vertex_shader_file_name, const std::string& fragment_shader_file_name, const std::string& partition)
  {
    auto render_program = init;

    auto vertex_stream = std::ifstream(vertex_shader_file_name);
    auto vertex_shader = ludo::add(instance, ludo::shader(), ludo::shader_type::VERTEX, vertex_stream, partition);
    render_program.vertex_shader_id = vertex_shader->id;

    auto fragment_stream = std::ifstream(fragment_shader_file_name);
    auto fragment_shader = ludo::add(instance, ludo::shader(), ludo::shader_type::FRAGMENT, fragment_stream, partition);
    render_program.fragment_shader_id = fragment_shader->id;

    return add(instance, render_program, partition);
  }

  render_program* add(instance& instance, const render_program& init, const vertex_format_options& options, const std::string& partition)
  {
    auto render_program = init;
    render_program.format = format(options);

    auto vertex_shader = ludo::add(instance, ludo::shader(), ludo::shader_type::VERTEX, options, partition);
    render_program.vertex_shader_id = vertex_shader->id;

    auto fragment_shader = ludo::add(instance, ludo::shader(), ludo::shader_type::FRAGMENT, options, partition);
    render_program.fragment_shader_id = fragment_shader->id;

    return add(instance, render_program, partition);
  }

  template<>
  void remove<render_program>(instance& instance, render_program* element, const std::string& partition)
  {
    glDeleteProgram(element->id); check_opengl_error();

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

    // Convert b4 to u4f4
    auto format = render_program.format;
    for (auto index = 0; index < format.components.size(); index++)
    {
      if (format.components[index] == 'b')
      {
        format.components[index] = 'u';
        index++;
        format.components.insert(format.components.begin() + index, 'f');
        format.component_counts.insert(format.component_counts.begin() + index, 4);
      }
    }

    auto offset = uint32_t(0);
    for (auto index = 0; index < format.components.size(); index++)
    {
      glEnableVertexAttribArray(index); check_opengl_error();

      if (format.components[index] == 'i' || format.components[index] == 'u')
      {
        glVertexAttribIPointer(
          index,
          format.component_counts[index],
          format.components[index] == 'i' ? GL_INT : GL_UNSIGNED_INT,
          format.size,
          reinterpret_cast<void*>(offset)
        ); check_opengl_error();

        offset += format.component_counts[index] * sizeof(uint32_t);
      }
      else
      {
        glVertexAttribPointer(
          index,
          format.component_counts[index],
          GL_FLOAT,
          GL_FALSE,
          format.size,
          reinterpret_cast<void*>(offset)
        ); check_opengl_error();

        offset += format.component_counts[index] * sizeof(float);
      }
    }
  }
}
