/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "octree.h"

namespace ludo
{
  void find(const octree& octree, const std::function<int32_t(const aabb3& bounds)>& test, uint32_t divisions, const aabb3& bounds, uint32_t cumulative_index, std::vector<uint32_t>& results);
  vec3 cell_dimensions(const octree& octree);
  uint32_t cell_element_index(const octree& octree, uint32_t cell_index, uint32_t element);
  std::vector<uint32_t> cell_elements(const octree& octree, uint32_t cell_index);
  uint64_t cell_offset(const octree& octree, uint32_t cell_index);
  std::array<aabb3, 8> octant_bounds(const aabb3& bounds);
  uint32_t to_index(const octree& octree, const std::array<uint32_t, 3>& cell_coordinates);
  std::array<uint32_t, 3> to_cell_coordinates(const octree& octree, const vec3& position);

  void init(octree& octree)
  {
    octree.id = next_id++;

    auto cell_count = static_cast<uint32_t>(std::pow(8, octree.divisions));
    auto cell_size = sizeof(uint32_t) + octree.cell_capacity * sizeof(uint32_t);

    auto data_size = cell_count * cell_size;
    octree.buffer = allocate(data_size);

    auto offset = uint32_t(0);
    for (auto cell_index = uint32_t(0); cell_index < cell_count; cell_index++)
    {
      cast<uint32_t>(octree.buffer, offset) = 0;
      offset += cell_size;
    }
  }

  void de_init(octree& octree)
  {
    deallocate(octree.buffer);
  }

  void add(octree& octree, uint32_t element, const ludo::vec3& position)
  {
    auto index = to_index(octree, to_cell_coordinates(octree, position));
    auto offset = cell_offset(octree, index);
    auto stream = ludo::stream(octree.buffer, offset);

    auto element_count = peek<uint32_t>(stream);

    assert(element_count < octree.cell_capacity && "cell is full");

    write(stream, element_count + 1);
    stream.position += element_count * sizeof(uint32_t);
    write(stream, element);
  }

  void remove(octree& octree, uint32_t element, const ludo::vec3& position)
  {
    auto cell_coordinates = to_cell_coordinates(octree, position);
    auto cell_index = to_index(octree, cell_coordinates);
    auto element_index = cell_element_index(octree, cell_index, element);

    if (element_index == octree.cell_capacity)
    {
      // Search adjacent cells in case of floating point precision errors
      auto cell_count_1d = uint32_t(std::pow(2, octree.divisions));
      auto offsets = std::array<int32_t, 3> { -1, 0, 1 };
      for (auto offset_x : offsets)
      {
        if ((cell_coordinates[0] == 0 && offset_x == -1) || cell_coordinates[0] == cell_count_1d - 1 && offset_x == 1)
        {
          continue;
        }

        for (auto offset_y : offsets)
        {
          if ((cell_coordinates[1] == 0 && offset_y == -1) || cell_coordinates[1] == cell_count_1d - 1 && offset_y == 1)
          {
            continue;
          }

          for (auto offset_z : offsets)
          {
            if ((cell_coordinates[2] == 0 && offset_z == -1) || cell_coordinates[2] == cell_count_1d - 1 && offset_z == 1)
            {
              continue;
            }

            auto adjacent_cell_coordinates = std::array<uint32_t, 3> { cell_coordinates[0] + offset_x, cell_coordinates[1] + offset_y, cell_coordinates[2] + offset_z };
            if (adjacent_cell_coordinates == cell_coordinates)
            {
              continue;
            }

            cell_index = to_index(octree, adjacent_cell_coordinates);
            element_index = cell_element_index(octree, cell_index, element);
            if (element_index < octree.cell_capacity)
            {
              break;
            }
          }

          if (element_index < octree.cell_capacity)
          {
            break;
          }
        }

        if (element_index < octree.cell_capacity)
        {
          break;
        }
      }
    }

    assert(element_index < octree.cell_capacity && "element not found");

    auto offset = cell_offset(octree, cell_index);
    auto stream = ludo::stream(octree.buffer, offset);

    auto element_count = peek<uint32_t>(stream) - 1;
    write(stream, element_count);
    stream.position += element_index * sizeof(uint32_t);
    std::memmove(
      octree.buffer.data + stream.position,
      octree.buffer.data + stream.position + sizeof(uint32_t),
      (element_count - element_index) * sizeof(uint32_t)
    );
  }

  std::vector<uint32_t> find(const octree& octree, const std::function<int32_t(const aabb3& bounds)>& test)
  {
    auto results = std::vector<uint32_t>();
    find(octree, test, octree.divisions, octree.bounds, 0, results);

    return results;
  }

