/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "quadtree.h"

namespace ludo
{
  void find(const quadtree& quadtree, const std::function<int32_t(const aabb2& bounds)>& test, uint32_t divisions, const aabb2& bounds, uint32_t cumulative_index, std::vector<uint32_t>& results);
  vec2 cell_dimensions(const quadtree& quadtree);
  uint32_t cell_element_index(const quadtree& quadtree, uint32_t cell_index, uint32_t element);
  std::vector<uint32_t> cell_elements(const quadtree& quadtree, uint32_t cell_index);
  uint64_t cell_offset(const quadtree& quadtree, uint32_t cell_index);
  std::array<aabb2, 4> quadrant_bounds(const aabb2& bounds);
  uint32_t to_index(const quadtree& quadtree, const std::array<uint32_t, 2>& cell_coordinates);
  std::array<uint32_t, 2> to_cell_coordinates(const quadtree& quadtree, const vec2& position);

  template<>
  quadtree* add(instance& instance, const quadtree& init, const std::string& partition)
  {
    auto quadtree = add(data<ludo::quadtree>(instance), init, partition);
    quadtree->id = next_id++;

    auto cell_count = static_cast<uint32_t>(std::pow(4, quadtree->divisions));
    auto cell_size = sizeof(uint32_t) + quadtree->cell_capacity * sizeof(uint32_t);

    auto data_size = cell_count * cell_size;
    quadtree->buffer = allocate(data_size);

    auto offset = uint32_t(0);
    for (auto cell_index = uint32_t(0); cell_index < cell_count; cell_index++)
    {
      cast<uint32_t>(quadtree->buffer, offset) = 0;
      offset += cell_size;
    }

    return quadtree;
  }

  template<>
  void remove<quadtree>(instance& instance, quadtree* element, const std::string& partition)
  {
    deallocate(element->buffer);

    remove(data<quadtree>(instance), element, partition);
  }

  void add(quadtree& quadtree, uint32_t element, const ludo::vec2& position)
  {
    auto index = to_index(quadtree, to_cell_coordinates(quadtree, position));
    auto offset = cell_offset(quadtree, index);
    auto stream = ludo::stream(quadtree.buffer, offset);

    auto element_count = peek<uint32_t>(stream);

    assert(element_count < quadtree.cell_capacity && "cell is full");

    write(stream, element_count + 1);
    stream.position += element_count * sizeof(uint32_t);
    write(stream, element);
  }

