#include "../constants.h"
#include "sol.h"

namespace astrum
{
  // TODO make smooth (requires indexed support for slods, or maybe this doesn't need to be a slod?)

  float sol_height(const ludo::vec3& position)
  {
    return 1.0f;
  }

  ludo::vec4 sol_color(float longitude, const std::array<float, 3>& heights, float gradient)
  {
    return ludo::vec4_one;
  }

  std::vector<tree> sol_tree(uint64_t patch_id)
  {
    return {};
  }

  void sync_light_with_sol(ludo::instance& inst)
  {
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto& point_mass = *ludo::first<astrum::point_mass>(inst, "celestial-bodies");

    ludo::set_light(rendering_context, ludo::light
    {
      .ambient = { 0.01f, 0.01f, 0.01f, 1.0f },
      .diffuse = { 0.7f, 0.7f, 0.7f, 1.0f },
      .specular = { 0.01f, 0.01f, 0.01f, 1.0f },
      .position = point_mass.transform.position,
      .attenuation = { 1.0f, 0.0f, 0.0f },
      .strength = 1.0f,
      .range = 5.0f * astronomical_unit
    }, 0);
  }
}
