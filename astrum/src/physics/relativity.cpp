#include "../types.h"
#include "relativity.h"

namespace astrum
{
  int32_t nearest_point_masses_index(const ludo::instance& inst, const ludo::array_buffer<point_mass>& point_masses);

  void relativize_universe(ludo::instance& inst)
  {
    auto& all_dynamic_bodies = ludo::data<ludo::dynamic_body>(inst);

    auto& all_point_masses = ludo::data<point_mass>(inst);
    auto& solar_system = *ludo::first<astrum::solar_system>(inst);

    auto& celestial_body_point_masses = ludo::data<point_mass>(inst, "celestial-bodies");

    auto new_celestial_body_relative_index = nearest_point_masses_index(inst, celestial_body_point_masses);
    if (new_celestial_body_relative_index == solar_system.relative_celestial_body_index)
    {
      return;
    }

    solar_system.relative_celestial_body_index = new_celestial_body_relative_index;

    auto target_linear_velocity = celestial_body_point_masses[solar_system.relative_celestial_body_index].linear_velocity;

    for (auto& body: all_dynamic_bodies)
    {
      ludo::pull(body);
      body.linear_velocity -= target_linear_velocity;
      ludo::push(body);
    }

    for (auto& point_mass: all_point_masses)
    {
      point_mass.linear_velocity -= target_linear_velocity;
    }
  }

  int32_t nearest_point_masses_index(const ludo::instance& inst, const ludo::array_buffer<point_mass>& point_masses)
  {
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto camera = ludo::get_camera(rendering_context);
    auto camera_position = ludo::position(camera.view);

    auto nearest_index = int32_t(-1);
    auto shortest_length2 = std::numeric_limits<float>::max();
    for (auto index = 0; index < point_masses.array_size; index++)
    {
      auto relative_length2 = length2(point_masses[index].transform.position - camera_position);
      if (relative_length2 < shortest_length2)
      {
        shortest_length2 = relative_length2;
        nearest_index = index;
      }
    }

    return nearest_index;
  }
}
