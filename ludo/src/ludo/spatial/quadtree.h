/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_SPATIAL_QUADTREE_H
#define LUDO_SPATIAL_QUADTREE_H

#include "../data/buffers.h"
#include "bounds.h"

namespace ludo
{
  ///
  /// A linear quadtree.
  struct LUDO_API quadtree
  {
    uint64_t id = 0; ///< The ID of the quadtree.

    aabb2 bounds; ///< The outer bounds.
    uint32_t divisions = 1; ///< The number of divisions (layers) in the quadtree.
    uint32_t cell_capacity = 16; ///< The maximum number of elements that can be added to a cell.

    ludo::buffer buffer;
  };

  template<>
  LUDO_API quadtree* add(instance& instance, const quadtree& init, const std::string& partition);

  template<>
  LUDO_API void remove<quadtree>(instance& instance, quadtree* element, const std::string& partition);

  ///
  /// Adds an element to an quadtree.
  /// \param quadtree The quadtree to add the element to.
  /// \param element The element to add to the quadtree.
  /// \param position The position of the element.
  LUDO_API void add(quadtree& quadtree, uint32_t element, const ludo::vec2& position);

  ///
  /// Removes an element from an quadtree.
  /// \param quadtree The quadtree to remove the element from.
  /// \param element The element to remove from the quadtree.
  /// \param position The position of the element.
  LUDO_API void remove(quadtree& quadtree, uint32_t element, const ludo::vec2& position);

  ///
  /// Finds elements within an quadtree.
  /// \param quadtree The quadtree to search.
  /// \param test The test to perform against the bounds of the nodes.
  /// \return The matching elements.
  LUDO_API std::vector<uint32_t> find(const quadtree& quadtree, const std::function<int32_t(const aabb2& bounds)>& test);
}

#endif // LUDO_SPATIAL_QUADTREE_H
