/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "../graphs.h"
#include "../tasks.h"

namespace ludo
{
  void find(std::vector<uint64_t>& results, const linear_octree& octree, uint32_t octant_count_1d, uint8_t depth, const aabb& bounds, const std::function<int32_t(const aabb& bounds)>& test, bool inside);
  std::array<ludo::aabb, 8> calculate_child_bounds(const aabb& bounds);
  std::vector<uint64_t> octant_mesh_instance_ids(const linear_octree& octree, uint32_t octant_index);
  uint32_t octant_mesh_instance_index(const linear_octree& octree, uint64_t mesh_instance_id, uint32_t octant_index);
  uint64_t octant_offset(const linear_octree& octree, uint32_t octant_index);
  uint32_t to_index(uint32_t octant_count_1d, const std::array<uint32_t, 3>& octant_coordinates);
  std::array<uint32_t, 3> to_octant_coordinates(const linear_octree& octree, const vec3& position);

  // 3 * vec3, padded to 16 bytes
  const auto front_buffer_header_size = sizeof(vec3) + 4 + sizeof(vec3) + 4 + sizeof(vec3) + 4;

  // The mesh instance count, padded to 8 bytes
  const auto octant_header_size = sizeof(uint32_t) + 4;

  // 2 IDs and 4 index/start/count values
  const auto mesh_instance_size = 2 * sizeof(uint64_t) + 4 * sizeof(uint32_t);

  template<>
  linear_octree* add(instance& instance, const linear_octree& init, const std::string& partition)
  {
    assert(init.depth <= 9 && "maximum supported depth (9) exceeded");

    auto octree = add(data<ludo::linear_octree>(instance), init, partition);
    octree->id = next_id++;
    octree->compute_program_id = add_linear_octree_compute_program(instance, *octree, 16)->id;

    auto octant_count_1d = static_cast<uint32_t>(std::pow(2.0f, octree->depth));
    auto octant_count = static_cast<uint32_t>(std::pow(octant_count_1d, 3));

    auto data_size = octant_offset(*octree, octant_count);

    octree->front_buffer = allocate_vram(front_buffer_header_size + data_size);
    octree->back_buffer = allocate(data_size);

    // TODO initialize counts to 0

    return octree;
  }

  void push(linear_octree& octree)
  {
    auto offset = 0;
    write(octree.front_buffer, offset, octree.bounds.min);
    offset += 16;
    write(octree.front_buffer, offset, octree.bounds.max);
    offset += 16;
    write(octree.front_buffer, offset, ludo::octant_size(octree));
    offset += 16;
    std::memcpy(octree.front_buffer.data + offset, octree.back_buffer.data, octree.back_buffer.size);
  }

  void add(linear_octree& octree, const mesh_instance& mesh_instance, const vec3& position)
  {
    auto octant_count_1d = static_cast<uint32_t>(std::pow(2.0f, octree.depth));
    auto offset = octant_offset(octree, to_index(octant_count_1d, to_octant_coordinates(octree, position)));

    auto mesh_instance_count = read<uint32_t>(octree.back_buffer, offset);

    assert(mesh_instance_count < octree.octant_capacity && "octant is full");

    write(octree.back_buffer, offset, mesh_instance_count + 1);
    offset += 4;
    offset += 4;
    offset += mesh_instance_count * mesh_instance_size;
    write(octree.back_buffer, offset, mesh_instance.id);
    offset += 8;
    write(octree.back_buffer, offset, mesh_instance.render_program_id);
    offset += 8;
    write(octree.back_buffer, offset, mesh_instance.instance_index);
    offset += 4;
    write(octree.back_buffer, offset, mesh_instance.indices.start);
    offset += 4;
    write(octree.back_buffer, offset, mesh_instance.indices.count);
    offset += 4;
    write(octree.back_buffer, offset, mesh_instance.vertices.start);
  }

  void remove(linear_octree& octree, const mesh_instance& mesh_instance, const vec3& position)
  {
    auto octant_count_1d = static_cast<uint32_t>(std::pow(2.0f, octree.depth));
    auto octant_coordinates = to_octant_coordinates(octree, position);
    auto octant_index = to_index(octant_count_1d, octant_coordinates);
    auto mesh_instance_index = octant_mesh_instance_index(octree, mesh_instance.id, octant_index);

    if (mesh_instance_index == octree.octant_capacity)
    {
      // Search adjacent octants in case of floating point precision errors
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

            octant_index = to_index(octant_count_1d, adjacent_octant_coordinates);
            mesh_instance_index = octant_mesh_instance_index(octree, mesh_instance.id, octant_index);
            if (mesh_instance_index < octree.octant_capacity)
            {
              break;
            }
          }

          if (mesh_instance_index < octree.octant_capacity)
          {
            break;
          }
        }

