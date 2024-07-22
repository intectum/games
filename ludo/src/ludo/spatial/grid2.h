/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_SPATIAL_GRID2_H
#define LUDO_SPATIAL_GRID2_H

#include "../compute.h"
#include "../rendering.h"
#include "bounds.h"

namespace ludo
{
  ///
  /// A 2D grid with uniformly-sized square cells.
  struct grid2
  {
    uint64_t id = 0; ///< A unique identifier.

    aabb2 bounds; ///< The outer bounds.
    uint8_t cell_count_1d = 1; ///< The number of cells in each dimension.
    uint32_t cell_capacity = 16; ///< The maximum number of render meshes that can be added to a cell.

    double_buffer buffer; ///< The cell data (the front buffer also contains a header).
  };

  ///
  /// Initializes a grid.
  /// \param grid The grid.
  void init(grid2& grid);

  ///
  /// De-initializes a grid.
  /// \param grid The grid.
  void de_init(grid2& grid);

  ///
  /// Commits to the front buffer.
  /// \param grid The grid.
  /// \param header_only Determines if only the header state should be pushed (i.e. not the cell data).
  void commit(grid2& grid);

  ///
  /// Commits the header state to the front buffer (i.e. not the cell data).
  /// \param grid The grid.
  void commit_header(grid2& grid);

  ///
  /// Adds a render mesh to a grid.
  /// \param grid The grid to add the render mesh to.
  /// \param render_mesh The render mesh to add.
  /// \param position The position of the render mesh.
  void add(grid2& grid, const render_mesh& render_mesh, const vec2& position);

  ///
  /// Removes a render mesh from a grid.
  /// \param grid The grid to remove the render mesh from.
  /// \param render_mesh The render mesh to remove.
  /// \param position The position of the render mesh.
  void remove(grid2& grid, const render_mesh& render_mesh, const vec2& position);

  ///
  /// Finds render meshes within a grid using a parallel search across all cells.
  /// \param grid The grid to search.
  /// \param test The test to perform against the bounds of the cells.
  /// \return The matching render mesh IDs.
  std::vector<uint64_t> find_parallel(const grid2& grid, const std::function<int32_t(const aabb2& bounds)>& test);
}

#endif // LUDO_SPATIAL_GRID2_H
