/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef ASTRUM_SPATIAL_ICO_TREE_H
#define ASTRUM_SPATIAL_ICO_TREE_H

#include <ludo/api.h>

namespace astrum
{
  ///
  /// An pointer-based ico tree that dynamically adds and removes nodes as needed.
  struct ico_tree
  {
    uint64_t id = 0; ///< The ID of the ico tree.

    uint32_t divisions = 1; ///< The number of divisions (layers) in the ico tree.
    uint32_t cell_capacity = 16; ///< The maximum number of render meshes that can be added to a cell.
    uint32_t cell_element_size = 0; ///< The size (in bytes) of an element in a cell.
    uint32_t max_populated_cells = 1; // TODO

    ludo::buffer buffer;
  };

  using ico_tree_test = std::function<bool(const std::array<ludo::vec3, 3>& face)>;

  void init(ico_tree& ico_tree);

  void destroy(ico_tree& ico_tree);

  ///
  /// Adds an element to an ico tree.
  /// \param ico tree The ico tree to add the element to.
  /// \param element_data The element data to add to the ico tree.
  /// \param position The position of the element.
  void add(ico_tree& ico_tree, const std::byte* element_data, const ludo::vec3& position);

  ///
  /// Removes an element from an ico tree.
  /// \param ico tree The ico tree to remove the element from.
  /// \param element_data The element data to remove from the ico tree.
  /// \param position The position of the element.
  void remove(ico_tree& ico_tree, const std::byte* element_data, const ludo::vec3& position);

  ///
  /// Finds a cell within an ico tree.
  /// \param ico tree The ico tree to search.
  /// \param test The test to perform against the faces.
  /// \return The matching cell index, or std::numeric_limits<uint32_t>::max() if a cell was not found.
  uint32_t find_cell(const ico_tree& ico_tree, const ico_tree_test& test);

  ///
  /// Finds cells within an ico tree.
  /// \param ico tree The ico tree to search.
  /// \param test The test to perform against the faces.
  /// \return The matching cell indices.
  std::vector<uint32_t> find_cells(const ico_tree& ico_tree, const ico_tree_test& test);

  uint32_t cell_count(const ico_tree& ico_tree);

  ludo::buffer cell_element_data(const ico_tree& ico_tree, uint32_t cell_index);

  ///
  /// Adds render commands to the render programs' command buffers and updates the active command count.
  /// \param ico_trees The ico trees.
  /// \param render_programs The render programs that can have render commands added.
  /// \param render_commands The render commands to sample from.
  /// \param camera The camera the render meshes are being viewed through.
  void add_render_commands(ludo::array<ico_tree>& ico_trees, ludo::array<ludo::render_program>& render_programs, const ludo::heap& render_commands, const ludo::camera& camera);
}

#endif // ASTRUM_SPATIAL_ICO_TREE_H