  void find(const octree& octree, const std::function<int32_t(const aabb3& bounds)>& test, uint32_t divisions, const aabb3& bounds, uint32_t cumulative_index, std::vector<uint32_t>& results)
  {
    auto test_result = test(bounds);
    if (test_result == -1)
    {
      return;
    }

    if (divisions == 0)
    {
      auto elements = cell_elements(octree, cumulative_index);
      results.insert(results.end(), elements.begin(), elements.end());

      return;
    }

    auto octant_bounds = ludo::octant_bounds(bounds);
    for (auto octant_index = uint32_t(0); octant_index < octant_bounds.size(); octant_index++)
    {
      find(octree, test, divisions - 1, octant_bounds[octant_index], cumulative_index * 8 + octant_index, results);
    }
  }

  vec3 cell_dimensions(const octree& octree)
  {
    auto cell_count_1d = uint32_t(std::pow(2, octree.divisions));
    auto bounds_size = octree.bounds.max - octree.bounds.min;
    return bounds_size / static_cast<float>(cell_count_1d);
  }

  uint32_t cell_element_index(const octree& octree, uint32_t cell_index, uint32_t element)
  {
    auto offset = cell_offset(octree, cell_index);
    auto stream = ludo::stream(octree.buffer, offset);

    auto element_count = read<uint32_t>(stream);
    for (auto element_index = uint32_t(0); element_index < element_count; element_index++)
    {
      if (read<uint32_t>(stream) == element)
      {
        return element_index;
      }
    }

    return octree.cell_capacity;
  }

  std::vector<uint32_t> cell_elements(const octree& octree, uint32_t cell_index)
  {
    auto elements = std::vector<uint32_t>();
    auto offset = cell_offset(octree, cell_index);
    auto stream = ludo::stream(octree.buffer, offset);

    auto element_count = read<uint32_t>(stream);
    for (auto element_index = uint32_t(0); element_index < element_count; element_index++)
    {
      elements.push_back(read<uint32_t>(stream));
    }

    return elements;
  }

  uint64_t cell_offset(const octree& octree, uint32_t cell_index)
  {
    return cell_index * (sizeof(uint32_t) + octree.cell_capacity * sizeof(uint32_t));
  }

  std::array<aabb3, 8> octant_bounds(const aabb3& bounds)
  {
    auto center = (bounds.min + bounds.max) / 2.0f;

    auto offset_x = center[0] - bounds.min[0];
    auto offset_y = center[1] - bounds.min[1];
    auto offset_z = center[2] - bounds.min[2];
    auto offsets = std::array<vec3, 8>
    {{
      { 0.0f, 0.0f, 0.0f },
      { offset_x, 0.0f, 0.0f },
      { 0.0f, offset_y, 0.0f },
      { offset_x, offset_y, 0.0f },
      { 0.0f, 0.0f, offset_z },
      { offset_x, 0.0f, offset_z },
      { 0.0f, offset_y, offset_z },
      { offset_x, offset_y, offset_z }
    }};

    return
    {{
      { .min = bounds.min + offsets[0], .max = center + offsets[0] },
      { .min = bounds.min + offsets[1], .max = center + offsets[1] },
      { .min = bounds.min + offsets[2], .max = center + offsets[2] },
      { .min = bounds.min + offsets[3], .max = center + offsets[3] },
      { .min = bounds.min + offsets[4], .max = center + offsets[4] },
      { .min = bounds.min + offsets[5], .max = center + offsets[5] },
      { .min = bounds.min + offsets[6], .max = center + offsets[6] },
      { .min = bounds.min + offsets[7], .max = center + offsets[7] }
    }};
  }

  uint32_t to_index(const octree& octree, const std::array<uint32_t, 3>& cell_coordinates)
  {
    auto cell_count_1d = uint32_t(std::pow(2, octree.divisions));
    return cell_coordinates[0] * cell_count_1d * cell_count_1d + cell_coordinates[1] * cell_count_1d + cell_coordinates[2];
  }

  std::array<uint32_t, 3> to_cell_coordinates(const octree& octree, const vec3& position)
  {
    assert(position >= octree.bounds.min && position <= octree.bounds.max && "position out of bounds");

    auto cell_dimensions = ludo::cell_dimensions(octree);

    auto cell_coordinates = (position - octree.bounds.min) / cell_dimensions;
    cell_coordinates[0] = std::floor(cell_coordinates[0]);
    cell_coordinates[1] = std::floor(cell_coordinates[1]);
    cell_coordinates[2] = std::floor(cell_coordinates[2]);

    return { static_cast<uint32_t>(cell_coordinates[0]), static_cast<uint32_t>(cell_coordinates[1]), static_cast<uint32_t>(cell_coordinates[2]) };
  }
}
