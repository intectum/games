#ifndef ASTRUM_POST_PROCESSING_BLOOM_H
#define ASTRUM_POST_PROCESSING_BLOOM_H

#include <ludo/api.h>

namespace astrum
{
  void add_bloom(ludo::instance& inst, const ludo::render_mesh& render_mesh, uint32_t iterations = 5, float final_texture_size = 1.0f);
}

#endif // ASTRUM_POST_PROCESSING_BLOOM_H
