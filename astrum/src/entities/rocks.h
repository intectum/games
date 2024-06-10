#ifndef ASTRUM_ENTITIES_ROCKS_H
#define ASTRUM_ENTITIES_ROCKS_H

#include <ludo/api.h>

namespace astrum
{
  void add_rocks(ludo::instance& inst, uint32_t celestial_body_index);

  void stream_rocks(ludo::instance& inst, uint32_t celestial_body_index);
}

#endif // ASTRUM_ENTITIES_ROCKS_H
