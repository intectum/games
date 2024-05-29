#include <ludo/opengl/util.h>

#include "bloom.h"
#include "util.h"

namespace astrum
{
  void add_bloom(ludo::instance& inst, const ludo::render_mesh& render_mesh, uint32_t iterations, float final_texture_size)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto original_frame_buffer = &frame_buffers[frame_buffers.length - 1];

    auto& render_commands = data_heap(inst, "ludo::vram_render_commands");

    auto brightness_render_program = ludo::add(inst, ludo::render_program { .format = ludo::vertex_format_pt });
    ludo::init(*brightness_render_program, ludo::asset_folder + "/shaders/post.vert", ludo::asset_folder + "/shaders/brightness.frag", render_commands, 1);

    auto gaussian_render_program = ludo::add(inst, ludo::render_program { .format = ludo::vertex_format_pt });
    ludo::init(*gaussian_render_program, ludo::asset_folder + "/shaders/post.vert", ludo::asset_folder + "/shaders/gaussian.frag", render_commands, 2 * iterations);

    auto additive_render_program = ludo::add(inst, ludo::render_program { .format = ludo::vertex_format_pt });
    ludo::init(*additive_render_program, ludo::asset_folder + "/shaders/post.vert", ludo::asset_folder + "/shaders/additive.frag", render_commands, 1);

    auto previous_frame_buffer = original_frame_buffer;
    auto current_frame_buffer = add_post_processing_frame_buffer(inst);

    brightness_render_program->shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer->color_texture_ids[0], 0);

    ludo::add<ludo::script>(inst, [=](ludo::instance& inst)
    {
      auto rendering_context = ludo::first<ludo::rendering_context>(inst);
      auto& render_programs = ludo::data<ludo::render_program>(inst);

      auto& render_commands = ludo::data_heap(inst, "ludo::vram_render_commands");
      auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
      auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

      ludo::use_and_clear(*current_frame_buffer);
      ludo::add_render_command(*brightness_render_program, render_mesh);
      ludo::commit_render_commands(*rendering_context, render_programs, render_commands, indices, vertices);
    });

    for (auto iteration = 0; iteration < iterations; iteration++)
    {
      auto texture_size = 1.0f - static_cast<float>(iteration) / static_cast<float>(iterations) * (1.0f - final_texture_size);

      previous_frame_buffer = current_frame_buffer;
      current_frame_buffer = add_post_processing_frame_buffer(inst, false, texture_size);

      // TODO: This will get overridden!
      gaussian_render_program->shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer->color_texture_ids[0], 0);
      ludo::cast<bool>(gaussian_render_program->shader_buffer.back, 8) = true;

      ludo::add<ludo::script>(inst, [=](ludo::instance& inst)
      {
        auto rendering_context = ludo::first<ludo::rendering_context>(inst);
        auto& render_programs = ludo::data<ludo::render_program>(inst);

        auto& render_commands = ludo::data_heap(inst, "ludo::vram_render_commands");
        auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
        auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

        ludo::use_and_clear(*current_frame_buffer);
        ludo::add_render_command(*gaussian_render_program, render_mesh);
        ludo::commit_render_commands(*rendering_context, render_programs, render_commands, indices, vertices);
      });

      previous_frame_buffer = current_frame_buffer;
      current_frame_buffer = add_post_processing_frame_buffer(inst, false, texture_size);

      // TODO: This will get overridden!
      gaussian_render_program->shader_buffer = create_post_processing_shader_buffer(previous_frame_buffer->color_texture_ids[0], 0);
      ludo::cast<bool>(gaussian_render_program->shader_buffer.back, 8) = false;

      ludo::add<ludo::script>(inst, [=](ludo::instance& inst)
      {
        auto rendering_context = ludo::first<ludo::rendering_context>(inst);
        auto& render_programs = ludo::data<ludo::render_program>(inst);

        auto& render_commands = ludo::data_heap(inst, "ludo::vram_render_commands");
        auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
        auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

        ludo::use_and_clear(*current_frame_buffer);
        ludo::add_render_command(*gaussian_render_program, render_mesh);
        ludo::commit_render_commands(*rendering_context, render_programs, render_commands, indices, vertices);
      });
    }

    previous_frame_buffer = current_frame_buffer;
    current_frame_buffer = add_post_processing_frame_buffer(inst);

    additive_render_program->shader_buffer = create_post_processing_shader_buffer(original_frame_buffer->color_texture_ids[0], previous_frame_buffer->color_texture_ids[0]);

    ludo::add<ludo::script>(inst, [=](ludo::instance& inst)
    {
      auto rendering_context = ludo::first<ludo::rendering_context>(inst);
      auto& render_programs = ludo::data<ludo::render_program>(inst);

      auto& render_commands = ludo::data_heap(inst, "ludo::vram_render_commands");
      auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
      auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

      ludo::use_and_clear(*current_frame_buffer);
      ludo::add_render_command(*additive_render_program, render_mesh);
      ludo::commit_render_commands(*rendering_context, render_programs, render_commands, indices, vertices);
    });
  }
}
