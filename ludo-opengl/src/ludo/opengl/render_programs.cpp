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
  void init(render_program& render_program, const vertex_format& format)
  {
    render_program.format = format;

    auto vertex_shader_code = default_vertex_shader_code(format);
    auto fragment_shader_code = default_fragment_shader_code(format);
    init(render_program, vertex_shader_code, fragment_shader_code);
  }

  void init(render_program& render_program, const std::string& vertex_shader_file_name, const std::string& fragment_shader_file_name)
  {
    auto vertex_shader_code = std::ifstream(vertex_shader_file_name);
    auto fragment_shader_code = std::ifstream(fragment_shader_file_name);

    init(render_program, vertex_shader_code, fragment_shader_code);
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

  void de_init(render_program& render_program)
  {
    glDeleteProgram(render_program.id); check_opengl_error();
    render_program.id = 0;
  }

  void use(const render_program& render_program)
  {
    glValidateProgram(render_program.id); check_opengl_error();

    auto validate_status = GLint();
    glGetProgramiv(render_program.id, GL_VALIDATE_STATUS, &validate_status); check_opengl_error();

    GLchar info_log[1024];
    glGetProgramInfoLog(render_program.id, sizeof(info_log), nullptr, info_log); check_opengl_error();

    if (info_log[0]) std::cout << "render program validation log: " << info_log << std::endl;
    assert(validate_status && "failed to validate render program");

    glUseProgram(render_program.id); check_opengl_error();

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
