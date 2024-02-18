/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_GLFW_INPUT_H
#define LUDO_GLFW_INPUT_H

#include <unordered_map>

#include <ludo/input.h>

namespace ludo
{
  extern std::unordered_map<int, keyboard_button> input_map; ///< A map from GLFW key codes to keyboard_button enum values
}

#endif // LUDO_GLFW_INPUT_H
