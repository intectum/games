/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ASSIMP_IMPORT_H
#define LUDO_ASSIMP_IMPORT_H

#include <string>

#include <ludo/core.h>
#include <ludo/meshes.h>
#include <ludo/rendering.h>

namespace ludo
{
  struct import_options
  {
    bool merge = false;
  };

  LUDO_API void import(instance& instance, const std::string& file_name, const import_options& options = {}, const std::string& partition = "default");

  LUDO_API std::pair<uint32_t, uint32_t> import_counts(const std::string& file_name);
}

#endif // LUDO_ASSIMP_IMPORT_H
