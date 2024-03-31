/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <unordered_map>

#include <ludo/buffers.h>

#include "util.h"

namespace ludo
{
  auto buffer_access_hints = std::unordered_map<vram_buffer_access_hint, GLenum>
  {
    { vram_buffer_access_hint::READ, GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT },
    { vram_buffer_access_hint::WRITE, GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT },
    { vram_buffer_access_hint::READ_WRITE, GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT }
  };

  buffer allocate_vram(uint64_t size, vram_buffer_access_hint access_hint)
  {
    auto buffer = ludo::buffer();
    buffer.size = size;

    auto name = GLuint();
    glGenBuffers(1, &name); check_opengl_error();
    buffer.id = name;

    // Quirk: Binding is what actually associates a buffer with the buffer name
    // Here we bind to an arbitrary target and then unbind
    glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glNamedBufferStorage(buffer.id, static_cast<GLsizeiptr>(size), nullptr, buffer_access_hints[access_hint]); check_opengl_error();
    buffer.data = static_cast<std::byte*>(glMapNamedBufferRange(buffer.id, 0, static_cast<GLsizeiptr>(size), buffer_access_hints[access_hint])); check_opengl_error();

    return buffer;
  }

  void deallocate_vram(buffer& buffer)
  {
    auto name = static_cast<GLuint>(buffer.id);

    glUnmapNamedBuffer(buffer.id); check_opengl_error();
    glDeleteBuffers(1, &name); check_opengl_error();

    buffer.data = nullptr;
    buffer.size = 0;
  }
}
