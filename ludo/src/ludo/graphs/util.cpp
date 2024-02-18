/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "./util.h"

namespace ludo
{
  void combine_meshes(std::set<mesh>& meshes)
  {
    auto start_iter = meshes.begin();
    auto combined_mesh = ludo::mesh { .instance_count = 0 };

    for (auto mesh_iter = meshes.begin(); mesh_iter != meshes.end(); mesh_iter++)
    {
      if (mesh_iter->id == combined_mesh.id && mesh_iter->instance_start == combined_mesh.instance_start + combined_mesh.instance_count)
      {
        combined_mesh.instance_count++;
      }
      else
      {
        if (combined_mesh.instance_count > 0)
        {
          meshes.erase(start_iter, mesh_iter);
          meshes.insert(combined_mesh);
        }

        combined_mesh = *mesh_iter;
        start_iter = mesh_iter;
      }
    }

    if (combined_mesh.instance_count > 0)
    {
      meshes.erase(start_iter, meshes.end());
      meshes.insert(combined_mesh);
    }
  }
}
