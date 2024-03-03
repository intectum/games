#include <ludo/opengl/util.h>

#include "util.h"

namespace astrum
{
  ludo::mesh* add_post_processing_mesh(ludo::instance& inst)
  {
    auto mesh_counts = ludo::rectangle_counts({});
    auto mesh_buffer = ludo::add(
      inst,
      ludo::mesh_buffer(),
      {
        .instance_count = 0,
        .index_count = mesh_counts.first,
        .vertex_count = mesh_counts.second,
        .texture_count = 1
      }
    );

    mesh_buffer->command_buffer = ludo::allocate_vram(sizeof(ludo::draw_command));

    auto mesh = ludo::add(
      inst,
      ludo::mesh
      {
        .mesh_buffer_id = mesh_buffer->id,
        .index_buffer = mesh_buffer->index_buffer,
        .vertex_buffer = mesh_buffer->vertex_buffer
      }
    );

    auto index_index = uint32_t(0);
    auto vertex_index = uint32_t(0);
    ludo::rectangle(*mesh, mesh_buffer->format, index_index, vertex_index, { .dimensions = { 2.0f,  2.0f, 0.0f } });

    return mesh;
  }

  ludo::frame_buffer* add_post_processing_frame_buffer(ludo::instance& inst, bool has_depth, float texture_size)
  {
    auto& window = *ludo::first<ludo::window>(inst);
    auto width = static_cast<uint32_t>(static_cast<float>(window.width) * texture_size);
    auto height = static_cast<uint32_t>(static_cast<float>(window.height) * texture_size);

    auto blank_color_data = std::vector<std::byte>(width * height * 6);
    auto color_texture = ludo::add(inst, ludo::texture { .format = ludo::pixel_format::RGB_HDR, .width = width, .height = height }, { .clamp = true });
    ludo::write(*color_texture, blank_color_data.data());

    auto depth_texture_id = uint32_t(0);
    if (has_depth)
    {
      auto blank_depth_data = std::vector<std::byte>(width * height * 4);
      auto depth_texture = ludo::add(inst, ludo::texture { .format = ludo::pixel_format::DEPTH, .width = width, .height = height }, { .clamp = true });
      ludo::write(*depth_texture, blank_depth_data.data());
      depth_texture_id = depth_texture->id;
    }

    return ludo::add(inst, ludo::frame_buffer { .width = width, .height = height, .color_texture_ids = { color_texture->id }, .depth_texture_id = depth_texture_id });
  }

  ludo::buffer create_post_processing_data_buffer(uint64_t texture_id_0, uint64_t texture_id_1)
  {
    auto buffer = ludo::allocate_vram(16);

    ludo::write(buffer, 0, 0L);
    ludo::write(buffer, 8, 0L);

    if (texture_id_0) ludo::write(buffer, 0, ludo::texture { .id = texture_id_0 });
    if (texture_id_1) ludo::write(buffer, 8, ludo::texture { .id = texture_id_1 });

    return buffer;
  }
}
