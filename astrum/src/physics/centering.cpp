#include "centering.h"

namespace astrum
{
  void center_universe(std::vector<ludo::container>& containers, const ludo::vec3& camera_position)
  {
    if (ludo::length(camera_position) < 10000.0f)
    {
      return;
    }

    ludo::run(
      containers,
      {
        ludo::job
        {
          .write_component_names = { "position" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto positions = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);

            // TODO
            /*for (auto& grid : grids)
            {
              grid.bounds.min -= camera_position;
              grid.bounds.max -= camera_position;
              ludo::commit_header(grid);
            }*/

            for (auto index = 0; index < entity_count; index++)
            {
              positions[index] -= camera_position;
            }
          }
        },ludo::job
        {
          .write_component_names = { "position_vram" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto positions = reinterpret_cast<ludo::vec3*>(write_component_data[0].data);

            for (auto index = 0; index < entity_count; index++)
            {
              positions[index] -= camera_position;
            }
          }
        }
      }
    );
  }
}