  void remove(quadtree& quadtree, uint32_t element, const ludo::vec2& position)
  {
    auto cell_coordinates = to_cell_coordinates(quadtree, position);
    auto cell_index = to_index(quadtree, cell_coordinates);
    auto element_index = cell_element_index(quadtree, cell_index, element);

    if (element_index == quadtree.cell_capacity)
    {
      // Search adjacent cells in case of floating point precision errors
      auto cell_count_1d = uint32_t(std::pow(2, quadtree.divisions));
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

          auto adjacent_cell_coordinates = std::array<uint32_t, 2> { cell_coordinates[0] + offset_x, cell_coordinates[1] + offset_y };
          if (adjacent_cell_coordinates == cell_coordinates)
          {
            continue;
          }

          cell_index = to_index(quadtree, adjacent_cell_coordinates);
          element_index = cell_element_index(quadtree, cell_index, element);
          if (element_index < quadtree.cell_capacity)
          {
            break;
          }
        }

        if (element_index < quadtree.cell_capacity)
        {
          break;
        }
      }
    }

    assert(element_index < quadtree.cell_capacity && "element not found");

    auto offset = cell_offset(quadtree, cell_index);
    auto stream = ludo::stream(quadtree.buffer, offset);

    auto element_count = peek<uint32_t>(stream) - 1;
    write(stream, element_count);
    stream.position += element_index * sizeof(uint32_t);
    std::memmove(
      quadtree.buffer.data + stream.position,
      quadtree.buffer.data + stream.position + sizeof(uint32_t),
      (element_count - element_index) * sizeof(uint32_t)
    );
  }

  std::vector<uint32_t> find(const quadtree& quadtree, const std::function<int32_t(const aabb2& bounds)>& test)
  {
    auto results = std::vector<uint32_t>();
    find(quadtree, test, quadtree.divisions, quadtree.bounds, 0, results);

    return results;
  }

  void find(const quadtree& quadtree, const std::function<int32_t(const aabb2& bounds)>& test, uint32_t divisions, const aabb2& bounds, uint32_t cumulative_index, std::vector<uint32_t>& results)
  {
    auto test_result = test(bounds);
    if (test_result == -1)
    {
      return;
    }

    if (divisions == 0)
    {
      auto elements = cell_elements(quadtree, cumulative_index);
      results.insert(results.end(), elements.begin(), elements.end());

      return;
    }

    auto quadrant_bounds = ludo::quadrant_bounds(bounds);
    for (auto quadrant_index = uint32_t(0); quadrant_index < quadrant_bounds.size(); quadrant_index++)
    {
      find(quadtree, test, divisions - 1, quadrant_bounds[quadrant_index], cumulative_index * 4 + quadrant_index, results);
    }
  }

  vec2 cell_dimensions(const quadtree& quadtree)
  {
    auto cell_count_1d = uint32_t(std::pow(2, quadtree.divisions));
    auto bounds_size = quadtree.bounds.max - quadtree.bounds.min;
    return bounds_size / static_cast<float>(cell_count_1d);
  }

  uint32_t cell_element_index(const quadtree& quadtree, uint32_t cell_index, uint32_t element)
  {
    auto offset = cell_offset(quadtree, cell_index);
    auto stream = ludo::stream(quadtree.buffer, offset);

    auto element_count = read<uint32_t>(stream);
    for (auto element_index = uint32_t(0); element_index < element_count; element_index++)
    {
      if (read<uint32_t>(stream) == element)
      {
        return element_index;
      }
    }

    return quadtree.cell_capacity;
  }

  std::vector<uint32_t> cell_elements(const quadtree& quadtree, uint32_t cell_index)
  {
    auto elements = std::vector<uint32_t>();
    auto offset = cell_offset(quadtree, cell_index);
    auto stream = ludo::stream(quadtree.buffer, offset);

    auto element_count = read<uint32_t>(stream);
    for (auto element_index = uint32_t(0); element_index < element_count; element_index++)
    {
      elements.push_back(read<uint32_t>(stream));
    }

    return elements;
  }

  uint64_t cell_offset(const quadtree& quadtree, uint32_t cell_index)
  {
    return cell_index * (sizeof(uint32_t) + quadtree.cell_capacity * sizeof(uint32_t));
  }

  std::array<aabb2, 4> quadrant_bounds(const aabb2& bounds)
  {
    auto center = (bounds.min + bounds.max) / 2.0f;

    auto offset_x = center[0] - bounds.min[0];
    auto offset_y = center[1] - bounds.min[1];
    auto offsets = std::array<vec2, 4>
    {{
      { 0.0f, 0.0f },
      { offset_x, 0.0f },
      { 0.0f, offset_y },
      { offset_x, offset_y },
    }};

    return
    {{
      { .min = bounds.min + offsets[0], .max = center + offsets[0] },
      { .min = bounds.min + offsets[1], .max = center + offsets[1] },
      { .min = bounds.min + offsets[2], .max = center + offsets[2] },
      { .min = bounds.min + offsets[3], .max = center + offsets[3] }
    }};
  }

  uint32_t to_index(const quadtree& quadtree, const std::array<uint32_t, 2>& cell_coordinates)
  {
    auto cell_count_1d = uint32_t(std::pow(2, quadtree.divisions));
    return cell_coordinates[0] * cell_count_1d + cell_coordinates[1];
  }

  std::array<uint32_t, 2> to_cell_coordinates(const quadtree& quadtree, const vec2& position)
  {
    assert(position >= quadtree.bounds.min && position <= quadtree.bounds.max && "position out of bounds");

    auto cell_dimensions = ludo::cell_dimensions(quadtree);

    auto cell_coordinates = (position - quadtree.bounds.min) / cell_dimensions;
    cell_coordinates[0] = std::floor(cell_coordinates[0]);
    cell_coordinates[1] = std::floor(cell_coordinates[1]);

    return { static_cast<uint32_t>(cell_coordinates[0]), static_cast<uint32_t>(cell_coordinates[1]) };
  }
}
