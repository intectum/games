/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_TIMER_H
#define LUDO_TIMER_H

#include <chrono>

#include "core.h"

namespace ludo
{
  ///
  /// A high precision timer
  struct LUDO_API timer
  {
    std::chrono::time_point<std::chrono::high_resolution_clock> started_at = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> paused_at = std::chrono::time_point<std::chrono::high_resolution_clock>::min();
    float pause_duration = 0.0f;
  };

  ///
  /// Pauses a timer.
  /// @param timer The timer.
  LUDO_API void pause(timer& timer);

  ///
  /// Determines if a timer is paused.
  /// @param timer The timer.
  /// @return True if the timer is paused, false otherwise.
  LUDO_API bool paused(const timer& timer);

  ///
  /// Resumes paused a timer.
  /// @param timer The timer.
  LUDO_API void resume(timer& timer);

  ///
  /// Resets a timer.
  /// @param timer The timer.
  LUDO_API void reset(timer& timer);

  ///
  /// Retrieves the time that has elapsed since the construction of a timer (in seconds)
  /// @param timer The timer.
  /// @return The time that has elapsed since the construction of a timer (in seconds)
  LUDO_API float elapsed(const timer& timer);
}

#endif // LUDO_TIMER_H
