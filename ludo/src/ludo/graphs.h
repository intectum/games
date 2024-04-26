/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_GRAPHS_H
#define LUDO_GRAPHS_H

#include <cmath>
#include <functional>

#include "compute.h"
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
    ludo::mesh_instance mesh_instance; ///< The mesh instance.
    aabb bounds; ///< The bounds of the mesh (in world coordinates). // TODO make this local to the mesh instance?
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
  /// An pointer-based octree that dynamically adds and removes nodes as needed.
  /// Contained meshes are considered unique based on the combination of their ID and instance start.
  struct LUDO_API dynamic_octree
  {
    uint64_t id = 0; ///< The ID of the dynamic octree.

    dynamic_octree_node root; ///< The root node of the dynamic octree.
    uint32_t max_depth = 0; ///< The maximum depth of the dynamic octree.
    uint32_t split_threshold = 10; ///< The maximum number of elements to add to a node before splitting it.

    std::unordered_map<uint64_t, dynamic_octree_node*> element_nodes; ///< Used to quickly find the node containing an element.
  };

  ///
  /// A linear octree that contains a full set of leaf octants.
  /// This allows for parallel search algorithms.
  /// A linear octree with a depth of 0 with have only a single octant.
  struct LUDO_API linear_octree
  {
    uint64_t id = 0; ///< The ID of the linear octree.
    uint64_t compute_program_id = 0; ///< The ID of the compute program used to build draw commands from this linear octree.

    aabb bounds; ///< The bounds of the linear octree.
    uint8_t depth = 1; ///< The depth of the linear octree.
    uint32_t octant_capacity = 16; ///< The maximum number of mesh instances that can be added to an octant.

    buffer front_buffer;
    buffer back_buffer;
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

  ///
  /// Adds an element to a dynamic octree.
  /// \param octree The dynamic octree to add the element to.
  /// \param element The element to add to the dynamic octree.
  LUDO_API void add(dynamic_octree& octree, const dynamic_octree_element& element);

  ///
  /// Removes an element from a dynamic octree.
  /// \param octree The dynamic octree to remove the element from.
  /// \param element The element to remove from the dynamic octree.
  /// \return True if the the element was removed, false otherwise.
  LUDO_API bool remove(dynamic_octree& octree, const dynamic_octree_element& element);

  /// TODO not sure about this...
  /// Retrieves an element from a dynamic octree.
  /// \param octree The dynamic octree to retrieve the element from.
  /// \param mesh_instance The mesh instance contained in the element to retrieve from the octree.
  LUDO_API dynamic_octree_element& get(dynamic_octree& octree, const mesh_instance& mesh_instance);

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
  LUDO_API std::vector<mesh_instance> find(const dynamic_octree& octree, const std::function<int32_t(const aabb& bounds)>& test);

  template<>
  LUDO_API linear_octree* add(instance& instance, const linear_octree& init, const std::string& partition);

  ///
  /// Pushes the state of a linear octree to the front buffer.
  /// \param octree The linear octree to push.
  LUDO_API void push(linear_octree& octree);

  ///
  /// Adds a compute program used to build draw commands from a linear octree to the data of an instance.
  /// \param instance The instance to add the compute program to.
  /// \param octree The initial state of the new compute program.
  /// \return A pointer to the new compute program. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  LUDO_API compute_program* add_linear_octree_compute_program(instance& instance, const linear_octree& octree);

  ///
  /// Adds a mesh instance to a linear octree.
  /// \param octree The linear octree to add the mesh instance to.
  /// \param mesh_instance The mesh instance to add to the linear octree.
  /// \param position The position of the mesh instance.
  LUDO_API void add(linear_octree& octree, const mesh_instance& mesh_instance, const vec3& position);

  ///
  /// Removes a mesh instance from a linear octree.
  /// \param octree The linear octree to remove the mesh instance from.
  /// \param mesh_instance The mesh instance to remove from the linear octree.
  /// \param position The position of the mesh instance.
  LUDO_API void remove(linear_octree& octree, const mesh_instance& mesh_instance, const vec3& position);

  ///
  /// Finds mesh instances within a linear octree using a hierarchical search.
  /// \param octree The linear octree to search.
  /// \param test The test to perform against the bounds of the nodes.
  /// \return The matching mesh instance IDs.
  LUDO_API std::vector<uint64_t> find(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test);

  ///
  /// Finds mesh instances within a linear octree using a parallel search across all octants.
  /// \param octree The linear octree to search.
  /// \param test The test to perform against the bounds of the nodes.
  /// \return The matching mesh instance IDs.
  LUDO_API std::vector<uint64_t> find_parallel(const linear_octree& octree, const std::function<int32_t(const aabb& bounds)>& test);
}

#endif // LUDO_GRAPHS_H
