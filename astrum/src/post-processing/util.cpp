#include <fstream>

#include <ludo/opengl/textures.h>

#include "util.h"

namespace astrum
{
  ludo::mesh* add_post_processing_mesh(ludo::instance& inst)
  {
    auto mesh_counts = ludo::rectangle_counts();
    auto mesh = ludo::add(
      inst,
      ludo::mesh(),
      mesh_counts.first,
      mesh_counts.second,
      ludo::vertex_format_pt.size
    );

    auto index_index = uint32_t(0);
    auto vertex_index = uint32_t(0);
    ludo::rectangle(*mesh, ludo::vertex_format_pt, index_index, vertex_index, { .dimensions = { 2.0f,  2.0f, 0.0f } });

    return mesh;
  }

  ludo::shader* add_post_processing_vertex_shader(ludo::instance& inst)
  {
    auto stream = std::ifstream("assets/shaders/post.vert");
    return ludo::add(inst, ludo::shader(), ludo::shader_type::VERTEX, stream);
  }

  ludo::frame_buffer* add_post_processing_frame_buffer(ludo::instance& inst, bool has_depth, float texture_size)
  {
    auto& window = *ludo::first<ludo::window>(inst);
    auto width = static_cast<uint32_t>(static_cast<float>(window.width) * texture_size);
    auto height = static_cast<uint32_t>(static_cast<float>(window.height) * texture_size);

    auto color_texture = ludo::add(inst, ludo::texture { .datatype = ludo::pixel_datatype::FLOAT16, .width = width, .height = height }, { .clamp = true });
    auto blank_color_data = std::vector<std::byte>(width * height * ludo::pixel_depth(*color_texture));
    ludo::write(*color_texture, blank_color_data.data());

    auto depth_texture_id = uint32_t(0);
    if (has_depth)
    {
      auto depth_texture = ludo::add(inst, ludo::texture { .components = ludo::pixel_components::DEPTH, .datatype = ludo::pixel_datatype::FLOAT32, .width = width, .height = height }, { .clamp = true });
      auto blank_depth_data = std::vector<std::byte>(width * height * ludo::pixel_depth(*depth_texture));
      ludo::write(*depth_texture, blank_depth_data.data());
      depth_texture_id = depth_texture->id;
    }

    return ludo::add(inst, ludo::frame_buffer { .width = width, .height = height, .color_texture_ids = { color_texture->id }, .depth_texture_id = depth_texture_id });
  }

  ludo::buffer create_post_processing_shader_buffer(uint64_t texture_id_0, uint64_t texture_id_1)
  {
    auto buffer = ludo::allocate_vram(16);

    ludo::write(buffer, 0, 0L);
    ludo::write(buffer, 8, 0L);

    if (texture_id_0) ludo::write(buffer, 0, ludo::handle(ludo::texture { .id = texture_id_0 }));
    if (texture_id_1) ludo::write(buffer, 8, ludo::handle(ludo::texture { .id = texture_id_1 }));

    return buffer;
  }
}
