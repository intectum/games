/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/spatial/octree.h>
#include <ludo/testing.h>

#include "octree.h"

namespace ludo
{
  std::vector<uint32_t> cell_elements(const octree& octree, uint32_t cell_index);

  void test_spatial_octree()
  {
    test_group("octree");

    auto bounds_1 = aabb3 { .min = { -1.0f, -1.0f, -1.0f }, .max = { 1.0f, 1.0f, 1.0f } };
    auto bounds_2 = aabb3 { .min = { -0.5f, -0.5f, -0.5f }, .max = { 0.5f, 0.5f, 0.5f } };
    auto bounds_3 = aabb3 { .min = { 0.25f, 0.25f, 0.25f }, .max = { 0.75f, 0.75f, 0.75f } };

    auto inst = instance();
    allocate<octree>(inst, 1);

    auto octree_1 = add(inst, octree { .bounds = bounds_1 });

    auto position_1 = vec3 { -0.25f, -0.25f, -0.25f };
    auto position_1_cell_index = 0;

    auto element_1 = 1;
    add(*octree_1, element_1, position_1);
    for (auto cell_index = 0; cell_index < 8; cell_index++)
    {
      auto elements = cell_elements(*octree_1, cell_index);
      if (cell_index == position_1_cell_index)
      {
        test_equal("octree: add (cell mesh instance count)", elements.size(), std::size_t(1));
      }
      else
      {
        test_equal("octree: add (cell mesh instance count)", elements.size(), std::size_t(0));
      }
    }

    auto element_2 = 2;
    add(*octree_1, element_2, position_1);

    remove(*octree_1, element_2, position_1);
    for (auto cell_index = 0; cell_index < 8; cell_index++)
    {
      auto elements = cell_elements(*octree_1, cell_index);
      if (cell_index == position_1_cell_index)
      {
        test_equal("octree: remove (cell mesh instance count)", elements.size(), std::size_t(1));
      }
      else
      {
        test_equal("octree: remove (cell mesh instance count)", elements.size(), std::size_t(0));
      }
    }

    auto meshes_3 = find(*octree_1, [&](const aabb3& bounds)
    {
      return intersect(bounds_2, bounds) ? 0 : -1;
    });
    test_equal("octree: find parallel", meshes_3.size(), std::size_t(1));

    auto meshes_4 = find(*octree_1, [&](const aabb3& bounds)
    {
      return intersect(bounds_3, bounds) ? 0 : -1;
    });
    test_equal("octree: find parallel 2", meshes_4.size(), std::size_t(0));
  }
}
