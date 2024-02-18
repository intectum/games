/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "frame_buffers.h"
#include "util.h"

namespace ludo
{
  template<>
  frame_buffer* add(instance& instance, const frame_buffer& init, const std::string& partition)
  {
    auto frame_buffer = add(data<ludo::frame_buffer>(instance), init, partition);

    auto name = GLuint();
    glGenFramebuffers(1, &name); check_opengl_error();
    frame_buffer->id = name;

    bind(*frame_buffer);

    // Bind the color textures.
    auto draw_buffers = std::vector<GLenum>();
    for (auto index = 0ul; index < frame_buffer->color_texture_ids.size(); index++)
    {
      glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, frame_buffer->color_texture_ids[index], 0); check_opengl_error();

      draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + index);
    }

    // Draw to all the color textures.
    glDrawBuffers(static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data()); check_opengl_error();

    // Bind the depth texture.
    if (frame_buffer->depth_texture_id)
    {
      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, frame_buffer->depth_texture_id, 0); check_opengl_error();
    }

    // Bind the stencil texture.
    if (frame_buffer->stencil_texture_id)
    {
      glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, frame_buffer->stencil_texture_id, 0); check_opengl_error();
    }

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_UNSUPPORTED)
    {
      log_error("ludo", "Frame buffer unsupported");
    }
    else if (status != GL_FRAMEBUFFER_COMPLETE)
    {
      log_error("ludo", "Failed to create frame buffer");
    }

    return frame_buffer;
  }

  template<>
  void remove<frame_buffer>(instance& instance, frame_buffer* element, const std::string& partition)
  {
    auto name = static_cast<GLuint>(element->id);

    glDeleteFramebuffers(1, &name); check_opengl_error();

    remove(data<frame_buffer>(instance), element, partition);
  }

  void blit(const frame_buffer& source, const frame_buffer& dest)
  {
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
      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
      GL_NEAREST
    ); check_opengl_error();
  }

  void bind(const frame_buffer& frame_buffer)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.id); check_opengl_error();
    glViewport(0, 0, static_cast<GLsizei>(frame_buffer.width), static_cast<GLsizei>(frame_buffer.height)); check_opengl_error();
  }
}
