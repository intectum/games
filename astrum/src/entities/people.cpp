#include "../constants.h"
#include "../ecs.h"
#include "../types.h"
#include "people.h"

namespace astrum
{
  void map_controls(const ludo::instance& inst, std::vector<ludo::container>& containers);

  void simulate_people(const ludo::instance& inst, std::vector<ludo::container>& containers, const ludo::animation& animation, const ludo::armature& armature, uint32_t relative_celestial_body_index)
  {
    auto relative_celestial_body_positions = reinterpret_cast<const ludo::vec3*>(
      get_components(
        containers[relative_celestial_body_index],
        "celestial_body",
        "position"
      )->data
    );

    map_controls(inst, containers);

    ludo::run(
      containers,
      {
        ludo::job
        {
          .read_component_names = { "resting", "person_controls" },
          .write_component_names = { "position", "rotation", "person" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto restings = reinterpret_cast<const bool*>(read_component_data[0].data);
            auto person_controlses = reinterpret_cast<const person_controls*>(read_component_data[1].data);
            auto positions = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);
            auto rotations = reinterpret_cast<ludo::quat*>(write_component_data[1].data);
            auto people = reinterpret_cast<person*>(write_component_data[2].data);

            for (auto index = 0; index < entity_count; index++)
            {
              auto& resting = restings[index];
              auto& person_controls = person_controlses[index];
              auto& position = positions[index];
              auto& rotation = rotations[index];
              auto& person = people[index];

              if (!person.standing) continue;
              if (resting) person.jumping = false;

              // Align to 'stand' on the celestial body
              auto relative_celestial_body_up = position - relative_celestial_body_positions[0];
              ludo::normalize(relative_celestial_body_up);
              rotation = ludo::quat(ludo::vec3_unit_y, relative_celestial_body_up);

              // Turn
              person.turn_angle += person.turn_speed * inst.delta_time;
              rotation *= ludo::quat(0.0f, person.turn_angle, 0.0f);

              // Run
              auto rotation_matrix = ludo::mat3(rotation);
              position += ludo::out(rotation_matrix) * person.run_speed * inst.delta_time;

              // Animate
              if (person.run_speed != 0.0f)
              {
                person.walk_animation_time += inst.delta_time;
              }
              else
              {
                person.walk_animation_time = 0.25f;
              }

              // TODO
              //ludo::interpolate(animation, armature, person.walk_animation_time, ludo::instance_bone_transforms(render_mesh));
            }
          }
        }
      }
    );
  }

  void map_controls(const ludo::instance& inst, std::vector<ludo::container>& containers)
  {
    ludo::run(
      containers,
      {
        ludo::job
        {
          .read_component_names = { "rotation", "person_controls" },
          .write_component_names = { "linear_velocity", "resting", "person" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto rotations = reinterpret_cast<ludo::quat*>(read_component_data[0].data);
            auto person_controlses = reinterpret_cast<const person_controls*>(read_component_data[0].data);
            auto linear_velocities = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);
            auto restings = reinterpret_cast<bool*>(write_component_data[1].data);
            auto people = reinterpret_cast<person*>(write_component_data[2].data);

            for (auto index = 0; index < entity_count; index++)
            {
              auto& person_controls = person_controlses[index];
              auto& rotation = rotations[index];
              auto& linear_velocity = linear_velocities[index];
              auto& resting = restings[index];
              auto& person = people[index];

              if (!person.standing) continue;

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
                resting = false;
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
                auto avatar_up = ludo::up(ludo::mat3(rotation));
                linear_velocity += avatar_up * person_jump_acceleration;
                resting = false;
                person.jumping = true;
              }
            }
          }
        }
      }
    );
  }
}
