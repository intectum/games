/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cmath>

#include "grid3.h"

namespace ludo
{
  vec3 cell_dimensions(const grid3& grid);
  std::vector<uint64_t> cell_render_mesh_ids(const grid3& grid, uint32_t cell_index);
  uint32_t cell_render_mesh_index(const grid3& grid, uint32_t cell_index, uint64_t render_mesh_id);
  uint64_t cell_offset(const grid3& grid, uint32_t cell_index);
  uint32_t to_index(const grid3& grid, const std::array<uint32_t, 3>& cell_coordinates);
  std::array<uint32_t, 3> to_cell_coordinates(const grid3& grid, const vec3& position);

  // 3 * vec3, padded to 16 bytes
  const auto front_buffer_header_size = sizeof(vec3) + 4 + sizeof(vec3) + 4 + sizeof(vec3) + 4;

  // The render mesh count, padded to 8 bytes
  const auto cell_header_size = sizeof(uint32_t) + 4;

  // 2 IDs and 6 start/count values
  const auto render_mesh_size = 2 * sizeof(uint64_t) + 6 * sizeof(uint32_t);

  void init(grid3& grid)
  {
    grid.id = next_id++;

    auto cell_count = static_cast<uint32_t>(std::pow(grid.cell_count_1d, 3));
    auto cell_size = cell_header_size + grid.cell_capacity * render_mesh_size;

    auto data_size = cell_count * cell_size;
    grid.buffer.front = allocate_vram(front_buffer_header_size + data_size);
    grid.buffer.back = allocate(data_size);

    auto offset = uint32_t(0);
    for (auto cell_index = uint32_t(0); cell_index < cell_count; cell_index++)
    {
      cast<uint32_t>(grid.buffer.back, offset) = 0;
      offset += cell_size;
    }
  }

  void de_init(grid3& grid)
  {
    grid.id = 0;

    deallocate_dual(grid.buffer);
  }

  void commit(grid3& grid)
  {
    commit_header(grid);

    std::memcpy(grid.buffer.front.data + front_buffer_header_size, grid.buffer.back.data, grid.buffer.back.size);
  }

  void commit_header(grid3& grid)
  {
    auto stream = ludo::stream(grid.buffer.front);
    write(stream, grid.bounds.min);
    stream.position += 4; // align 16
    write(stream, grid.bounds.max);
    stream.position += 4; // align 16
    write(stream, ludo::cell_dimensions(grid));
  }

  void add(grid3& grid, const render_mesh& render_mesh, const vec3& position)
  {
    auto index = to_index(grid, to_cell_coordinates(grid, position));
    auto offset = cell_offset(grid, index);
    auto stream = ludo::stream(grid.buffer.back, offset);

    auto render_mesh_count = peek<uint32_t>(stream);

    assert(render_mesh_count < grid.cell_capacity && "cell is full");

    write(stream, render_mesh_count + 1);
    stream.position += 4; // align 8
    stream.position += render_mesh_count * render_mesh_size;
    write(stream, render_mesh.id);
    write(stream, render_mesh.render_program_id);
    write(stream, render_mesh.instances.start);
    write(stream, render_mesh.instances.count);
    write(stream, render_mesh.indices.start);
    write(stream, render_mesh.indices.count);
    write(stream, render_mesh.vertices.start);
    write(stream, render_mesh.vertices.count);
  }

  void remove(grid3& grid, const render_mesh& render_mesh, const vec3& position)
  {
    auto cell_coordinates = to_cell_coordinates(grid, position);
    auto cell_index = to_index(grid, cell_coordinates);
    auto render_mesh_index = cell_render_mesh_index(grid, cell_index, render_mesh.id);

    if (render_mesh_index == grid.cell_capacity)
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

          for (auto offset_z : offsets)
          {
            if ((cell_coordinates[2] == 0 && offset_z == -1) || cell_coordinates[2] == grid.cell_count_1d - 1 && offset_z == 1)
            {
              continue;
            }

            auto adjacent_cell_coordinates = std::array<uint32_t, 3> { cell_coordinates[0] + offset_x, cell_coordinates[1] + offset_y, cell_coordinates[2] + offset_z };
            if (adjacent_cell_coordinates == cell_coordinates)
            {
              continue;
            }

            cell_index = to_index(grid, adjacent_cell_coordinates);
            render_mesh_index = cell_render_mesh_index(grid, cell_index, render_mesh.id);
            if (render_mesh_index < grid.cell_capacity)
            {
              break;
            }
          }

          if (render_mesh_index < grid.cell_capacity)
          {
            break;
          }
        }

