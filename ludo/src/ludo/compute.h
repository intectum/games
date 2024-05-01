/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_COMPUTE_H
#define LUDO_COMPUTE_H

#include "data.h"

namespace ludo
{
  ///
  /// A compute program that runs on the GPU.
  struct LUDO_API compute_program
  {
    uint64_t id = 0; ///< The ID of this compute program.
    uint64_t compute_shader_id = 0; ///< The ID of the compute shader.

    buffer shader_buffer; ///< A buffer containing data available to this compute program.
  };

  template<>
  LUDO_API compute_program* add(instance& instance, const compute_program& init, const std::string& partition);

  ///
  /// Adds a compute program to the data of an instance.
  /// \param instance The instance to add the compute program to.
  /// \param init The initial state of the new compute program.
  /// \param code The source code of the compute program.
  /// \param partition The name of the partition.
  /// \return A pointer to the new compute program. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  LUDO_API compute_program* add(instance& instance, const compute_program& init, std::istream& code, const std::string& partition = "default");

  template<>
  LUDO_API void remove<compute_program>(instance& instance, compute_program* element, const std::string& partition);

  ///
  /// Executes a compute program on the GPU.
  /// \param compute_program The compute program to execute.
  /// \param groups_x The number of groups to execute on the x axis.
  /// \param groups_y The number of groups to execute on the y axis.
  /// \param groups_z The number of groups to execute on the z axis.
  LUDO_API void execute(compute_program& compute_program, uint32_t groups_x, uint32_t groups_y, uint32_t groups_z);
}

#endif // LUDO_COMPUTE_H
