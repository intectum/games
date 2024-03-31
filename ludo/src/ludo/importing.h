/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_IMPORTING_H
#define LUDO_IMPORTING_H

#include "ludo/meshes.h"

namespace ludo
{
  struct LUDO_API import_options
  {
    bool merge_meshes = false; ///< Determines if the meshes being imported should be merged into a single mesh.
  };

  ///
  /// Imports body shapes and meshes from a file.
  /// \param instance The instance to import into.
  /// \param file_name The name of the file to import from.
  /// \param options The options used to modify the import behavior.
  /// \param partition The partition to import into.
  /// \return The imported meshes.
  LUDO_API std::vector<mesh> import(instance& instance, const std::string& file_name, const import_options& options = {}, const std::string& partition = "default");

  ///
  /// Determines the total and unique vertex counts in a file.
  /// \param file_name The name of the file to count the vertices in.
  /// \return The total and unique vertex counts of a file. Of the form { total, unique }.
  LUDO_API std::pair<uint32_t, uint32_t> import_counts(const std::string& file_name);
}

#endif // LUDO_IMPORTING_H
