/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include "util.h"

namespace ludo
{
  auto shader_types = std::unordered_map<shader_type, GLenum>
  {
    { shader_type::VERTEX, GL_VERTEX_SHADER },
    { shader_type::GEOMETRY, GL_GEOMETRY_SHADER },
    { shader_type::FRAGMENT, GL_FRAGMENT_SHADER }
  };

  template<>
  shader* add(instance& instance, const shader& init, const std::string& partition)
  {
    auto shader = add(data<ludo::shader>(instance), init, partition);

    shader->id = glCreateShader(shader_types[init.type]); check_opengl_error();

    return shader;
  }

  shader* add(instance& instance, const shader& init, std::istream& code, const std::string& partition)
  {
    auto shader = add(instance, init, partition);

    code.seekg(0, std::ios_base::end);
    auto code_string = std::string(code.tellg(), 'x');
    code.seekg(0);
    code.read(&code_string[0], static_cast<std::streamsize>(code_string.size()));

    const char* source_ptr = code_string.data();
    const int source_length = -1;
    glShaderSource(shader->id, 1, &source_ptr, &source_length); check_opengl_error();
    glCompileShader(shader->id); check_opengl_error();

    auto compile_status = GLint();
    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &compile_status); check_opengl_error();

    if (compile_status == 0)
    {
      GLchar info_log[1024];
      glGetShaderInfoLog(shader->id, sizeof(info_log), nullptr, info_log); check_opengl_error();

      std::cout << "failed to compile shader: " << info_log << std::endl;
      assert(false && "failed to compile shader");
    }

    return shader;
  }

  template<>
  void remove<shader>(instance& instance, shader* element, const std::string& partition)
  {
    glDeleteShader(element->id); check_opengl_error();

    remove(data<shader>(instance), element, partition);
  }
}
