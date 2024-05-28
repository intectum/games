/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MESHES_COLLAPSE_H
#define LUDO_MESHES_COLLAPSE_H

#include "../meshes.h"

namespace ludo
{
  LUDO_API void collapse(mesh& mesh, const vertex_format& format, uint32_t iterations);
}

#endif // LUDO_MESHES_COLLAPSE_H
