/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "core.h"
#include "timer.h"

namespace ludo
{
  uint64_t next_id = 1;

  void play(instance& instance, const std::function<void(ludo::instance& instance)>& kernel)
  {
    auto total_timer = timer();

    instance.playing = true;
    while (instance.playing)
    {
      frame(instance, kernel);

      instance.total_time = elapsed(total_timer);
    }
  }

  void stop(instance& instance)
  {
    instance.playing = false;
  }

  void frame(instance& instance, const std::function<void(ludo::instance& instance)>& kernel)
  {
    auto delta_timer = timer();

    kernel(instance);

    instance.delta_time = elapsed(delta_timer);
  }
}
