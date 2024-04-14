#include "../types.h"
#include "centering.h"

namespace astrum
{
  void center_universe(ludo::instance& inst)
  {
    auto& dynamic_bodies = ludo::data<ludo::dynamic_body>(inst);
    auto& linear_octrees = ludo::data<ludo::linear_octree>(inst);
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);
    auto& static_bodies = ludo::data<ludo::static_body>(inst);

    auto& patchworks = ludo::data<patchwork>(inst);
    auto& point_masses = ludo::data<point_mass>(inst);
    auto& solar_system = *ludo::first<astrum::solar_system>(inst);

    auto camera = ludo::get_camera(rendering_context);
    auto position = ludo::position(camera.view);

    if (ludo::length(position) < 10000.0f)
    {
      solar_system.center_delta = ludo::vec3_zero;
      return;
    }

    solar_system.center_delta = position * -1.0f;

    for (auto& linear_octree: linear_octrees)
    {
      ludo::move(linear_octree, solar_system.center_delta);
    }

    for (auto& body: static_bodies)
    {
      body.transform.position += solar_system.center_delta;
      ludo::push(body);
    }

    for (auto& body: dynamic_bodies)
    {
      body.transform.position += solar_system.center_delta;
      ludo::push(body);
    }

    for (auto& patchwork: patchworks)
    {
      patchwork.transform.position += solar_system.center_delta;
    }

    for (auto& point_mass : point_masses)
    {
      point_mass.transform.position += solar_system.center_delta;
    }
  }
}
