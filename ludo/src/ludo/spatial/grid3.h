/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_SPATIAL_GRID3_H
#define LUDO_SPATIAL_GRID3_H

#include "../compute.h"
#include "../rendering.h"
#include "bounds.h"

namespace ludo
{
  ///
  /// A grid with uniformly-sized cubic cells.
  struct grid3
  {
    uint64_t id = 0; ///< A unique identifier.
    uint64_t compute_program_id = 0; ///< The compute program used to add render commands.

    aabb3 bounds; ///< The outer bounds.
    uint8_t cell_count_1d = 1; ///< The number of cells in each dimension.
    uint32_t cell_capacity = 16; ///< The maximum number of render meshes that can be added to a cell.

    double_buffer buffer; ///< The cell data (the front buffer also contains a header).
  };

  ///
  /// Initializes a grid.
  /// \param grid The grid.
  void init(grid3& grid);

  ///
  /// De-initializes a grid.
  /// \param grid The grid.
  void de_init(grid3& grid);

  ///
  /// Commits to the front buffer.
  /// \param grid The grid.
  /// \param header_only Determines if only the header state should be pushed (i.e. not the cell data).
  void commit(grid3& grid);

  ///
  /// Commits the header state to the front buffer (i.e. not the cell data).
  /// \param grid The grid.
  void commit_header(grid3& grid);

  ///
  /// Adds a render mesh to a grid.
  /// \param grid The grid to add the render mesh to.
  /// \param render_mesh The render mesh to add.
  /// \param position The position of the render mesh.
  void add(grid3& grid3, const render_mesh& render_mesh, const vec3& position);

  ///
  /// Removes a render mesh from a grid.
  /// \param grid The grid to remove the render mesh from.
  /// \param render_mesh The render mesh to remove.
  /// \param position The position of the render mesh.
  void remove(grid3& grid3, const render_mesh& render_mesh, const vec3& position);

  ///
  /// Finds render meshes within a grid using a parallel search across all cells.
  /// \param grid The grid to search.
  /// \param test The test to perform against the bounds of the cells.
  /// \return The matching render mesh IDs.
  std::vector<uint64_t> find_parallel(const grid3& grid, const std::function<int32_t(const aabb3& bounds)>& test);

  ///
  /// Builds a compute program used to build render commands from a grid.
  /// \param grid The grid.
  /// \return The compute program.
  compute_program build_compute_program(const grid3& grid);

  ///
  /// Adds render commands to the render programs' command buffers and updates the active command count.
  /// \param grids The render program.
  /// \param compute_programs The compute programs to execute.
  /// \param render_programs The render programs that can have render commands added.
  /// \param render_commands The render commands to sample from.
  /// \param camera The camera the render meshes are being viewed through.
  void add_render_commands(array<grid3>& grids, array<compute_program>& compute_programs, array<render_program>& render_programs, const heap& render_commands, const camera& camera);
}

#endif // LUDO_SPATIAL_GRID3_H
