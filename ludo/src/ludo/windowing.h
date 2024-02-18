/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_WINDOWING_H
#define LUDO_WINDOWING_H

#include <array>

#include "core.h"
#include "data.h"
#include "input.h"

namespace ludo
{
  ///
  /// A windowing context.
  struct LUDO_API windowing_context
  {
    uint64_t id = 0; ///< The ID of the windowing context.
  };

  struct LUDO_API window
  {
    uint64_t id = 0; ///< The ID of the window.

    std::string title;

    uint32_t width = 1280;
    uint32_t height = 720;

    bool fullscreen = false;
    bool v_sync = true;

    std::unordered_map<keyboard_button, button_state> active_keyboard_button_states;

    bool mouse_captured = false;
    std::array<int32_t, 2> mouse_position;
    std::array<int32_t, 2> mouse_movement;
    std::unordered_map<mouse_button, button_state> active_mouse_button_states;

    std::array<float, 2> scroll;
  };

  ///
  /// Updates the windows within the given context.
  /// \param instance The instance to update windows for.
  void update_windows(instance& instance);

  template<>
  windowing_context* add(instance& instance, const windowing_context& init, const std::string& partition);

  template<>
  void remove<windowing_context>(instance& instance, windowing_context* element, const std::string& partition);

  template<>
  window* add(instance& instance, const window& init, const std::string& partition);

  template<>
  void remove<window>(instance& instance, window* element, const std::string& partition);

  LUDO_API void capture_mouse(window& window);

  LUDO_API void release_mouse(window& window);
}

#endif // LUDO_WINDOWING_H
