#ifndef ASTRUM_MESHES_DYNAMIC_MESH_H
#define ASTRUM_MESHES_DYNAMIC_MESH_H

#include <ludo/api.h>

#include "../types.h"

namespace astrum
{
  void save(const patchwork& patchwork, std::ostream& stream);

  void load(patchwork& patchwork, std::istream& stream);

  patchwork* add(ludo::instance& inst, const patchwork& init, const std::string& partition);

  void update_patchworks(ludo::instance& inst);
}

#endif // ASTRUM_MESHES_DYNAMIC_MESH_H
