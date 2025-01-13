#include <ludo/bullet/api.h>

#include "point_masses.h"

namespace astrum
{
  void simulate_point_mass_physics(ludo::instance& inst, std::vector<ludo::container>& containers)
  {
    ludo::run(
      containers,
      {
        // TODO static body collision
        /*ludo::job
        {
          .read_component_names = { "kinematic_body" },
          .write_component_names = { "position", "linear_velocity", "resting" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto kinematic_bodies = reinterpret_cast<const btRigidBody*>(read_component_data[0].data);
            auto positions = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);
            auto linear_velocities = reinterpret_cast<ludo::vec3*>(write_component_data[1].data);
            auto restings = reinterpret_cast<bool*>(write_component_data[2].data);

            for (auto index = 0; index < entity_count; index++)
            {
              auto& kinematic_body = kinematic_bodies[index];
              auto& position = positions[index];
              auto& linear_velocity = linear_velocities[index];
              auto& resting = restings[index];

              if (resting) continue;

              // Temporarily update the kinematic body to predict contacts.
              kinematic_body.transform.position += linear_velocity * inst.delta_time;
              ludo::commit(kinematic_body);

              auto contacts = ludo::contacts(*physics_context, kinematic_body.id);
              auto deepest_contacts = astrum::deepest_contacts(contacts);
              for (auto& deepest_contact : deepest_contacts)
              {
                auto static_body = ludo::find_by_id(static_bodies.begin(), static_bodies.end(), deepest_contact.body_b_id);
                if (static_body == static_bodies.end()) continue;
                if (deepest_contact.distance > 0.0f) continue;

                // Remove velocity towards the contact
                auto velocity_toward_contact = ludo::project(linear_velocity, deepest_contact.normal_b * -1.0f);
                linear_velocity -= velocity_toward_contact;

                // Since we ar no longer moving toward the contact, we need to advance to the contact
                position += velocity_toward_contact * inst.delta_time;
                position += deepest_contact.normal_b * -deepest_contact.distance;

                if (static_body >= &celestial_body_static_bodies[0] && static_body <= &celestial_body_static_bodies[celestial_body_static_bodies.length - 1])
                {
                  resting = true;
                  linear_velocity = ludo::vec3_zero;
                }
              }

              // Revert kinematic body
              kinematic_body.transform = point_mass.transform;
            }
          }
        },*/
        // apply velocity
        ludo::job
        {
          .read_component_names = { "linear_velocity", "resting" },
          .write_component_names = { "position" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto restings = reinterpret_cast<bool*>(read_component_data[0].data);
            auto linear_velocities = reinterpret_cast<ludo::vec3*>(read_component_data[1].data);
            auto positions = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);

            for (auto index = 0; index < entity_count; index++)
            {
              auto& linear_velocity = linear_velocities[index];
              auto& position = positions[index];

              if (restings[index]) continue;

              position += linear_velocity * inst.delta_time;
            }
          }
        },
        ludo::update_bullet_kinematic_bodies,
        // TODO this was to update the person when in the ship. Maybe just despawn instead?
        /*ludo::job
        {
          .write_component_names = { "astrum::point_mass" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto point_masses_start = reinterpret_cast<point_mass*>(write_component_data[0].start);

            for (auto index = 0; index < entity_count; index++)
            {
              auto& point_mass = point_masses_start[index];

              for (auto& child : point_mass.children)
              {
                child->transform = point_mass.transform;
              }
            }
          }
        }*/
      }
    );
  }
}
