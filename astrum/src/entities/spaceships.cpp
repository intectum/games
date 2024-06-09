#include "../types.h"
#include "spaceships.h"

namespace astrum
{
  void add_spaceship(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity)
  {
    auto grid = ludo::first<ludo::grid3>(inst, "default");
    auto mesh = ludo::first<ludo::mesh>(inst, "spaceships");
    auto physics_context = ludo::first<ludo::physics_context>(inst);

    auto& render_commands = ludo::data_heap(inst, "ludo::vram_render_commands");
    auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
    auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

    auto render_program = ludo::add(inst, ludo::render_program(), "spaceships");
    ludo::init(*render_program, ludo::vertex_format_pn, render_commands, 1);

    auto render_mesh = ludo::add(inst, ludo::render_mesh(), "spaceships");
    ludo::init(*render_mesh, *render_program, *mesh, indices, vertices, 1);

    ludo::instance_transform(*render_mesh) = ludo::mat4(initial_transform.position, ludo::mat3(initial_transform.rotation));
    ludo::add(*grid, *render_mesh, initial_transform.position);

    ludo::add(
      inst,
      astrum::point_mass
      {
        .mass = 0.1f * gravitational_constant,
        .transform = initial_transform,
        .linear_velocity = initial_velocity
      },
      "spaceships"
    );

    auto kinematic_shape = ludo::add(
      inst,
      ludo::dynamic_body_shape
      {
        .convex_hulls =
        {{
          { 0.8f, 0.5f, 4.25f },
          { 0.8f, -0.5f, 4.25f },
          { -0.8f, 0.5f, 4.25f },
          { -0.8f, -0.5f, 4.25f },
          { 0.8f, 0.5f, -1.25f },
          { 0.8f, -0.5f, -1.25f },
          { -0.8f, 0.5f, -1.25f },
          { -0.8f, -0.5f, -1.25f }
        }}
      },
      "spaceships"
    );
    ludo::init(*kinematic_shape);

    auto kinematic_body = ludo::add(
      inst,
      ludo::kinematic_body
      {
        .transform = initial_transform,
        .linear_velocity = initial_velocity,
        .angular_velocity = ludo::vec3_zero
      },
      "spaceships"
    );
    ludo::init(*kinematic_body, *physics_context);
    ludo::connect(*kinematic_body, *physics_context, { *kinematic_shape });

    auto ghost_shape = ludo::add(
      inst,
      ludo::dynamic_body_shape
      {
        .convex_hulls =
        {{
          { 1.8f, 1.5f, 5.25f },
          { 1.8f, -1.5f, 5.25f },
          { -1.8f, 1.5f, 5.25f },
          { -1.8f, -1.5f, 5.25f },
          { 1.8f, 1.5f, -2.25f },
          { 1.8f, -1.5f, -2.25f },
          { -1.8f, 1.5f, -2.25f },
          { -1.8f, -1.5f, -2.25f }
        }}
      }
    );
    ludo::init(*ghost_shape);

    auto ghost_body = ludo::add(
      inst,
      ludo::ghost_body { .transform = initial_transform },
      "spaceships"
    );
    ludo::init(*ghost_body, *physics_context);
    ludo::connect(*ghost_body, *physics_context, { *ghost_shape });

    ludo::add(inst, spaceship_controls(), "spaceships");
  }

  void enter_spaceship(ludo::instance& inst, uint32_t person_index, uint32_t spaceship_index)
  {
    auto& person_controls = ludo::data<astrum::person_controls>(inst, "people")[person_index];
    person_controls.camera_rotation = ludo::vec2_zero;

    auto& person = ludo::data<astrum::person>(inst, "people")[person_index];
    person.standing = false;
    person.jumping = false;
    person.turn_angle = 0.0f;
    person.run_speed = 0.0f;
    person.turn_speed = 0.0f;
    person.walk_animation_time = 0.0f;

    auto& person_point_mass = ludo::data<point_mass>(inst, "people")[person_index];
    person_point_mass.resting = true;

    ludo::data<point_mass>(inst, "spaceships")[spaceship_index].children.emplace_back(&person_point_mass);
  }

  void exit_spaceship(ludo::instance& inst, uint32_t person_index, uint32_t spaceship_index)
  {
    auto& spaceship_point_mass = ludo::data<point_mass>(inst, "spaceships")[spaceship_index];
    spaceship_point_mass.children.clear();

    auto& person_point_mass = ludo::data<point_mass>(inst, "people")[person_index];
    person_point_mass.resting = false;
    person_point_mass.linear_velocity = spaceship_point_mass.linear_velocity;

    auto spaceship_rotation = ludo::mat3(spaceship_point_mass.transform.rotation);
    auto spaceship_right = ludo::right(spaceship_rotation);
    auto spaceship_up = ludo::up(spaceship_rotation);
    person_point_mass.transform.position += spaceship_right * 1.5f;
    person_point_mass.transform.position += spaceship_up * 1.0f;

    auto& person = ludo::data<astrum::person>(inst, "people")[person_index];
    person.standing = true;
  }

  void simulate_spaceships(ludo::instance& inst)
  {
    auto& ghost_bodies = ludo::data<ludo::ghost_body>(inst, "spaceships");

    auto& spaceship_controls_list = ludo::data<astrum::spaceship_controls>(inst, "spaceships");
    auto& point_masses = ludo::data<point_mass>(inst, "spaceships");

    for (auto index = 0; index < point_masses.length; index++)
    {
      auto& ghost_body = ghost_bodies[index];

      auto& spaceship_controls = spaceship_controls_list[index];
      auto& point_mass = point_masses[index];

      auto heading_matrix = ludo::mat3(point_mass.transform.rotation);
      auto heading_right = ludo::right(heading_matrix);
      auto heading_up = ludo::up(heading_matrix);
      auto heading_out = ludo::out(heading_matrix);

      if (spaceship_controls.forward || spaceship_controls.back ||
          spaceship_controls.left || spaceship_controls.right ||
          spaceship_controls.up || spaceship_controls.down ||
          spaceship_controls.roll_left || spaceship_controls.roll_right)
      {
        point_mass.resting = false;
      }

      if (spaceship_controls.forward)
      {
        auto forward_acceleration = spaceship_rcs_acceleration;
        if (spaceship_controls.thrust)
        {
          forward_acceleration = spaceship_thrust_acceleration;
        }

        point_mass.linear_velocity += heading_out * forward_acceleration * inst.delta_time;
      }
      if (spaceship_controls.back)
      {
        point_mass.linear_velocity -= heading_out * spaceship_rcs_acceleration * inst.delta_time;
      }
      if (spaceship_controls.left)
      {
        point_mass.linear_velocity += heading_right * spaceship_rcs_acceleration * inst.delta_time;
      }
      if (spaceship_controls.right)
      {
        point_mass.linear_velocity -= heading_right * spaceship_rcs_acceleration * inst.delta_time;
      }
      if (spaceship_controls.up)
      {
        point_mass.linear_velocity += heading_up * spaceship_rcs_acceleration * inst.delta_time;
      }
      if (spaceship_controls.down)
      {
        point_mass.linear_velocity -= heading_up * spaceship_rcs_acceleration * inst.delta_time;
      }

      point_mass.transform.rotation *= ludo::quat(
        spaceship_controls.yaw * spaceship_turn_speed * inst.delta_time,
        spaceship_controls.pitch * spaceship_turn_speed * inst.delta_time,
        spaceship_controls.roll * spaceship_turn_speed * inst.delta_time
      );

      // Drag the ghost body along for the ride
      ghost_body.transform = point_mass.transform;
      ludo::commit(ghost_body);
    }
  }
}
