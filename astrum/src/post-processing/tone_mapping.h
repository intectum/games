#ifndef ASTRUM_POST_PROCESSING_TONE_MAPPING_H
#define ASTRUM_POST_PROCESSING_TONE_MAPPING_H

#include <ludo/api.h>

namespace astrum
{
  void add_tone_mapping(ludo::instance& inst, const ludo::render_mesh& render_mesh);
}

#endif // ASTRUM_POST_PROCESSING_TONE_MAPPING_H
