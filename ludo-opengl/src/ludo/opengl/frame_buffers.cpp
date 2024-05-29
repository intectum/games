/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include <ludo/rendering.h>

#include "util.h"

namespace ludo
{
  void init(frame_buffer& frame_buffer)
  {
    auto name = GLuint();
    glGenFramebuffers(1, &name); check_opengl_error();
    frame_buffer.id = name;

    use(frame_buffer);

    // Bind the color textures.
    auto draw_buffers = std::vector<GLenum>();
    for (auto index = 0ul; index < frame_buffer.color_texture_ids.size(); index++)
    {
      glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, frame_buffer.color_texture_ids[index], 0); check_opengl_error();

      draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + index);
    }

    // Draw to all the color textures.
    glDrawBuffers(static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data()); check_opengl_error();

    // Bind the depth texture.
    if (frame_buffer.depth_texture_id)
    {
      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, frame_buffer.depth_texture_id, 0); check_opengl_error();
    }

    // Bind the stencil texture.
    if (frame_buffer.stencil_texture_id)
    {
      glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, frame_buffer.stencil_texture_id, 0); check_opengl_error();
    }

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_UNSUPPORTED)
    {
      std::cout << "frame buffer unsupported" << std::endl;
      assert(false && "frame buffer unsupported");
    }
    else if (status != GL_FRAMEBUFFER_COMPLETE)
    {
      std::cout << "failed to create frame buffer, status: " << status << std::endl;
      assert(false && "failed to create frame buffer");
    }
  }

  void de_init(frame_buffer& frame_buffer)
  {
    auto name = static_cast<GLuint>(frame_buffer.id);
    glDeleteFramebuffers(1, &name); check_opengl_error();
    frame_buffer.id = 0;
  }

  void use(const frame_buffer& frame_buffer)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.id); check_opengl_error();
    glViewport(0, 0, static_cast<GLsizei>(frame_buffer.width), static_cast<GLsizei>(frame_buffer.height)); check_opengl_error();
  }

  void use_and_clear(const frame_buffer& frame_buffer, const vec4& color)
  {
    use(frame_buffer);

    glClearColor(color[0], color[1], color[2], color[3]); check_opengl_error();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); check_opengl_error();
  }

  void blit(const frame_buffer& source, const frame_buffer& dest)
  {
    auto mask = GL_COLOR_BUFFER_BIT;

    if (source.depth_texture_id && dest.depth_texture_id)
    {
      mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (source.stencil_texture_id && dest.stencil_texture_id)
    {
      mask |= GL_STENCIL_BUFFER_BIT;
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, source.id); check_opengl_error();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.id); check_opengl_error();
    glBlitFramebuffer(
      0,
      0,
      static_cast<GLint>(source.width),
      static_cast<GLint>(source.height),
      0,
      0,
      static_cast<GLint>(dest.width),
      static_cast<GLint>(dest.height),
      mask,
      GL_NEAREST
    ); check_opengl_error();
  }
}
