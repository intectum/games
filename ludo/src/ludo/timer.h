/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <chrono>

#include "core.h"

namespace ludo
{
  ///
  /// A high precision timer
  struct timer
  {
    std::chrono::time_point<std::chrono::high_resolution_clock> started_at = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> paused_at = std::chrono::time_point<std::chrono::high_resolution_clock>::min();
    float pause_duration = 0.0f;
  };

  ///
  /// Pauses a timer.
  /// @param timer The timer.
  void pause(timer& timer);

  ///
  /// Determines if a timer is paused.
  /// @param timer The timer.
  /// @return True if the timer is paused, false otherwise.
  bool paused(const timer& timer);

  ///
  /// Resumes paused a timer.
  /// @param timer The timer.
  void resume(timer& timer);

  ///
  /// Resets a timer.
  /// @param timer The timer.
  void reset(timer& timer);

  ///
  /// Retrieves the time that has elapsed since the construction of a timer (in seconds)
  /// @param timer The timer.
  /// @return The time that has elapsed since the construction of a timer (in seconds)
  float elapsed(const timer& timer);
}
