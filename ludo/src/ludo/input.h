/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_INPUT_H
#define LUDO_INPUT_H

#include <unordered_map>

#include "core.h"
#include "math.h"

namespace ludo
{
  ///
  /// The different states a physical button can be in
  enum class button_state
  {
    NONE, ///< The button is inactive
    DOWN, ///< The button was just pressed
    HOLD, ///< The button is being held down
    UP ///< The button was just released
  };

  ///
  /// The buttons on a keyboard
  enum class keyboard_button
  {
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,

    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    APOSTROPHE,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    ARROW_UP,
    ASTERISK,
    AT,
    BACKSLASH,
    BACKSPACE,
    COLON,
    COMMA,
    DEL,
    END,
    ENTER,
    ESCAPE,
    EQUALS,
    GRAVE,
    HOME,
    INSERT,
    LEFT_ALT,
    LEFT_BRACKET,
    LEFT_CTRL,
    LEFT_SHIFT,
    MINUS,
    PAGE_DOWN,
    PAGE_UP,
    PERIOD,
    RIGHT_ALT,
    RIGHT_BRACKET,
    RIGHT_CTRL,
    RIGHT_SHIFT,
    SEMICOLON,
    SLASH,
    SPACE,
    TAB,
    UNDERLINE
  };

  ///
  /// The buttons on a mouse
  enum class mouse_button
  {
    MIDDLE,
    LEFT,
    RIGHT
  };
}

#endif // LUDO_INPUT_H
