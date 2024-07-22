/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "../meshes.h"

namespace ludo
{
  void collapse(mesh& mesh, const vertex_format& format, uint32_t iterations);
}
