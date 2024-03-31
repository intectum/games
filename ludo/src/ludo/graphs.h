/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_GRAPHS_H
#define LUDO_GRAPHS_H

#include <cmath>
#include <functional>
#include <set>

#include "core.h"
#include "data.h"
#include "math/vec.h"
#include "meshes.h"

namespace ludo
{
  ///
  /// An axis-aligned bounding box.
  struct LUDO_API aabb
  {
    vec3 min = vec3_zero;
    vec3 max = vec3_zero;
  };

  ///
  /// An element of a dynamic octree.
  struct LUDO_API dynamic_octree_element
  {
    ludo::mesh mesh; ///< The mesh.
    aabb bounds; ///< The bounds of the mesh (in world coordinates). // TODO make this local to the mesh?
  };

  ///
  /// A node of a dynamic octree.
  struct LUDO_API dynamic_octree_node
  {
    uint32_t depth = 1; ///< The depth at which the node resides.
    aabb bounds; ///< The bounds of the node. // TODO remove these?

    dynamic_octree_node *parent = nullptr; ///< The parent of the node.
    std::vector<dynamic_octree_node> children; ///< The children of the node.

    std::vector<dynamic_octree_element> elements; ///< The elements contained in the node.
  };

  ///
  /// A hash implementation for dynamic octree 'element nodes'.
  struct dynamic_octree_element_node_hash
  {
    std::size_t operator()(const std::pair<uint64_t, uint32_t>& pair) const noexcept;
  };

  ///
  /// An pointer-based octree that dynamically adds and removes nodes as needed.
  /// Contained meshes are considered unique based on the combination of their ID and instance start.
  struct LUDO_API dynamic_octree
  {
    uint64_t id = 0; ///< The ID of the dynamic octree.

    dynamic_octree_node root; ///< The root node of the dynamic octree.
    uint32_t max_depth = 0; ///< The maximum depth of the dynamic octree.
    uint32_t split_threshold = 10; ///< The maximum number of elements to add to a node before splitting it.

    std::unordered_map<std::pair<uint64_t, uint32_t>, dynamic_octree_node*, dynamic_octree_element_node_hash> element_nodes; ///< Used to quickly find the node containing an element.
  };

  ///
  /// A linear octree that contains a full set of leaf octants.
  /// This allows for parallel search algorithms.
  /// A linear octree with a depth of 0 with have only a single octant.
  /// Contained meshes are considered unique based on the combination of their ID and instance start.
  struct LUDO_API linear_octree
  {
    uint64_t id = 0; ///< The ID of the linear octree.

    aabb bounds; ///< The bounds of the linear octree.
    uint8_t depth = 1; ///< The depth of the linear octree (the maximum supported depth is 9).

    std::unordered_map<uint32_t, std::vector<mesh>> octants; ///< The leaf octants of the linear octree. TODO use std::set for faster sorting?
  };

  ///
  /// Calculates the bounds of the given mesh.
  /// \param mesh The mesh.
  /// \param format The vertex format of the mesh.
  /// \return The bounds of the given mesh.
  LUDO_API aabb bounds(const mesh& mesh, const vertex_format& format);

  ///
  /// Determines whether an AABB is contained within another AABB.
  /// \param container The containing AABB.
  /// \param containee The AABB to check against the containing AABB.
  /// \return True if the containee AABB is wholly within the container AABB, false otherwise.
  LUDO_API bool contains(const aabb& container, const aabb& containee);

  ///
  /// Determines whether a position is contained within another AABB.
  /// \param container The containing AABB.
  /// \param position The position to check against the containing AABB.
  /// \return True if the position is within the container AABB, false otherwise.
  bool contains(const aabb& container, const vec3& position);

  ///
  /// Determines whether two AABBs intersect.
  /// \param a The first AABB.
  /// \param b The second AABB.
  /// \return True if the AABBs intersect, false otherwise.
  LUDO_API bool intersect(const aabb& a, const aabb& b);

  template<>
  LUDO_API dynamic_octree* add(instance& instance, const dynamic_octree& init, const std::string& partition);

  ///
  /// Removes an element from a dynamic octree.
  /// \param octree The dynamic octree to remove the element from.
  /// \param element The element to remove from the dynamic octree.
  /// \return True if the the element was removed, false otherwise.
  LUDO_API bool remove(dynamic_octree& octree, const dynamic_octree_element& element);

  /// TODO not sure about this...
  /// Retrieves an element from a dynamic octree.
  /// \param octree The dynamic octree to retrieve the element from.
  /// \param mesh The mesh contained in the element to retrieve from the octree.
  LUDO_API dynamic_octree_element& get(dynamic_octree& octree, const mesh& mesh);

  ///
  /// Updates an element in a dynamic octree.
  /// \param octree The dynamic octree to update the element within.
  /// \param element The element to update within the dynamic octree.
  /// \param position The position of the element.
  LUDO_API void update(dynamic_octree& octree, const dynamic_octree_element& element);

  ///
  /// Finds elements within a dynamic octree.
  /// \param octree The dynamic octree to search.
  /// \param test The test to perform against the bounds of the nodes.
  /// \return The matching elements.
  LUDO_API std::set<mesh> find(const dynamic_octree& octree, const std::function<int32_t(const aabb& bounds)>& test);

  template<>
  LUDO_API linear_octree* add(instance& instance, const linear_octree& init, const std::string& partition);

  ///
  /// Adds an element to a linear octree.
  /// \param octree The linear octree to add the element to.
  /// \param element The element to add to the linear octree.
  /// \param position The position of the element.
  LUDO_API void add(linear_octree& octree, const mesh& element, const vec3& position);

  ///
  /// Removes an element from a linear octree.
  /// \param octree The linear octree to remove the element from.
  /// \param element The element to remove to the linear octree.
  /// \param position The position of the element.
  /// \return True if the the element was removed, false otherwise.
  LUDO_API bool remove(linear_octree& octree, const mesh& element, const vec3& position);

  ///
  /// Moves a linear octree.
  /// \param octree The linear octree to move.
  /// \param movement The movement to apply to the linear octree.
  LUDO_API void move(linear_octree& octree, const vec3& movement);

  ///
  /// Finds elements within a linear octree using a hierarchical search.
  /// \param octree The linear octree to search.
  /// \param test The test to perform against the bounds of the nodes.
  /// \return The matching elements.
  LUDO_API std::set<mesh> find(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test);

  ///
  /// Finds elements within a linear octree using a parallel search across all octants.
  /// \param octree The linear octree to search.
  /// \param test The test to perform against the bounds of the nodes.
  /// \return The matching elements.
  LUDO_API std::set<mesh> find_parallel(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test);

  ///
  /// Retrieves the size of an octant in a linear octree.
  /// \param octree The linear octree to retrieve the octant size from.
  LUDO_API vec3 octant_size(const linear_octree& octree);
}

#endif // LUDO_GRAPHS_H
