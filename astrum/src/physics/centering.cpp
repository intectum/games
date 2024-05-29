#include "../types.h"
#include "centering.h"

namespace astrum
{
  void center_universe(ludo::instance& inst)
  {
    auto& dynamic_bodies = ludo::data<ludo::dynamic_body>(inst);
    auto& ghost_bodies = ludo::data<ludo::ghost_body>(inst);
    auto& grids = ludo::data<ludo::grid3>(inst);
    auto& kinematic_bodies = ludo::data<ludo::kinematic_body>(inst);
    auto& render_meshes = ludo::data<ludo::render_mesh>(inst);
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

    for (auto& grid : grids)
    {
      grid.bounds.min += delta;
      grid.bounds.max += delta;
      ludo::commit_header(grid);
    }

    for (auto& partition_pair : render_meshes.partitions)
    {
      if (partition_pair.first == "physics" || partition_pair.first == "terrain")
      {
        continue;
      }

      for (auto& render_mesh: partition_pair.second)
      {
        if (render_mesh.instance_buffer.data)
        {
          for (auto instance_index = uint32_t(0); instance_index < render_mesh.instances.count; instance_index++)
          {
            auto& transform = ludo::instance_transform(render_mesh, instance_index);
            ludo::position(transform, ludo::position(transform) + delta);
          }
        }
      }
    }

    ludo::position(camera.view, camera_position + delta);
    ludo::set_camera(*rendering_context, camera);

    for (auto& body : static_bodies)
    {
      body.transform.position += delta;
      ludo::commit(body);
    }

    for (auto& body : dynamic_bodies)
    {
      body.transform.position += delta;
      ludo::commit(body);
    }

    for (auto& body : ghost_bodies)
    {
      body.transform.position += delta;
      ludo::commit(body);
    }

    for (auto& body : kinematic_bodies)
    {
      body.transform.position += delta;
      ludo::commit(body);
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
