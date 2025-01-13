/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/ecs.h>
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

  void init_rendering()
  {
    glewInit();

    // Sometimes glewInit() gives false negatives. Let's clear the OpenGL error, so it doesn't confuse us elsewhere.
    glGetError();

    // Ensure objects further from the viewpoint are not drawn over the top of closer objects. To assist multi
    // pass rendering, objects at the exact same distance can be rendered over (i.e. the object will be rendered
    // using the result of the last draw).
    glDepthFunc(GL_LEQUAL); check_opengl_error();
    glEnable(GL_DEPTH_TEST); check_opengl_error();

    // Only render the front (counter-clockwise) side of a polygon
    glEnable(GL_CULL_FACE); check_opengl_error();

    // Enable blending for rendering transparency
    glEnable(GL_BLEND); check_opengl_error();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); check_opengl_error();

    // Enable multisampling for anti-aliasing
    glEnable(GL_MULTISAMPLE); check_opengl_error();
  }

  void render(const render_program& render_program, uint32_t start, uint32_t count)
  {
    use(render_program);

    glMultiDrawElementsIndirect(
      draw_modes[render_program.primitive],
      GL_UNSIGNED_INT,
      reinterpret_cast<void*>(start * sizeof(render_command)),
      static_cast<GLsizei>(count),
      sizeof(render_command)
    ); check_opengl_error();
  }

  void bind_commands(uint32_t id)
  {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, id); check_opengl_error();
  }

  void bind_indices(uint32_t id)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id); check_opengl_error();
  }

  void bind_vertices(uint32_t id)
  {
    glBindBuffer(GL_ARRAY_BUFFER, id); check_opengl_error();
  }

  void bind_data(uint32_t position, uint32_t id)
  {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position, id); check_opengl_error();
  }

  // Based on http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
  std::array<vec4, 6> frustum_planes(const mat4& view_projection)
  {
    auto rows = std::array
    {
      vec4 { view_projection[0], view_projection[4], view_projection[8], view_projection[12] },
      vec4 { view_projection[1], view_projection[5], view_projection[9], view_projection[13] },
      vec4 { view_projection[2], view_projection[6], view_projection[10], view_projection[14] },
      vec4 { view_projection[3], view_projection[7], view_projection[11], view_projection[15] },
    };

    return
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
