#include "../constants.h"
#include "point_masses.h"
#include "util.h"

namespace astrum
{
  void simulate_point_mass_physics(ludo::instance& inst, const std::vector<std::string>& kinematic_partitions)
  {
    auto& kinematic_bodies = ludo::data<ludo::kinematic_body>(inst);
    auto& static_bodies = ludo::data<ludo::static_body>(inst);

    auto& point_masses = ludo::data<point_mass>(inst);

    auto physics_context = ludo::first<ludo::physics_context>(inst);
    auto& celestial_body_static_bodies = ludo::data<ludo::static_body>(inst, "celestial-bodies");

    for (auto& point_mass_partition : point_masses.partitions)
    {
      auto kinematics = std::find(kinematic_partitions.begin(), kinematic_partitions.end(), point_mass_partition.first) != kinematic_partitions.end();
      auto kinematic_body_partition = kinematics ? &(*ludo::find(kinematic_bodies, point_mass_partition.first)).second : nullptr;

      for (auto index = 0; index < point_mass_partition.second.length; index++)
      {
        auto& point_mass = point_mass_partition.second[index];

        if (!point_mass.resting)
        {
          if (kinematic_body_partition)
          {
            auto& kinematic_body = (*kinematic_body_partition)[index];

            // Temporarily update the kinematic body to predict contacts.
            kinematic_body.transform.position += point_mass.linear_velocity * inst.delta_time;
            ludo::commit(kinematic_body);

            auto contacts = ludo::contacts(*physics_context, kinematic_body.id);
            auto deepest_contacts = astrum::deepest_contacts(contacts);
            for (auto& deepest_contact : deepest_contacts)
            {
              auto static_body = ludo::find_by_id(static_bodies.begin(), static_bodies.end(), deepest_contact.body_b_id);
              if (static_body == static_bodies.end())
              {
                continue;
              }

              if (deepest_contact.distance < 0.0f)
              {
                // Remove velocity towards the contact
                auto velocity_toward_contact = ludo::project(point_mass.linear_velocity, deepest_contact.normal_b * -1.0f);
                point_mass.linear_velocity -= velocity_toward_contact;

                // Since we ar no longer moving toward the contact, we need to advance to the contact
                point_mass.transform.position += velocity_toward_contact * inst.delta_time;
                point_mass.transform.position += deepest_contact.normal_b * -deepest_contact.distance;

                if (static_body >= &celestial_body_static_bodies[0] && static_body <= &celestial_body_static_bodies[celestial_body_static_bodies.length - 1])
                {
                  point_mass.resting = true;
                  point_mass.linear_velocity = ludo::vec3_zero;
                }
              }
            }

            point_mass.transform.position += point_mass.linear_velocity * inst.delta_time;
            kinematic_body.transform = point_mass.transform;
            ludo::commit(kinematic_body);
          }
          else
          {
            point_mass.transform.position += point_mass.linear_velocity * inst.delta_time;
          }
        }

        for (auto& child : point_mass.children)
        {
          child->transform = point_mass.transform;
        }
      }
    }
  }

  void sync_render_meshes_with_point_masses(ludo::instance& inst, const std::vector<std::string>& partitions)
  {
    auto& grid = *ludo::first<ludo::grid3>(inst, "default");
    auto& render_meshes = ludo::data<ludo::render_mesh>(inst);

    auto& point_masses = ludo::data<point_mass>(inst);

    for (auto& partition : partitions)
    {
      auto& partition_point_masses = ludo::find(point_masses, partition)->second;
      auto& partition_render_meshes = ludo::find(render_meshes, partition)->second;

      for (auto index = 0; index < partition_point_masses.length; index++)
      {
        auto& render_mesh = partition_render_meshes[index];
        auto& point_mass = partition_point_masses[index];

        auto old_transform = ludo::instance_transform(render_mesh);
        auto new_transform = ludo::mat4(point_mass.transform.position, ludo::mat3(point_mass.transform.rotation));

        ludo::instance_transform(render_mesh) = new_transform;

        auto old_position = ludo::position(old_transform);
        auto new_position = ludo::position(new_transform);

        auto movement = new_position - old_position;
        if (ludo::length2(movement) > 0.0f)
        {
          ludo::remove(grid, render_mesh, old_position);
          ludo::add(grid, render_mesh, new_position);
        }
      }
    }
  }
}
