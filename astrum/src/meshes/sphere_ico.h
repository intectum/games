#ifndef ASTRUM_MESHES_SPHERE_ICO_H
#define ASTRUM_MESHES_SPHERE_ICO_H

#include <ludo/api.h>

namespace astrum
{
  void ico_section(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t index, uint32_t section_divisions, uint32_t divisions, uint8_t position_offset);
}

#endif // ASTRUM_MESHES_SPHERE_ICO_H
