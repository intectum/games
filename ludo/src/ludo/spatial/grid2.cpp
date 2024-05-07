/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "../tasks.h"
#include "grid2.h"

namespace ludo
{
  vec2 cell_dimensions(const grid2& grid);
  std::vector<uint64_t> cell_mesh_instance_ids(const grid2& grid, uint32_t cell_index);
  uint32_t cell_mesh_instance_index(const grid2& grid, uint32_t cell_index, uint64_t mesh_instance_id);
  uint64_t cell_offset(const grid2& grid, uint32_t cell_index);
  uint32_t to_index(const grid2& grid, const std::array<uint32_t, 2>& cell_coordinates);
  std::array<uint32_t, 2> to_cell_coordinates(const grid2& grid, const vec2& position);

  // 3 * vec2
  const auto front_buffer_header_size = 3 * sizeof(vec2);

  // The mesh instance count, padded to 8 bytes
  const auto cell_header_size = sizeof(uint32_t) + 4;

  // 2 IDs and 6 start/count values
  const auto mesh_instance_size = 2 * sizeof(uint64_t) + 6 * sizeof(uint32_t);

  template<>
  grid2* add(instance& instance, const grid2& init, const std::string& partition)
  {
    auto grid = add(data<ludo::grid2>(instance), init, partition);
    grid->id = next_id++;

    auto cell_count = static_cast<uint32_t>(std::pow(grid->cell_count_1d, 2));
    auto cell_size = cell_header_size + grid->cell_capacity * mesh_instance_size;

    auto data_size = cell_count * cell_size;
    grid->buffer.front = allocate_vram(front_buffer_header_size + data_size);
    grid->buffer.back = allocate(data_size);

    auto offset = uint32_t(0);
    for (auto cell_index = uint32_t(0); cell_index < cell_count; cell_index++)
    {
      cast<uint32_t>(grid->buffer.back, offset) = 0;
      offset += cell_size;
    }

    return grid;
  }

  template<>
  void remove<grid2>(instance& instance, grid2* element, const std::string& partition)
  {
    deallocate_dual(element->buffer);

    remove(data<grid2>(instance), element, partition);
  }

  void push(grid2& grid, bool header_only)
  {
    auto stream = ludo::stream(grid.buffer.front);
    write(stream, grid.bounds.min);
    write(stream, grid.bounds.max);
    write(stream, ludo::cell_dimensions(grid));

    if (!header_only)
    {
      std::memcpy(grid.buffer.front.data + stream.position, grid.buffer.back.data, grid.buffer.back.size);
    }
  }

  void add(grid2& grid, const mesh_instance& mesh_instance, const vec2& position, bool push_cell)
  {
    auto index = to_index(grid, to_cell_coordinates(grid, position));
    auto offset = cell_offset(grid, index);
    auto stream = ludo::stream(grid.buffer.back, offset);

    auto mesh_instance_count = peek<uint32_t>(stream);

    assert(mesh_instance_count < grid.cell_capacity && "cell is full");

    write(stream, mesh_instance_count + 1);
    stream.position += 4; // align 8
    stream.position += mesh_instance_count * mesh_instance_size;
    write(stream, mesh_instance.id);
    write(stream, mesh_instance.render_program_id);
    write(stream, mesh_instance.instances.start);
    write(stream, mesh_instance.instances.count);
    write(stream, mesh_instance.indices.start);
    write(stream, mesh_instance.indices.count);
    write(stream, mesh_instance.vertices.start);
    write(stream, mesh_instance.vertices.count);

    if (push_cell)
    {
      std::memcpy(
        grid.buffer.front.data + front_buffer_header_size + offset,
        grid.buffer.back.data + offset,
        cell_header_size + grid.cell_capacity * mesh_instance_size
      );
    }
  }

  void remove(grid2& grid, const mesh_instance& mesh_instance, const vec2& position, bool push_cell)
  {
    auto cell_coordinates = to_cell_coordinates(grid, position);
    auto cell_index = to_index(grid, cell_coordinates);
    auto mesh_instance_index = cell_mesh_instance_index(grid, cell_index, mesh_instance.id);

    if (mesh_instance_index == grid.cell_capacity)
    {
      // Search adjacent cells in case of floating point precision errors
      auto offsets = std::array<int32_t, 3> { -1, 0, 1 };
      for (auto offset_x : offsets)
      {
        if ((cell_coordinates[0] == 0 && offset_x == -1) || cell_coordinates[0] == grid.cell_count_1d - 1 && offset_x == 1)
        {
          continue;
        }

        for (auto offset_y : offsets)
        {
          if ((cell_coordinates[1] == 0 && offset_y == -1) || cell_coordinates[1] == grid.cell_count_1d - 1 && offset_y == 1)
          {
            continue;
          }

          auto adjacent_cell_coordinates = std::array<uint32_t, 2> { cell_coordinates[0] + offset_x, cell_coordinates[1] + offset_y };
          if (adjacent_cell_coordinates == cell_coordinates)
          {
            continue;
          }

          cell_index = to_index(grid, adjacent_cell_coordinates);
          mesh_instance_index = cell_mesh_instance_index(grid, cell_index, mesh_instance.id);
          if (mesh_instance_index < grid.cell_capacity)
          {
            break;
          }
        }

        if (mesh_instance_index < grid.cell_capacity)
        {
          break;
        }
      }
    }

    assert(mesh_instance_index < grid.cell_capacity && "mesh instance not found");

    auto offset = cell_offset(grid, cell_index);
    auto stream = ludo::stream(grid.buffer.back, offset);

    auto mesh_instance_count = peek<uint32_t>(stream) - 1;
    write(stream, mesh_instance_count);
    stream.position += 4; // align 8
    stream.position += mesh_instance_index * mesh_instance_size;
    std::memmove(
      grid.buffer.back.data + stream.position,
      grid.buffer.back.data + stream.position + mesh_instance_size,
      (mesh_instance_count - mesh_instance_index) * mesh_instance_size
    );

    if (push_cell)
    {
      std::memcpy(
        grid.buffer.front.data + front_buffer_header_size + offset,
        grid.buffer.back.data + offset,
        cell_header_size + grid.cell_capacity * mesh_instance_size
      );
    }
  }

