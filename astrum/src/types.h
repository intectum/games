#ifndef ASTRUM_TYPES_H
#define ASTRUM_TYPES_H

#include <ludo/api.h>

namespace astrum
{
  struct patch;
  struct tree;

  struct celestial_body
  {
    std::string name;
    float radius = 0.0f;
    float mass = 0.0f;
    ludo::vertex_format format;

    std::unordered_map<uint32_t, uint64_t> static_body_ids;

    std::function<float(const ludo::vec3& position)> height_func;
    std::function<ludo::vec4(float longitude, const std::array<float, 3>& heights, float gradient)> color_func;
    std::function<std::vector<tree>(uint64_t patch_id)> tree_func;
  };

  struct patch
  {
    uint64_t mesh_id = 0;

    ludo::vec3 center;
    ludo::vec3 normal;
    uint32_t variant_index = 0;

    std::vector<uint32_t> adjacent_patch_indices;

    bool locked = false;
  };

  struct patch_variant
  {
    std::vector<std::vector<uint32_t>> border_indices;
    std::vector<uint32_t> first_unique_indices;
  };

  struct patchwork
  {
    uint64_t id = 0;
    uint64_t render_program_id = 0;

    ludo::transform transform;

    std::vector<patch_variant> variants;
    std::vector<patch> patches;

    std::function<uint32_t(const patchwork& patchwork, uint32_t patch_index)> variant_index;
    std::function<std::pair<uint32_t, uint32_t>(const patchwork& patchwork, uint32_t patch_index, uint32_t variant_index)> counts;
    std::function<void(const patchwork& patchwork, uint32_t patch_index, uint32_t variant_index, ludo::mesh& mesh)> load;
    std::function<void(const patchwork& patchwork, uint32_t anchor_patch_index, uint32_t patch_index)> sew;
    std::function<void(const patchwork& patchwork, uint32_t patch_index)> on_load = [](const patchwork& patchwork, uint32_t patch_index) {};
    std::function<void(const patchwork& patchwork, uint32_t patch_index)> on_unload = [](const patchwork& patchwork, uint32_t patch_index) {};
  };

  struct game_controls
  {
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

  struct lod
  {
    uint32_t level;
    float max_distance;
  };

  struct map_controls
  {
    uint32_t target_index = 0;
    float target_radius = 0.0f;

    ludo::vec2 camera_rotation = ludo::vec2_zero;
    float camera_zoom = 5.0f;
  };

  struct person
  {
    bool standing = true;
    bool jumping = false;

    float turn_angle = 0.0f;
    float run_speed = 0.0f;
    float turn_speed = 0.0f;

    float walk_animation_time = 0.0f;
  };

  struct person_controls
  {
    bool forward = false;
    bool back = false;
    bool left = false;
    bool right = false;
    bool jump = false;

    ludo::vec2 camera_rotation = ludo::vec2_zero;
  };

  struct point_mass
  {
    float mass = 0.0f;

    ludo::transform transform;
    ludo::vec3 linear_velocity = ludo::vec3_zero;
    bool resting = false;

    std::vector<point_mass*> children;
  };

  struct solar_system
  {
    int32_t relative_celestial_body_index = -1;
    ludo::vec3 center_delta = ludo::vec3_zero;
  };

  struct spaceship_controls
  {
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
