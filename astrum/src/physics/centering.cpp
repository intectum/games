#include "../entities/celestial_bodies.h"
#include "centering.h"

namespace astrum
{
  void center_universe(ludo::instance& inst)
  {
    auto& dynamic_bodies = ludo::data<ludo::dynamic_body>(inst);
    auto& linear_octrees = ludo::data<ludo::linear_octree>(inst);
    auto& mesh_instances = ludo::data<ludo::mesh_instance>(inst);
    auto rendering_context = ludo::first<ludo::rendering_context>(inst);
    auto& static_bodies = ludo::data<ludo::static_body>(inst);

    auto& patchworks = ludo::data<patchwork>(inst);
    auto& point_masses = ludo::data<point_mass>(inst);

    auto camera = ludo::get_camera(*rendering_context);
    auto camera_position = ludo::position(camera.view);

    if (ludo::length(camera_position) < 10000.0f)
    {
      return;
    }

    auto delta = camera_position * -1.0f;

    for (auto& linear_octree : linear_octrees)
    {
      linear_octree.bounds.min += delta;
      linear_octree.bounds.max += delta;
    }

    for (auto& mesh_instance : mesh_instances)
    {
      if (mesh_instance.instance_buffer.data)
      {
        // TODO replace with at()
        auto transform = reinterpret_cast<ludo::mat4*>(mesh_instance.instance_buffer.data);
        ludo::position(*transform, ludo::position(*transform) + delta);
      }
    }

    ludo::position(camera.view, camera_position + delta);
    ludo::set_camera(*rendering_context, camera);

    for (auto& body : static_bodies)
    {
      body.transform.position += delta;
      ludo::push(body);
    }

    for (auto& body : dynamic_bodies)
    {
      body.transform.position += delta;
      ludo::push(body);
    }

    for (auto& patchwork : patchworks)
    {
      patchwork.transform.position += delta;
    }

    for (auto& point_mass : point_masses)
    {
      point_mass.transform.position += delta;
    }
  }
}