        if (render_mesh_index < grid.cell_capacity)
        {
          break;
        }
      }
    }

    assert(render_mesh_index < grid.cell_capacity && "render mesh not found");

    auto offset = cell_offset(grid, cell_index);
    auto stream = ludo::stream(grid.buffer.back, offset);

    auto render_mesh_count = peek<uint32_t>(stream) - 1;
    write(stream, render_mesh_count);
    stream.position += 4; // align 8
    stream.position += render_mesh_index * render_mesh_size;
    std::memmove(
      grid.buffer.back.data + stream.position,
      grid.buffer.back.data + stream.position + render_mesh_size,
      (render_mesh_count - render_mesh_index) * render_mesh_size
    );
  }

  std::vector<uint64_t> find(const grid3& grid, const std::function<int32_t(const aabb3& bounds)>& test)
  {
    auto render_mesh_ids = std::vector<uint64_t>();
    auto cell_count = static_cast<uint32_t>(std::pow(grid.cell_count_1d, 3));
    auto cell_dimensions = ludo::cell_dimensions(grid);

    for (auto index = uint32_t(0); index < cell_count; index++)
    {
      auto x = index / (grid.cell_count_1d * grid.cell_count_1d);
      auto y = (index / grid.cell_count_1d) % grid.cell_count_1d;
      auto z = index % grid.cell_count_1d;

      auto xyz = vec3 { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
      auto min = grid.bounds.min + xyz * cell_dimensions;

      auto bounds = aabb3
      {
        .min = min,
        .max = min + cell_dimensions
      };

      if (test(bounds) != -1)
      {
        auto ids = cell_render_mesh_ids(grid, index);
        render_mesh_ids.insert(render_mesh_ids.end(), ids.begin(), ids.end());
      }
    }

    return render_mesh_ids;
  }

  vec3 cell_dimensions(const grid3& grid)
  {
    auto bounds_size = grid.bounds.max - grid.bounds.min;
    return bounds_size / static_cast<float>(grid.cell_count_1d);
  }

  std::vector<uint64_t> cell_render_mesh_ids(const grid3& grid, uint32_t cell_index)
  {
    auto cell_render_mesh_ids = std::vector<uint64_t>();
    auto offset = cell_offset(grid, cell_index);

    auto render_mesh_count = cast<uint32_t>(grid.buffer.back, offset);
    offset += 4;
    offset += 4; // align 8

    for (auto render_mesh_index = uint32_t(0); render_mesh_index < render_mesh_count; render_mesh_index++)
    {
      cell_render_mesh_ids.push_back(cast<uint64_t>(grid.buffer.back, offset));
      offset += render_mesh_size;
    }

    return cell_render_mesh_ids;
  }

  uint32_t cell_render_mesh_index(const grid3& grid, uint32_t cell_index, uint64_t render_mesh_id)
  {
    auto offset = cell_offset(grid, cell_index);

    auto render_mesh_count = cast<uint32_t>(grid.buffer.back, offset);
    offset += 4;
    offset += 4; // align 8

    for (auto render_mesh_index = uint32_t(0); render_mesh_index < render_mesh_count; render_mesh_index++)
    {
      if (cast<uint64_t>(grid.buffer.back, offset) == render_mesh_id)
      {
        return render_mesh_index;
      }

      offset += render_mesh_size;
    }

    return grid.cell_capacity;
  }

  uint64_t cell_offset(const grid3& grid, uint32_t cell_index)
  {
    return cell_index * (cell_header_size + grid.cell_capacity * render_mesh_size);
  }

  uint32_t to_index(const grid3& grid, const std::array<uint32_t, 3>& cell_coordinates)
  {
    return cell_coordinates[0] * grid.cell_count_1d * grid.cell_count_1d + cell_coordinates[1] * grid.cell_count_1d + cell_coordinates[2];
  }

  std::array<uint32_t, 3> to_cell_coordinates(const grid3& grid, const vec3& position)
  {
    assert(position >= grid.bounds.min && position <= grid.bounds.max && "position out of bounds");

    auto cell_dimensions = ludo::cell_dimensions(grid);

    auto cell_coordinates = (position - grid.bounds.min) / cell_dimensions;
    cell_coordinates[0] = std::floor(cell_coordinates[0]);
    cell_coordinates[1] = std::floor(cell_coordinates[1]);
    cell_coordinates[2] = std::floor(cell_coordinates[2]);

    return { static_cast<uint32_t>(cell_coordinates[0]), static_cast<uint32_t>(cell_coordinates[1]), static_cast<uint32_t>(cell_coordinates[2]) };
  }
}
