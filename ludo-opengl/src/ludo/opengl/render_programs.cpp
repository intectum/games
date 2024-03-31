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

    if (link_status == 0)
    {
      GLchar info_log[1024];
      glGetProgramInfoLog(render_program->id, sizeof(info_log), nullptr, info_log); check_opengl_error();

      std::cout << "failed to link shader program: " << info_log << std::endl;
      assert(false && "failed to link shader program:");
    }

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
    // TODO Only needed for debugging apparently...
    glValidateProgram(render_program.id); check_opengl_error();

    auto validate_status = GLint();
    glGetProgramiv(render_program.id, GL_VALIDATE_STATUS, &validate_status); check_opengl_error();

    if (validate_status == 0)
    {
      GLchar info_log[1024];
      glGetProgramInfoLog(render_program.id, sizeof(info_log), nullptr, info_log); check_opengl_error();

      std::cout << "failed to validate shader program: " << info_log << std::endl;
      assert(false && "failed to validate shader program:");
    }

    glUseProgram(render_program.id); check_opengl_error();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, render_program.shader_buffer.id); check_opengl_error();

    auto offset = uint32_t(0);
    for (auto index = 0; index < render_program.format.components.size(); index++)
    {
      glEnableVertexAttribArray(index); check_opengl_error();

      if (render_program.format.components[index] == 'i' || render_program.format.components[index] == 'u')
      {
        glVertexAttribIPointer(
          index,
          render_program.format.component_counts[index],
          render_program.format.components[index] == 'i' ? GL_INT : GL_UNSIGNED_INT,
          render_program.format.size,
          reinterpret_cast<void*>(offset)
        ); check_opengl_error();

        offset += render_program.format.component_counts[index] * sizeof(uint32_t);
      }
      else
      {
        glVertexAttribPointer(
          index,
          render_program.format.component_counts[index],
          GL_FLOAT,
          GL_FALSE,
          render_program.format.size,
          reinterpret_cast<void*>(offset)
        ); check_opengl_error();

        offset += render_program.format.component_counts[index] * sizeof(float);
      }
    }
  }
}
