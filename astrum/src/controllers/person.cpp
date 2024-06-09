#include "../types.h"

namespace astrum
{
  void control_person(ludo::instance& inst, uint32_t index)
  {
    auto& window = *ludo::first<ludo::window>(inst);
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto& person_controls = ludo::data<astrum::person_controls>(inst, "people")[index];
    auto& person = ludo::data<astrum::person>(inst, "people")[index];
    const auto& point_mass = ludo::data<astrum::point_mass>(inst, "people")[index];

    person_controls.forward = window.active_keyboard_button_states[ludo::keyboard_button::W] == ludo::button_state::HOLD;
    person_controls.back = window.active_keyboard_button_states[ludo::keyboard_button::S] == ludo::button_state::HOLD;
    person_controls.left = window.active_keyboard_button_states[ludo::keyboard_button::A] == ludo::button_state::HOLD;
    person_controls.right = window.active_keyboard_button_states[ludo::keyboard_button::D] == ludo::button_state::HOLD;
    person_controls.jump = window.active_keyboard_button_states[ludo::keyboard_button::SPACE] == ludo::button_state::UP;

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

    ludo::set_camera(
      rendering_context,
      {
        .near_clipping_distance = 0.1f,
        .far_clipping_distance = 2.0f * astronomical_unit,
        .view =
          ludo::mat4(point_mass.transform.position, ludo::mat3(point_mass.transform.rotation)) *
          ludo::mat4(ludo::vec3_zero, ludo::mat3(ludo::quat(person_controls.camera_rotation[0], person_controls.camera_rotation[1] - person.turn_angle, 0.0f))) * // TODO why do the quat and mat constructors work differently? I want consistency!
          ludo::mat4({ 0.0f, 1.0f, -3.0f }, ludo::mat3_identity) * // 3rd person - move away from the avatar
          ludo::mat4(ludo::vec3_zero, ludo::mat3(0.0f, ludo::pi, 0.0f)), // Look at the avatar
        .projection = ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 2.0f * astronomical_unit)
      }
    );
  }
}
