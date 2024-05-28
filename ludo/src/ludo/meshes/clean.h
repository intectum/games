/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MESHES_CLEAN_H
#define LUDO_MESHES_CLEAN_H

#include "../meshes.h"

namespace ludo
{
  LUDO_API std::pair<uint32_t, uint32_t> clean(mesh& destination, const mesh& source, const vertex_format& destination_format, const vertex_format& source_format, bool dry_run = false);
}

#endif // LUDO_MESHES_CLEAN_H
