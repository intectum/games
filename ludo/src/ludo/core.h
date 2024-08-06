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
  /// A counter used to provide unique IDs.
  extern uint64_t next_id;

  ///
  /// An instance of ludo.
  struct instance
  {
    bool playing = false; ///< Determines if this ludo instance ids currently playing.

    float delta_time = 0.0f; ///< The elapsed time since the last frame.
    float total_time = 0.0f; ///< The elapsed time since ludo started playing.

    std::unordered_map<std::string, void*> data; ///< The data of the instance.
  };

  ///
  /// Play!
  /// Executes frames forever! ...or until it is stopped.
  /// @param instance The instance to execute frames of.
  /// @param frame The frame function.
  void play(instance& instance, const std::function<void(ludo::instance& instance)>& kernel);

  ///
  /// Stop!
  /// Stops the executing of frames.
  /// This will cause the previous call of play() to return.
  /// @param instance The instance to stop execution of frames for.
  void stop(instance& instance);

  ///
  /// Executes a single frame.
  /// @param instance The instance to execute a frame of.
  void frame(instance& instance, const std::function<void(ludo::instance& instance)>& kernel);
}
