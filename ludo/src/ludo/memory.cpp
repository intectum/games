/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <cstdlib>

#include "memory.h"

namespace ludo
{
  static uint32_t next_id = 0;

  std::byte* buffer::operator[](uint32_t index)
  {
    assert(index < size && "index out of bounds");

    return data + index;
  }

  const std::byte* buffer::operator[](uint32_t index) const
  {
    assert(index < size && "index out of bounds");

    return data + index;
  }

  buffer allocate_buffer(uint64_t size, access_hint access_hint)
  {
    auto data = static_cast<std::byte*>(malloc(size));

    return {
      .id = next_id++,
      .data = data,
      .size = size,
    };
  }

  void free_buffer(buffer& buffer)
  {
    free(buffer.data);

    buffer.id = 0;
    buffer.data = nullptr;
    buffer.size = 0;
  }

  arena allocate_arena(uint64_t size, access_hint access_hint)
  {
    auto buffer = allocate_buffer(size, access_hint);

    return {
      buffer,
      buffer.data
    };
  }

  arena allocate_arena_vram(uint64_t size, access_hint access_hint)
  {
    auto buffer = allocate_buffer_vram(size, access_hint);

    return {
      buffer,
      buffer.data
    };
  }

  void free_arena(arena& arena)
  {
    free_buffer(arena);

    arena.next = nullptr;
  }

  void free_arena_vram(arena& arena)
  {
    free_buffer_vram(arena);

    arena.next = nullptr;
  }

  pool allocate_pool(uint32_t block_count, uint32_t block_size, access_hint access_hint)
  {
    auto buffer = allocate_buffer(block_count * block_size, access_hint);

    return {
      buffer,
      block_count,
      block_size,
      static_cast<bool*>(calloc(block_count, sizeof(bool)))
    };
  }

  pool allocate_pool_vram(uint32_t block_count, uint32_t block_size, access_hint access_hint)
  {
    auto buffer = allocate_buffer_vram(block_count * block_size, access_hint);

    return {
      buffer,
      block_count,
      block_size,
      static_cast<bool*>(calloc(block_count, sizeof(bool)))
    };
  }

  void free_pool(pool& pool)
  {
    free_buffer(pool);

    pool.block_count = 0;
    pool.block_size = 0;

    ::free(pool.blocks);
    pool.blocks = nullptr;
  }

  void free_pool_vram(pool& pool)
  {
    free_buffer_vram(pool);

    pool.block_count = 0;
    pool.block_size = 0;

    ::free(pool.blocks);
    pool.blocks = nullptr;
  }

  std::byte* allocate(pool& pool)
  {
    for (auto block = 0; block < pool.block_count; block++)
    {
      if (pool.blocks[block]) continue;

      pool.blocks[block] = true;
      return pool.data + block * pool.block_size;
    }

    assert(false && "not enough space");
    return nullptr;
  }

  void free(pool& pool, std::byte* data)
  {
    pool.blocks[(data - pool.data) / pool.block_size] = false;
  }

  std::byte* allocate(arena& arena, uint64_t size)
  {
    assert(arena.next - arena.data + size <= arena.size && "not enough space");

    auto next = arena.next;
    arena.next += size;

    return next;
  }

  void reset(arena& arena)
  {
    arena.next = arena.data;
  }

  stream::stream(const buffer& buffer, uint32_t position)
  {
    this->id = buffer.id;
    this->data = buffer.data;
    this->size = buffer.size;
    this->position = position;
  }

  bool ended(stream& stream)
  {
    return stream.position >= stream.size;
  }
}
