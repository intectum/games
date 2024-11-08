/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "../meshes.h"

namespace ludo
{
  ///
  /// Subtracts one mesh from another. *UNDER CONSTRUCTION*
  /// \param lhs The mesh to subtract from
  /// \param rhs The mesh to subtract
  /// \param relative_transform The position and orientation of the rhs mesh relative to the lhs mesh
  /// \return The difference
  mesh* subtract(instance& instance, const vertex_format& format, const mesh& lhs, const mesh& rhs, const mat4& relative_transform);
}