        if (mesh_instance_index < octree.octant_capacity)
        {
          break;
        }
      }
    }

    assert(mesh_instance_index < octree.octant_capacity && "mesh instance not found");

    auto offset = octant_offset(octree, octant_index);

    auto mesh_instance_count = read<uint32_t>(octree.back_buffer, offset) - 1;
    write(octree.back_buffer, offset, mesh_instance_count);
    offset += 4;
    offset += 4;

    offset += mesh_instance_index * mesh_instance_size;
    std::memmove(
      octree.back_buffer.data + offset,
      octree.back_buffer.data + offset + mesh_instance_size,
      (mesh_instance_count - mesh_instance_index) * mesh_instance_size
    );
  }

  std::vector<uint64_t> find(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test)
  {
    auto mesh_instance_ids = std::vector<uint64_t>();
    auto octant_count_1d = static_cast<uint32_t>(std::pow(2.0f, octree.depth));
    find(mesh_instance_ids, octree, octant_count_1d, 0, octree.bounds, test, false);

    return mesh_instance_ids;
  }

  void find(std::vector<uint64_t>& results, const linear_octree& octree, uint32_t octant_count_1d, uint8_t depth, const aabb& bounds, const std::function<int32_t(const aabb& bounds)>& test, bool inside)
  {
    auto test_result = inside ? 1 : test(bounds);
    if (test_result == -1)
    {
      return;
    }

    if (depth == octree.depth)
    {
      auto center = bounds.min + (bounds.max - bounds.min) / 2.0f;
      auto octant_index = to_index(octant_count_1d, to_octant_coordinates(octree, center));
      auto ids = octant_mesh_instance_ids(octree, octant_index);
      results.insert(results.end(), ids.begin(), ids.end());

      return;
    }

    auto child_bounds = calculate_child_bounds(bounds);
    for (auto child_bound : child_bounds)
    {
      find(results, octree, octant_count_1d, depth + 1, child_bound, test, test_result == 1);
    }
  }

  std::vector<uint64_t> find_parallel(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test)
  {
    auto mesh_instance_ids = std::vector<uint64_t>();
    auto octant_count_1d = static_cast<uint32_t>(std::pow(2, octree.depth));
    auto octant_count = static_cast<uint32_t>(std::pow(octant_count_1d, 3));
    auto octant_size = ludo::octant_size(octree);

    divide_and_conquer(octant_count, [&octree, &test, &mesh_instance_ids, octant_count_1d, octant_size](uint32_t start, uint32_t end)
    {
      auto task_mesh_instance_ids = std::vector<uint64_t>();

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
          auto ids = octant_mesh_instance_ids(octree, index);
          task_mesh_instance_ids.insert(task_mesh_instance_ids.end(), ids.begin(), ids.end());
        }
      }

      return [&mesh_instance_ids, task_mesh_instance_ids]()
      {
        mesh_instance_ids.insert(mesh_instance_ids.end(), task_mesh_instance_ids.begin(), task_mesh_instance_ids.end());
      };
    });

    return mesh_instance_ids;
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

  std::vector<uint64_t> octant_mesh_instance_ids(const linear_octree& octree, uint32_t octant_index)
  {
    auto octant_mesh_instance_ids = std::vector<uint64_t>();
    auto offset = octant_offset(octree, octant_index);

    auto mesh_instance_count = read<uint32_t>(octree.back_buffer, offset);
    offset += 4;
    offset += 4;

    for (auto mesh_instance_index = uint32_t(0); mesh_instance_index < mesh_instance_count; mesh_instance_index++)
    {
      octant_mesh_instance_ids.push_back(read<uint64_t>(octree.back_buffer, offset));
      offset += mesh_instance_size;
    }

    return octant_mesh_instance_ids;
  }

  uint32_t octant_mesh_instance_index(const linear_octree& octree, uint64_t mesh_instance_id, uint32_t octant_index)
  {
    auto offset = octant_offset(octree, octant_index);

    auto mesh_instance_count = read<uint32_t>(octree.back_buffer, offset);
    offset += 4;
    offset += 4;

    for (auto mesh_instance_index = uint32_t(0); mesh_instance_index < mesh_instance_count; mesh_instance_index++)
    {
      if (read<uint64_t>(octree.back_buffer, offset) == mesh_instance_id)
      {
        return mesh_instance_index;
      }

      offset += mesh_instance_size;
    }

    return octree.octant_capacity;
  }

  uint64_t octant_offset(const linear_octree& octree, uint32_t octant_index)
  {
    return octant_index * (octant_header_size + octree.octant_capacity * mesh_instance_size);
  }

  uint32_t to_index(uint32_t octant_count_1d, const std::array<uint32_t, 3>& octant_coordinates)
  {
    return octant_coordinates[0] * octant_count_1d * octant_count_1d + octant_coordinates[1] * octant_count_1d + octant_coordinates[2];
  }

  std::array<uint32_t, 3> to_octant_coordinates(const linear_octree& octree, const vec3& position)
  {
    assert(position >= octree.bounds.min && position <= octree.bounds.max && "position out of bounds");

    auto octant_size = ludo::octant_size(octree);

    auto octant_coordinates = (position - octree.bounds.min) / octant_size;
    octant_coordinates[0] = std::floor(octant_coordinates[0]);
    octant_coordinates[1] = std::floor(octant_coordinates[1]);
    octant_coordinates[2] = std::floor(octant_coordinates[2]);

    return { static_cast<uint32_t>(octant_coordinates[0]), static_cast<uint32_t>(octant_coordinates[1]), static_cast<uint32_t>(octant_coordinates[2]) };
  }
}
