/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_OPENGL_SHADERS_H
#define LUDO_OPENGL_SHADERS_H

#include <istream>

#include "util.h"

namespace ludo
{
  GLuint compile(std::istream& code, GLenum type);
}

#endif // LUDO_OPENGL_SHADERS_H
