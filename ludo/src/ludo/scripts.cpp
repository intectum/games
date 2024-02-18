/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "scripts.h"
#include "tasks.h"
#include "timer.h"

namespace ludo
{
  std::vector<float> total_script_times;

  script series(const std::vector<script>& scripts)
  {
    return [scripts](ludo::instance& instance)
    {
      for (auto index = 0; index < scripts.size(); index++)
      {
        auto timer = ludo::timer();

        scripts[index](instance);

        if (total_script_times.size() < index)
        {
          // Don't record the time. We're in no-man's land now...
        }
        else
        {
          if (total_script_times.size() < index + 1)
          {
            total_script_times.emplace_back();
          }

          total_script_times[index] += elapsed(timer);
        }
      }
    };
  }

  script parallel(const std::vector<script>& scripts)
  {
    return [scripts](ludo::instance& instance)
    {
      auto tasks = std::vector<task>();
      for (auto& script : scripts)
      {
        tasks.emplace_back([&instance, &script]()
        {
          script(instance);

          return []() {};
        });
      }

      parallel(tasks);
    };
  }
}
