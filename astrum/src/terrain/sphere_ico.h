#ifndef ASTRUM_TERRAIN_SPHERE_ICO_H
#define ASTRUM_TERRAIN_SPHERE_ICO_H

#include <ludo/api.h>

namespace astrum
{
  std::unordered_map<uint32_t, std::array<ludo::vec3, 3>> find_sphere_ico_chunks(uint32_t section_divisions, const std::function<bool(const std::array<ludo::vec3, 3>& triangle)>& test);
}

#endif // ASTRUM_TERRAIN_SPHERE_ICO_H
