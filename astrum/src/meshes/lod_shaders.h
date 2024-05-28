/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef ASTRUM_MESHES_LOD_SHADERS_H
#define ASTRUM_MESHES_LOD_SHADERS_H

#include <sstream>

#include <ludo/meshes.h>
#include <ludo/rendering.h>

namespace astrum
{
  std::stringstream lod_vertex_shader_code(const ludo::vertex_format& format, bool shared_transform);

  std::stringstream lod_fragment_shader_code(const ludo::vertex_format& format, bool shared_transform);
}

#endif // ASTRUM_MESHES_LOD_SHADERS_H
