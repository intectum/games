/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_CORE_H
#define LUDO_CORE_H

#include <string>
#include <unordered_map>

// Platforms
#if defined(_WIN32)
  #define LUDO_WINDOWS
#elif defined(__linux__)
  #define LUDO_LINUX
#endif

// Export/Import
#if defined(LUDO_WINDOWS) && defined(LUDO_SHARED)
  #define LUDO_API __declspec(dllexport)
  #define LUDO_API_TEMPLATE
#elif defined(LUDO_WINDOWS) && defined(LUDO_SHARED_EXE)
  #define LUDO_API __declspec(dllimport)
  #define LUDO_API_TEMPLATE extern
#else
  #define LUDO_API
#endif

namespace ludo
{
  ///
  /// A counter used to provide unique IDs.
  extern uint64_t next_id;

  ///
  /// An instance of ludo.
  struct LUDO_API instance
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
  LUDO_API void play(instance& instance);

  ///
  /// Stop!
  /// Stops the executing of frames.
  /// This will cause the previous call of play() to return.
  /// @param instance The instance to stop execution of frames for.
  LUDO_API void stop(instance& instance);

  ///
  /// Executes a single frame.
  /// @param instance The instance to execute a frame of.
  LUDO_API void frame(instance& instance);
}

#endif // LUDO_CORE_H
