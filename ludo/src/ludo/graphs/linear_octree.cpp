/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "../graphs.h"
#include "../tasks.h"
#include "./util.h"

namespace ludo
{
  void find(std::set<mesh>& results, const linear_octree& octree, uint8_t depth, const aabb& bounds, const std::function<int32_t(const aabb& bounds)>& test, bool inside);
  std::array<ludo::aabb, 8> calculate_child_bounds(const aabb& bounds);
  uint32_t to_key(const linear_octree& octree, const vec3& position);
  uint32_t to_key(uint32_t x, uint32_t y, uint32_t z);

  template<>
  linear_octree* add(instance& instance, const linear_octree& init, const std::string& partition)
  {
    assert(init.depth <= 9 && "maximum supported depth (9) exceeded");

    auto octree = add(data<ludo::linear_octree>(instance), init, partition);
    octree->id = next_id++;

    auto octant_count_1d = static_cast<uint32_t>(std::pow(2.0f, octree->depth));
    for (auto x = 0; x < octant_count_1d; x++)
    {
      for (auto y = 0; y < octant_count_1d; y++)
      {
        for (auto z = 0; z < octant_count_1d; z++)
        {
          octree->octants[to_key(x, y, z)] = std::vector<mesh>();
        }
      }
    }

    return octree;
  }

  void add(linear_octree& octree, const mesh& element, const vec3& position)
  {
    octree.octants[to_key(octree, position)].emplace_back(element);
  }

  bool remove(linear_octree& octree, const mesh& element, const vec3& position)
  {
    auto& elements = octree.octants[to_key(octree, position)];
    for (auto element_iter = elements.begin(); element_iter < elements.end(); element_iter++)
    {
      if (element_iter->id == element.id && element_iter->instance_start == element.instance_start)
      {
        elements.erase(element_iter);
        return true;
      }
    }

    return false;
  }

  std::set<mesh> find(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test)
  {
    auto meshes = std::set<mesh>();
    find(meshes, octree, 0, octree.bounds, test, false);
    combine_meshes(meshes);

    return meshes;
  }

  void find(std::set<mesh>& meshes, const linear_octree& octree, uint8_t depth, const aabb& bounds, const std::function<int32_t(const aabb& bounds)>& test, bool inside)
  {
    auto test_result = inside ? 1 : test(bounds);
    if (test_result == -1)
    {
      return;
    }

    if (depth == octree.depth)
    {
      auto center = bounds.min + (bounds.max - bounds.min) / 2.0f;
      auto& elements = octree.octants.at(to_key(octree, center));
      meshes.insert(elements.begin(), elements.end());

      return;
    }

    auto child_bounds = calculate_child_bounds(bounds);
    for (auto child_bound : child_bounds)
    {
      find(meshes, octree, depth + 1, child_bound, test, test_result == 1);
    }
  }

  std::set<mesh> find_parallel(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test)
  {
    auto meshes = std::set<mesh>();
    auto octant_count_1d = static_cast<uint32_t>(std::pow(2, octree.depth));
    auto octant_count = static_cast<uint32_t>(std::pow(octant_count_1d, 3));
    auto octant_size = ludo::octant_size(octree);

    divide_and_conquer(octant_count, [&octree, &test, &meshes, octant_count_1d, octant_size](uint32_t start, uint32_t end)
    {
      auto task_meshes = std::set<mesh>();

      for (auto index = start; index < end; index++)
      {
        auto x = index / (octant_count_1d * octant_count_1d);
        auto y = (index / octant_count_1d) % octant_count_1d;
        auto z = index % octant_count_1d;

        auto xyz = vec3 { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
        auto min = octree.bounds.min + xyz * octant_size;

        auto bounds = aabb
        {
          .min = min,
          .max = min + octant_size
        };

        if (test(bounds) != -1)
        {
          auto& octant_meshes = octree.octants.at(to_key(x, y, z));
          task_meshes.insert(octant_meshes.begin(), octant_meshes.end());
        }
      }

      return [&meshes, task_meshes]()
      {
        meshes.insert(task_meshes.begin(), task_meshes.end());
      };
    });

    combine_meshes(meshes);

    return meshes;
  }

  vec3 octant_size(const linear_octree& octree)
  {
    auto bounds_size = octree.bounds.max - octree.bounds.min;
    auto octant_count_1d = std::pow(2, octree.depth);
    return bounds_size / static_cast<float>(octant_count_1d);
  }

  std::array<ludo::aabb, 8> calculate_child_bounds(const aabb& bounds)
  {
    auto half_dimensions = (bounds.max - bounds.min) / 2.0f;
    auto half_x = ludo::vec3 { half_dimensions[0], 0.0f, 0.0f };
    auto half_y = ludo::vec3 { 0.0f, half_dimensions[1], 0.0f };
    auto half_z = ludo::vec3 { 0.0f, 0.0f, half_dimensions[2] };

    return std::array<ludo::aabb, 8>
    {
      ludo::aabb
      {
        .min = bounds.min,
        .max = bounds.min + half_dimensions
      },
      ludo::aabb
      {
        .min = bounds.min + half_x,
        .max = bounds.min + half_dimensions + half_x
      },
      ludo::aabb
      {
        .min = bounds.min + half_y,
        .max = bounds.min + half_dimensions + half_y
      },
      ludo::aabb
      {
        .min = bounds.min + half_z,
        .max = bounds.min + half_dimensions + half_z
      },
      ludo::aabb
      {
        .min = bounds.min + half_x + half_y,
        .max = bounds.min + half_dimensions + half_x + half_y
      },
      ludo::aabb
      {
        .min = bounds.min + half_x + half_z,
        .max = bounds.min + half_dimensions + half_x + half_z
      },
      ludo::aabb
      {
        .min = bounds.min + half_y + half_z,
        .max = bounds.min + half_dimensions + half_y + half_z
      },
      ludo::aabb
      {
        .min = bounds.min + half_dimensions,
        .max = bounds.max
      }
    };
  }

  uint32_t to_key(const linear_octree& octree, const vec3& position)
  {
    assert(position >= octree.bounds.min && position <= octree.bounds.max && "out of bounds!");

    auto octant_size = ludo::octant_size(octree);

    auto octant_coordinates = (position - octree.bounds.min) / octant_size;
    octant_coordinates[0] = std::floor(octant_coordinates[0]);
    octant_coordinates[1] = std::floor(octant_coordinates[1]);
    octant_coordinates[2] = std::floor(octant_coordinates[2]);

    return to_key(
      static_cast<uint32_t>(octant_coordinates[0]),
      static_cast<uint32_t>(octant_coordinates[1]),
        static_cast<uint32_t>(octant_coordinates[2])
    );
  }

  uint32_t to_key(uint32_t x, uint32_t y, uint32_t z)
  {
    return x | y << 10 | z << 20;
  }
}
