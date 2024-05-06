/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <vector>

#include "core.h"
#include "data/data.h"
#include "scripts.h"
#include "timer.h"

namespace ludo
{
  uint64_t next_id = 1;

  void play(instance& instance)
  {
    auto total_timer = timer();

    instance.playing = true;
    while (instance.playing)
    {
      frame(instance);

      instance.total_time = elapsed(total_timer);
    }
  }

  void stop(instance& instance)
  {
    instance.playing = false;
  }

  void frame(instance& instance)
  {
    auto delta_timer = timer();

    assert(exists<ludo::script>(instance) && "scripts not found");
    auto& scripts = data<ludo::script>(instance, "default");

    // Make a copy of the scripts array to allow for insert/removal of scripts from within a script.
    auto scripts_copy = std::vector<ludo::script>(scripts.begin(), scripts.end());

    series(scripts_copy)(instance);

    instance.delta_time = elapsed(delta_timer);
  }
}
