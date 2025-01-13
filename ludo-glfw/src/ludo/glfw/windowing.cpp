/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <iostream>

#include <GLFW/glfw3.h>

#include <ludo/windowing.h>

#include "input.h"

// TODO maintains a user pointer which can easily be invalidated!

namespace ludo
{
  void open(window& window)
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

      auto keyboard_button = ludo::keyboard_button();
      auto button_pair = input_map.find(key);
      if (button_pair != input_map.end())
      {
        keyboard_button = button_pair->second;
      }

      if (action == GLFW_PRESS)
      {
        window.keyboard_button_states[keyboard_button] = button_state_down;
      }
      else if (action == GLFW_RELEASE)
      {
        window.keyboard_button_states[keyboard_button] = button_state_up;
      }
    });

    glfwSetMouseButtonCallback(glfw_window, [](GLFWwindow* glfw_window, int button, int action, int mods)
    {
      auto& window = *static_cast<ludo::window*>(glfwGetWindowUserPointer(glfw_window));

      auto mouse_button = ludo::mouse_button();
      if (button == GLFW_MOUSE_BUTTON_MIDDLE)
      {
        mouse_button = mouse_button_middle;
      }
      else if (button == GLFW_MOUSE_BUTTON_LEFT)
      {
        mouse_button = mouse_button_left;
      }
      else if (button == GLFW_MOUSE_BUTTON_RIGHT)
      {
        mouse_button = mouse_button_right;
      }

      if (action == GLFW_PRESS)
      {
        window.mouse_button_states[mouse_button] = button_state_down;
      }
      else if (action == GLFW_RELEASE)
      {
        window.mouse_button_states[mouse_button] = button_state_up;
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

      window.frame_button_states[window_frame_button_close] = button_state_down;
    });
  }

  void close(window& window)
  {
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(window.id));
    window.id = 0;
  }

  void swap_buffers(window& window)
  {
    glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(window.id));
  }

  void receive_input(window& window)
  {
    for (auto& keyboard_button_state : window.keyboard_button_states)
    {
      if (keyboard_button_state == button_state_down)
      {
        keyboard_button_state = button_state_hold;
      }
      else if (keyboard_button_state == button_state_up)
      {
        keyboard_button_state = button_state_none;
      }
    }

    for (auto& mouse_button_state : window.mouse_button_states)
    {
      if (mouse_button_state == button_state_down)
      {
        mouse_button_state = button_state_hold;
      }
      else if (mouse_button_state == button_state_up)
      {
        mouse_button_state = button_state_none;
      }
    }

    for (auto& frame_button_states : window.frame_button_states)
    {
      if (frame_button_states == button_state_down)
      {
        frame_button_states = button_state_up;
      }
      else if (frame_button_states == button_state_up)
      {
        frame_button_states = button_state_none;
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