  std::vector<uint64_t> find_parallel(const grid2& grid, const std::function<int32_t(const aabb2& bounds)>& test)
  {
    auto mesh_instance_ids = std::vector<uint64_t>();
    auto cell_count = static_cast<uint32_t>(std::pow(grid.cell_count_1d, 2));
    auto cell_dimensions = ludo::cell_dimensions(grid);

    divide_and_conquer(cell_count, [&grid, &test, &mesh_instance_ids, cell_dimensions](uint32_t start, uint32_t end)
    {
      auto task_mesh_instance_ids = std::vector<uint64_t>();

      for (auto index = start; index < end; index++)
      {
        auto x = index / (grid.cell_count_1d * grid.cell_count_1d);
        auto y = (index / grid.cell_count_1d) % grid.cell_count_1d;
        auto z = index % grid.cell_count_1d;

        auto xy = vec2 { static_cast<float>(x), static_cast<float>(y) };
        auto min = grid.bounds.min + xy * cell_dimensions;

        auto bounds = aabb2
        {
          .min = min,
          .max = min + cell_dimensions
        };

        if (test(bounds) != -1)
        {
          auto ids = cell_mesh_instance_ids(grid, index);
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

  vec2 cell_dimensions(const grid2& grid)
  {
    auto bounds_size = grid.bounds.max - grid.bounds.min;
    return bounds_size / static_cast<float>(grid.cell_count_1d);
  }

  std::vector<uint64_t> cell_mesh_instance_ids(const grid2& grid, uint32_t cell_index)
  {
    auto cell_mesh_instance_ids = std::vector<uint64_t>();
    auto offset = cell_offset(grid, cell_index);

    auto mesh_instance_count = cast<uint32_t>(grid.buffer.back, offset);
    offset += 4;
    offset += 4; // align 8

    for (auto mesh_instance_index = uint32_t(0); mesh_instance_index < mesh_instance_count; mesh_instance_index++)
    {
      cell_mesh_instance_ids.push_back(cast<uint64_t>(grid.buffer.back, offset));
      offset += mesh_instance_size;
    }

    return cell_mesh_instance_ids;
  }

  uint32_t cell_mesh_instance_index(const grid2& grid, uint32_t cell_index, uint64_t mesh_instance_id)
  {
    auto offset = cell_offset(grid, cell_index);

    auto mesh_instance_count = cast<uint32_t>(grid.buffer.back, offset);
    offset += 4;
    offset += 4; // align 8

    for (auto mesh_instance_index = uint32_t(0); mesh_instance_index < mesh_instance_count; mesh_instance_index++)
    {
      if (cast<uint64_t>(grid.buffer.back, offset) == mesh_instance_id)
      {
        return mesh_instance_index;
      }

      offset += mesh_instance_size;
    }

    return grid.cell_capacity;
  }

  uint64_t cell_offset(const grid2& grid, uint32_t cell_index)
  {
    return cell_index * (cell_header_size + grid.cell_capacity * mesh_instance_size);
  }

  uint32_t to_index(const grid2& grid, const std::array<uint32_t, 2>& cell_coordinates)
  {
    return cell_coordinates[0] * grid.cell_count_1d + cell_coordinates[1];
  }

  std::array<uint32_t, 2> to_cell_coordinates(const grid2& grid, const vec2& position)
  {
    assert(position >= grid.bounds.min && position <= grid.bounds.max && "position out of bounds");

    auto cell_dimensions = ludo::cell_dimensions(grid);

    auto cell_coordinates = (position - grid.bounds.min) / cell_dimensions;
    cell_coordinates[0] = std::floor(cell_coordinates[0]);
    cell_coordinates[1] = std::floor(cell_coordinates[1]);

    return { static_cast<uint32_t>(cell_coordinates[0]), static_cast<uint32_t>(cell_coordinates[1]) };
  }
}
