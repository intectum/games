/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/rendering.h>

#include "util.h"

namespace ludo
{
  auto draw_modes = std::unordered_map<mesh_primitive, GLenum>
  {
    { mesh_primitive::POINT_LIST, GL_POINTS },
    { mesh_primitive::LINE_LIST, GL_LINES },
    { mesh_primitive::LINE_STRIP, GL_LINE_STRIP },
    { mesh_primitive::TRIANGLE_LIST, GL_TRIANGLES },
    { mesh_primitive::TRIANGLE_STRIP, GL_TRIANGLE_STRIP }
  };

  void start_render_transaction(rendering_context& rendering_context, array<render_program>& render_programs)
  {
    if (rendering_context.fence.id)
    {
      wait(rendering_context.fence);
    }

    for (auto& render_program : render_programs)
    {
      render_program.active_commands.start = 0;
    }
  }

  void commit_render_commands(rendering_context& rendering_context, array<render_program>& render_programs, const heap& render_commands, const heap& indices, const heap& vertices)
  {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, render_commands.id); check_opengl_error();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.id); check_opengl_error();
    glBindBuffer(GL_ARRAY_BUFFER, vertices.id); check_opengl_error();

    commit(rendering_context.shader_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, rendering_context.shader_buffer.front.id); check_opengl_error();

    for (auto& render_program : render_programs)
    {
      if (!render_program.active_commands.count)
      {
        continue;
      }

      use(render_program);

      glMultiDrawElementsIndirect(
        draw_modes[render_program.primitive],
        GL_UNSIGNED_INT,
        reinterpret_cast<void*>((render_program.command_buffer.data - render_commands.data) + render_program.active_commands.start * sizeof(render_command)),
        static_cast<GLsizei>(render_program.active_commands.count),
        sizeof(render_command)
      ); check_opengl_error();

      render_program.active_commands.start += render_program.active_commands.count;
      render_program.active_commands.count = 0;
    }
  }

  void commit_render_transaction(rendering_context& rendering_context)
  {
    init(rendering_context.fence);
  }

  // Based on: http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
  std::array<vec4, 6> frustum_planes(const camera& camera)
  {
    auto view_inverse = camera.view;
    invert(view_inverse);
    auto view_projection = camera.projection * view_inverse;

    auto rows = std::array<vec4, 4>
    {
      vec4 { view_projection[0], view_projection[4], view_projection[8], view_projection[12] },
      vec4 { view_projection[1], view_projection[5], view_projection[9], view_projection[13] },
      vec4 { view_projection[2], view_projection[6], view_projection[10], view_projection[14] },
      vec4 { view_projection[3], view_projection[7], view_projection[11], view_projection[15] },
    };

    return std::array<vec4, 6>
    {
      rows[3] + rows[0], // Left
      rows[3] - rows[0], // Right
      rows[3] + rows[1], // Bottom
      rows[3] - rows[1], // Top
      rows[3] + rows[2], // Near
      rows[3] - rows[2] // Far
    };
  }
}
