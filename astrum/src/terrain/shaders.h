/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef ASTRUM_TERRAIN_SHADERS_H
#define ASTRUM_TERRAIN_SHADERS_H

#include <ludo/meshes.h>
#include <ludo/rendering.h>

namespace astrum
{
  ludo::shader* add_terrain_vertex_shader(ludo::instance& inst, const ludo::vertex_format& format, const std::string& partition = "default");

  ludo::shader* add_terrain_fragment_shader(ludo::instance& inst, const ludo::vertex_format& format, const std::string& partition = "default");
}

#endif // ASTRUM_TERRAIN_SHADERS_H
