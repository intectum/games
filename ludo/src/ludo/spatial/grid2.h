/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_SPATIAL_GRID2_H
#define LUDO_SPATIAL_GRID2_H

#include "../compute.h"
#include "bounds.h"

namespace ludo
{
  ///
  /// A 2D grid with uniformly sized cubic cells.
  /// This allows for parallel search algorithms.
  struct LUDO_API grid2
  {
    uint64_t id = 0; ///< The ID of the grid.
    uint64_t compute_program_id = 0; ///< The ID of the compute program used to build draw commands.

    aabb2 bounds; ///< The outer bounds.
    uint8_t cell_count_1d = 1; ///< The number of cells in each dimension.
    uint32_t cell_capacity = 16; ///< The maximum number of mesh instances that can be added to a cell.

    double_buffer buffer;
  };

  template<>
  LUDO_API grid2* add(instance& instance, const grid2& init, const std::string& partition);

  template<>
  LUDO_API void remove<grid2>(instance& instance, grid2* element, const std::string& partition);

  ///
  /// Pushes the state of a grid to the front buffer.
  /// \param grid The grid to push.
  /// \param header_only Determines if only the header state should be pushed (i.e. not the cell data).
  LUDO_API void push(grid2& grid, bool header_only = false);

  ///
  /// Adds a mesh instance to a grid.
  /// \param grid The grid to add the mesh instance to.
  /// \param mesh_instance The mesh instance to add.
  /// \param position The position of the mesh instance.
  /// \param push_cell Determines if the modified cell state should be pushed to the front buffer.
  LUDO_API void add(grid2& grid, const mesh_instance& mesh_instance, const vec2& position, bool push_cell = false);

  ///
  /// Removes a mesh instance from a grid.
  /// \param grid The grid to remove the mesh instance from.
  /// \param mesh_instance The mesh instance to remove.
  /// \param position The position of the mesh instance.
  /// \param push_cell Determines if the modified cell state should be pushed to the front buffer.
  LUDO_API void remove(grid2& grid, const mesh_instance& mesh_instance, const vec2& position, bool push_cell = false);

  ///
  /// Finds mesh instances within a grid using a parallel search across all cells.
  /// \param grid The grid to search.
  /// \param test The test to perform against the bounds of the cells.
  /// \return The matching mesh instance IDs.
  LUDO_API std::vector<uint64_t> find_parallel(const grid2& grid, const std::function<int32_t(const aabb2& bounds)>& test);
}

#endif // LUDO_SPATIAL_GRID2_H
