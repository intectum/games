/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <array>

#include "core.h"
#include "data/data.h"
#include "input.h"

namespace ludo
{
  ///
  /// The buttons on a window frame.
  enum class window_frame_button
  {
    CLOSE
  };

  struct window
  {
    uint64_t id = 0; ///< A unique identifier.

    std::string title; ///< The title.

    uint32_t width = 1280; ///< The width.
    uint32_t height = 720; ///< The height.

    bool fullscreen = false; ///< Determines if the window is fullscreen.
    bool v_sync = true; ///< Determines if the window is vertically synchronized.

    std::unordered_map<window_frame_button, button_state> active_window_frame_button_states; ///< The currently active window frame buttons.

    std::unordered_map<keyboard_button, button_state> active_keyboard_button_states; ///< The currently active keyboard buttons.

    bool mouse_captured = false; ///< Determines if the mouse is captured.
    std::array<int32_t, 2> mouse_position; ///< The current position of the mouse.
    std::array<int32_t, 2> mouse_movement; ///< The current movement of the mouse.
    std::array<float, 2> mouse_scroll; ///< The current mouse scroll.
    std::unordered_map<mouse_button, button_state> active_mouse_button_states; ///< The currently active mouse buttons.
  };

  ///
  /// Initializes a window.
  /// \param window The window.
  void init(window& window);

  ///
  /// De-initializes a window.
  /// \param window The window.
  void de_init(window& window);

  ///
  /// Swaps the frame buffers in a window.
  /// \param window The window.
  void swap_buffers(window& window);

  ///
  /// Receives input from a window.
  /// \param window The window.
  /// \param instance The instance.
  void receive_input(window& window, instance& instance);

  ///
  /// Captures the mouse to a window.
  /// \param window The window.
  void capture_mouse(window& window);

  ///
  /// Releases a mouse from a window.
  /// \param window The window.
  void release_mouse(window& window);
}
