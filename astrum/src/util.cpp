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
    "ludo::finalize_background",

    "astrum::center_universe",
    "astrum::relativize_universe",

    "astrum::simulate_gravity",
    "ludo::simulate_physics",
    "astrum::simulate_point_mass_physics",

    "astrum::stream_terrain",
    //"astrum::stream_trees",

    "astrum::sync_light_with_sol",

    "astrum::simulate_people",
    "astrum::simulate_spaceships",

    "astrum::control_game",

    "astrum::sync_mesh_instances_with_point_masses",

    "ludo::update_windows",
    "ludo::clear_vram_data",
    "ludo::render/geometry",
    "ludo::blit",
    "astrum::update_atmosphere",
    "ludo::render/atmosphere",
    "ludo::render/bloom/brightness",
    "ludo::render/bloom/horizontal0",
    "ludo::render/bloom/vertical0",
    "ludo::render/bloom/horizontal1",
    "ludo::render/bloom/vertical1",
    "ludo::render/bloom/horizontal2",
    "ludo::render/bloom/vertical2",
    "ludo::render/bloom/horizontal3",
    "ludo::render/bloom/vertical3",
    "ludo::render/bloom/horizontal4",
    "ludo::render/bloom/vertical4",
    "ludo::render/bloom/additive",
    "ludo::render/hdr_resolve",
    "ludo::blit",
    "ludo::wait_for_render",

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
        final_script_names.insert(final_script_names.begin() + 16, "ludo::render/physics");
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
