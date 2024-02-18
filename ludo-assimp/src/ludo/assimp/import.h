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

    ludo::mesh_buffer* mesh_buffer = nullptr;

    uint32_t instance_count = 1;

    uint32_t index_start = 0; ///< The starting index of the indices.
    uint32_t vertex_start = 0; ///< The starting index of the vertices.
  };

  LUDO_API mesh_buffer_options build_mesh_buffer_options(const std::string& file_name, const import_options& options = {});

  LUDO_API void import(instance& instance, const std::string& file_name, const import_options& options = {}, const std::string& partition = "default");
}

#endif // LUDO_ASSIMP_IMPORT_H
