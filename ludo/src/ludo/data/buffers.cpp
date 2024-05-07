/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cstring>

#include "buffers.h"

namespace ludo
{
  stream::stream(const buffer& buffer, uint32_t position)
  {
    this->data = buffer.data;
    this->size = buffer.size;
    this->position = position;
  }

  buffer allocate(uint64_t size)
  {
    return
    {
      .id = next_id++,
      .data = static_cast<std::byte*>(std::malloc(size)),
      .size = size
    };
  }

  void deallocate(buffer& buffer)
  {
    std::free(buffer.data);
    buffer.data = nullptr;
    buffer.size = 0;
  }

  double_buffer allocate_dual(uint64_t size, vram_buffer_access_hint access_hint)
  {
    return
    {
      .front = allocate_vram(size, access_hint),
      .back = allocate(size),
    };
  }

  void deallocate_dual(double_buffer& buffer)
  {
    deallocate(buffer.front);
    deallocate_vram(buffer.back);
  }

  void push(double_buffer& buffer)
  {
    std::memcpy(buffer.front.data, buffer.back.data, buffer.front.size);
  }
}
