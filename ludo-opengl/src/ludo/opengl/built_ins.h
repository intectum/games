/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_OPENGL_SHADERS_H
#define LUDO_OPENGL_SHADERS_H

#include <ludo/meshes.h>
#include <ludo/rendering.h>

namespace ludo
{
  uint64_t built_in_render_program(instance& instance, const mesh_buffer_options& options);

  uint64_t built_in_shader(instance& instance, const mesh_buffer_options& options, shader_type type);

  uint64_t post_processing_vertex_shader(instance& instance);
}

#endif // LUDO_OPENGL_SHADERS_H
