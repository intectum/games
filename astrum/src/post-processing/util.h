#pragma once

#include <ludo/api.h>

namespace astrum
{
  ludo::frame_buffer build_post_processing_frame_buffer(const ludo::window& window, bool has_depth = false, float texture_size = 1.0f);

  ludo::buffer build_post_processing_data_buffer(uint64_t texture_id_0, uint64_t texture_id_1, uint64_t size = 16);
}
