/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_COMPUTE_H
#define LUDO_COMPUTE_H

#include <istream>

#include "data/data.h"

namespace ludo
{
  ///
  /// A compute program that runs on the GPU.
  struct LUDO_API compute_program
  {
    uint64_t id = 0; ///< The ID of this compute program.

    buffer shader_buffer; ///< A buffer containing data available to this compute program.
  };

  ///
  /// Initializes a compute program with the given code.
  /// \param compute_program The compute program.
  /// \param shader_file_name The name of the file containing the shader source code.
  LUDO_API void init(compute_program& compute_program, const std::string& shader_file_name);

  ///
  /// Initializes a compute program with the given code.
  /// \param compute_program The compute program.
  /// \param code The source code.
  LUDO_API void init(compute_program& compute_program, std::istream& code);

  ///
  /// De-initializes a compute program.
  /// \param compute_program The compute program.
  LUDO_API void de_init(compute_program& compute_program);

  ///
  /// Executes a compute program on the GPU.
  /// \param compute_program The compute program to execute.
  /// \param groups_x The number of groups to execute on the x axis.
  /// \param groups_y The number of groups to execute on the y axis.
  /// \param groups_z The number of groups to execute on the z axis.
  LUDO_API void execute(compute_program& compute_program, uint32_t groups_x, uint32_t groups_y, uint32_t groups_z);
}

#endif // LUDO_COMPUTE_H
