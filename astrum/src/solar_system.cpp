#include <future>

#include <ludo/api.h>

#include "constants.h"
#include "controllers/game.h"
#include "entities/luna.h"
#include "entities/people.h"
#include "entities/sol.h"
#include "entities/spaceships.h"
#include "entities/terra.h"
#include "entities/trees.h"
#include "paths.h"
#include "physics/centering.h"
#include "physics/gravity.h"
#include "physics/point_masses.h"
#include "physics/relativity.h"
#include "physics/util.h"
#include "solar_system.h"
#include "terrain/terrain.h"

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

    // Initialize camera to roughly correct position to ensure the correct terrain LODs are pre-loaded
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

    add_sol(inst, {}, ludo::vec3_zero);
    add_terra(inst, { .position = terra_initial_position }, terra_initial_velocity);
    add_luna(inst, { .position = luna_initial_position }, luna_initial_velocity);

    //add_trees(inst, 1);

    ludo::import(inst, "assets/models/minifig.dae", {}, "people");
    add_person(inst, { .position = person_initial_position }, person_initial_velocity);

    ludo::import(inst, "assets/models/spaceship.obj", {}, "spaceships");
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

    ludo::add<ludo::script>(inst, ludo::finalize_background_tasks);

    ludo::add<ludo::script>(inst, center_universe);
    ludo::add<ludo::script>(inst, relativize_universe);

    ludo::add<ludo::script>(inst, simulate_gravity);
    ludo::add<ludo::script, float, float>(inst, ludo::simulate_physics, 1.0f / 60.0f, game_speed);
    ludo::add<ludo::script, std::vector<std::string>>(inst, simulate_point_mass_physics, { "people", "spaceships" });

    ludo::add<ludo::script>(inst, stream_terrain);
    //ludo::add<ludo::script, uint32_t>(inst, stream_trees, 1);

    ludo::add<ludo::script>(inst, sync_light_with_sol);

    ludo::add<ludo::script>(inst, simulate_people);
    ludo::add<ludo::script>(inst, simulate_spaceships);

    ludo::add<ludo::script>(inst, control_game);

    ludo::add<ludo::script, std::vector<std::string>>(inst, sync_mesh_instances_with_point_masses, { "people", "spaceships" });

    if (show_paths)
    {
      ludo::add<ludo::script>(inst, update_prediction_paths);
    }
  }
}
