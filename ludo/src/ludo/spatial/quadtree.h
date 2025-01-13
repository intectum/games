/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <functional>

#include "bounds.h"

namespace ludo
{
  ///
  /// A linear quadtree.
  struct quadtree
  {
    aabb2 bounds; ///< The outer bounds.
    uint32_t divisions = 1; ///< The number of divisions (layers).
    uint32_t cell_capacity = 16; ///< The maximum number of elements that can be added to a cell.

    ludo::arena arena; ///< The cell data.
  };

  ///
  /// Initializes a quadtree.
  /// \param quadtree The quadtree.
  void init(quadtree& quadtree);

  ///
  /// De-initializes a quadtree.
  /// \param quadtree The quadtree.
  void de_init(quadtree& quadtree);

  ///
  /// Adds an element to a quadtree.
  /// \param quadtree The quadtree to add the element to.
  /// \param element The element to add to the quadtree.
  /// \param position The position of the element.
  void add(quadtree& quadtree, uint32_t element, const vec2& position);

  ///
  /// Removes an element from a quadtree.
  /// \param quadtree The quadtree to remove the element from.
  /// \param element The element to remove from the quadtree.
  /// \param position The position of the element.
  void remove(quadtree& quadtree, uint32_t element, const vec2& position);

  ///
  /// Finds elements within a quadtree.
  /// \param quadtree The quadtree to search.
  /// \param test The test to perform against the bounds of the nodes.
  /// \return The matching elements.
  std::vector<uint32_t> find(const quadtree& quadtree, const std::function<int32_t(const aabb2& bounds)>& test);
}
