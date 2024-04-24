#include <iomanip>
#include <iostream>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

#include "constants.h"
#include "entities/celestial_bodies.h"
#include "post-processing/atmosphere.h"
#include "post-processing/bloom.h"
#include "post-processing/hdr_resolve.h"
#include "post-processing/pass.h"
#include "post-processing/util.h"
#include "solar_system.h"
#include "util.h"

int main()
{
  auto timer = ludo::timer();

  auto inst = ludo::instance();

  auto max_terrain_chunks = 25;
  auto post_processing_rectangle_counts = ludo::rectangle_counts();
  auto sol_mesh_counts = astrum::celestial_body_counts(astrum::sol_lods);
  auto terra_mesh_counts = astrum::celestial_body_counts(astrum::terra_lods);
  auto luna_mesh_counts = astrum::celestial_body_counts(astrum::luna_lods);
  auto person_mesh_counts = ludo::import_counts("assets/models/minifig.dae");
  auto spaceship_mesh_counts = ludo::import_counts("assets/models/spaceship.obj");
  auto bullet_debug_counts = std::pair<uint32_t, uint32_t> { max_terrain_chunks * 48 * 2, max_terrain_chunks * 48 * 2 };

  auto max_rendered_instances =
    1 + // post-processing rectangle
    3 * (5120 * 2); // celestial bodies (doubled to account for re-allocations)
    1 + // person
    1; // spaceship
  auto max_indices =
    post_processing_rectangle_counts.first +
    sol_mesh_counts.first +
    terra_mesh_counts.first +
    luna_mesh_counts.first +
    person_mesh_counts.first +
    spaceship_mesh_counts.first;
  auto max_vertices =
    post_processing_rectangle_counts.second +
    sol_mesh_counts.second +
    terra_mesh_counts.second +
    luna_mesh_counts.second +
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
  ludo::allocate<ludo::armature_instance>(inst, 1);
  ludo::allocate<ludo::body_shape>(inst, 3);
  ludo::allocate<ludo::dynamic_body>(inst, 0);
  ludo::allocate<ludo::frame_buffer>(inst, 16);
  ludo::allocate<ludo::ghost_body>(inst, 1);
  ludo::allocate<ludo::kinematic_body>(inst, 2);
  ludo::allocate<ludo::linear_octree>(inst, 4);
  ludo::allocate<ludo::mesh>(inst, max_rendered_instances);
  ludo::allocate<ludo::mesh_instance>(inst, max_rendered_instances);
  ludo::allocate<ludo::render_program>(inst, 12);
  ludo::allocate<ludo::script>(inst, 36);
  ludo::allocate<ludo::shader>(inst, 19);
  ludo::allocate<ludo::static_body>(inst, max_terrain_chunks);
  ludo::allocate<ludo::texture>(inst, 21);
  ludo::allocate<ludo::window>(inst, 1);

  ludo::allocate<astrum::celestial_body>(inst, 3);
  ludo::allocate<astrum::game_controls>(inst, 1);
  ludo::allocate<astrum::map_controls>(inst, 1);
  ludo::allocate<astrum::patchwork>(inst, 3);
  ludo::allocate<astrum::person>(inst, 1);
  ludo::allocate<astrum::person_controls>(inst, 1);
  ludo::allocate<astrum::point_mass>(inst, 5);
  ludo::allocate<astrum::solar_system>(inst, 1);
  ludo::allocate<astrum::spaceship_controls>(inst, 1);

  ludo::add(inst, ludo::windowing_context());
  auto window = ludo::add(inst, ludo::window { .title = "astrum", .width = 1920, .height = 1080, .v_sync = false });
  //ludo::capture_mouse(*window);

  ludo::add(inst, ludo::rendering_context(), 1);

  ludo::allocate_vram<ludo::draw_command>(inst, max_rendered_instances);
  ludo::allocate_vram<ludo::instance_t>(inst, max_rendered_instances * sizeof(ludo::mat4) + 10 * sizeof(uint64_t) + ludo::max_bones_per_armature * sizeof(ludo::mat4));
  ludo::allocate_heap_vram<ludo::index_t>(inst, max_indices);
  ludo::allocate_heap_vram<ludo::vertex_t>(inst, max_vertices * ludo::vertex_format_pnc.size);

  ludo::add(
    inst,
    ludo::linear_octree
      {
        .bounds =
        {
          .min = { -2.0f * astrum::astronomical_unit, -2.0f * astrum::astronomical_unit, -2.0f * astrum::astronomical_unit },
          .max = { 2.0f * astrum::astronomical_unit, 2.0f * astrum::astronomical_unit, 2.0f * astrum::astronomical_unit }
        },
        .depth = 4
      }
  );

  auto msaa_color_texture = ludo::add(inst, ludo::texture { .datatype = ludo::pixel_datatype::FLOAT16, .width = window->width, .height = window->height }, { .samples = astrum::msaa_samples });
  auto msaa_depth_texture = ludo::add(inst, ludo::texture { .components = ludo::pixel_components::DEPTH, .datatype = ludo::pixel_datatype::FLOAT32, .width = window->width, .height = window->height }, { .samples = astrum::msaa_samples });
  auto msaa_frame_buffer = ludo::add(inst, ludo::frame_buffer { .width = window->width, .height = window->height, .color_texture_ids = { msaa_color_texture->id }, .depth_texture_id = msaa_depth_texture->id });

  ludo::add(inst, ludo::physics_context { .gravity = ludo::vec3_zero, .visualize = astrum::visualize_physics });

  astrum::add_solar_system(inst);

  ludo::add<ludo::script>(inst, ludo::update_windows);

  // TODO Maybe find a better way to do this?
  ludo::add<ludo::script>(inst, [](ludo::instance& inst)
  {
    auto& vram_draw_commands = data<ludo::draw_command>(inst);
    auto& vram_instances = data<ludo::instance_t>(inst);

    ludo::clear(vram_draw_commands);
    ludo::clear(vram_instances);
  });

  auto& linear_octrees = data<ludo::linear_octree>(inst);
  auto linear_octree_ids = std::vector<uint64_t>();
  for (auto& linear_octree : linear_octrees)
  {
    linear_octree_ids.push_back(linear_octree.id);
  }

  ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
  {
    .frame_buffer_id = msaa_frame_buffer->id,
    .linear_octree_ids = linear_octree_ids
  });

  if (astrum::visualize_physics)
  {
    auto bullet_debug_render_program = ludo::add(
      inst,
      ludo::render_program { .primitive = ludo::mesh_primitive::LINE_LIST },
      ludo::format(false, true),
      "ludo-bullet::visualizations"
    );

    auto bullet_debug_mesh = ludo::add(
      inst,
      ludo::mesh { .render_program_id = bullet_debug_render_program->id },
      bullet_debug_counts.first,
      bullet_debug_counts.second,
      bullet_debug_render_program->format.size,
      "ludo-bullet::visualizations"
    );

    auto bullet_debug_mesh_ininstance = ludo::add(inst, ludo::mesh_instance(), *bullet_debug_mesh, "ludo-bullet::visualizations");

    ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
    {
      .frame_buffer_id = msaa_frame_buffer->id,
      .mesh_instance_ids = { bullet_debug_mesh_ininstance->id },
      .clear_frame_buffer = false
    });
  }

  // Post-processing
  auto post_processing_mesh_instance = astrum::add_post_processing_mesh_instance(inst);
  auto post_processing_vertex_shader = astrum::add_post_processing_vertex_shader(inst);
  astrum::add_pass(inst); // Implicitly converts MSAA textures to regular textures
  //astrum::write_atmosphere_texture(50, 0.25f, 1.0f, astrum::terra_atmosphere_scale, "assets/effects/atmosphere.tiff", 1024);
  astrum::add_atmosphere(inst, post_processing_vertex_shader->id, post_processing_mesh_instance->id, 1, astrum::terra_radius, astrum::terra_radius * astrum::terra_atmosphere_scale);
  astrum::add_bloom(inst, post_processing_vertex_shader->id, post_processing_mesh_instance->id, 5, 0.1f);
  astrum::add_hdr_resolve(inst, post_processing_vertex_shader->id, post_processing_mesh_instance->id);
  astrum::add_pass(inst, true);

  ludo::add<ludo::script>(inst, ludo::wait_for_render);

  ludo::add<ludo::script>(inst, astrum::print_timings);

  std::cout << std::fixed << std::setprecision(2) << "load time (seconds): " << ludo::elapsed(timer) << std::endl;

  ludo::play(inst);

  ludo::deallocate<ludo::animation>(inst);
  ludo::deallocate<ludo::armature>(inst);
  ludo::deallocate<ludo::armature_instance>(inst);
  ludo::deallocate<ludo::body_shape>(inst);
  ludo::deallocate<ludo::dynamic_body>(inst);
  ludo::deallocate<ludo::frame_buffer>(inst);
  ludo::deallocate<ludo::ghost_body>(inst);
  ludo::deallocate<ludo::kinematic_body>(inst);
  ludo::deallocate<ludo::linear_octree>(inst);
  ludo::deallocate<ludo::mesh>(inst);
  ludo::deallocate<ludo::mesh_instance>(inst);
  ludo::deallocate<ludo::render_program>(inst);
  ludo::deallocate<ludo::script>(inst);
  ludo::deallocate<ludo::shader>(inst);
  ludo::deallocate<ludo::static_body>(inst);
  ludo::deallocate<ludo::texture>(inst);
  ludo::deallocate<ludo::window>(inst);

  ludo::deallocate<ludo::physics_context>(inst);
  ludo::deallocate<ludo::rendering_context>(inst);
  ludo::deallocate<ludo::windowing_context>(inst);

  ludo::deallocate<astrum::celestial_body>(inst);
  ludo::deallocate<astrum::game_controls>(inst);
  ludo::deallocate<astrum::map_controls>(inst);
  ludo::deallocate<astrum::patchwork>(inst);
  ludo::deallocate<astrum::person>(inst);
  ludo::deallocate<astrum::person_controls>(inst);
  ludo::deallocate<astrum::point_mass>(inst);
  ludo::deallocate<astrum::solar_system>(inst);
  ludo::deallocate<astrum::spaceship_controls>(inst);

  ludo::deallocate_vram<ludo::draw_command>(inst);
  ludo::deallocate_vram<ludo::mat4>(inst);
  ludo::deallocate_heap_vram<ludo::index_t>(inst);
  ludo::deallocate_heap_vram<ludo::vertex_t>(inst);

  return 0;
}
