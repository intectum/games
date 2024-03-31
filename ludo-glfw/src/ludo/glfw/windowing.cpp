/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include <GLFW/glfw3.h>

#include <ludo/data.h>
#include <ludo/windowing.h>

#include "input.h"

namespace ludo
{
  void shift_user_pointers(const window* begin, const window* end, int32_t amount);

  void update_windows(instance& instance)
  {
    auto& windows = data<window>(instance);

    for (auto& window : windows)
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

      window.mouse_movement = { 0, 0 };

      window.scroll = { 0.0f, 0.0f };

      glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(window.id));
    }

    glfwPollEvents();
  }

  template<>
  windowing_context* add(instance& instance, const windowing_context& init, const std::string& partition)
  {
    auto windowing_context = add(data<ludo::windowing_context>(instance), init, partition);
    windowing_context->id = next_id++;

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

    return windowing_context;
  }

  template<>
  void remove<windowing_context>(instance& instance, windowing_context* element, const std::string& partition)
  {
    glfwTerminate();

    remove(data<windowing_context>(instance), element, partition);
  }

  template<>
  window* add(instance& instance, const window& init, const std::string& partition)
  {
    auto& windows = data<ludo::window>(instance);
    auto window = add(windows, init, partition);
    shift_user_pointers(window + 1, windows.end(), 1);

    auto glfw_window = glfwCreateWindow(static_cast<int32_t>(window->width), static_cast<int32_t>(window->height), window->title.c_str(), nullptr, nullptr);
    window->id = reinterpret_cast<uint64_t>(glfw_window);
    glfwSetWindowUserPointer(glfw_window, window);

    glfwMakeContextCurrent(glfw_window);

    if (!window->v_sync)
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

      window.scroll = { static_cast<float>(xoffset), static_cast<float>(yoffset) };
    });

    glfwSetWindowCloseCallback(glfw_window, [](GLFWwindow* glfw_window)
    {
      // TODO how to handle this?
    });

    return window;
  }

  template<>
  void remove<window>(instance& instance, window* element, const std::string& partition)
  {
    auto& windows = data<window>(instance, partition);

    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(element->id));

    shift_user_pointers(element, windows.end(), -1);
    remove(data<window>(instance), element, partition);
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

  void shift_user_pointers(const window* begin, const window* end, int32_t amount)
  {
    for (auto current = begin; current < end; current++)
    {
      auto glfw_window = reinterpret_cast<GLFWwindow*>(current->id);
      glfwSetWindowUserPointer(glfw_window, static_cast<window*>(glfwGetWindowUserPointer(glfw_window)) + amount);
    }
  }
}
