#include "../constants.h"
#include "../ecs.h"
#include "../types.h"
#include "spaceships.h"

namespace astrum
{
  void enter_spaceship(ludo::container& container, uint32_t person_index, uint32_t spaceship_index)
  {
    auto person_controlses = reinterpret_cast<person_controls*>(
      get_components(
        container,
        "person",
        "person_controls"
      )->data
    );

    auto& person_controls = person_controlses[person_index];
    person_controls.camera_rotation = ludo::vec2_zero;

    auto people = reinterpret_cast<person*>(
      get_components(
        container,
        "person",
        "person"
      )->data
    );

    auto& person = people[person_index];
    person.standing = false;
    person.jumping = false;
    person.turn_angle = 0.0f;
    person.run_speed = 0.0f;
    person.turn_speed = 0.0f;
    person.walk_animation_time = 0.0f;

    auto restings = reinterpret_cast<bool*>(
      get_components(
        container,
        "person",
        "resting"
      )->data
    );

    restings[person_index] = true;

    // TODO
    //ludo::data<point_mass>(inst, "spaceships")[spaceship_index].children.emplace_back(&person_point_mass);
  }

  void exit_spaceship(ludo::container& container, uint32_t person_index, uint32_t spaceship_index)
  {
    // TODO
    //auto& spaceship_point_mass = ludo::data<point_mass>(inst, "spaceships")[spaceship_index];
    //spaceship_point_mass.children.clear();

    auto person_positions = reinterpret_cast<ludo::vec3*>(
      get_components(
        container,
        "person",
        "position"
      )->data
    );

    auto person_linear_velocities = reinterpret_cast<ludo::vec3*>(
      get_components(
        container,
        "person",
        "linear_velocity"
      )->data
    );

    auto person_restings = reinterpret_cast<bool*>(
      get_components(
        container,
        "person",
        "resting"
      )->data
    );

    auto people = reinterpret_cast<person*>(
      get_components(
        container,
        "person",
        "person"
      )->data
    );

    auto spaceship_rotations = reinterpret_cast<ludo::quat*>(
      get_components(
        container,
        "spaceship",
        "rotation"
      )->data
    );

    auto spaceship_linear_velocities = reinterpret_cast<ludo::vec3*>(
      get_components(
        container,
        "spaceship",
        "linear_velocity"
      )->data
    );

    person_restings[person_index] = false;
    person_linear_velocities[person_index] = spaceship_linear_velocities[spaceship_index];

    auto spaceship_rotation = ludo::mat3(spaceship_rotations[spaceship_index]);
    auto spaceship_right = ludo::right(spaceship_rotation);
    auto spaceship_up = ludo::up(spaceship_rotation);
    person_positions[person_index] += spaceship_right * 1.5f;
    person_positions[person_index] += spaceship_up * 1.0f;

    people[person_index].standing = true;
  }

  void simulate_spaceships(const ludo::instance& inst, std::vector<ludo::container>& containers)
  {
    ludo::run(
      containers,
      {
        ludo::job
        {
          .read_component_names = { "spaceship_controls" },
          .write_component_names = { "rotation", "linear_velocity", "resting" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto spaceship_controlses = reinterpret_cast<const spaceship_controls*>(read_component_data[0].data);
            auto rotations = reinterpret_cast<ludo::quat*>(write_component_data[0].data);
            auto linear_velocities = reinterpret_cast<ludo::vec3*>(write_component_data[1].data);
            auto restings = reinterpret_cast<bool*>(write_component_data[2].data);

            for (auto index = 0; index < entity_count; index++)
            {
              auto& spaceship_controls = spaceship_controlses[index];
              auto& rotation = rotations[index];
              auto& linear_velocity = linear_velocities[index];
              auto& resting = restings[index];

              auto heading_matrix = ludo::mat3(rotation);
              auto heading_right = ludo::right(heading_matrix);
              auto heading_up = ludo::up(heading_matrix);
              auto heading_out = ludo::out(heading_matrix);

              if (spaceship_controls.forward || spaceship_controls.back ||
                  spaceship_controls.left || spaceship_controls.right ||
                  spaceship_controls.up || spaceship_controls.down ||
                  spaceship_controls.roll_left || spaceship_controls.roll_right)
              {
                resting = false;
              }

              if (spaceship_controls.forward)
              {
                auto forward_acceleration = spaceship_rcs_acceleration;
                if (spaceship_controls.thrust)
                {
                  forward_acceleration = spaceship_thrust_acceleration;
                }

                linear_velocity += heading_out * forward_acceleration * inst.delta_time;
              }
              if (spaceship_controls.back)
              {
                linear_velocity -= heading_out * spaceship_rcs_acceleration * inst.delta_time;
              }
              if (spaceship_controls.left)
              {
                linear_velocity += heading_right * spaceship_rcs_acceleration * inst.delta_time;
              }
              if (spaceship_controls.right)
              {
                linear_velocity -= heading_right * spaceship_rcs_acceleration * inst.delta_time;
              }
              if (spaceship_controls.up)
              {
                linear_velocity += heading_up * spaceship_rcs_acceleration * inst.delta_time;
              }
              if (spaceship_controls.down)
              {
                linear_velocity -= heading_up * spaceship_rcs_acceleration * inst.delta_time;
              }

              rotation *= ludo::quat(
                spaceship_controls.yaw * spaceship_turn_speed * inst.delta_time,
                spaceship_controls.pitch * spaceship_turn_speed * inst.delta_time,
                spaceship_controls.roll * spaceship_turn_speed * inst.delta_time
              );

              // Drag the ghost body along for the ride TODO make sure this is done elsewhere...
              //ghost_body.transform = point_mass.transform;
              //ludo::commit(ghost_body);
            }
          }
        }
      }
    );
  }
}
