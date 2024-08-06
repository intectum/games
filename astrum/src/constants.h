#pragma once

#include <ludo/api.h>

#include "meshes/lods.h"

namespace astrum
{
  // Game
  const auto visualize_physics = true;

  // Assets
  const auto import_assets = false;

  // Rendering
  // TODO Handle this better, 16 here crashed on my new laptop (Zephyrus G14)
  const auto multi_sample_count = uint8_t(8);

  // Physics
  const auto astronomical_unit = 149597870700.0f * 0.00005f;
  const auto gravitational_constant = 0.0000000000667408f * 10000000000.0f;
  const auto planetary_scale = 0.001f;

  // Controllers
  const auto camera_rotate_speed = ludo::pi / 2.0f;
  const auto person_run_acceleration = 6.0f;
  const auto person_run_deceleration = 12.0f;
  const auto person_run_max_speed = 6.0f;
  const auto person_turn_acceleration = ludo::two_pi * 2.0f;
  const auto person_turn_deceleration = ludo::two_pi * 4.0f;
  const auto person_turn_max_speed = ludo::two_pi * 2.0f;
  const auto person_jump_acceleration = 3.0f;
  const auto spaceship_rcs_acceleration = 20.0f;
  const auto spaceship_thrust_acceleration = 50.0f;
  const auto spaceship_turn_speed = ludo::pi / 4.0f;

  // Paths
  const auto show_paths = false;
  const auto path_delta_time = 60.0f;
  const auto path_steps = uint32_t(60);
  const auto path_central_index = int32_t(0);

  // Sol
  const auto sol_radius = 695508000.0f * planetary_scale;
  const auto sol_surface_gravity = 274.0f;
  const auto sol_mass = sol_surface_gravity * sol_radius * sol_radius * gravitational_constant;
  const auto sol_lods = std::vector<lod> { { 5, 0.0f } };

  // Terra
  const auto terra_radius = 6371000.0f * planetary_scale;
  const auto terra_atmosphere_scale = 1.3f;
  const auto terra_surface_gravity = 9.8f;
  const auto terra_mass = terra_surface_gravity * terra_radius * terra_radius * gravitational_constant;
  const auto terra_lods = std::vector<lod> { { 5, terra_radius * 25.6f }, { 6, terra_radius * 6.4f }, { 8, terra_radius * 1.6f }, { 10, terra_radius * 0.4f }, { 12, terra_radius * 0.1f } };

  // Luna
  const auto luna_orbit_distance = 0.00257f * astronomical_unit;
  const auto luna_radius = 1737400.0f * planetary_scale;
  const auto luna_surface_gravity = 1.6209f;
  const auto luna_mass = luna_surface_gravity * luna_radius * luna_radius * gravitational_constant;
  const auto luna_lods = std::vector<lod> { { 5, luna_radius * 6.4f }, { 7, luna_radius * 1.6f }, { 9, luna_radius * 0.4f }, { 11, luna_radius * 0.1f } };

  // Trees
  const auto tree_type_count = uint32_t(4);
  const auto tree_types = std::vector<std::string> { "fruit", "oak", "palm", "pine" };
  const auto tree_lods = std::vector<lod> { { 0, terra_radius * 1.6f }, { 1, terra_radius * 0.3f }, { 2, terra_radius * 0.2f }, { 3, terra_radius * 0.1f } };
}
