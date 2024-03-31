/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "buffers.h"

namespace ludo
{
  void sort_free(heap_buffer& buffer);

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

  void read(const buffer& buffer, uint64_t position, std::byte* value, uint32_t size)
  {
    assert(position >= 0 && position <= buffer.size - size && "position out of range");

    std::memcpy(value, buffer.data + position, size);
  }

  void write(const buffer& buffer, uint64_t position, std::byte* value, uint32_t size)
  {
    assert(position >= 0 && position <= buffer.size - size && "position out of range");

    std::memcpy(buffer.data + position, value, size);
  }

  heap_buffer allocate_heap(uint64_t size)
  {
    auto simple_buffer = allocate(size);

    auto buffer = heap_buffer();
    buffer.id = simple_buffer.id;
    buffer.data = simple_buffer.data;
    buffer.size = simple_buffer.size;
    buffer.free = { { .data = simple_buffer.data, .size = simple_buffer.size } };

    return buffer;
  }

  heap_buffer allocate_heap_vram(uint64_t size, vram_buffer_access_hint access_hint)
  {
    auto simple_buffer = allocate_vram(size, access_hint);

    auto buffer = heap_buffer();
    buffer.id = simple_buffer.id;
    buffer.data = simple_buffer.data;
    buffer.size = simple_buffer.size;
    buffer.free = { { .data = simple_buffer.data, .size = simple_buffer.size } };

    return buffer;
  }

  void deallocate(heap_buffer& buffer)
  {
    auto simple_buffer = ludo::buffer
    {
      .id = buffer.id,
      .data = buffer.data,
      .size = buffer.size
    };

    deallocate(simple_buffer);

    buffer.data = nullptr;
    buffer.size = 0;
    buffer.free.clear();
  }

  void deallocate_vram(heap_buffer& buffer)
  {
    auto simple_buffer = ludo::buffer
    {
      .id = buffer.id,
      .data = buffer.data,
      .size = buffer.size
    };

    deallocate_vram(simple_buffer);

    buffer.data = nullptr;
    buffer.size = 0;
  }

  buffer allocate(heap_buffer& buffer, uint64_t size, uint8_t byte_alignment)
  {
    for (auto free_iter = buffer.free.begin(); free_iter < buffer.free.end(); free_iter++)
    {
      auto start = free_iter->data;
      auto start_offset = free_iter->data - buffer.data;
      auto start_misalignment = start_offset % static_cast<uint64_t>(byte_alignment);
      auto alignment_offset = start_misalignment && byte_alignment > 1 ? byte_alignment - start_misalignment : 0;
      auto aligned_data = free_iter->data + alignment_offset;
      auto aligned_size = free_iter->size > alignment_offset ? free_iter->size - alignment_offset : 0;

      if (aligned_size < size)
      {
        continue;
      }

      auto child_buffer = ludo::buffer
      {
        .data = aligned_data,
        .size = size
      };

      if (aligned_size == size)
      {
        buffer.free.erase(free_iter);
      }
      else
      {
        free_iter->data += size + alignment_offset;
        free_iter->size -= size + alignment_offset;
      }

      if (alignment_offset)
      {
        buffer.free.push_back(ludo::buffer
        {
          .data = start,
          .size = alignment_offset
        });
      }

      sort_free(buffer);

      return child_buffer;
    }

    assert(false && "could not fit buffer");
  }

  void deallocate(heap_buffer& buffer, ludo::buffer& child_buffer)
  {
    auto free_before_iter = std::find_if(buffer.free.begin(), buffer.free.end(),
      [&child_buffer](const ludo::buffer& free)
      {
        return free.data + free.size == child_buffer.data;
      }
    );

    auto free_after_iter = std::find_if(buffer.free.begin(), buffer.free.end(),
      [&child_buffer](const ludo::buffer& free)
      {
        return free.data == child_buffer.data + child_buffer.size;
      }
    );

    if (free_before_iter != buffer.free.end() && free_after_iter != buffer.free.end())
    {
      free_before_iter->size += child_buffer.size + free_after_iter->size;
      buffer.free.erase(free_after_iter);
    }
    else if (free_before_iter != buffer.free.end())
    {
      free_before_iter->size += child_buffer.size;
    }
    else if (free_after_iter != buffer.free.end())
    {
      free_after_iter->data = child_buffer.data;
      free_after_iter->size += child_buffer.size;
    }
    else
    {
      buffer.free.emplace_back(child_buffer);
    }

    child_buffer.data = nullptr;
    child_buffer.size = 0;

    sort_free(buffer);
  }

  void clear(heap_buffer& buffer)
  {
    buffer.free = { { .data = buffer.data, .size = buffer.size } };
  }

  void sort_free(heap_buffer& buffer)
  {
    // Smallest free sections first so that allocations are within the smallest free section that will fit.
    std::sort(
      buffer.free.begin(),
      buffer.free.end(),
      [](const ludo::buffer& a, const ludo::buffer& b)
      {
        return a.size < b.size || a.data < b.data;
      }
    );
  }
}
