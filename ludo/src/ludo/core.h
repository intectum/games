/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <functional>
#include <string>
#include <unordered_map>

namespace ludo
{
  ///
  /// An instance of ludo.
  struct instance
  {
    bool playing = false; ///< Determines if this ludo instance ids currently playing.

    float delta_time = 0.0f; ///< The elapsed time since the last frame.
    float total_time = 0.0f; ///< The elapsed time since ludo started playing.
  };

  ///
  /// Play!
  /// Executes frames forever! ...or until it is stopped.
  /// @param instance The instance to execute frames of.
  /// @param kernel The frame function to execute.
  void play(instance& instance, const std::function<void()>& kernel);

  ///
  /// Stop!
  /// Stops the executing of frames.
  /// This will cause the previous call of play() to return.
  /// @param instance The instance to stop execution of frames for.
  void stop(instance& instance);

  ///
  /// Executes a single frame.
  /// @param instance The instance to execute a frame of.
  /// @param kernel The frame function to execute.
  void frame(instance& instance, const std::function<void()>& kernel);
}
