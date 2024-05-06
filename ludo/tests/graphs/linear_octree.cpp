/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/graphs.h>
#include <ludo/rendering.h>
#include <ludo/testing.h>

#include "linear_octree.h"

namespace ludo
{
  std::vector<uint64_t> octant_mesh_instance_ids(const linear_octree& octree, uint32_t octant_index);

  void test_graphs_linear_octree()
  {
    test_group("linear_octree");

    auto bounds_1 = aabb { .min = { -1.0f, -1.0f, -1.0f }, .max = { 1.0f, 1.0f, 1.0f } };
    auto bounds_2 = aabb { .min = { -0.5f, -0.5f, -0.5f }, .max = { 0.5f, 0.5f, 0.5f } };
    auto bounds_3 = aabb { .min = { 0.25f, 0.25f, 0.25f }, .max = { 0.75f, 0.75f, 0.75f } };

    auto inst = instance();
    allocate<linear_octree>(inst, 1);

    auto linear_octree_1 = add(inst, linear_octree { .bounds = bounds_1 });

    auto position_1 = vec3 { -0.25f, -0.25f, -0.25f };
    auto position_1_octant_index = 0;

    auto mesh_instance_1 = mesh_instance { .id = 1 };
    add(*linear_octree_1, mesh_instance_1, position_1);
    for (auto octant_index = 0; octant_index < 8; octant_index++)
    {
      auto ids = octant_mesh_instance_ids(*linear_octree_1, octant_index);
      if (octant_index == position_1_octant_index)
      {
        test_equal("linear_octree: add (octant element count)", ids.size(), std::size_t(1));
      }
      else
      {
        test_equal("linear_octree: add (octant element count)", ids.size(), std::size_t(0));
      }
    }

    auto mesh_instance_2 = mesh_instance { .id = 2 };
    add(*linear_octree_1, mesh_instance_2, position_1);

    remove(*linear_octree_1, mesh_instance_2, position_1);
    for (auto octant_index = 0; octant_index < 8; octant_index++)
    {
      auto ids = octant_mesh_instance_ids(*linear_octree_1, octant_index);
      if (octant_index == position_1_octant_index)
      {
        test_equal("linear_octree: remove (octant element count)", ids.size(), std::size_t(1));
      }
      else
      {
        test_equal("linear_octree: remove (octant element count)", ids.size(), std::size_t(0));
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

  // dummy functions so that buffers compile
  compute_program* add_linear_octree_compute_program(instance& instance, const linear_octree& octree)
  {
    return new compute_program();
  }

  void set_texture(mesh_instance& mesh_instance, const texture& texture)
  {
  }
}
