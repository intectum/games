/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "../graphs.h"
#include "../tasks.h"

namespace ludo
{
  void find(std::vector<mesh_instance>& results, const linear_octree& octree, uint8_t depth, const aabb& bounds, const std::function<int32_t(const aabb& bounds)>& test, bool inside);
  std::array<ludo::aabb, 8> calculate_child_bounds(const aabb& bounds);
  std::array<uint32_t, 3> to_octant_coordinates(uint32_t key);
  std::array<uint32_t, 3> to_octant_coordinates(const linear_octree& octree, const vec3& position);
  uint32_t to_key(const std::array<uint32_t, 3>& octant_coordinates);
  uint32_t to_key(const linear_octree& octree, const vec3& position);

  template<>
  linear_octree* add(instance& instance, const linear_octree& init, const std::string& partition)
  {
    assert(init.depth <= 9 && "maximum supported depth (9) exceeded");

    auto octree = add(data<ludo::linear_octree>(instance), init, partition);
    octree->id = next_id++;

    auto octant_count_1d = static_cast<uint32_t>(std::pow(2.0f, octree->depth));
    for (auto x = uint32_t(0); x < octant_count_1d; x++)
    {
      for (auto y = uint32_t(0); y < octant_count_1d; y++)
      {
        for (auto z = uint32_t(0); z < octant_count_1d; z++)
        {
          octree->octants[to_key({ x, y, z })] = std::vector<mesh_instance>();
        }
      }
    }

    return octree;
  }

  void add(linear_octree& octree, const mesh_instance& element, const vec3& position)
  {
    octree.octants[to_key(octree, position)].emplace_back(element);
  }

  void remove(linear_octree& octree, const mesh_instance& element, const vec3& position)
  {
    auto octant_coordinates = to_octant_coordinates(octree, position);

    auto& elements = octree.octants[to_key(octant_coordinates)];
    auto element_iter = std::find_if(elements.begin(), elements.end(), [&](const mesh_instance& the_mesh_instance) { return the_mesh_instance.id == element.id; });
    if (element_iter != elements.end())
    {
      elements.erase(element_iter);
      return;
    }

    // Search adjacent octants in case of floating point precision errors
    auto octant_count_1d = static_cast<uint32_t>(std::pow(2.0f, octree.depth));
    auto offsets = std::array<int32_t, 3> { -1, 0, 1 };
    for (auto offset_x : offsets)
    {
      if ((octant_coordinates[0] == 0 && offset_x == -1) || octant_coordinates[0] == octant_count_1d - 1 && offset_x == 1)
      {
        continue;
      }

      for (auto offset_y : offsets)
      {
        if ((octant_coordinates[1] == 0 && offset_y == -1) || octant_coordinates[1] == octant_count_1d - 1 && offset_y == 1)
        {
          continue;
        }

        for (auto offset_z : offsets)
        {
          if ((octant_coordinates[2] == 0 && offset_z == -1) || octant_coordinates[2] == octant_count_1d - 1 && offset_z == 1)
          {
            continue;
          }

          auto adjacent_octant_coordinates = std::array<uint32_t, 3> { octant_coordinates[0] + offset_x, octant_coordinates[1] + offset_y, octant_coordinates[2] + offset_z };
          if (adjacent_octant_coordinates == octant_coordinates)
          {
            continue;
          }

          auto& adjacent_elements = octree.octants[to_key(adjacent_octant_coordinates)];
          auto adjacent_element_iter = std::find_if(adjacent_elements.begin(), adjacent_elements.end(), [&](const mesh_instance& the_mesh_instance) { return the_mesh_instance.id == element.id; });
          if (adjacent_element_iter != adjacent_elements.end())
          {
            adjacent_elements.erase(adjacent_element_iter);
            return;
          }
        }
      }
    }

    assert(false && "element not found");
  }

  void move(linear_octree& octree, const vec3& movement)
  {
    octree.bounds.min += movement;
    octree.bounds.max += movement;

    for (auto& octant: octree.octants)
    {
      for (auto& mesh: octant.second)
      {
        position(mesh.transform, position(mesh.transform) + movement);
      }
    }
  }

  std::vector<mesh_instance> find(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test)
  {
    auto mesh_instances = std::vector<mesh_instance>();
    find(mesh_instances, octree, 0, octree.bounds, test, false);

    return mesh_instances;
  }

  void find(std::vector<mesh_instance>& results, const linear_octree& octree, uint8_t depth, const aabb& bounds, const std::function<int32_t(const aabb& bounds)>& test, bool inside)
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
      results.insert(results.end(), elements.begin(), elements.end());

      return;
    }

    auto child_bounds = calculate_child_bounds(bounds);
    for (auto child_bound : child_bounds)
    {
      find(results, octree, depth + 1, child_bound, test, test_result == 1);
    }
  }

  std::vector<mesh_instance> find_parallel(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test)
  {
    auto mesh_instances = std::vector<mesh_instance>();
    auto octant_count_1d = static_cast<uint32_t>(std::pow(2, octree.depth));
    auto octant_count = static_cast<uint32_t>(std::pow(octant_count_1d, 3));
    auto octant_size = ludo::octant_size(octree);

    divide_and_conquer(octant_count, [&octree, &test, &mesh_instances, octant_count_1d, octant_size](uint32_t start, uint32_t end)
    {
      auto task_mesh_instances = std::vector<mesh_instance>();

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
          auto& octant_meshes = octree.octants.at(to_key({ x, y, z }));
          task_mesh_instances.insert(task_mesh_instances.end(), octant_meshes.begin(), octant_meshes.end());
        }
      }

      return [&mesh_instances, task_mesh_instances]()
      {
        mesh_instances.insert(mesh_instances.end(), task_mesh_instances.begin(), task_mesh_instances.end());
      };
    });

    return mesh_instances;
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

  uint32_t to_key(const std::array<uint32_t, 3>& octant_coordinates)
  {
    return octant_coordinates[0] | octant_coordinates[1] << 10 | octant_coordinates[2] << 20;
  }

  uint32_t to_key(const linear_octree& octree, const vec3& position)
  {
    return to_key(to_octant_coordinates(octree, position));
  }

  std::array<uint32_t, 3> to_octant_coordinates(uint32_t key)
  {
    return { key & 0x3FF, (key >> 10) & 0x3FF, (key >> 20) & 0xFFF };
  }

  std::array<uint32_t, 3> to_octant_coordinates(const linear_octree& octree, const vec3& position)
  {
    assert(position >= octree.bounds.min && position <= octree.bounds.max && "out of bounds!");

    auto octant_size = ludo::octant_size(octree);

    auto octant_coordinates = (position - octree.bounds.min) / octant_size;
    octant_coordinates[0] = std::floor(octant_coordinates[0]);
    octant_coordinates[1] = std::floor(octant_coordinates[1]);
    octant_coordinates[2] = std::floor(octant_coordinates[2]);

    return { static_cast<uint32_t>(octant_coordinates[0]), static_cast<uint32_t>(octant_coordinates[1]), static_cast<uint32_t>(octant_coordinates[2]) };
  }
}
