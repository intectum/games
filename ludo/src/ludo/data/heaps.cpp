/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "heaps.h"

namespace ludo
{
  void sort_free(heap& heap);

  heap allocate_heap(uint64_t size)
  {
    auto buffer = allocate(size);

    auto heap = ludo::heap();
    heap.id = buffer.id;
    heap.data = buffer.data;
    heap.size = buffer.size;
    heap.free = { { .data = heap.data, .size = heap.size } };

    return heap;
  }

  heap allocate_heap_vram(uint64_t size, vram_buffer_access_hint access_hint)
  {
    auto buffer = allocate_vram(size, access_hint);

    auto heap = ludo::heap();
    heap.id = buffer.id;
    heap.data = buffer.data;
    heap.size = buffer.size;
    heap.free = { { .data = heap.data, .size = heap.size } };

    return heap;
  }

  void deallocate(heap& heap)
  {
    auto buffer = ludo::buffer
    {
      .id = heap.id,
      .data = heap.data,
      .size = heap.size
    };

    deallocate(buffer);

    heap.data = nullptr;
    heap.size = 0;
    heap.free.clear();
  }

  void deallocate_vram(heap& heap)
  {
    auto buffer = ludo::buffer
    {
      .id = heap.id,
      .data = heap.data,
      .size = heap.size
    };

    deallocate_vram(buffer);

    heap.data = nullptr;
    heap.size = 0;
    heap.free.clear();
  }

  buffer allocate(heap& heap, uint64_t size, uint8_t alignment)
  {
    if (size == 0)
    {
      return {};
    }

    for (auto free_iter = heap.free.begin(); free_iter < heap.free.end(); free_iter++)
    {
      auto start = free_iter->data;
      auto start_offset = free_iter->data - heap.data;
      auto start_misalignment = start_offset % static_cast<uint64_t>(alignment);
      auto alignment_offset = start_misalignment && alignment > 1 ? alignment - start_misalignment : 0;
      auto aligned_data = free_iter->data + alignment_offset;
      auto aligned_size = free_iter->size > alignment_offset ? free_iter->size - alignment_offset : 0;

      if (aligned_size < size)
      {
        continue;
      }

      auto buffer = ludo::buffer
      {
        .data = aligned_data,
        .size = size
      };

      if (aligned_size == size)
      {
        heap.free.erase(free_iter);
      }
      else
      {
        free_iter->data += size + alignment_offset;
        free_iter->size -= size + alignment_offset;
      }

      if (alignment_offset)
      {
        heap.free.push_back(ludo::buffer
        {
          .data = start,
          .size = alignment_offset
        });
      }

      sort_free(heap);

      return buffer;
    }

    assert(false && "could not fit buffer");
    return {};
  }

  void deallocate(heap& heap, ludo::buffer& buffer)
  {
    auto free_before_iter = std::find_if(
      heap.free.begin(),
      heap.free.end(),
      [&buffer](const ludo::buffer& free)
      {
        return free.data + free.size == buffer.data;
      }
    );

    auto free_after_iter = std::find_if(
      heap.free.begin(),
      heap.free.end(),
      [&buffer](const ludo::buffer& free)
      {
        return free.data == buffer.data + buffer.size;
      }
    );

    if (free_before_iter != heap.free.end() && free_after_iter != heap.free.end())
    {
      free_before_iter->size += buffer.size + free_after_iter->size;
      heap.free.erase(free_after_iter);
    }
    else if (free_before_iter != heap.free.end())
    {
      free_before_iter->size += buffer.size;
    }
    else if (free_after_iter != heap.free.end())
    {
      free_after_iter->data = buffer.data;
      free_after_iter->size += buffer.size;
    }
    else
    {
      heap.free.emplace_back(buffer);
    }

    buffer.data = nullptr;
    buffer.size = 0;

    sort_free(heap);
  }

  void clear(heap& heap)
  {
    heap.free = { { .data = heap.data, .size = heap.size } };
  }

  void sort_free(heap& heap)
  {
    // Smallest free sections first so that allocations are within the smallest free section that will fit.
    std::sort(
      heap.free.begin(),
      heap.free.end(),
      [](const ludo::buffer& a, const ludo::buffer& b)
      {
        return a.size < b.size;
      }
    );
  }
}
