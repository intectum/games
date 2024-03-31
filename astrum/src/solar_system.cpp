#include <future>

#include <ludo/api.h>
#include <ludo/assimp/api.h>

#include "constants.h"
#include "controllers/game.h"
#include "entities/celestial_bodies.h"
#include "entities/luna.h"
#include "entities/people.h"
#include "entities/sol.h"
#include "entities/spaceships.h"
#include "entities/terra.h"
#include "entities/trees.h"
#include "meshes/patchwork.h"
#include "paths.h"
#include "physics/centering.h"
#include "physics/gravity.h"
#include "physics/point_masses.h"
#include "physics/util.h"
#include "solar_system.h"

namespace astrum
{
  void add_solar_system(ludo::instance& inst)
  {
    ludo::add(inst, solar_system());

    const auto terra_initial_position = ludo::vec3 { -1.0f * astronomical_unit, 0.0f, 0.0f };
    const auto terra_initial_velocity = ludo::vec3 { 0.0f, orbital_speed(ludo::length(terra_initial_position), sol_mass), 0.0f };

    const auto luna_initial_position = ludo::vec3 { -1.0f * astronomical_unit + luna_orbit_distance, 0.0f, 0.0f };
    const auto luna_initial_velocity = terra_initial_velocity + ludo::vec3 { 0.0f, orbital_speed(luna_orbit_distance, terra_mass), 0.0f };

    auto person_surface_position = ludo::vec3 { 1.0f, -1.0f, 0.0f };
    ludo::normalize(person_surface_position);
    person_surface_position *= terra_radius + 5.0f;
    const auto person_initial_position = terra_initial_position + person_surface_position;
    const auto person_initial_velocity = ludo::vec3 { 0.0f, orbital_speed(ludo::length(person_initial_position), sol_mass), 0.0f };

    const auto spaceship_initial_position = person_initial_position + ludo::vec3 { 0.0f, 0.0f, 10.0f };
    const auto spaceship_initial_velocity = ludo::vec3 { 0.0f, orbital_speed(ludo::length(spaceship_initial_position), sol_mass), 0.0f };

    // Initialize camera to roughly correct position to ensure the correct LODs are pre-loaded
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);
    ludo::set_camera(
      rendering_context,
      {
        .near_clipping_distance = 0.1f,
        .far_clipping_distance = 2.0f * astronomical_unit,
        .view = ludo::mat4(person_initial_position, ludo::mat3_identity),
        .projection = ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 2.0f * astronomical_unit)
      }
    );

    add_celestial_body(
      inst,
      celestial_body
        {
          .name = "sol",
          .radius = sol_radius,
          .mass = sol_mass,
          .format = ludo::vertex_format_p,
          .height_func = sol_height,
          .color_func = sol_color,
          .tree_func = sol_tree
        },
      sol_lods,
      {},
      ludo::vec3_zero
    );

    add_celestial_body(
      inst,
      celestial_body
        {
          .name = "terra",
          .radius = terra_radius,
          .mass = terra_mass,
          .format = ludo::vertex_format_pnc,
          .height_func = terra_height,
          .color_func = terra_color,
          .tree_func = terra_tree
        },
      terra_lods,
      { .position = terra_initial_position },
      terra_initial_velocity
    );

    add_celestial_body(
      inst,
      celestial_body
        {
          .name = "luna",
          .radius = luna_radius,
          .mass = luna_mass,
          .format = ludo::vertex_format_pn,
          .height_func = luna_height,
          .color_func = luna_color,
          .tree_func = luna_tree
        },
      luna_lods,
      { .position = luna_initial_position },
      luna_initial_velocity
    );

    //add_trees(inst, 1);

    add_person(inst, { .position = person_initial_position }, person_initial_velocity);

    auto spaceship_up = spaceship_initial_position - terra_initial_position;
    ludo::normalize(spaceship_up);
    add_spaceship(inst, { .position = spaceship_initial_position, .rotation = ludo::quat(ludo::vec3_unit_y, spaceship_up) }, spaceship_initial_velocity);

    ludo::add(inst, game_controls());
    ludo::add(inst, map_controls { .target_radius = sol_radius });

    if (show_paths)
    {
      add_prediction_paths(
        inst,
        {
          { 1.0f, 1.0f, 0.0f, 1.0f },
          { 0.0f, 0.5f, 1.0f, 1.0f },
          { 1.0f, 1.0f, 1.0f, 1.0f },
          { 1.0f, 0.0f, 0.0f, 1.0f },
          { 0.0f, 1.0f, 0.0f, 1.0f }
        }
      );
    }

    // Had to put this at the start to prevent frames with unaligned LOD sections
    // Buffer sync issues?
    ludo::add<ludo::script>(inst, ludo::finalize_background);

    ludo::add<ludo::script>(inst, center_universe);

    ludo::add<ludo::script, float, float>(inst, ludo::simulate_physics, 1.0f / 60.0f, game_speed);

    if (visualize_physics)
    {
      ludo::add<ludo::script>(inst, [](ludo::instance& inst)
      {
        auto& always_render_linear_octree = ludo::data<ludo::linear_octree>(inst, "default")[1];
        auto& bullet_debug_mesh = *ludo::first<ludo::mesh>(inst, "ludo-bullet::visualizations");

        // TODO Where should this go? The physics system updates the mesh counts each time so we need to update it in the octree too...
        ludo::remove(always_render_linear_octree, bullet_debug_mesh, ludo::vec3_zero);
        ludo::add(always_render_linear_octree, bullet_debug_mesh, ludo::vec3_zero);
      });
    }

    ludo::add<ludo::script>(inst, simulate_gravity);
    ludo::add<ludo::script>(inst, relativize_to_nearest_celestial_body);
    ludo::add<ludo::script, std::vector<std::string>>(inst, simulate_point_mass_physics, { "people", "spaceships" });

    if (show_paths)
    {
      ludo::add<ludo::script>(inst, update_prediction_paths);
    }

    ludo::add<ludo::script>(inst, simulate_people);
    ludo::add<ludo::script>(inst, simulate_spaceships);

    ludo::add<ludo::script>(inst, control_game);

    //ludo::add<ludo::script, uint32_t>(inst, stream_trees, 1);

    ludo::add<ludo::script, std::vector<std::string>>(inst, sync_meshes_with_point_masses, { "people", "spaceships" });

    ludo::add<ludo::script, std::vector<std::vector<lod>>>(inst, update_celestial_bodies, { sol_lods, terra_lods, luna_lods });
    ludo::add<ludo::script>(inst, update_patchworks);
    ludo::add<ludo::script>(inst, sync_light_with_sol);
  }
}
