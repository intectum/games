#ifndef ASTRUM_TERRAIN_METADATA_H
#define ASTRUM_TERRAIN_METADATA_H

#include <iostream>

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  void build_terrain_metadata(terrain& terrain, float radius);

  void read_terrain_metadata(std::istream& stream, terrain& terrain);

  void write_terrain_metadata(std::ostream& stream, const terrain& terrain);
}

#endif // ASTRUM_TERRAIN_METADATA_H
