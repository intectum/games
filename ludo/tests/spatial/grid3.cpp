/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/rendering.h>
#include <ludo/spatial/grid3.h>
#include <ludo/testing.h>

#include "grid3.h"

namespace ludo
{
  std::vector<uint64_t> cell_mesh_instance_ids(const grid3& grid, uint32_t cell_index);

  void test_spatial_grid3()
  {
    test_group("grid3");

    auto bounds_1 = aabb3 { .min = { -1.0f, -1.0f, -1.0f }, .max = { 1.0f, 1.0f, 1.0f } };
    auto bounds_2 = aabb3 { .min = { -0.5f, -0.5f, -0.5f }, .max = { 0.5f, 0.5f, 0.5f } };
    auto bounds_3 = aabb3 { .min = { 0.25f, 0.25f, 0.25f }, .max = { 0.75f, 0.75f, 0.75f } };

    auto inst = instance();
    allocate<grid3>(inst, 1);

    auto grid_1 = add(inst, grid3 { .bounds = bounds_1, .cell_count_1d = 2 });

    auto position_1 = vec3 { -0.25f, -0.25f, -0.25f };
    auto position_1_cell_index = 0;

    auto mesh_instance_1 = mesh_instance { .id = 1 };
    add(*grid_1, mesh_instance_1, position_1);
    for (auto cell_index = 0; cell_index < 8; cell_index++)
    {
      auto ids = cell_mesh_instance_ids(*grid_1, cell_index);
      if (cell_index == position_1_cell_index)
      {
        test_equal("grid3: add (cell mesh instance count)", ids.size(), std::size_t(1));
      }
      else
      {
        test_equal("grid3: add (cell mesh instance count)", ids.size(), std::size_t(0));
      }
    }

    auto mesh_instance_2 = mesh_instance { .id = 2 };
    add(*grid_1, mesh_instance_2, position_1);

    remove(*grid_1, mesh_instance_2, position_1);
    for (auto cell_index = 0; cell_index < 8; cell_index++)
    {
      auto ids = cell_mesh_instance_ids(*grid_1, cell_index);
      if (cell_index == position_1_cell_index)
      {
        test_equal("grid3: remove (cell mesh instance count)", ids.size(), std::size_t(1));
      }
      else
      {
        test_equal("grid3: remove (cell mesh instance count)", ids.size(), std::size_t(0));
      }
    }

    auto meshes_3 = find_parallel(*grid_1, [&](const aabb3& bounds)
    {
      return intersect(bounds_2, bounds) ? 0 : -1;
    });
    test_equal("grid3: find parallel", meshes_3.size(), std::size_t(1));

    auto meshes_4 = find_parallel(*grid_1, [&](const aabb3& bounds)
    {
      return intersect(bounds_3, bounds) ? 0 : -1;
    });
    test_equal("grid3: find parallel 2", meshes_4.size(), std::size_t(0));
  }
}
