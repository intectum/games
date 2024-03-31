/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_BUFFERS_H
#define LUDO_BUFFERS_H

#include <vector>

#include "core.h"

namespace ludo
{
  ///
  /// A buffer
  struct LUDO_API buffer
  {
    uint64_t id = 0; ///< The ID of the buffer (buffers allocated in VRAM may have overlapping IDs with buffers not allocated in VRAM).
    std::byte* data = nullptr; ///< The data.
    uint64_t size = 0; ///< The size of the buffer.
  };

  ///
  /// A buffer from which other buffers can be allocated.
  struct LUDO_API heap_buffer : public buffer
  {
    std::vector<buffer> free; ///< The 'free' sections of the heap.
  };

  ///
  /// An "array" buffer that keeps the elements in a contiguous block of memory.
  template<typename T>
  struct LUDO_API array_buffer : public buffer
  {
    uint64_t array_size = 0; ///< The number of elements in this buffer.

    T& operator[](uint64_t index);
    const T& operator[](uint64_t index) const;

    T* begin();
    const T* begin() const;
    const T* cbegin() const;

    T* end();
    const T* end() const;
    const T* cend() const;
  };

  ///
  /// A buffer that maintains named "partitions". Partitions are just named sections of the array.
  template<typename T>
  struct LUDO_API partitioned_buffer : public array_buffer<T>
  {
    std::vector<std::pair<std::string, array_buffer<T>>> partitions; ///< The partitions within the buffer.
  };

  ///
  /// The type of access provided by a VRAM buffer.
  enum class vram_buffer_access_hint
  {
    READ,
    WRITE,
    READ_WRITE
  };

  ///
  /// Allocates a buffer.
  /// \param size The size of the buffer.
  /// \return The buffer.
  LUDO_API buffer allocate(uint64_t size);

