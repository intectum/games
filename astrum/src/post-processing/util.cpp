#include <fstream>

#include <ludo/opengl/textures.h>

#include "util.h"

namespace astrum
{
  ludo::render_mesh* add_post_processing_render_mesh(ludo::instance& inst)
  {
    auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
    auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

    auto mesh_counts = ludo::rectangle_counts(ludo::vertex_format_pt);
    auto mesh = ludo::add(inst, ludo::mesh());
    ludo::init(*mesh, indices, vertices, mesh_counts.first, mesh_counts.second, ludo::vertex_format_pt.size);
    ludo::rectangle(*mesh, ludo::vertex_format_pt, 0, 0, { .dimensions = { 2.0f,  2.0f, 0.0f } });

    auto render_mesh = ludo::add(inst, ludo::render_mesh());
    ludo::init(*render_mesh);
    ludo::connect(*render_mesh, *mesh, indices, vertices);

    return render_mesh;
  }

  ludo::frame_buffer* add_post_processing_frame_buffer(ludo::instance& inst, bool has_depth, float texture_size)
  {
    auto& window = *ludo::first<ludo::window>(inst);
    auto width = static_cast<uint32_t>(static_cast<float>(window.width) * texture_size);
    auto height = static_cast<uint32_t>(static_cast<float>(window.height) * texture_size);

    auto color_texture = ludo::add(inst, ludo::texture { .datatype = ludo::pixel_datatype::FLOAT16, .width = width, .height = height });
    ludo::init(*color_texture, { .clamp = true });

    auto depth_texture_id = uint32_t(0);
    if (has_depth)
    {
      auto depth_texture = ludo::add(inst, ludo::texture { .components = ludo::pixel_components::DEPTH, .datatype = ludo::pixel_datatype::FLOAT32, .width = width, .height = height });
      ludo::init(*depth_texture, { .clamp = true });
      depth_texture_id = depth_texture->id;
    }

    auto frame_buffer = ludo::add(inst, ludo::frame_buffer { .width = width, .height = height, .color_texture_ids = { color_texture->id }, .depth_texture_id = depth_texture_id });
    ludo::init(*frame_buffer);

    return frame_buffer;
  }

  ludo::double_buffer create_post_processing_shader_buffer(uint64_t texture_id_0, uint64_t texture_id_1)
  {
    auto buffer = ludo::allocate_dual(16);
    auto stream = ludo::stream(buffer.back);

    ludo::write(stream, texture_id_0 == 0 ? uint64_t(0) : ludo::handle(ludo::texture { .id = texture_id_0 }));
    ludo::write(stream, texture_id_1 == 0 ? uint64_t(0) : ludo::handle(ludo::texture { .id = texture_id_1 }));

    return buffer;
  }
}
