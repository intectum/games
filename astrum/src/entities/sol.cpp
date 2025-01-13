#include "sol.h"

#include <ecs.h>

namespace astrum
{
  void sync_light_with_sol(ludo::container& container)
  {
    auto light_positions = reinterpret_cast<ludo::vec3*>(get_components(container, "light", "position_vram")->data);
    auto celestial_body_positions = reinterpret_cast<ludo::vec3*>(get_components(container, "celestial_body", "position")->data);

    light_positions[0] = celestial_body_positions[0];
  }
}
