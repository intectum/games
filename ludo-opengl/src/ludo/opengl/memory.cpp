/*
* This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <unordered_map>

#include <ludo/memory.h>

#include "util.h"

namespace ludo
{
  auto access_hints = std::unordered_map<access_hint, GLenum>
  {
    { access_hint::READ, GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT },
    { access_hint::WRITE, GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT },
    { access_hint::READ_WRITE, GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT }
  };

  buffer allocate_buffer_vram(uint64_t size, access_hint access_hint)
  {
    auto buffer = ludo::buffer();

    auto name = GLuint();
    glGenBuffers(1, &name); check_opengl_error();
    buffer.id = name;

    // Quirk: Binding is what actually associates a buffer with the buffer name
    // Here we bind to an arbitrary target and then unbind
    glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glNamedBufferStorage(buffer.id, static_cast<GLsizeiptr>(size), nullptr, access_hints[access_hint]); check_opengl_error();
    buffer.data = static_cast<std::byte*>(glMapNamedBufferRange(buffer.id, 0, static_cast<GLsizeiptr>(size), access_hints[access_hint])); check_opengl_error();

    buffer.size = size;

    return buffer;
  }

  void free_buffer_vram(buffer& buffer)
  {
    auto name = buffer.id;

    glUnmapNamedBuffer(buffer.id); check_opengl_error();
    glDeleteBuffers(1, &name); check_opengl_error();

    buffer.id = 0;
    buffer.data = nullptr;
    buffer.size = 0;
  }
}
