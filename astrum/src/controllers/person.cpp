#include "../ecs.h"
#include "../constants.h"
#include "../types.h"
#include "person.h"

namespace astrum
{
  void control_person(const ludo::instance& inst, ludo::container& container, const ludo::window& window, uint32_t index, ludo::vec3& camera_position, ludo::quat& camera_rotation)
  {
    auto positions = reinterpret_cast<const ludo::vec3*>(
      get_components(
        container,
        "person",
        "position"
      )->data
    );

    auto rotations = reinterpret_cast<const ludo::quat*>(
      get_components(
        container,
        "person",
        "rotation"
      )->data
    );

    auto people = reinterpret_cast<person*>(
      get_components(
        container,
        "person",
        "person"
      )->data
    );

    auto person_controlses = reinterpret_cast<person_controls*>(
      get_components(
        container,
        "person",
        "person_controls"
      )->data
    );

    auto& position = positions[index];
    auto& rotation = rotations[index];
    auto& person = people[index];
    auto& person_controls = person_controlses[index];

    person_controls.forward = window.keyboard_button_states.at(ludo::keyboard_button_w) == ludo::button_state_hold;
    person_controls.back = window.keyboard_button_states.at(ludo::keyboard_button_s) == ludo::button_state_hold;
    person_controls.left = window.keyboard_button_states.at(ludo::keyboard_button_s) == ludo::button_state_hold;
    person_controls.right = window.keyboard_button_states.at(ludo::keyboard_button_d) == ludo::button_state_hold;
    person_controls.jump = window.keyboard_button_states.at(ludo::keyboard_button_space) == ludo::button_state_up;

    // Camera
    auto rotation_delta_angles = ludo::vec2
    {
      static_cast<float>(window.mouse_movement[1]),
      static_cast<float>(-window.mouse_movement[0])
    };
    rotation_delta_angles *= camera_rotate_speed * inst.delta_time;

    person_controls.camera_rotation += rotation_delta_angles;
    person_controls.camera_rotation[0] = ludo::shortest_angle(person_controls.camera_rotation[0]);
    person_controls.camera_rotation[1] = ludo::shortest_angle(person_controls.camera_rotation[1]);

    auto view =
      ludo::mat4(position, ludo::mat3(rotation)) *
      ludo::mat4(ludo::vec3_zero, ludo::mat3(ludo::quat(person_controls.camera_rotation[0], person_controls.camera_rotation[1] - person.turn_angle, 0.0f))) * // TODO why do the quat and mat constructors work differently? I want consistency!
      ludo::mat4({ 0.0f, 1.0f, -3.0f }, ludo::mat3_identity) * // 3rd person - move away from the avatar
      ludo::mat4(ludo::vec3_zero, ludo::mat3(0.0f, ludo::pi, 0.0f)); // Look at the avatar

    camera_position = ludo::position(view);
    camera_rotation = ludo::quat(ludo::mat3(view));
  }
}
