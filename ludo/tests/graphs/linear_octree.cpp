/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/graphs.h>
#include <ludo/testing.h>

#include "linear_octree.h"

namespace ludo
{
  void test_graphs_linear_octree()
  {
    test_group("linear_octree");

    auto bounds_1 = aabb { .min = { -1.0f, -1.0f, -1.0f }, .max = { 1.0f, 1.0f, 1.0f } };
    auto bounds_2 = aabb { .min = { -0.5f, -0.5f, -0.5f }, .max = { 0.5f, 0.5f, 0.5f } };
    auto bounds_3 = aabb { .min = { 0.25f, 0.25f, 0.25f }, .max = { 0.75f, 0.75f, 0.75f } };

    auto inst = instance();
    allocate<linear_octree>(inst, 1);

    auto linear_octree_1 = add(inst, linear_octree { .bounds = bounds_1 });
    test_equal("linear_octree: init (octant count)", linear_octree_1->octants.size(), std::size_t(8));

    auto position_1 = vec3 { -0.25f, -0.25f, -0.25f };
    auto position_1_octant_key = 0;

    auto mesh_1 = mesh { .id = 1 };
    add(*linear_octree_1, mesh_1, position_1);
    for (auto& octant : linear_octree_1->octants)
    {
      if (octant.first == position_1_octant_key)
      {
        test_equal("linear_octree: add (octant element count)", octant.second.size(), std::size_t(1));
      }
      else
      {
        test_equal("linear_octree: add (octant element count)", octant.second.size(), std::size_t(0));
      }
    }

    auto mesh_2 = mesh { .id = 2 };
    add(*linear_octree_1, mesh_2, position_1);

    remove(*linear_octree_1, mesh_2, position_1);
    for (auto& octant : linear_octree_1->octants)
    {
      if (octant.first == position_1_octant_key)
      {
        test_equal("linear_octree: add (octant element count)", octant.second.size(), std::size_t(1));
      }
      else
      {
        test_equal("linear_octree: add (octant element count)", octant.second.size(), std::size_t(0));
      }
    }

    auto meshes_1 = find(*linear_octree_1, [&bounds_2](const aabb& bounds)
    {
      return intersect(bounds_2, bounds) ? 0 : -1;
    });
    test_equal("octree: find", meshes_1.size(), std::size_t(1));

    auto meshes_2 = find(*linear_octree_1, [&bounds_3](const aabb& bounds)
    {
      return intersect(bounds_3, bounds) ? 0 : -1;
    });
    test_equal("octree: find 2", meshes_2.size(), std::size_t(0));

    auto meshes_3 = find_parallel(*linear_octree_1, [&bounds_2](const aabb& bounds)
    {
      return intersect(bounds_2, bounds) ? 0 : -1;
    });
    test_equal("octree: find parallel", meshes_3.size(), std::size_t(1));

    auto meshes_4 = find_parallel(*linear_octree_1, [&bounds_3](const aabb& bounds)
    {
      return intersect(bounds_3, bounds) ? 0 : -1;
    });
    test_equal("octree: find parallel 2", meshes_4.size(), std::size_t(0));
  }
}