  ///
  /// Allocates a buffer in VRAM.
  /// \param size The size of the buffer.
  /// \param access_hint The type of access provided by the buffer.
  /// \return The buffer.
  LUDO_API buffer allocate_vram(uint64_t size, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates a buffer.
  /// \param buffer The buffer to deallocate.
  LUDO_API void deallocate(buffer& buffer);

  ///
  /// Deallocates a buffer from VRAM.
  /// \param buffer The buffer to deallocate.
  LUDO_API void deallocate_vram(buffer& buffer);

  ///
  /// Reads a value from a buffer.
  /// \param buffer The buffer to read from.
  /// \param position The position in the buffer to read from.
  /// \return The value.
  template<typename T>
  LUDO_API T read(const buffer& buffer, uint64_t position);

  ///
  /// Reads bytes from a buffer.
  /// \param buffer The buffer to read from.
  /// \param position The position in the buffer to read from.
  /// \param value The bytes in which to store the data.
  /// \param size The number of bytes to read.
  LUDO_API void read(const buffer& buffer, uint64_t position, std::byte* value, uint32_t size);

  ///
  /// Writes a value to a buffer.
  /// \param buffer The buffer to write to.
  /// \param position The position in the buffer to write to.
  /// \param value The value to write.
  template<typename T>
  LUDO_API void write(buffer& buffer, uint64_t position, const T& value);

  ///
  /// Writes bytes to a buffer.
  /// \param buffer The buffer to write to.
  /// \param position The position in the buffer to write to.
  /// \param value The bytes to write.
  /// \param size The number of bytes to write.
  LUDO_API void write(const buffer& buffer, uint64_t position, std::byte* value, uint32_t size);

  ///
  /// Allocates a heap buffer.
  /// \param size The size of the heap buffer.
  /// \return The heap buffer.
  LUDO_API heap_buffer allocate_heap(uint64_t size);

  ///
  /// Allocates a heap buffer in VRAM.
  /// \param size The size of the heap buffer.
  /// \param access_hint The type of access provided by the heap buffer.
  /// \return The heap buffer.
  LUDO_API heap_buffer allocate_heap_vram(uint64_t size, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates a heap buffer.
  /// \param buffer The heap buffer to deallocate.
  LUDO_API void deallocate(heap_buffer& buffer);

  ///
  /// Deallocates a heap buffer from VRAM.
  /// \param buffer The heap buffer to deallocate.
  LUDO_API void deallocate_vram(heap_buffer& buffer);

  ///
  /// Allocates a buffer within a heap buffer.
  /// \param buffer The heap buffer to allocate a buffer within.
  /// \param size The size of the buffer to allocate.
  /// \param byte_alignment The byte alignment within the heap buffer to allocate the buffer at.
  /// \return The buffer.
  LUDO_API buffer allocate(heap_buffer& buffer, uint64_t size, uint8_t byte_alignment = 1);

  ///
  /// Allocates an array buffer within a heap buffer.
  /// \param buffer The heap buffer to allocate an array buffer within.
  /// \param capacity The maximum number of elements that can be contained by the array buffer.
  /// \param byte_alignment The byte alignment within the heap buffer to allocate the array buffer at.
  /// \return The array buffer.
  template<typename T>
  LUDO_API array_buffer<T> allocate_array(heap_buffer& buffer, uint64_t capacity, uint8_t byte_alignment = 1);

  ///
  /// Allocates a partitioned buffer within a heap buffer.
  /// \param buffer The heap buffer to allocate a partitioned buffer within.
  /// \param capacity The maximum number of elements that can be contained by the partitioned buffer.
  /// \param byte_alignment The byte alignment within the heap buffer to allocate the partitioned buffer at.
  /// \return The partitioned buffer.
  template<typename T>
  LUDO_API partitioned_buffer<T> allocate_partitioned(heap_buffer& buffer, uint64_t capacity, uint8_t byte_alignment = 1);

  ///
  /// Deallocates a buffer from within a heap buffer.
  /// \param buffer The heap buffer to deallocate a buffer from.
  /// \param child_buffer The buffer to deallocate.
  LUDO_API void deallocate(heap_buffer& buffer, ludo::buffer& child_buffer);

  ///
  /// Deallocates an array buffer from within a heap buffer.
  /// \param buffer The heap buffer to deallocate an array buffer from.
  /// \param child_buffer The array buffer to deallocate.
  template<typename T>
  LUDO_API void deallocate(heap_buffer& buffer, array_buffer<T>& child_buffer);

  ///
  /// Deallocates a partitioned buffer from within a heap buffer.
  /// \param buffer The heap buffer to deallocate a partitioned buffer from.
  /// \param child_buffer The partitioned buffer to deallocate.
  template<typename T>
  LUDO_API void deallocate(heap_buffer& buffer, partitioned_buffer<T>& child_buffer);

  ///
  /// Removes all allocations from a heap buffer.
  /// \param buffer The buffer to remove all allocations from.
  LUDO_API void clear(heap_buffer& buffer);

  ///
  /// Allocates an array buffer.
  /// \param capacity The maximum number of elements that can be contained by the buffer.
  /// \return The array buffer.
  template<typename T>
  LUDO_API array_buffer<T> allocate_array(uint64_t capacity);

  ///
  /// Allocates an array buffer in VRAM.
  /// \param size The size of the array buffer.
  /// \param access_hint The type of access provided by the array buffer.
  /// \return The array buffer.
  template<typename T>
  LUDO_API array_buffer<T> allocate_array_vram(uint64_t capacity, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates an array buffer.
  /// \param buffer The buffer to deallocate.
  template<typename T>
  LUDO_API void deallocate(array_buffer<T>& buffer);

  ///
  /// Deallocates an array buffer from VRAM.
  /// \param buffer The array buffer to deallocate.
  template<typename T>
  LUDO_API void deallocate_vram(array_buffer<T>& buffer);

  ///
  /// Adds an element to the end of an array buffer.
  /// \param buffer The buffer to add the element to.
  /// \param init The initial state of the new element.
  /// \return A pointer to the new element. This pointer is not guaranteed to remain valid after subsequent removals.
  template<typename T>
  LUDO_API T* add(array_buffer<T>& buffer, const T& init);

  ///
  /// Removes an element from an array buffer.
  /// \param buffer The buffer to remove the element from.
  /// \param element A pointer to the element to be removed.
  template<typename T>
  LUDO_API void remove(array_buffer<T>& buffer, T* element);

  ///
  /// Removes all elements from an array buffer.
  /// \param buffer The buffer to remove all elements from.
  template<typename T>
  LUDO_API void clear(array_buffer<T>& buffer);

  ///
  /// Allocates a partitioned buffer.
  /// \param capacity The maximum number of elements that can be contained by the buffer.
  /// \return The partitioned buffer.
  template<typename T>
  LUDO_API partitioned_buffer<T> allocate_partitioned(uint64_t capacity);

  ///
  /// Allocates a partitioned buffer in VRAM.
  /// \param size The size of the partitioned buffer.
  /// \param access_hint The type of access provided by the partitioned buffer.
  /// \return The partitioned buffer.
  template<typename T>
  LUDO_API partitioned_buffer<T> allocate_partitioned_vram(uint64_t capacity, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates a partitioned buffer.
  /// \param buffer The buffer to deallocate.
  template<typename T>
  LUDO_API void deallocate(partitioned_buffer<T>& buffer);

  ///
  /// Deallocates a partitioned buffer from VRAM.
  /// \param buffer The partitioned buffer to deallocate.
  template<typename T>
  LUDO_API void deallocate_vram(partitioned_buffer<T>& buffer);

  ///
  /// Adds an element to the end of a partition within a partitioned buffer.
  /// \param buffer The buffer to add the element to.
  /// \param init The initial state of the new element.
  /// \param partition The name of the partition.
  /// \return A pointer to the new element. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  template<typename T>
  LUDO_API T* add(partitioned_buffer<T>& buffer, const T& init, const std::string& partition = "default");

  ///
  /// Removes an element from a partition within a partitioned buffer.
  /// \param buffer The buffer to remove the element from.
  /// \param element A pointer to the element to be removed (within the partition).
  /// \param partition The name of the partition.
  template<typename T>
  LUDO_API void remove(partitioned_buffer<T>& buffer, T* element, const std::string& partition = "default");

  ///
  /// Removes all elements from a partitioned buffer.
  /// \param buffer The buffer to remove all elements from.
  template<typename T>
  LUDO_API void clear(partitioned_buffer<T>& buffer);

  ///
  /// Finds the partition with the given name in a partitioned buffer.
  /// \param buffer The buffer containing the partition.
  /// \param partition The name of the partition.
  template<typename T>
  LUDO_API typename std::vector<std::pair<std::string, array_buffer<T>>>::iterator find(partitioned_buffer<T>& buffer, const std::string& partition);
  template<typename T>
  LUDO_API typename std::vector<std::pair<std::string, array_buffer<T>>>::const_iterator find(const partitioned_buffer<T>& buffer, const std::string& partition);

  ///
  /// Finds the partition with the given name in a partitioned buffer. If the partition does not exist, it creates it.
  /// \param buffer The buffer containing the partition.
  /// \param partition The name of the partition.
  template<typename T>
  LUDO_API typename std::vector<std::pair<std::string, array_buffer<T>>>::iterator find_or_create(partitioned_buffer<T>& buffer, const std::string& partition);
}

#include "buffers.hpp"

#endif // LUDO_BUFFERS_H
