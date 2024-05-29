/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <fstream>
#include <iostream>

#include <ludo/compute.h>

#include "shaders.h"

namespace ludo
{
  void init(compute_program& compute_program, const std::string& shader_file_name)
  {
    auto shader_code = std::ifstream(shader_file_name);

    init(compute_program, shader_code);
  }

  void init(compute_program& compute_program, std::istream& code)
  {
    compute_program.id = glCreateProgram(); check_opengl_error();

    auto compute_shader = compile(code, GL_COMPUTE_SHADER);

    glAttachShader(compute_program.id, compute_shader); check_opengl_error();
    glLinkProgram(compute_program.id); check_opengl_error();

    auto link_status = GLint();
    glGetProgramiv(compute_program.id, GL_LINK_STATUS, &link_status); check_opengl_error();

    GLchar link_info_log[1024];
    glGetProgramInfoLog(compute_program.id, sizeof(link_info_log), nullptr, link_info_log); check_opengl_error();

    if (link_info_log[0])
    {
      std::cout << "compute program link log: " << link_info_log << std::endl;
    }
    assert(link_status && "failed to link compute program");

    glDeleteShader(compute_shader); check_opengl_error();
  }

  void de_init(compute_program& compute_program)
  {
    glDeleteProgram(compute_program.id); check_opengl_error();
    compute_program.id = 0;

    if (compute_program.shader_buffer.data)
    {
      deallocate_vram(compute_program.shader_buffer);
    }
  }

  void execute(compute_program& compute_program, uint32_t groups_x, uint32_t groups_y, uint32_t groups_z)
  {
    glValidateProgram(compute_program.id); check_opengl_error();

    auto validate_status = GLint();
    glGetProgramiv(compute_program.id, GL_VALIDATE_STATUS, &validate_status); check_opengl_error();

    GLchar info_log[1024];
    glGetProgramInfoLog(compute_program.id, sizeof(info_log), nullptr, info_log); check_opengl_error();

    if (info_log[0])
    {
      std::cout << "compute program validation log: " << info_log << std::endl;
    }
    assert(validate_status && "failed to validate compute program");

    glUseProgram(compute_program.id); check_opengl_error();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, compute_program.shader_buffer.id); check_opengl_error();

    glDispatchCompute(groups_x, groups_y, groups_z); check_opengl_error();
  }
}
