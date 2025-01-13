/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "shapes.h"

namespace ludo
{
  void append_box(mesh& mesh, buffer& indices, buffer& vertices, const vertex_format& format, const shape_options& options, bool unique_only, bool no_normal_check);
}
