/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <array>

#include "core.h"
#include "input.h"

namespace ludo
{
  ///
  /// The buttons on a window frame.
  enum window_frame_button
  {
    window_frame_button_close,
    window_frame_button_count
  };

  struct window
  {
    uint64_t id = 0; ///< A unique identifier.

    std::string title; ///< The title.

    uint32_t width = 1280; ///< The width.
    uint32_t height = 720; ///< The height.

    bool fullscreen = false; ///< Determines if the window is fullscreen.
    bool v_sync = true; ///< Determines if the window is vertically synchronized.

    std::array<button_state, window_frame_button_count> frame_button_states; ///< The currently active window frame buttons.

    std::array<button_state, keyboard_button_count> keyboard_button_states; ///< The currently active keyboard buttons.

    bool mouse_captured = false; ///< Determines if the mouse is captured.
    std::array<int32_t, 2> mouse_position; ///< The current position of the mouse.
    std::array<int32_t, 2> mouse_movement; ///< The current movement of the mouse.
    std::array<float, 2> mouse_scroll; ///< The current mouse scroll.
    std::array<button_state, mouse_button_count> mouse_button_states; ///< The currently active mouse buttons.
  };

  ///
  /// Opens a window.
  /// \param window The window.
  void open(window& window);

  ///
  /// Closes a window.
  /// \param window The window.
  void close(window& window);

  ///
  /// Swaps the frame buffers in a window.
  /// \param window The window.
  void swap_buffers(window& window);

  ///
  /// Receives input from a window.
  /// \param window The window.
  void receive_input(window& window);

  ///
  /// Captures the mouse to a window.
  /// \param window The window.
  void capture_mouse(window& window);

  ///
  /// Releases a mouse from a window.
  /// \param window The window.
  void release_mouse(window& window);
}
