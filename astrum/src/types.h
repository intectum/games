#pragma once

#include <ludo/api.h>

namespace astrum
{
  struct patch;
  struct tree;

  struct terrain_funcs
  {
    std::function<float(const ludo::vec3& position)> height;
    std::function<ludo::vec4(float longitude, const std::array<float, 3>& heights, float gradient)> color;
    //std::function<std::array<std::vector<tree>, tree_type_count>(float radius, uint32_t chunk_index)> tree; TODO
  };

  struct game_controls
  {
    uint64_t id = 0;

    enum class mode
    {
      person,
      spaceship,
      map
    };

    enum mode mode = mode::person;
    enum mode previous_mode = mode::person;

    uint32_t person_index = 0;
    uint32_t spaceship_index = 0;
  };

  struct map_controls
  {
    uint64_t id = 0;

    uint32_t target_index = 0;
    float target_radius = 0.0f;

    ludo::vec2 camera_rotation = ludo::vec2_zero;
    float camera_zoom = 5.0f;
  };

  struct person
  {
    uint64_t id = 0;

    bool standing = true;
    bool jumping = false;

    float turn_angle = 0.0f;
    float run_speed = 0.0f;
    float turn_speed = 0.0f;

    float walk_animation_time = 0.0f;
  };

  struct person_controls
  {
    uint64_t id = 0;

    bool forward = false;
    bool back = false;
    bool left = false;
    bool right = false;
    bool jump = false;

    ludo::vec2 camera_rotation = ludo::vec2_zero;
  };

  struct spaceship_controls
  {
    uint64_t id = 0;

    bool forward = false;
    bool back = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool roll_left = false;
    bool roll_right = false;
    bool thrust = false;

    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
  };

  // TODO
  /*struct tree
  {
    ludo::vec3 position;
    float rotation = 0.0f;
    float scale = 0.0f;
  };*/
}
