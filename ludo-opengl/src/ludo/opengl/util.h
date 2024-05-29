/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_OPENGL_UTIL_H
#define LUDO_OPENGL_UTIL_H

#include <GL/glew.h>

namespace ludo
{
  struct render_command
  {
    GLuint index_count = 0;
    GLuint instance_count = 1;
    GLuint index_start = 0;
    GLuint vertex_start = 0;
    GLuint instance_start = 0;
  };

  void check_opengl_error();
}

#endif // LUDO_OPENGL_UTIL_H
