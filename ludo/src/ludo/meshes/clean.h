/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "../meshes.h"

namespace ludo
{
  void clean(mesh& dest_mesh, buffer& dest_indices, buffer& dest_vertices, const buffer& src_indices, const buffer& src_vertices, uint32_t count, const vertex_format& dest_format, const vertex_format& src_format, bool dry_run);
}
