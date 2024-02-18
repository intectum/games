#include "../constants.h"

namespace astrum
{
  void control_map(ludo::instance& inst)
  {
    // TODO remove this hack and make celestial_body data type
    auto radii = std::vector<float> { sol_radius, terra_radius, luna_radius };

    auto& window = *ludo::first<ludo::window>(inst);
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto& map_controls = *ludo::first<astrum::map_controls>(inst);
    auto& celestial_body_point_masses = ludo::data<point_mass>(inst, "celestial-bodies");

    auto next_target = window.active_keyboard_button_states[ludo::keyboard_button::N] == ludo::button_state::UP;
    if (next_target)
    {
      map_controls.target_index = (map_controls.target_index + 1) % celestial_body_point_masses.array_size;
      map_controls.target_radius = radii[map_controls.target_index];

      map_controls.camera_rotation = ludo::vec2_zero;
      map_controls.camera_zoom = 5.0f;
    }

    auto& target_point_mass = celestial_body_point_masses[map_controls.target_index];

    // Camera.
    auto rotation_delta_angles = ludo::vec2
    {
      static_cast<float>(window.mouse_movement[1]),
      static_cast<float>(-window.mouse_movement[0])
    };
    rotation_delta_angles *= camera_rotate_speed * inst.delta_time * game_speed;

    map_controls.camera_rotation += rotation_delta_angles;
    map_controls.camera_rotation[0] = ludo::shortest_angle(map_controls.camera_rotation[0]);
    map_controls.camera_rotation[1] = ludo::shortest_angle(map_controls.camera_rotation[1]);

    map_controls.camera_zoom -= window.scroll[1];

    ludo::set_camera(
      rendering_context,
      {
        .near_clipping_distance = 0.1f,
        .far_clipping_distance = 2.0f * astronomical_unit,
        .view =
          ludo::mat4(target_point_mass.transform.position, ludo::mat3_identity) *
          ludo::mat4(ludo::vec3_zero, ludo::mat3(ludo::quat(map_controls.camera_rotation[0], map_controls.camera_rotation[1], 0.0f))) * // TODO why do the quat and mat constructors work differently? I want consistency!
          ludo::mat4({ 0.0f, 0.0f, map_controls.target_radius * map_controls.camera_zoom }, ludo::mat3_identity), // Zoom
        .projection = ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 2.0f * astronomical_unit)
      }
    );
  }
}
