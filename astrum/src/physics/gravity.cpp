#include "gravity.h"
#include "../constants.h"
#include "../ecs.h"

namespace astrum
{
  ludo::vec3 gravitational_force(const ludo::vec3& relative_position, float mass_a, float mass_b);

  void simulate_gravity(ludo::instance& inst, std::vector<ludo::container>& containers, uint32_t relative_celestial_body_index)
  {
    auto relative_celestial_body_gravitational_accelerations = reinterpret_cast<const ludo::vec3*>(
      get_components(
        containers[relative_celestial_body_index],
        "celestial_body",
        "gravitational_acceleration"
      )->data
    );

    ludo::run(
      containers,
      {
        // Calculate gravitational acceleration
        ludo::job
        {
          .read_component_names = { "position", "mass" },
          .write_component_names = { "gravitational_acceleration" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto positions = reinterpret_cast<const ludo::vec3*>(read_component_data[0].data);
            auto masses = reinterpret_cast<const float*>(read_component_data[1].data);
            auto gravitational_accelerations = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);

            for (auto index = 0; index < entity_count; index++)
            {
              auto& position = positions[index];
              auto& mass = masses[index];
              auto& gravitational_acceleration = gravitational_accelerations[index];

              gravitational_acceleration = ludo::vec3_zero;

              ludo::run(
                containers,
                {
                  ludo::job
                  {
                    .read_component_names = { "position", "mass" },
                    .kernel = [&](uint32_t other_entity_start, uint32_t other_entity_count, const std::vector<ludo::arena>& other_read_component_data, std::vector<ludo::arena>& other_write_component_data)
                    {
                      auto other_positions = reinterpret_cast<const ludo::vec3*>(read_component_data[0].data);
                      auto other_masses = reinterpret_cast<const float*>(read_component_data[1].data);

                      for (auto other_index = 0; other_index < other_entity_count; other_index++)
                      {
                        if (other_entity_start + other_index == entity_start + index) continue;

                        auto& other_position = other_positions[index];
                        auto& other_mass = other_masses[index];

                        auto relative_position = other_position - position;

                        // TODO A bit of a hack, this should only occur when our pilot is in the spaceship. Probably eventually we can get rid of this when we make the overall vehicle system better.
                        if (relative_position == ludo::vec3_zero) continue;

                        auto force = gravitational_force(relative_position, mass, other_mass);

                        gravitational_acceleration += force / other_mass;
                      }
                    }
                  }
                }
              );
            }
          }
        },
        // Cancel out the relative celestial body (since it is static!)
        ludo::job
        {
          .write_component_names = { "gravitational_acceleration" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto gravitational_accelerations = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);

            for (auto index = 0; index < entity_count; index++)
            {
              auto& gravitational_acceleration = gravitational_accelerations[index];

              if (&gravitational_acceleration == relative_celestial_body_gravitational_accelerations) return;

              gravitational_acceleration -= relative_celestial_body_gravitational_accelerations[0];
            }
          }
        },
        // Apply gravitational acceleration
        ludo::job
        {
          .read_component_names = { "gravitational_acceleration", "resting" },
          .write_component_names = { "linear_velocity" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto gravitational_accelerations = reinterpret_cast<const ludo::vec3*>(read_component_data[0].data);
            auto restings = reinterpret_cast<const float*>(read_component_data[1].data);
            auto linear_velocities = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);

            for (auto index = 0; index < entity_count; index++)
            {
              if (restings[index]) continue;

              linear_velocities[index] += gravitational_accelerations[index] * inst.delta_time;
            }
          }
        }
      }
    );
  }

  ludo::vec3 gravitational_force(const ludo::vec3& relative_position, float mass_a, float mass_b)
  {
    auto force_normal = relative_position;
    normalize(force_normal);

    return force_normal * gravitational_constant * mass_a * mass_b / ludo::length2(relative_position);
  }
}
