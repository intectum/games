/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <vector>

#include "arrays.h"

namespace ludo
{
  ///
  /// A (very basically managed) heap from which buffers can be allocated.
  struct heap
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
  heap allocate_heap(uint64_t size);

  ///
  /// Allocates a heap in VRAM.
  /// \param size The size (in bytes).
  /// \param access_hint The type of access desired.
  /// \return The heap buffer.
  heap allocate_heap_vram(uint64_t size, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates a heap.
  /// \param buffer The heap to deallocate.
  void deallocate(heap& heap);

  ///
  /// Deallocates a heap from VRAM.
  /// \param buffer The heap to deallocate.
  void deallocate_vram(heap& heap);

  ///
  /// Allocates a buffer from a heap.
  /// \param heap The heap to allocate from.
  /// \param size The size (in bytes) to allocate.
  /// \param alignment The alignment (in bytes) of the allocation.
  /// \return The buffer.
  buffer allocate(heap& heap, uint64_t size, uint8_t alignment = 1);

  ///
  /// Deallocates a buffer from a heap.
  /// \param heap The heap to deallocate from.
  /// \param buffer The buffer to deallocate.
  void deallocate(heap& heap, ludo::buffer& buffer);

  ///
  /// Deallocates an array from a heap.
  /// \param heap The heap to deallocate from.
  /// \param array The array to deallocate.
  template<typename T>
  void deallocate(heap& heap, array<T>& array);

  ///
  /// Deallocates a partitioned array from a heap.
  /// \param heap The heap to deallocate from.
  /// \param array The partitioned array to deallocate.
  template<typename T>
  void deallocate(heap& heap, partitioned_array<T>& array);

  ///
  /// Removes all allocations from a heap.
  /// \param heap The heap to remove all allocations from.
  void clear(heap& heap);
}

#include "heaps.hpp"
