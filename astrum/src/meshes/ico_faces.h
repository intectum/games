#ifndef ASTRUM_MESHES_ICO_FACES_H
#define ASTRUM_MESHES_ICO_FACES_H

#include <ludo/api.h>

namespace astrum
{
  const std::array<std::array<ludo::vec3, 3>, 20>& get_ico_faces();
}

#endif // ASTRUM_MESHES_ICO_FACES_H
