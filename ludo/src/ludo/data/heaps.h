/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_DATA_HEAPS_H
#define LUDO_DATA_HEAPS_H

#include <vector>

#include "arrays.h"

namespace ludo
{
  ///
  /// A (very basically managed) heap from which buffers can be allocated.
  struct LUDO_API heap
  {
    uint64_t id = 0; ///< The ID of the heap (heaps allocated in VRAM may have overlapping IDs with heaps allocated in RAM).
    std::byte* data = nullptr; ///< The data.
    uint64_t size = 0; ///< The size (in bytes).
    std::vector<ludo::buffer> free; ///< The data available for allocation.
  };

  ///
  /// Allocates a heap.
  /// \param size The size (in bytes).
  /// \return The heap.
  LUDO_API heap allocate_heap(uint64_t size);

  ///
  /// Allocates a heap in VRAM.
  /// \param size The size (in bytes).
  /// \param access_hint The type of access desired.
  /// \return The heap buffer.
  LUDO_API heap allocate_heap_vram(uint64_t size, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates a heap.
  /// \param buffer The heap to deallocate.
  LUDO_API void deallocate(heap& heap);

  ///
  /// Deallocates a heap from VRAM.
  /// \param buffer The heap to deallocate.
  LUDO_API void deallocate_vram(heap& heap);

  ///
  /// Allocates a buffer from a heap.
  /// \param heap The heap to allocate from.
  /// \param size The size (in bytes) to allocate.
  /// \param alignment The alignment (in bytes) of the allocation.
  /// \return The buffer.
  LUDO_API buffer allocate(heap& heap, uint64_t size, uint8_t alignment = 1);

  ///
  /// Deallocates a buffer from a heap.
  /// \param heap The heap to deallocate from.
  /// \param buffer The buffer to deallocate.
  LUDO_API void deallocate(heap& heap, ludo::buffer& buffer);

  ///
  /// Deallocates an array from a heap.
  /// \param heap The heap to deallocate from.
  /// \param array The array to deallocate.
  template<typename T>
  LUDO_API void deallocate(heap& heap, array<T>& array);

  ///
  /// Deallocates a partitioned array from a heap.
  /// \param heap The heap to deallocate from.
  /// \param array The partitioned array to deallocate.
  template<typename T>
  LUDO_API void deallocate(heap& heap, partitioned_array<T>& array);

  ///
  /// Removes all allocations from a heap.
  /// \param heap The heap to remove all allocations from.
  LUDO_API void clear(heap& heap);
}

#include "heaps.hpp"

#endif // LUDO_DATA_HEAPS_H
