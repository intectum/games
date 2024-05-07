/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_DATA_ARRAYS_H
#define LUDO_DATA_ARRAYS_H

#include <vector>

#include "buffers.h"

namespace ludo
{
  ///
  /// An array with fixed capacity but variable length within that capacity.
  template<typename T>
  struct LUDO_API array
  {
    uint64_t id = 0; ///< The ID of the array (arrays allocated in VRAM may have overlapping IDs with arrays allocated in RAM).
    T* data = nullptr; ///< The data.
    uint32_t capacity = 0; ///< The maximum number of elements.
    uint32_t length = 0; ///< The current number of elements.

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
  /// A array that maintains named "partitions". Partitions are just named sections of the array.
  template<typename T>
  struct LUDO_API partitioned_array : public array<T>
  {
    std::vector<std::pair<std::string, array<T>>> partitions; ///< The partitions within the array.
  };

  ///
  /// Allocates an array.
  /// \param capacity The maximum number of elements.
  /// \return The array.
  template<typename T>
  LUDO_API array<T> allocate_array(uint64_t capacity);

  ///
  /// Allocates an array in VRAM.
  /// \param capacity The maximum number of elements.
  /// \param access_hint The type of access desired.
  /// \return The array.
  template<typename T>
  LUDO_API array<T> allocate_array_vram(uint64_t capacity, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates an array.
  /// \param array The array to deallocate.
  template<typename T>
  LUDO_API void deallocate(array<T>& array);

  ///
  /// Deallocates an array from VRAM.
  /// \param buffer The array to deallocate.
  template<typename T>
  LUDO_API void deallocate_vram(array<T>& array);

  ///
  /// Adds an element to the end of an array.
  /// \param array The array to add the element to.
  /// \param init The initial state of the new element.
  template<typename T>
  LUDO_API T* add(array<T>& array, const T& init);

  ///
  /// Removes an element from an array.
  /// \param array The array to remove the element from.
  /// \param element The element to be removed.
  template<typename T>
  LUDO_API void remove(array<T>& array, T* element);

  ///
  /// Removes all elements from an array.
  /// \param array The array to remove all elements from.
  template<typename T>
  LUDO_API void clear(array<T>& array);

  ///
  /// Allocates a partitioned array.
  /// \param capacity The maximum number of elements.
  /// \return The partitioned array.
  template<typename T>
  LUDO_API partitioned_array<T> allocate_partitioned_array(uint64_t capacity);

  ///
  /// Allocates a partitioned array in VRAM.
  /// \param capacity The maximum number of elements.
  /// \param access_hint The type of access desired.
  /// \return The partitioned array.
  template<typename T>
  LUDO_API partitioned_array<T> allocate_partitioned_array_vram(uint64_t capacity, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates a partitioned array.
  /// \param array The partitioned array to deallocate.
  template<typename T>
  LUDO_API void deallocate(partitioned_array<T>& array);

  ///
  /// Deallocates a partitioned array from VRAM.
  /// \param buffer The partitioned array to deallocate.
  template<typename T>
  LUDO_API void deallocate_vram(partitioned_array<T>& array);

  ///
  /// Adds an element to the end of a partition within a partitioned array.
  /// \param array The partitioned array to add the element to.
  /// \param init The initial state of the new element.
  /// \param partition The name of the partition.
  template<typename T>
  LUDO_API T* add(partitioned_array<T>& array, const T& init, const std::string& partition = "default");

  ///
  /// Removes an element from a partition within a partitioned array.
  /// \param array The partitioned array to remove the element from.
  /// \param element The element to be removed.
  /// \param partition The name of the partition.
  template<typename T>
  LUDO_API void remove(partitioned_array<T>& array, T* element, const std::string& partition = "default");

  ///
  /// Removes all elements from a partitioned array.
  /// \param array The partitioned array to remove all elements from.
  template<typename T>
  LUDO_API void clear(partitioned_array<T>& array);

  ///
  /// Finds a partition within a partitioned array.
  /// \param array The partitioned array containing the partition.
  /// \param partition The name of the partition.
  template<typename T>
  LUDO_API typename std::vector<std::pair<std::string, array<T>>>::iterator find(partitioned_array<T>& array, const std::string& partition);
  template<typename T>
  LUDO_API typename std::vector<std::pair<std::string, array<T>>>::const_iterator find(const partitioned_array<T>& array, const std::string& partition);

  ///
  /// Finds a partition within a partitioned array. If the partition does not exist, it creates it.
  /// \param array The partitioned array containing the partition.
  /// \param partition The name of the partition.
  template<typename T>
  LUDO_API typename std::vector<std::pair<std::string, array<T>>>::iterator find_or_create(partitioned_array<T>& array, const std::string& partition);
}

#include "arrays.hpp"

#endif // LUDO_DATA_ARRAYS_H
