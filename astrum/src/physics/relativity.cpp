#include "../ecs.h"
#include "../types.h"
#include "relativity.h"

namespace astrum
{
  uint32_t relativize_universe(std::vector<ludo::container>& containers, uint32_t relative_celestial_body_index, const ludo::vec3& camera_position)
  {
    auto new_celestial_body_relative_index = uint32_t(0);

    ludo::run(
      containers,
      {
        ludo::job
        {
          // inclusion of 'radius' is a hack to ensure only celestial bodies are selected
          .read_component_names = { "position", "radius" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto positions = reinterpret_cast<const ludo::vec3*>(read_component_data[0].data);

            auto shortest_length2 = std::numeric_limits<float>::max();
            for (auto index = 0; index < entity_count; index++)
            {
              auto relative_length2 = length2(positions[index] - camera_position);
              if (relative_length2 < shortest_length2)
              {
                shortest_length2 = relative_length2;
                new_celestial_body_relative_index = index;
              }
            }
          }
        },
        ludo::job
        {
          .write_component_names = { "linear_velocity" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto linear_velocities = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);

            if (new_celestial_body_relative_index == relative_celestial_body_index) return relative_celestial_body_index;

            auto relative_celestial_body_linear_velocities = reinterpret_cast<const ludo::vec3*>(
              get_components(
                containers[new_celestial_body_relative_index],
                "celestial_body",
                "linear_velocity"
              )->data
            );

            for (auto index = 0; index < entity_count; index++)
            {
              linear_velocities[index] -= relative_celestial_body_linear_velocities[0];
            }
          }
        }
      }
    );

    return new_celestial_body_relative_index;
  }
}
