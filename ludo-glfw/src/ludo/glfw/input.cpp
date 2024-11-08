/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <GLFW/glfw3.h>

#include "input.h"

namespace ludo
{
  std::unordered_map<int, keyboard_button> input_map =
  {
    { GLFW_KEY_0, keyboard_button::ZERO },
    { GLFW_KEY_1, keyboard_button::ONE },
    { GLFW_KEY_2, keyboard_button::TWO },
    { GLFW_KEY_3, keyboard_button::THREE },
    { GLFW_KEY_4, keyboard_button::FOUR },
    { GLFW_KEY_5, keyboard_button::FIVE },
    { GLFW_KEY_6, keyboard_button::SIX },
    { GLFW_KEY_7, keyboard_button::SEVEN },
    { GLFW_KEY_8, keyboard_button::EIGHT },
    { GLFW_KEY_9, keyboard_button::NINE },

    { GLFW_KEY_A, keyboard_button::A },
    { GLFW_KEY_B, keyboard_button::B },
    { GLFW_KEY_C, keyboard_button::C },
    { GLFW_KEY_D, keyboard_button::D },
    { GLFW_KEY_E, keyboard_button::E },
    { GLFW_KEY_F, keyboard_button::F },
    { GLFW_KEY_G, keyboard_button::G },
    { GLFW_KEY_H, keyboard_button::H },
    { GLFW_KEY_I, keyboard_button::I },
    { GLFW_KEY_J, keyboard_button::J },
    { GLFW_KEY_K, keyboard_button::K },
    { GLFW_KEY_L, keyboard_button::L },
    { GLFW_KEY_M, keyboard_button::M },
    { GLFW_KEY_N, keyboard_button::N },
    { GLFW_KEY_O, keyboard_button::O },
    { GLFW_KEY_P, keyboard_button::P },
    { GLFW_KEY_Q, keyboard_button::Q },
    { GLFW_KEY_R, keyboard_button::R },
    { GLFW_KEY_S, keyboard_button::S },
    { GLFW_KEY_T, keyboard_button::T },
    { GLFW_KEY_U, keyboard_button::U },
    { GLFW_KEY_V, keyboard_button::V },
    { GLFW_KEY_W, keyboard_button::W },
    { GLFW_KEY_X, keyboard_button::X },
    { GLFW_KEY_Y, keyboard_button::Y },
    { GLFW_KEY_Z, keyboard_button::Z },

    { GLFW_KEY_F1, keyboard_button::F1 },
    { GLFW_KEY_F2, keyboard_button::F2 },
    { GLFW_KEY_F3, keyboard_button::F3 },
    { GLFW_KEY_F4, keyboard_button::F4 },
    { GLFW_KEY_F5, keyboard_button::F5 },
    { GLFW_KEY_F6, keyboard_button::F6 },
    { GLFW_KEY_F7, keyboard_button::F7 },
    { GLFW_KEY_F8, keyboard_button::F8 },
    { GLFW_KEY_F9, keyboard_button::F9 },
    { GLFW_KEY_F10, keyboard_button::F10 },
    { GLFW_KEY_F11, keyboard_button::F11 },
    { GLFW_KEY_F12, keyboard_button::F12 },

    { GLFW_KEY_APOSTROPHE, keyboard_button::APOSTROPHE },
    { GLFW_KEY_BACKSLASH, keyboard_button::BACKSLASH },
    { GLFW_KEY_BACKSPACE, keyboard_button::BACKSPACE },
    { GLFW_KEY_COMMA, keyboard_button::COMMA },
    { GLFW_KEY_DELETE, keyboard_button::DEL },
    { GLFW_KEY_ESCAPE, keyboard_button::ESCAPE },
    { GLFW_KEY_EQUAL, keyboard_button::EQUALS },
    { GLFW_KEY_GRAVE_ACCENT, keyboard_button::GRAVE },
    { GLFW_KEY_LEFT_BRACKET, keyboard_button::LEFT_BRACKET },
    { GLFW_KEY_MINUS, keyboard_button::MINUS },
    { GLFW_KEY_PERIOD, keyboard_button::PERIOD },
    { GLFW_KEY_ENTER, keyboard_button::ENTER },
    { GLFW_KEY_RIGHT_BRACKET, keyboard_button::RIGHT_BRACKET },
    { GLFW_KEY_SEMICOLON, keyboard_button::COLON },
    { GLFW_KEY_SLASH, keyboard_button::SLASH },
    { GLFW_KEY_SPACE, keyboard_button::SPACE },
    { GLFW_KEY_TAB, keyboard_button::TAB },
    { GLFW_KEY_DOWN, keyboard_button::ARROW_DOWN },
    { GLFW_KEY_LEFT, keyboard_button::ARROW_LEFT },
    { GLFW_KEY_RIGHT, keyboard_button::ARROW_RIGHT },
    { GLFW_KEY_UP, keyboard_button::ARROW_UP },
    { GLFW_KEY_DELETE, keyboard_button::DEL },
    { GLFW_KEY_END, keyboard_button::END },
    { GLFW_KEY_HOME, keyboard_button::HOME },
    { GLFW_KEY_INSERT, keyboard_button::INSERT },
    { GLFW_KEY_LEFT_ALT, keyboard_button::LEFT_ALT },
    { GLFW_KEY_LEFT_CONTROL, keyboard_button::LEFT_CTRL },
    { GLFW_KEY_LEFT_SHIFT, keyboard_button::LEFT_SHIFT },
    { GLFW_KEY_PAGE_DOWN, keyboard_button::PAGE_DOWN },
    { GLFW_KEY_PAGE_UP, keyboard_button::PAGE_UP },
    { GLFW_KEY_RIGHT_ALT, keyboard_button::RIGHT_ALT },
    { GLFW_KEY_RIGHT_CONTROL, keyboard_button::RIGHT_CTRL },
    { GLFW_KEY_RIGHT_SHIFT, keyboard_button::RIGHT_SHIFT }
  };
}
