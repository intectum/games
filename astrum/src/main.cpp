#include <iomanip>
#include <iostream>

#include <ludo/api.h>

#include "constants.h"
#include "post-processing/atmosphere.h"
#include "post-processing/bloom.h"
#include "post-processing/hdr_resolve.h"
#include "post-processing/pass.h"
#include "post-processing/util.h"
#include "solar_system.h"
#include "types.h"
#include "util.h"

int main()
{
  auto timer = ludo::timer();

  auto inst = ludo::instance();

  ludo::allocate<ludo::animation>(inst, 1);
  ludo::allocate<ludo::armature>(inst, 1);
  ludo::allocate<ludo::dynamic_body>(inst, 0);
  ludo::allocate<ludo::frame_buffer>(inst, 16);
  ludo::allocate<ludo::ghost_body>(inst, 1);
  ludo::allocate<ludo::kinematic_body>(inst, 2);
  ludo::allocate<ludo::linear_octree>(inst, 5);
  ludo::allocate<ludo::mesh>(inst, 5 + 3 * (5120 * 2));
  ludo::allocate<ludo::mesh_buffer>(inst, 8);
  ludo::allocate<ludo::physics_context>(inst, 1);
  ludo::allocate<ludo::render_program>(inst, 12);
  ludo::allocate<ludo::rendering_context>(inst, 1);
  ludo::allocate<ludo::script>(inst, 36);
  ludo::allocate<ludo::shader>(inst, 19);
  ludo::allocate<ludo::static_body>(inst, 25); // TODO bit of a guess really since they're loaded dynamically
  ludo::allocate<ludo::texture>(inst, 21);
  ludo::allocate<ludo::window>(inst, 1);
  ludo::allocate<ludo::windowing_context>(inst, 1);

  ludo::allocate<astrum::celestial_body>(inst, 3);
  ludo::allocate<astrum::game_controls>(inst, 1);
  ludo::allocate<astrum::map_controls>(inst, 1);
  ludo::allocate<astrum::patchwork>(inst, 3);
  ludo::allocate<astrum::person>(inst, 1);
  ludo::allocate<astrum::person_controls>(inst, 1);
  ludo::allocate<astrum::point_mass>(inst, 5);
  ludo::allocate<astrum::solar_system>(inst, 1);
  ludo::allocate<astrum::spaceship_controls>(inst, 1);

  ludo::allocate<std::vector<ludo::vec3>>(inst, 1);

  ludo::add(inst, ludo::windowing_context());
  auto window = ludo::add(inst, ludo::window { .title = "astrum", .width = 1920, .height = 1080, .v_sync = false });
  //ludo::capture_mouse(*window);

  ludo::add(inst, ludo::rendering_context(), 1);

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

  // Meshes in this linear octree should always render
  ludo::add(
    inst,
    ludo::linear_octree
    {
      .bounds =
      {
        .min = { -2.0f * astrum::astronomical_unit, -2.0f * astrum::astronomical_unit, -2.0f * astrum::astronomical_unit },
        .max = { 2.0f * astrum::astronomical_unit, 2.0f * astrum::astronomical_unit, 2.0f * astrum::astronomical_unit }
      },
      .depth = 0
    }
  );

  auto msaa_color_texture = ludo::add(inst, ludo::texture { .datatype = ludo::pixel_datatype::FLOAT16, .width = window->width, .height = window->height }, { .samples = astrum::msaa_samples });
  auto msaa_depth_texture = ludo::add(inst, ludo::texture { .components = ludo::pixel_components::DEPTH, .datatype = ludo::pixel_datatype::FLOAT32, .width = window->width, .height = window->height }, { .samples = astrum::msaa_samples });
  auto msaa_frame_buffer = ludo::add(inst, ludo::frame_buffer { .width = window->width, .height = window->height, .color_texture_ids = { msaa_color_texture->id }, .depth_texture_id = msaa_depth_texture->id });

  ludo::add(inst, ludo::physics_context { .gravity = ludo::vec3_zero, .visualize = astrum::visualize_physics });

  if (astrum::visualize_physics)
  {
    auto bullet_debug_mesh_buffer = ludo::add(
      inst,
      ludo::mesh_buffer { .primitive = ludo::mesh_primitive::LINE_LIST },
      { .index_count = 10000000, .vertex_count = 10000000, .colors = true },
      "ludo-bullet::visualizations"
    );
    ludo::add(
      inst,
      ludo::mesh
      {
        .mesh_buffer_id = bullet_debug_mesh_buffer->id,
        .index_buffer = bullet_debug_mesh_buffer->index_buffer,
        .vertex_buffer = bullet_debug_mesh_buffer->vertex_buffer
      },
      "ludo-bullet::visualizations"
    );
  }

  ludo::add<ludo::script>(inst, ludo::update_windows);

  ludo::add<ludo::script, ludo::render_options>(inst, ludo::render,
  {
    .frame_buffer_id = msaa_frame_buffer->id
  });

  // Post-processing
  auto post_processing_mesh = astrum::add_post_processing_mesh(inst);
  astrum::add_pass(inst); // Implicitly converts MSAA textures to regular textures
  //astrum::write_atmosphere_texture(50, 0.25f, 1.0f, astrum::terra_atmosphere_scale, "assets/effects/atmosphere.tiff", 1024);
  astrum::add_atmosphere(inst, post_processing_mesh->id, 1, astrum::terra_radius, astrum::terra_radius * astrum::terra_atmosphere_scale);
  astrum::add_bloom(inst, post_processing_mesh->id, 5, 0.1f);
  astrum::add_hdr_resolve(inst, post_processing_mesh->id);
  astrum::add_pass(inst, true);

  ludo::add<ludo::script>(inst, ludo::wait_for_render);

  astrum::add_solar_system(inst);

  ludo::add<ludo::script>(inst, astrum::print_timings);

  std::cout << std::fixed << std::setprecision(2) << "load time (seconds): " << ludo::elapsed(timer) << std::endl;

  ludo::play(inst);

  ludo::deallocate<ludo::animation>(inst);
  ludo::deallocate<ludo::armature>(inst);
  ludo::deallocate<ludo::dynamic_body>(inst);
  ludo::deallocate<ludo::frame_buffer>(inst);
  ludo::deallocate<ludo::ghost_body>(inst);
  ludo::deallocate<ludo::kinematic_body>(inst);
  ludo::deallocate<ludo::linear_octree>(inst);
  ludo::deallocate<ludo::mesh>(inst);
  ludo::deallocate<ludo::mesh_buffer>(inst);
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

  ludo::deallocate<std::vector<ludo::vec3>>(inst);

  return 0;
}
