/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <istream>

#include "util.h"

namespace ludo
{
  GLuint compile(std::istream& code, GLenum type);
}
