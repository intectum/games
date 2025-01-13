#include "bloom.h"
#include "util.h"

namespace astrum
{
  std::tuple<std::vector<ludo::render_program>, std::vector<ludo::buffer>, std::vector<ludo::frame_buffer>> build_bloom(const ludo::window& window, const ludo::frame_buffer& previous_frame_buffer, uint32_t iterations, float final_texture_size)
  {
    auto pass_count = 1 + iterations * 2 + 1;
    auto render_programs = std::vector<ludo::render_program>(pass_count);
    auto data_buffers = std::vector<ludo::buffer>(pass_count);
    auto frame_buffers = std::vector<ludo::frame_buffer>(pass_count);

    auto brightness_render_program = ludo::render_program { .format = ludo::vertex_format_pt };
    ludo::init(brightness_render_program, ludo::asset_folder + "/shaders/post.vert", ludo::asset_folder + "/shaders/brightness.frag");

    // TODO split to horizontal/vertical programs to avoid conditional
    auto gaussian_render_program = ludo::render_program { .format = ludo::vertex_format_pt };
    ludo::init(gaussian_render_program, ludo::asset_folder + "/shaders/post.vert", ludo::asset_folder + "/shaders/gaussian.frag");

    auto additive_render_program = ludo::render_program { .format = ludo::vertex_format_pt };
    ludo::init(additive_render_program, ludo::asset_folder + "/shaders/post.vert", ludo::asset_folder + "/shaders/additive.frag");

    auto pass = uint32_t(0);
    render_programs[pass] = brightness_render_program;
    frame_buffers[pass] = build_post_processing_frame_buffer(window);
    data_buffers[pass] = build_post_processing_data_buffer(previous_frame_buffer.color_texture_ids[0], 0);

    for (auto iteration = 0; iteration < iterations; iteration++)
    {
      auto texture_size = 1.0f - static_cast<float>(iteration) / static_cast<float>(iterations) * (1.0f - final_texture_size);

      pass++;
      render_programs[pass] = gaussian_render_program;
      frame_buffers[pass] = build_post_processing_frame_buffer(window, false, texture_size);
      data_buffers[pass] = build_post_processing_data_buffer(frame_buffers[pass - 1].color_texture_ids[0], 0);
      ludo::cast<bool>(data_buffers[pass], 8) = true;

      pass++;
      render_programs[pass] = gaussian_render_program;
      frame_buffers[pass] = build_post_processing_frame_buffer(window, false, texture_size);
      data_buffers[pass] = build_post_processing_data_buffer(frame_buffers[pass - 1].color_texture_ids[0], 0);
      ludo::cast<bool>(data_buffers[pass], 8) = false;
    }

    pass++;
    render_programs[pass] = additive_render_program;
    frame_buffers[pass] = build_post_processing_frame_buffer(window);
    data_buffers[pass] = build_post_processing_data_buffer(previous_frame_buffer.color_texture_ids[0], frame_buffers[pass - 1].color_texture_ids[0]);

    return {
      render_programs,
      data_buffers,
      frame_buffers
    };
  }
}
