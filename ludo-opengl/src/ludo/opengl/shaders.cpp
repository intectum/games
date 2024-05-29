/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <iostream>

#include "shaders.h"

namespace ludo
{
  GLuint compile(std::istream& code, GLenum type)
  {
    auto shader = glCreateShader(type); check_opengl_error();

    code.seekg(0, std::ios_base::end);
    auto code_string = std::string(code.tellg(), 'x');
    code.seekg(0);
    code.read(&code_string[0], static_cast<std::streamsize>(code_string.size()));

    const char* source_ptr = code_string.data();
    const int source_length = -1;
    glShaderSource(shader, 1, &source_ptr, &source_length); check_opengl_error();
    glCompileShader(shader); check_opengl_error();

    auto compile_status = GLint();
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status); check_opengl_error();

    GLchar info_log[1024];
    glGetShaderInfoLog(shader, sizeof(info_log), nullptr, info_log); check_opengl_error();

    if (compile_status == 0)
    {
      std::cout << "shader compile log: " << info_log << std::endl;
    }
    assert(compile_status && "failed to compile shader");

    return shader;
  }
}
