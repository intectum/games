#include "../constants.h"
#include "../ecs.h"
#include "../types.h"
#include "map.h"

namespace astrum
{
  void control_map(const ludo::instance& inst, const std::vector<ludo::container>& containers, const ludo::window& window, map_controls& map_controls, ludo::vec3& camera_position, ludo::quat& camera_rotation)
  {
    // TODO remove this hack and make celestial_body data type
    auto radii = std::vector { sol_radius, terra_radius, luna_radius };

    auto next_target = window.keyboard_button_states[ludo::keyboard_button_n] == ludo::button_state_up;
    if (next_target)
    {
      map_controls.target_index = (map_controls.target_index + 1) % containers.size();
      map_controls.target_radius = radii[map_controls.target_index];

      map_controls.camera_rotation = ludo::vec2_zero;
      map_controls.camera_zoom = 5.0f;
    }

    auto celestial_body_positions = reinterpret_cast<ludo::vec3*>(
      get_components(
        containers[map_controls.target_index],
        "celestial_body",
        "position"
      )->data
    );

    auto& target_position = celestial_body_positions[0];

    // Camera.
    auto rotation_delta_angles = ludo::vec2
    {
      static_cast<float>(window.mouse_movement[1]),
      static_cast<float>(-window.mouse_movement[0])
    };
    rotation_delta_angles *= camera_rotate_speed * inst.delta_time;

    map_controls.camera_rotation += rotation_delta_angles;
    map_controls.camera_rotation[0] = ludo::shortest_angle(map_controls.camera_rotation[0]);
    map_controls.camera_rotation[1] = ludo::shortest_angle(map_controls.camera_rotation[1]);

    map_controls.camera_zoom -= window.mouse_scroll[1] * 0.25f;

    auto view =
      ludo::mat4(target_position, ludo::mat3_identity) *
      ludo::mat4(ludo::vec3_zero, ludo::mat3(ludo::quat(map_controls.camera_rotation[0], map_controls.camera_rotation[1], 0.0f))) * // TODO why do the quat and mat constructors work differently? I want consistency!
      ludo::mat4({ 0.0f, 0.0f, map_controls.target_radius * map_controls.camera_zoom }, ludo::mat3_identity); // Zoom

    camera_position = ludo::position(view);
    camera_rotation = ludo::quat(ludo::mat3(view));
  }
}
