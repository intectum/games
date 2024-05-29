/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_IMPORTING_H
#define LUDO_IMPORTING_H

#include "ludo/physics.h"
#include "ludo/rendering.h"

namespace ludo
{
  ///
  /// A results of an import.
  struct LUDO_API import_results
  {
    std::vector<animation> animations; ///< The imported animations.
    std::vector<armature> armatures; ///< The imported animations.
    std::vector<dynamic_body_shape> dynamic_body_shapes; ///< The imported dynamic body shapes.
    std::vector<mesh> meshes; ///< The imported meshes.
    std::vector<texture> textures; ///< The imported textures.
  };

  ///
  /// A set of options for performing an import.
  struct LUDO_API import_options
  {
    bool merge_meshes = false; ///< Determines if the meshes being imported should be merged into a single mesh.
  };

  ///
  /// Imports body shapes and meshes from a file.
  /// \param file_name The name of the file.
  /// \param indices The indices to allocate from.
  /// \param vertices The indices to allocate from.
  /// \param options The options used to modify the import behavior.
  /// \return The imported meshes.
  LUDO_API import_results import(const std::string& file_name, heap& indices, heap& vertices, const import_options& options = {});

  ///
  /// Determines the total and unique vertex counts in a file.
  /// \param file_name The name of the file to count the vertices in.
  /// \return The total and unique vertex counts of a file. Of the form { total, unique }.
  LUDO_API std::pair<uint32_t, uint32_t> import_counts(const std::string& file_name);
}

#endif // LUDO_IMPORTING_H
