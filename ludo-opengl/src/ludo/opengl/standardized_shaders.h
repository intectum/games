/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_OPENGL_SHADERS_H
#define LUDO_OPENGL_SHADERS_H

#include <ludo/meshes.h>
#include <ludo/rendering.h>

namespace ludo
{
  void write_header(std::ostream& stream, const vertex_format_options& options);

  void write_types(std::ostream& stream, const vertex_format_options& options);

  void write_inputs(std::ostream& stream, const vertex_format_options& options);

  void write_buffers(std::ostream& stream, const vertex_format_options& options);

  void write_vertex_main(std::ostream& stream, const vertex_format_options& options);

  void write_lighting_functions(std::ostream& stream, const vertex_format_options& options);

  void write_fragment_main(std::ostream& stream, const vertex_format_options& options);
}

#endif // LUDO_OPENGL_SHADERS_H
