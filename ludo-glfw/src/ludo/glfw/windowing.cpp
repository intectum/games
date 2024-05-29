/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include <GLFW/glfw3.h>

#include <ludo/windowing.h>

#include "input.h"

// TODO maintains a user pointer which can easily be invalidated!

namespace ludo
{
  void init(window& window)
  {
    // This first part only really needs to happen once, but it is idempotent.
    {
      glfwSetErrorCallback([](int error, const char* description)
      {
        std::cout << "GLFW error " << error << ": " << description << std::endl;
        assert(false && "GLFW error");
      });

      if (!glfwInit())
      {
        std::cout << "failed to initialize GLFW" << std::endl;
        assert(false && "failed to initialize GLFW");
      }
    }

    auto glfw_window = glfwCreateWindow(static_cast<int32_t>(window.width), static_cast<int32_t>(window.height), window.title.c_str(), nullptr, nullptr);
    window.id = reinterpret_cast<uint64_t>(glfw_window);
    glfwSetWindowUserPointer(glfw_window, &window);

    glfwMakeContextCurrent(glfw_window);

    if (!window.v_sync)
    {
      glfwSwapInterval(0);
    }

    glfwSetCursorPosCallback(glfw_window, [](GLFWwindow* glfw_window, double x, double y)
    {
      auto window = static_cast<ludo::window*>(glfwGetWindowUserPointer(glfw_window));

      window->mouse_movement[0] = static_cast<int32_t>(x) - window->mouse_position[0];
      window->mouse_movement[1] = static_cast<int32_t>(y) - window->mouse_position[1];
      window->mouse_position[0] = static_cast<int32_t>(x);
      window->mouse_position[1] = static_cast<int32_t>(y);
    });

    glfwSetKeyCallback(glfw_window, [](GLFWwindow* glfw_window, int key, int scancode, int action, int mods)
    {
      auto& window = *static_cast<ludo::window*>(glfwGetWindowUserPointer(glfw_window));

      if (action == GLFW_REPEAT)
      {
        return;
      }

      auto simple_button = keyboard_button();
      auto button_pair = input_map.find(key);
      if (button_pair != input_map.end())
      {
        simple_button = button_pair->second;
      }

      if (action == GLFW_PRESS)
      {
        window.active_keyboard_button_states[simple_button] = button_state::DOWN;
      }
      else if (action == GLFW_RELEASE)
      {
        window.active_keyboard_button_states[simple_button] = button_state::UP;
      }
    });

    glfwSetMouseButtonCallback(glfw_window, [](GLFWwindow* glfw_window, int button, int action, int mods)
    {
      auto& window = *static_cast<ludo::window*>(glfwGetWindowUserPointer(glfw_window));

      auto simple_button = mouse_button();
      if (button == GLFW_MOUSE_BUTTON_MIDDLE)
      {
        simple_button = mouse_button::MIDDLE;
      }
      else if (button == GLFW_MOUSE_BUTTON_LEFT)
      {
        simple_button = mouse_button::LEFT;
      }
      else if (button == GLFW_MOUSE_BUTTON_RIGHT)
      {
        simple_button = mouse_button::RIGHT;
      }

      if (action == GLFW_PRESS)
      {
        window.active_mouse_button_states[simple_button] = button_state::DOWN;
      }
      else if (action == GLFW_RELEASE)
      {
        window.active_mouse_button_states[simple_button] = button_state::UP;
      }
    });

    glfwSetScrollCallback(glfw_window, [](GLFWwindow* glfw_window, double xoffset, double yoffset)
    {
      auto& window = *static_cast<ludo::window*>(glfwGetWindowUserPointer(glfw_window));

      window.mouse_scroll = { static_cast<float>(xoffset), static_cast<float>(yoffset) };
    });

    glfwSetWindowCloseCallback(glfw_window, [](GLFWwindow* glfw_window)
    {
      auto& window = *static_cast<ludo::window*>(glfwGetWindowUserPointer(glfw_window));

      window.active_window_frame_button_states[window_frame_button::CLOSE] = button_state::UP;
    });
  }

  void de_init(window& window)
  {
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(window.id));
    window.id = 0;
  }


  void swap_buffers(window& window)
  {
    glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(window.id));
  }

  void receive_input(window& window, instance& instance)
  {
    for (auto& active_keyboard_button_state : window.active_keyboard_button_states)
    {
      if (active_keyboard_button_state.second == button_state::DOWN)
      {
        active_keyboard_button_state.second = button_state::HOLD;
      }
      else if (active_keyboard_button_state.second == button_state::UP)
      {
        active_keyboard_button_state.second = button_state::NONE;
      }
    }

    for (auto& active_mouse_button_state : window.active_mouse_button_states)
    {
      if (active_mouse_button_state.second == button_state::DOWN)
      {
        active_mouse_button_state.second = button_state::HOLD;
      }
      else if (active_mouse_button_state.second == button_state::UP)
      {
        active_mouse_button_state.second = button_state::NONE;
      }
    }

    for (auto& active_window_frame_button_states : window.active_window_frame_button_states)
    {
      if (active_window_frame_button_states.second == button_state::UP)
      {
        active_window_frame_button_states.second = button_state::NONE;
      }
    }

    window.mouse_movement = { 0, 0 };

    window.mouse_scroll = { 0.0f, 0.0f };

    glfwPollEvents();
  }

  void capture_mouse(window& window)
  {
    if (window.mouse_captured)
    {
      return;
    }

    window.mouse_captured = true;
    glfwSetInputMode(reinterpret_cast<GLFWwindow*>(window.id), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  void release_mouse(window& window)
  {
    if (!window.mouse_captured)
    {
      return;
    }

    window.mouse_captured = false;
    glfwSetInputMode(reinterpret_cast<GLFWwindow*>(window.id), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}
