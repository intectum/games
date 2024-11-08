#include "../constants.h"
#include "../terrain/terrain.h"
#include "sol.h"

namespace astrum
{
  // TODO make smooth (requires indexed support for slods, or maybe this doesn't need to be a slod?)

  float sol_height(const ludo::vec3& position);
  ludo::vec4 sol_color(float longitude, const std::array<float, 3>& heights, float gradient);
  std::vector<tree> sol_tree(const terrain& terrain, float radius, uint32_t chunk_index);

  void add_sol(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity)
  {
    auto celestial_body = ludo::add(
      inst,
      astrum::celestial_body
      {
        .name = "sol",
        .radius = sol_radius,
        .mass = sol_mass,
      },
      "celestial-bodies"
    );

    ludo::add(
      inst,
      point_mass
      {
        .mass = celestial_body->mass,
        .transform = initial_transform,
        .linear_velocity = initial_velocity
      },
      "celestial-bodies"
    );

    add_terrain(
      inst,
      terrain
      {
        .format = ludo::vertex_format_p,
        .lods = sol_lods,
        .height_func = [](const ludo::vec3& position) { return 1.0f; },
        .color_func = [](float longitude, const std::array<float, 3>& heights, float gradient) { return ludo::vec4_one; },
        .tree_func = [](const terrain& terrain, float radius, uint32_t chunk_index) { return std::array<std::vector<tree>, tree_type_count>(); }
      },
      *celestial_body,
      "celestial-bodies"
    );
  }

  void sync_light_with_sol(ludo::instance& inst)
  {
    auto rendering_context = ludo::first<ludo::rendering_context>(inst);

    auto point_mass = ludo::first<astrum::point_mass>(inst, "celestial-bodies");

    ludo::set_light(*rendering_context, ludo::light
    {
      .ambient = { 0.01f, 0.01f, 0.01f, 1.0f },
      .diffuse = { 0.7f, 0.7f, 0.7f, 1.0f },
      .specular = { 0.01f, 0.01f, 0.01f, 1.0f },
      .position = point_mass->transform.position,
      .attenuation = { 1.0f, 0.0f, 0.0f },
      .strength = 1.0f,
      .range = 5.0f * astronomical_unit
    }, 0);
  }
}
