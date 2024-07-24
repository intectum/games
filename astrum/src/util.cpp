#include <iostream>

#include <ludo/api.h>

#include "constants.h"
#include "util.h"

namespace astrum
{
  auto last_print_time = 0.0f;
  auto frame_count = 0;

  auto script_names = std::vector<std::string>
  {
    "astrum::center_universe",
    "astrum::relativize_universe",

    "astrum::simulate_gravity",
    "ludo::simulate_physics",
    "astrum::simulate_point_mass_physics",

    "astrum::stream_terrain",
    "astrum::stream_trees",

    "astrum::sync_light_with_sol",

    "astrum::simulate_people",
    "astrum::simulate_spaceships",

    "astrum::control_game",

    "astrum::sync_render_meshes_with_point_masses",

    "ludo::receive_input",
    "ludo::start_render_transaction",
    "ludo::add_render_commands/geometry",
    "ludo::commit_render_commands/geometry",
    "ludo::blit",
    "ludo::commit_render_commands/atmosphere",
    "ludo::commit_render_commands/bloom/brightness",
    "ludo::commit_render_commands/bloom/horizontal0",
    "ludo::commit_render_commands/bloom/vertical0",
    "ludo::commit_render_commands/bloom/horizontal1",
    "ludo::commit_render_commands/bloom/vertical1",
    "ludo::commit_render_commands/bloom/horizontal2",
    "ludo::commit_render_commands/bloom/vertical2",
    "ludo::commit_render_commands/bloom/horizontal3",
    "ludo::commit_render_commands/bloom/vertical3",
    "ludo::commit_render_commands/bloom/horizontal4",
    "ludo::commit_render_commands/bloom/vertical4",
    "ludo::commit_render_commands/bloom/additive",
    "ludo::commit_render_commands/tone_mapping",
    "ludo::blit",
    "ludo::commit_render_transaction",

    "astrum::print_timings"
  };

  void print_timings(ludo::instance& inst)
  {
    if (inst.total_time - last_print_time > 1.0f)
    {
      last_print_time = inst.total_time;

      auto final_script_names = script_names;
      if (visualize_physics)
      {
        final_script_names.insert(final_script_names.begin() + 16, "ludo::add_render_commands/geometry/physics");
      }
      if (show_paths)
      {
        final_script_names.insert(final_script_names.begin() + 27, "astrum::update_prediction_paths");
      }

      auto longest_script_name_size = std::size_t(0);
      for (auto& script_name : final_script_names)
      {
        longest_script_name_size = std::max(script_name.size(), longest_script_name_size);
      }

      std::cout << "FPS: " << frame_count << ", Average script times:" << std::endl;
      auto average_frame_time = 0.0f;
      for (auto index = 0; index < ludo::total_script_times.size(); index++)
      {
        auto script_name = final_script_names[index];
        auto padding_size = longest_script_name_size - script_name.size() + 2;

        auto average_script_time = ludo::total_script_times[index] / static_cast<float>(frame_count) * 1000.0f;
        average_frame_time += average_script_time;
        std::cout << "  " << script_name << std::string(padding_size, ' ') << average_script_time << "ms" << std::endl;
      }

      auto padding_size = longest_script_name_size - std::string("all_scripts").size() + 2;
      std::cout << "  all_scripts" + std::string(padding_size, ' ') << average_frame_time << "ms" << std::endl;

      ludo::total_script_times.clear();
      frame_count = 0;
    }

    frame_count++;
  }
}
