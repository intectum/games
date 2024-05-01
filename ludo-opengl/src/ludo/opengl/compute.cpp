/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include <ludo/compute.h>

#include "util.h"

namespace ludo
{
  template<>
  compute_program* add(instance& instance, const compute_program& init, const std::string& partition)
  {
    auto compute_program = add(data<ludo::compute_program>(instance), init, partition);
    compute_program->id = glCreateProgram(); check_opengl_error();

    if (compute_program->compute_shader_id)
    {
      glAttachShader(compute_program->id, compute_program->compute_shader_id); check_opengl_error();
    }

    glLinkProgram(compute_program->id); check_opengl_error();

    auto link_status = GLint();
    glGetProgramiv(compute_program->id, GL_LINK_STATUS, &link_status); check_opengl_error();

    GLchar info_log[1024];
    glGetProgramInfoLog(compute_program->id, sizeof(info_log), nullptr, info_log); check_opengl_error();

    if (info_log[0])
    {
      std::cout << "compute program link log: " << info_log << std::endl;
    }
    assert(link_status && "failed to link compute program");

    return compute_program;
  }

  compute_program* add(instance& instance, const compute_program& init, std::istream& code, const std::string& partition)
  {
    auto compute_program = init;

    compute_program.compute_shader_id = glCreateShader(GL_COMPUTE_SHADER); check_opengl_error();

    code.seekg(0, std::ios_base::end);
    auto code_string = std::string(code.tellg(), 'x');
    code.seekg(0);
    code.read(&code_string[0], static_cast<std::streamsize>(code_string.size()));

    const char* source_ptr = code_string.data();
    const int source_length = -1;
    glShaderSource(compute_program.compute_shader_id, 1, &source_ptr, &source_length); check_opengl_error();
    glCompileShader(compute_program.compute_shader_id); check_opengl_error();

    auto compile_status = GLint();
    glGetShaderiv(compute_program.compute_shader_id, GL_COMPILE_STATUS, &compile_status); check_opengl_error();

    GLchar info_log[1024];
    glGetShaderInfoLog(compute_program.compute_shader_id, sizeof(info_log), nullptr, info_log); check_opengl_error();

    if (compile_status == 0)
    {
      std::cout << "shader compile log: " << info_log << std::endl;
    }
    assert(compile_status && "failed to compile shader");

    return add(instance, compute_program, partition);
  }

  template<>
  void remove<compute_program>(instance& instance, compute_program* element, const std::string& partition)
  {
    glDeleteShader(element->compute_shader_id); check_opengl_error();
    glDeleteProgram(element->id); check_opengl_error();

    if (element->shader_buffer.data)
    {
      deallocate_vram(element->shader_buffer);
    }

    remove(data<compute_program>(instance), element, partition);
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
