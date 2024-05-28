#ifndef ASTRUM_TYPES_H
#define ASTRUM_TYPES_H

#include <ludo/api.h>

#include "meshes/lods.h"

namespace astrum
{
  struct patch;
  struct tree;

  struct celestial_body
  {
    uint64_t id;

    std::string name;
    float radius = 0.0f;
    float mass = 0.0f;
  };

  struct terrain_chunk
  {
    uint64_t mesh_id = 0;
    uint64_t render_mesh_id = 0;
    uint64_t tree_render_mesh_id = 0;

    ludo::vec3 center;
    ludo::vec3 normal;
    uint32_t lod_index = 0;

    bool locked = false;
  };

  struct terrain
  {
    uint64_t id = 0;

    ludo::vertex_format format;
    std::vector<lod> lods;
    std::function<float(const ludo::vec3& position)> height_func;
    std::function<ludo::vec4(float longitude, const std::array<float, 3>& heights, float gradient)> color_func;
    std::function<std::vector<tree>(const terrain& terrain, float radius, uint32_t chunk_index)> tree_func;

    std::vector<terrain_chunk> chunks;

    std::unordered_map<uint32_t, uint64_t> static_body_ids;
    std::unordered_map<uint32_t, uint64_t> static_body_mesh_ids;
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

  struct point_mass
  {
    uint64_t id = 0;

    float mass = 0.0f;

    ludo::transform transform;
    ludo::vec3 linear_velocity = ludo::vec3_zero;
    bool resting = false;

    std::vector<point_mass*> children;
  };

  struct solar_system
  {
    uint64_t id = 0;

    int32_t relative_celestial_body_index = -1;
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

  struct tree
  {
    ludo::vec3 position;
    float rotation = 0.0f;
    float scale = 0.0f;
  };
}

#endif // ASTRUM_TYPES_H
