/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <ludo/api.h>

namespace astrum
{
  ///
  /// An pointer-based icotree that dynamically adds and removes nodes as needed.
  struct icotree
  {
    uint64_t id = 0; ///< The ID of the icotree.

    uint32_t divisions = 1; ///< The number of divisions (layers) in the icotree.
    uint32_t cell_capacity = 16; ///< The maximum number of render meshes that can be added to a cell.
    uint32_t max_populated_cells = 1;

    ludo::buffer buffer;
  };

  void init(icotree& icotree);

  void destroy(icotree& icotree);

  ///
  /// Adds an element to an icotree.
  /// \param icotree The icotree to add the element to.
  /// \param element The element to add to the icotree.
  /// \param position The position of the element.
  void add(icotree& icotree, const ludo::vec3& element, const ludo::vec3& position);

  ///
  /// Removes an element from an icotree.
  /// \param icotree The icotree to remove the element from.
  /// \param element The element to remove from the icotree.
  /// \param position The position of the element.
  void remove(icotree& icotree, const ludo::vec3& element, const ludo::vec3& position);

  ///
  /// Finds a cell within an icotree.
  /// \param icotree The icotree to search.
  /// \param test The test to perform against the faces.
  /// \return The matching cell index, or std::numeric_limits<uint32_t>::max() if a cell was not found.
  uint32_t find_cell(const icotree& icotree, const std::function<int32_t(const std::array<ludo::vec3, 3>& face)>& test);

  ///
  /// Finds cells within an icotree.
  /// \param icotree The icotree to search.
  /// \param test The test to perform against the faces.
  /// \return The matching cell indices.
  std::vector<uint32_t> find_cells(const icotree& icotree, const std::function<int32_t(const std::array<ludo::vec3, 3>& face)>& test);

  uint32_t cell_count(const icotree& icotree);

  ludo::buffer cell_element_data(const icotree& icotree, uint32_t cell_index);
}
