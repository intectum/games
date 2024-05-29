#include "../constants.h"
#include "people.h"

namespace astrum
{
  void map_controls(ludo::instance& inst, const person_controls& person_controls, person& person, point_mass& point_mass);

  void add_person(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity)
  {
    auto dynamic_body_shape = ludo::first<ludo::dynamic_body_shape>(inst, "people");
    auto grid = ludo::first<ludo::grid3>(inst, "default");
    auto mesh = ludo::first<ludo::mesh>(inst, "people");
    auto physics_context = ludo::first<ludo::physics_context>(inst);

    auto& render_commands = ludo::data_heap(inst, "ludo::vram_render_commands");
    auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
    auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

    auto render_program = ludo::add(inst, ludo::render_program(), "people");
    ludo::init(*render_program, ludo::format(true, true, true, true), render_commands, 1);

    auto render_mesh = ludo::add(inst, ludo::render_mesh(), "people");
    ludo::init(*render_mesh);
    ludo::connect(*render_mesh, *render_program, 1);
    ludo::connect(*render_mesh, *mesh, indices, vertices);

    ludo::instance_transform(*render_mesh) = ludo::mat4(initial_transform.position, ludo::mat3(initial_transform.rotation));
    ludo::add(*grid, *render_mesh, initial_transform.position);

    ludo::add(
      inst,
      point_mass
      {
        .mass = 0.001f * gravitational_constant,
        .transform = initial_transform,
        .linear_velocity = initial_velocity
      },
      "people"
    );

    auto kinematic_body = ludo::add(
      inst,
      ludo::kinematic_body { .transform = initial_transform },
      "people"
    );
    ludo::init(*kinematic_body, *physics_context);
    ludo::connect(*kinematic_body, *physics_context, { *dynamic_body_shape });

    ludo::add(inst, person(), "people");
    ludo::add(inst, person_controls(), "people");
  }

  void simulate_people(ludo::instance& inst)
  {
    auto& animation = *ludo::first<ludo::animation>(inst, "people");
    auto& armature = *ludo::first<ludo::armature>(inst, "people");
    auto& render_meshes = ludo::data<ludo::render_mesh>(inst, "people");

    auto& celestial_body_point_masses = ludo::data<point_mass>(inst, "celestial-bodies");

    auto solar_system = ludo::first<astrum::solar_system>(inst);
    auto& person_controls_list = ludo::data<astrum::person_controls>(inst, "people");
    auto& people = ludo::data<astrum::person>(inst, "people");
    auto& point_masses = ludo::data<astrum::point_mass>(inst, "people");

    for (auto index = 0; index < people.length; index++)
    {
      auto& render_mesh = render_meshes[index];

      auto& person_controls = person_controls_list[index];
      auto& person = people[index];
      auto& point_mass = point_masses[index];

      if (!person.standing)
      {
        continue;
      }

      map_controls(inst, person_controls, person, point_mass);

      if (point_mass.resting)
      {
        person.jumping = false;
      }

      // Align to 'stand' on the celestial body
      if (solar_system->relative_celestial_body_index != -1)
      {
        auto relative_celestial_body_up = point_mass.transform.position - celestial_body_point_masses[solar_system->relative_celestial_body_index].transform.position;
        ludo::normalize(relative_celestial_body_up);
        point_mass.transform.rotation = ludo::quat(ludo::vec3_unit_y, relative_celestial_body_up);
      }

      // Turn
      person.turn_angle += person.turn_speed * inst.delta_time;
      point_mass.transform.rotation *= ludo::quat(0.0f, person.turn_angle, 0.0f);

      // Run
      auto transform_matrix = ludo::mat3(point_mass.transform.rotation);
      point_mass.transform.position += ludo::out(transform_matrix) * person.run_speed * inst.delta_time;

      // Animate
      if (person.run_speed != 0.0f)
      {
        person.walk_animation_time += inst.delta_time;
      }
      else
      {
        person.walk_animation_time = 0.25f;
      }

      ludo::interpolate(animation, armature, person.walk_animation_time, ludo::instance_bone_transforms(render_mesh));
    }
  }

  void map_controls(ludo::instance& inst, const person_controls& person_controls, person& person, point_mass& point_mass)
  {
    auto angle_to_desired_heading = person_controls.camera_rotation[1] - person.turn_angle;
    if (person_controls.forward)
    {
      if (person_controls.left)
      {
        angle_to_desired_heading += ludo::pi * 0.25f;
      }
      else if (person_controls.right)
      {
        angle_to_desired_heading -= ludo::pi * 0.25f;
      }
    }
    else if (person_controls.back)
    {
      if (person_controls.left)
      {
        angle_to_desired_heading += ludo::pi * 0.75f;
      }
      else if (person_controls.right)
      {
        angle_to_desired_heading -= ludo::pi * 0.75f;
      }
      else
      {
        angle_to_desired_heading += ludo::pi;
      }
    }
    else
    {
      if (person_controls.left)
      {
        angle_to_desired_heading += ludo::pi * 0.5f;
      }
      else if (person_controls.right)
      {
        angle_to_desired_heading -= ludo::pi * 0.5f;
      }
    }

    angle_to_desired_heading = ludo::shortest_angle(angle_to_desired_heading);

    if (person_controls.forward || person_controls.back || person_controls.left || person_controls.right)
    {
      person.run_speed += person_run_acceleration * inst.delta_time;
      person.turn_speed += person_turn_acceleration * ludo::sign(angle_to_desired_heading) * inst.delta_time;
      point_mass.resting = false;
    }
    else
    {
      if (person.run_speed != 0)
      {
        person.run_speed -= person_run_deceleration * ludo::sign(person.run_speed) * inst.delta_time;
        if (std::abs(person.run_speed) < person_run_deceleration * inst.delta_time)
        {
          person.run_speed = 0.0f;
        }
      }

      if (person.turn_speed != 0)
      {
        person.turn_speed -= person_turn_deceleration * ludo::sign(angle_to_desired_heading) * inst.delta_time;
        if (std::abs(person.turn_speed) < person_turn_deceleration * inst.delta_time)
        {
          person.turn_speed = 0.0f;
        }
      }
    }

    person.run_speed = std::min(person.run_speed, person_run_max_speed); // Clamp to max
    person.turn_speed = std::min(std::abs(person.turn_speed), std::abs(angle_to_desired_heading)) * ludo::sign(person.turn_speed); // Slow turn when nearing correct angle
    person.turn_speed = std::max(std::min(person.turn_speed, person_turn_max_speed), -person_turn_max_speed); // Clamp to max/min

    if (person_controls.jump && !person.jumping)
    {
      auto avatar_up = ludo::up(ludo::mat3(point_mass.transform.rotation));
      point_mass.linear_velocity += avatar_up * person_jump_acceleration;
      point_mass.resting = false;
      person.jumping = true;
    }
  }
}
