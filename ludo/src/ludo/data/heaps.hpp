/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "heaps.h"

namespace ludo
{
  template<typename T>
  array<T> allocate_array(heap& heap, uint64_t capacity)
  {
    auto buffer = allocate(heap, capacity * sizeof(T));

    auto array = ludo::array<T>();
    array.data = buffer.data;
    array.capacity = capacity;

    return array;
  }

  template<typename T>
  partitioned_array<T> allocate_partitioned_array(heap& heap, uint64_t capacity)
  {
    auto buffer = allocate(heap, capacity * sizeof(T));

    auto array = partitioned_array<T>();
    array.data = buffer.data;
    array.capacity = capacity;

    return array;
  }

  template<typename T>
  void deallocate(heap& heap, array<T>& array)
  {
    auto buffer = ludo::buffer
    {
      .id = array.id,
      .data = array.data,
      .size = array.capacity * sizeof(T)
    };

    deallocate(heap, buffer);

    array.data = nullptr;
    array.capacity = 0;
    array.length = 0;
  }

  template<typename T>
  void deallocate(heap& heap, partitioned_array<T>& array)
  {
    auto buffer = ludo::buffer
    {
      .id = array.id,
      .data = array.data,
      .size = array.capacity * sizeof(T)
    };

    deallocate(heap, buffer);

    array.data = nullptr;
    array.capacity = 0;
    array.length = 0;
  }
}
