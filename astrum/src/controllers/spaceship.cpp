#include "../ecs.h"
#include "../types.h"
#include "spaceship.h"

namespace astrum
{
  void control_spaceship(ludo::container& container, const ludo::window& window, uint32_t index, ludo::vec3& camera_position, ludo::quat& camera_rotation)
  {
    auto positions = reinterpret_cast<ludo::vec3*>(
      get_components(
        container,
        "spaceship",
        "position"
      )->data
    );

    auto rotations = reinterpret_cast<ludo::quat*>(
      get_components(
        container,
        "spaceship",
        "rotation"
      )->data
    );

    auto spaceship_controlses = reinterpret_cast<spaceship_controls*>(
      get_components(
        container,
        "spaceship",
        "spaceship_controls"
      )->data
    );

    auto& position = positions[index];
    auto& rotation = rotations[index];
    auto& spaceship_controls = spaceship_controlses[index];

    spaceship_controls.forward = window.keyboard_button_states[ludo::keyboard_button_w] == ludo::button_state_hold;
    spaceship_controls.back = window.keyboard_button_states[ludo::keyboard_button_s] == ludo::button_state_hold;
    spaceship_controls.left = window.keyboard_button_states[ludo::keyboard_button_a] == ludo::button_state_hold;
    spaceship_controls.right = window.keyboard_button_states[ludo::keyboard_button_d] == ludo::button_state_hold;
    spaceship_controls.up = window.keyboard_button_states[ludo::keyboard_button_space] == ludo::button_state_hold;
    spaceship_controls.down = window.keyboard_button_states[ludo::keyboard_button_left_ctrl] == ludo::button_state_hold;
    spaceship_controls.roll_left = window.keyboard_button_states[ludo::keyboard_button_q] == ludo::button_state_hold;
    spaceship_controls.roll_right = window.keyboard_button_states[ludo::keyboard_button_e] == ludo::button_state_hold;
    spaceship_controls.thrust = window.keyboard_button_states[ludo::keyboard_button_left_shift] == ludo::button_state_hold;

    spaceship_controls.yaw = static_cast<float>(window.mouse_movement[1]);
    spaceship_controls.pitch = static_cast<float>(-window.mouse_movement[0]);
    spaceship_controls.roll = spaceship_controls.roll_left ? -1.0f : (spaceship_controls.roll_right ? 1.0f : 0.0f);

    // Camera
    auto view =
      ludo::mat4(position, ludo::mat3(rotation)) *
      ludo::mat4({ 0.0f, 6.0f, -15.0f }, ludo::mat3_identity) * // 3rd person - move away from the avatar
      ludo::mat4(ludo::vec3_zero, ludo::mat3(0.0f, ludo::pi, 0.0f)); // Look at the spaceship

    camera_position = ludo::position(view);
    camera_rotation = ludo::quat(ludo::mat3(view));
  }
}
