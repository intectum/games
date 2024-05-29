#ifndef ASTRUM_PHYSICS_POINT_MASSES_H
#define ASTRUM_PHYSICS_POINT_MASSES_H

#include <ludo/api.h>

namespace astrum
{
  void simulate_point_mass_physics(ludo::instance& inst, const std::vector<std::string>& kinematic_partitions);

  void sync_render_meshes_with_point_masses(ludo::instance& inst, const std::vector<std::string>& partitions);
}

#endif // ASTRUM_PHYSICS_POINT_MASSES_H
