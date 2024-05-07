#include "../types.h"
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

    auto& point_masses = ludo::data<point_mass>(inst);
    auto& terrains = ludo::data<terrain>(inst);

    auto& terrain_render_programs = ludo::data<ludo::render_program>(inst, "terrain");

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

    for (auto& partition_pair : mesh_instances.partitions)
    {
      if (partition_pair.first == "terrain")
      {
        continue;
      }

      for (auto& mesh_instance: partition_pair.second)
      {
        if (mesh_instance.instance_buffer.data)
        {
          auto& transform = ludo::instance_transform(mesh_instance);
          ludo::position(transform, ludo::position(transform) + delta);
        }
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

    for (auto index = uint32_t(0); index < terrains.length; index++)
    {
      auto& transform = ludo::cast<ludo::mat4>(terrain_render_programs[index].shader_buffer.back, 0);
      ludo::position(transform, ludo::position(transform) + delta);
    }

    for (auto& point_mass : point_masses)
    {
      point_mass.transform.position += delta;
    }
  }
}
