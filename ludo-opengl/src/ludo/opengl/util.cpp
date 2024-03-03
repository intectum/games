/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include <GL/glew.h>

#include "util.h"

namespace ludo
{
  void check_opengl_error()
  {
    auto error = glGetError();
    if (error != GL_NO_ERROR)
    {
      std::cout << "OpenGL error " << error << ": " << gluErrorString(error);
      assert(false && "OpenGL error");
    }
  }

  uint64_t handle(texture& texture)
  {
    auto handle = glGetTextureHandleARB(texture.id); check_opengl_error();
    glMakeTextureHandleResidentARB(handle); check_opengl_error();

    return handle;
  }
}
