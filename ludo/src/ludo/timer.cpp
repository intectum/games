/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "timer.h"

namespace ludo
{
  float time_since(std::chrono::time_point<std::chrono::high_resolution_clock> start);

  void pause(timer& timer)
  {
    timer.paused_at = std::chrono::high_resolution_clock::now();
  }

  bool paused(const timer& timer)
  {
    return timer.paused_at != std::chrono::time_point<std::chrono::high_resolution_clock>::min();
  }

  void resume(timer& timer)
  {
    if (paused(timer))
    {
      timer.pause_duration += time_since(timer.paused_at);
      timer.paused_at = std::chrono::time_point<std::chrono::high_resolution_clock>::min();
    }
  }

  void reset(timer& timer)
  {
    timer.started_at = std::chrono::high_resolution_clock::now();
    timer.paused_at = std::chrono::time_point<std::chrono::high_resolution_clock>::min();
    timer.pause_duration = 0.0f;
  }

  float elapsed(const timer& timer)
  {
    return time_since(timer.started_at) - timer.pause_duration;
  }

  float time_since(std::chrono::time_point<std::chrono::high_resolution_clock> start)
  {
    auto now = std::chrono::high_resolution_clock::now();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();

    return static_cast<float>(nanoseconds) / 1000000000.0f;
  }
}
