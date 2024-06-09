#include "../types.h"

namespace astrum
{
  void control_spaceship(ludo::instance& inst, uint32_t index)
  {
    auto& window = *ludo::first<ludo::window>(inst);
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto& spaceship_controls = ludo::data<astrum::spaceship_controls>(inst, "spaceships")[index];
    const auto& point_mass = ludo::data<astrum::point_mass>(inst, "spaceships")[index];

    spaceship_controls.forward = window.active_keyboard_button_states[ludo::keyboard_button::W] == ludo::button_state::HOLD;
    spaceship_controls.back = window.active_keyboard_button_states[ludo::keyboard_button::S] == ludo::button_state::HOLD;
    spaceship_controls.left = window.active_keyboard_button_states[ludo::keyboard_button::A] == ludo::button_state::HOLD;
    spaceship_controls.right = window.active_keyboard_button_states[ludo::keyboard_button::D] == ludo::button_state::HOLD;
    spaceship_controls.up = window.active_keyboard_button_states[ludo::keyboard_button::SPACE] == ludo::button_state::HOLD;
    spaceship_controls.down = window.active_keyboard_button_states[ludo::keyboard_button::LEFT_CTRL] == ludo::button_state::HOLD;
    spaceship_controls.roll_left = window.active_keyboard_button_states[ludo::keyboard_button::Q] == ludo::button_state::HOLD;
    spaceship_controls.roll_right = window.active_keyboard_button_states[ludo::keyboard_button::E] == ludo::button_state::HOLD;
    spaceship_controls.thrust = window.active_keyboard_button_states[ludo::keyboard_button::LEFT_SHIFT] == ludo::button_state::HOLD;

    spaceship_controls.yaw = static_cast<float>(window.mouse_movement[1]);
    spaceship_controls.pitch = static_cast<float>(-window.mouse_movement[0]);
    spaceship_controls.roll = spaceship_controls.roll_left ? -1.0f : (spaceship_controls.roll_right ? 1.0f : 0.0f);

    // Camera
    ludo::set_camera(
      rendering_context,
      {
        .near_clipping_distance = 0.1f,
        .far_clipping_distance = 2.0f * astronomical_unit,
        .view =
          ludo::mat4(point_mass.transform.position, ludo::mat3(point_mass.transform.rotation)) *
          ludo::mat4({ 0.0f, 2.0f, -5.0f }, ludo::mat3_identity) * // 3rd person - move away from the avatar
          ludo::mat4(ludo::vec3_zero, ludo::mat3(0.0f, ludo::pi, 0.0f)), // Look at the spaceship
        .projection = ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 2.0f * astronomical_unit)
      }
    );
  }
}
