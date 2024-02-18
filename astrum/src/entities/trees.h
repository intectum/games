#ifndef ASTRUM_ENTITIES_TREES_H
#define ASTRUM_ENTITIES_TREES_H

#include <ludo/api.h>

namespace astrum
{
  void add_trees(ludo::instance& inst, uint32_t celestial_body_index);

  void stream_trees(ludo::instance& inst, uint32_t celestial_body_index);
}

#endif // ASTRUM_ENTITIES_TREES_H
