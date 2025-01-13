#pragma once

#include <ludo/api.h>

#include "meshes/lods.h"
#include "types.h"

namespace astrum
{
  ludo::container build_container(const std::string& name, float radius, const std::vector<lod>& lods, const terrain_funcs& terrain_funcs);
}
