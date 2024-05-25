#include <iomanip>
#include <iostream>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

#include "constants.h"
#include "post-processing/atmosphere.h"
#include "post-processing/bloom.h"
#include "post-processing/tone_mapping.h"
#include "post-processing/pass.h"
#include "post-processing/util.h"
#include "solar_system.h"
#include "terrain/terrain.h"
#include "util.h"

int main()
{
  auto timer = ludo::timer();

  auto inst = ludo::instance();

  auto max_terrain_bodies = 25;
  auto post_processing_rectangle_counts = ludo::rectangle_counts();
  auto sol_mesh_counts = astrum::terrain_counts(astrum::sol_lods);
  auto terra_mesh_counts = astrum::terrain_counts(astrum::terra_lods);
  auto luna_mesh_counts = astrum::terrain_counts(astrum::luna_lods);
  auto fruit_tree_1_counts = ludo::import_counts("assets/models/fruit-tree-1.dae");
  auto fruit_tree_2_counts = ludo::import_counts("assets/models/fruit-tree-2.dae");
  auto person_mesh_counts = ludo::import_counts("assets/models/minifig.dae");
  auto spaceship_mesh_counts = ludo::import_counts("assets/models/spaceship.obj");
  auto bullet_debug_counts = std::pair<uint32_t, uint32_t> { max_terrain_bodies * 48 * 2, max_terrain_bodies * 48 * 2 };

  auto max_rendered_instances =
    14 + // post-processing
    3 * (5120 * 2) + // terrains (doubled to account for re-allocations - overkill)
    5120 * 200 + // trees
    1 + // person
    1; // spaceship
  auto max_indices =
    post_processing_rectangle_counts.first +
    sol_mesh_counts.first +
    terra_mesh_counts.first +
    luna_mesh_counts.first +
    fruit_tree_1_counts.first +
    fruit_tree_2_counts.first +
    person_mesh_counts.first +
    spaceship_mesh_counts.first;
  auto max_vertices =
    post_processing_rectangle_counts.second +
    sol_mesh_counts.second +
    terra_mesh_counts.second +
    luna_mesh_counts.second +
    fruit_tree_1_counts.second +
    fruit_tree_2_counts.second +
    person_mesh_counts.second +
    spaceship_mesh_counts.second;

  if (astrum::show_paths)
  {
    max_rendered_instances += 5;
    max_indices += astrum::path_steps * 5;
    max_vertices += astrum::path_steps * 5;
  }

  if (astrum::visualize_physics)
  {
    max_rendered_instances++;
    max_indices += bullet_debug_counts.first;
    max_vertices += bullet_debug_counts.second;
  }

  ludo::allocate<ludo::physics_context>(inst, 1);
  ludo::allocate<ludo::rendering_context>(inst, 1);
  ludo::allocate<ludo::windowing_context>(inst, 1);

  ludo::allocate<ludo::animation>(inst, 1);
  ludo::allocate<ludo::armature>(inst, 1);
  ludo::allocate<ludo::body_shape>(inst, 3);
  ludo::allocate<ludo::compute_program>(inst, 5);
  ludo::allocate<ludo::dynamic_body>(inst, 0);
  ludo::allocate<ludo::frame_buffer>(inst, 16);
  ludo::allocate<ludo::ghost_body>(inst, 1);
  ludo::allocate<ludo::grid3>(inst, 5);
  ludo::allocate<ludo::kinematic_body>(inst, 2);
  ludo::allocate<ludo::mesh>(inst, max_rendered_instances);
  ludo::allocate<ludo::mesh_instance>(inst, max_rendered_instances);
  ludo::allocate<ludo::render_program>(inst, 12);
  ludo::allocate<ludo::script>(inst, 36);
  ludo::allocate<ludo::shader>(inst, 22);
  ludo::allocate<ludo::static_body>(inst, max_terrain_bodies);
  ludo::allocate<ludo::texture>(inst, 21);
  ludo::allocate<ludo::window>(inst, 1);

  ludo::allocate<astrum::celestial_body>(inst, 3);
  ludo::allocate<astrum::game_controls>(inst, 1);
  ludo::allocate<astrum::map_controls>(inst, 1);
  ludo::allocate<astrum::person>(inst, 1);
  ludo::allocate<astrum::person_controls>(inst, 1);
  ludo::allocate<astrum::point_mass>(inst, 5);
  ludo::allocate<astrum::solar_system>(inst, 1);
  ludo::allocate<astrum::spaceship_controls>(inst, 1);
  ludo::allocate<astrum::terrain>(inst, 3);

  ludo::add(inst, ludo::windowing_context());
  auto window = ludo::add(inst, ludo::window { .title = "astrum", .width = 1920, .height = 1080, .v_sync = false });
  //ludo::capture_mouse(*window);

  ludo::add(inst, ludo::rendering_context(), 1);

  ludo::allocate_heap_vram(inst, "ludo::vram_draw_commands", max_rendered_instances * sizeof(ludo::draw_command));
  ludo::allocate_heap_vram(inst, "ludo::vram_indices", max_indices * sizeof(uint32_t));
  ludo::allocate_heap_vram(inst, "ludo::vram_vertices", max_vertices * ludo::vertex_format_pnc.size);

  auto default_grid = ludo::add(
    inst,
    ludo::grid3
      {
        .bounds =
        {
          .min = { -2.0f * astrum::astronomical_unit, -2.0f * astrum::astronomical_unit, -2.0f * astrum::astronomical_unit },
          .max = { 2.0f * astrum::astronomical_unit, 2.0f * astrum::astronomical_unit, 2.0f * astrum::astronomical_unit }
        },
        .cell_count_1d = 16
      }
  );

  auto msaa_color_texture = ludo::add(inst, ludo::texture { .datatype = ludo::pixel_datatype::FLOAT16, .width = window->width, .height = window->height }, { .samples = astrum::msaa_samples });
  auto msaa_depth_texture = ludo::add(inst, ludo::texture { .components = ludo::pixel_components::DEPTH, .datatype = ludo::pixel_datatype::FLOAT32, .width = window->width, .height = window->height }, { .samples = astrum::msaa_samples });
  auto msaa_frame_buffer = ludo::add(inst, ludo::frame_buffer { .width = window->width, .height = window->height, .color_texture_ids = { msaa_color_texture->id }, .depth_texture_id = msaa_depth_texture->id });

  ludo::add(inst, ludo::physics_context { .gravity = ludo::vec3_zero, .visualize = astrum::visualize_physics });

  astrum::add_solar_system(inst);

  ludo::push(*default_grid);

  ludo::add<ludo::script>(inst, ludo::prepare_render);
  ludo::add<ludo::script>(inst, ludo::update_windows);

  auto& grids = data<ludo::grid3>(inst);
  auto grid_ids = std::vector<uint64_t>();
  for (auto& grid : grids)
  {
    grid_ids.push_back(grid.id);
  }

  ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
  {
    .frame_buffer_id = msaa_frame_buffer->id,
    .grid_ids = grid_ids
  });

  if (astrum::visualize_physics)
  {
    auto bullet_debug_render_program = ludo::add(
      inst,
      ludo::render_program { .primitive = ludo::mesh_primitive::LINE_LIST },
      ludo::format(false, true),
      1,
      "ludo-bullet::visualizations"
    );

    auto bullet_debug_mesh = ludo::add(
      inst,
      ludo::mesh(),
      bullet_debug_counts.first,
      bullet_debug_counts.second,
      bullet_debug_render_program->format.size,
      "ludo-bullet::visualizations"
    );

    auto bullet_debug_mesh_instance = ludo::add(
      inst,
      ludo::mesh_instance { .render_program_id = bullet_debug_render_program->id },
      *bullet_debug_mesh,
      1,
      "ludo-bullet::visualizations"
    );

    ludo::add<ludo::script>(inst, [&](ludo::instance& inst)
    {
      auto mesh_instance = ludo::first<ludo::mesh_instance>(inst, "ludo-bullet::visualizations");
      auto render_program = ludo::first<ludo::render_program>(inst, "ludo-bullet::visualizations");

      ludo::add_draw_command(*render_program, *mesh_instance);
      ludo::render(inst, {
        .frame_buffer_id = msaa_frame_buffer->id,
        .clear_frame_buffer = false
      });
    });
  }

  // Post-processing
  auto post_processing_mesh_instance = astrum::add_post_processing_mesh_instance(inst);
  auto post_processing_vertex_shader = astrum::add_post_processing_vertex_shader(inst);
  astrum::add_pass(inst); // Implicitly converts MSAA textures to regular textures
  //astrum::write_atmosphere_texture(50, 0.25f, 1.0f, astrum::terra_atmosphere_scale, "assets/effects/atmosphere.tiff", 1024);
  astrum::add_atmosphere(inst, post_processing_vertex_shader->id, *post_processing_mesh_instance, 1, astrum::terra_radius, astrum::terra_radius * astrum::terra_atmosphere_scale);
  astrum::add_bloom(inst, post_processing_vertex_shader->id, *post_processing_mesh_instance, 5, 0.1f);
  astrum::add_tone_mapping(inst, post_processing_vertex_shader->id, *post_processing_mesh_instance);
  astrum::add_pass(inst, true);

  ludo::add<ludo::script>(inst, ludo::finalize_render);

  ludo::add<ludo::script>(inst, astrum::print_timings);

  std::cout << std::fixed << std::setprecision(4) << "load time (seconds): " << ludo::elapsed(timer) << std::endl;

  ludo::play(inst);
}
