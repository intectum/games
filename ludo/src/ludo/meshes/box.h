/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MESHES_BOX_H
#define LUDO_MESHES_BOX_H

#include "shapes.h"

namespace ludo
{
  void box(mesh& mesh, const vertex_format& format, uint32_t& index_index, uint32_t& vertex_index, const shape_options& options, bool unique_only, bool no_normal_check);
}

#endif // LUDO_MESHES_BOX_H
