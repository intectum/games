/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

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
}
