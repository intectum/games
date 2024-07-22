/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <ludo/rendering.h>

namespace ludo
{
  void write_header(std::ostream& stream, const vertex_format& format);

  void write_types(std::ostream& stream, const vertex_format& format);

  void write_inputs(std::ostream& stream, const vertex_format& format);

  void write_buffers(std::ostream& stream, const vertex_format& format);

  void write_vertex_main(std::ostream& stream, const vertex_format& format);

  void write_lighting_functions(std::ostream& stream, const vertex_format& format);

  void write_fragment_main(std::ostream& stream, const vertex_format& format);
}
