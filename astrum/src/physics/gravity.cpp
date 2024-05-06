#include "../constants.h"
#include "gravity.h"

namespace astrum
{
  ludo::vec3 gravitational_force(const ludo::vec3& relative_position, float mass_a, float mass_b);

  void simulate_gravity(ludo::instance& inst)
  {
    auto& dynamic_bodies = ludo::data<ludo::dynamic_body>(inst);

    auto& point_masses = ludo::data<point_mass>(inst);
    auto& solar_system = *ludo::first<astrum::solar_system>(inst);

    auto& celestial_body_point_masses = ludo::data<point_mass>(inst, "celestial-bodies");

    auto body_accelerations = std::vector<ludo::vec3>(dynamic_bodies.length, ludo::vec3_zero);
    auto point_mass_accelerations = std::vector<ludo::vec3>(point_masses.length, ludo::vec3_zero);

    // Body <-> body gravitational acceleration
    for (auto index_a = 0; dynamic_bodies.length && index_a < dynamic_bodies.length - 1; index_a++)
    {
      auto& body_a = dynamic_bodies[index_a];

      for (auto index_b = index_a + 1; index_b < dynamic_bodies.length; index_b++)
      {
        auto& body_b = dynamic_bodies[index_b];

        auto relative_position = body_b.transform.position - body_a.transform.position;
        auto force = gravitational_force(relative_position, body_a.mass, body_b.mass);

        body_accelerations[index_a] += force / body_a.mass;
        body_accelerations[index_b] -= force / body_b.mass;
      }
    }

    // Point mass <-> point mass gravitational acceleration
    for (auto index_a = 0; index_a < point_masses.length && point_masses.length - 1; index_a++)
    {
      auto& point_mass_a = point_masses[index_a];

      for (auto index_b = index_a + 1; index_b < point_masses.length; index_b++)
      {
        auto& point_mass_b = point_masses[index_b];

        auto relative_position = point_mass_b.transform.position - point_mass_a.transform.position;
        if (relative_position == ludo::vec3_zero)
        {
          // TODO A bit of a hack, this should only occur when our pilot is in the spaceship. Probably eventually we can get rid of this when we make the overall vehicle system better.
          continue;
        }

        auto force = gravitational_force(relative_position, point_mass_a.mass, point_mass_b.mass);

        point_mass_accelerations[index_a] += force / point_mass_a.mass;
        point_mass_accelerations[index_b] -= force / point_mass_b.mass;
      }
    }

    // Body <-> point mass gravitational acceleration
    for (auto index_a = 0; index_a < dynamic_bodies.length; index_a++)
    {
      auto& body = dynamic_bodies[index_a];

      for (auto index_b = 0; index_b < point_masses.length; index_b++)
      {
        auto& point_mass = point_masses[index_b];

        auto relative_position = point_mass.transform.position - body.transform.position;
        auto force = gravitational_force(relative_position, body.mass, point_mass.mass);

        body_accelerations[index_a] += force / body.mass;
        point_mass_accelerations[index_b] -= force / point_mass.mass;
      }
    }

    // Cancel out the relative celestial body (since it is static!)
    if (solar_system.relative_celestial_body_index != -1)
    {
      auto relative_point_mass_index = celestial_body_point_masses.begin() - point_masses.begin() + solar_system.relative_celestial_body_index;
      auto relative_point_mass_acceleration = point_mass_accelerations[relative_point_mass_index];

      for (auto& body_acceleration : body_accelerations)
      {
        body_acceleration -= relative_point_mass_acceleration;
      }

      for (auto& point_mass_acceleration : point_mass_accelerations)
      {
        point_mass_acceleration -= relative_point_mass_acceleration;
      }
    }

    // Apply gravitational acceleration to bodies and point masses
    for (auto index = 0; index < dynamic_bodies.length; index++)
    {
      auto& body = dynamic_bodies[index];
      ludo::apply_force(body, body.mass * body_accelerations[index]);
    }

    for (auto index = 0; index < point_masses.length; index++)
    {
      auto& point_mass = point_masses[index];
      if (point_mass.resting)
      {
        continue;
      }

      point_mass.linear_velocity += point_mass_accelerations[index] * inst.delta_time * game_speed;
    }
  }

  ludo::vec3 gravitational_force(const ludo::vec3& relative_position, float mass_a, float mass_b)
  {
    auto force_normal = relative_position;
    normalize(force_normal);

    return force_normal * gravitational_constant * mass_a * mass_b / ludo::length2(relative_position);
  }
}
