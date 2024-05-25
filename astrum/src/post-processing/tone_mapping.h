#ifndef ASTRUM_POST_PROCESSING_TONE_MAPPING_H
#define ASTRUM_POST_PROCESSING_TONE_MAPPING_H

#include <ludo/api.h>

namespace astrum
{
  void add_tone_mapping(ludo::instance& inst, uint64_t vertex_shader_id, const ludo::mesh_instance& mesh_instance);
}

#endif // ASTRUM_POST_PROCESSING_TONE_MAPPING_H
