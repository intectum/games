/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

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

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, render_program.data_buffer.id); check_opengl_error();
  }
}
