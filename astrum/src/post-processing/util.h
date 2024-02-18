#ifndef ASTRUM_POST_PROCESSING_UTIL_H
#define ASTRUM_POST_PROCESSING_UTIL_H

#include <ludo/api.h>

namespace astrum
{
  ludo::mesh* add_post_processing_mesh(ludo::instance& inst);

  ludo::frame_buffer* add_post_processing_frame_buffer(ludo::instance& inst, bool has_depth = false, float texture_size = 1.0f);

  ludo::buffer create_post_processing_data_buffer(uint64_t texture_id_0, uint64_t texture_id_1);
}

#endif // ASTRUM_POST_PROCESSING_UTIL_H
