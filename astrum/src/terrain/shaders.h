/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef ASTRUM_TERRAIN_SHADERS_H
#define ASTRUM_TERRAIN_SHADERS_H

#include <sstream>

#include <ludo/meshes.h>
#include <ludo/rendering.h>

namespace astrum
{
  std::stringstream terrain_vertex_shader_code(const ludo::vertex_format& format);

  std::stringstream terrain_fragment_shader_code(const ludo::vertex_format& format);
}

#endif // ASTRUM_TERRAIN_SHADERS_H
