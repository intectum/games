/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "../data/buffers.h"
#include "bounds.h"

namespace ludo
{
  ///
  /// A linear octree.
  struct octree
  {
    uint64_t id = 0; ///< A unique identifier.

    aabb3 bounds; ///< The outer bounds.
    uint32_t divisions = 1; ///< The number of divisions (layers).
    uint32_t cell_capacity = 16; ///< The maximum number of elements that can be added to a cell.

    ludo::buffer buffer; ///< The cell data.
  };

  ///
  /// Initializes an octree.
  /// \param octree The octree.
  void init(octree& octree);

  ///
  /// De-initializes an octree.
  /// \param octree The octree.
  void de_init(octree& octree);

  ///
  /// Adds an element to an octree.
  /// \param octree The octree to add the element to.
  /// \param element The element to add to the octree.
  /// \param position The position of the element.
  void add(octree& octree, uint32_t element, const ludo::vec3& position);

  ///
  /// Removes an element from an octree.
  /// \param octree The octree to remove the element from.
  /// \param element The element to remove from the octree.
  /// \param position The position of the element.
  void remove(octree& octree, uint32_t element, const ludo::vec3& position);

  ///
  /// Finds elements within an octree.
  /// \param octree The octree to search.
  /// \param test The test to perform against the bounds of the nodes.
  /// \return The matching elements.
  std::vector<uint32_t> find(const octree& octree, const std::function<int32_t(const aabb3& bounds)>& test);
}
