#include <fstream>

#include <ludo/opengl/textures.h>

#include "util.h"

namespace astrum
{
  ludo::frame_buffer build_post_processing_frame_buffer(const ludo::window& window, bool has_depth, float texture_size)
  {
    auto width = static_cast<uint32_t>(static_cast<float>(window.width) * texture_size);
    auto height = static_cast<uint32_t>(static_cast<float>(window.height) * texture_size);

    auto color_texture = ludo::texture { .datatype = ludo::pixel_datatype::FLOAT16, .width = width, .height = height };
    ludo::init(color_texture, { .clamp = true });

    auto depth_texture_id = uint32_t(0);
    if (has_depth)
    {
      auto depth_texture = ludo::texture { .components = ludo::pixel_components::DEPTH, .datatype = ludo::pixel_datatype::FLOAT32, .width = width, .height = height };
      ludo::init(depth_texture, { .clamp = true });
      depth_texture_id = depth_texture.id;
    }

    auto frame_buffer = ludo::frame_buffer { .width = width, .height = height, .color_texture_ids = { color_texture.id }, .depth_texture_id = depth_texture_id };
    ludo::init(frame_buffer);

    return frame_buffer;
  }

  ludo::buffer build_post_processing_data_buffer(uint64_t texture_id_0, uint64_t texture_id_1, uint64_t size)
  {
    auto buffer = ludo::allocate_buffer_vram(size);
    auto stream = ludo::stream(buffer);

    ludo::write(stream, texture_id_0 == 0 ? uint64_t(0) : ludo::handle(ludo::texture { .id = texture_id_0 }));
    ludo::write(stream, texture_id_1 == 0 ? uint64_t(0) : ludo::handle(ludo::texture { .id = texture_id_1 }));

    return buffer;
  }
}
