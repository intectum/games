/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_DATA_H
#define LUDO_DATA_H

#include "buffers.h"
#include "core.h"

namespace ludo
{
  ///
  /// Allocates capacity for a particular type of data within an instance.
  /// \param instance The instance to allocate capacity within.
  /// \param capacity The maximum number of elements that can be contained within the instance.
  /// \return The allocated buffer.
  template<typename T>
  LUDO_API partitioned_buffer<T>& allocate(instance& instance, uint64_t capacity);

  ///
  /// Allocates capacity for a particular type of data within an instance in VRAM.
  /// \param instance The instance to allocate capacity within.
  /// \param capacity The maximum number of elements that can be contained within the instance.
  /// \param access_hint The type of access provided by the buffer.
  /// \return The allocated buffer.
  template<typename T>
  LUDO_API partitioned_buffer<T>& allocate_vram(instance& instance, uint64_t capacity, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates capacity for a particular type of data within an instance.
  /// \param instance The instance to deallocate capacity from.
  template<typename T>
  LUDO_API void deallocate(instance& instance);

  ///
  /// Deallocates capacity for a particular type of data within an instance from VRAM.
  /// \param instance The instance to deallocate capacity from.
  template<typename T>
  LUDO_API void deallocate_vram(instance& instance);

  ///
  /// Retrieves the buffer for a particular type of data within an instance.
  /// \param instance The instance containing the buffer.
  /// \param partition The name of the partition.
  /// \return The buffer.
  template<typename T>
  LUDO_API partitioned_buffer<T>& data(instance& instance);
  template<typename T>
  LUDO_API const partitioned_buffer<T>& data(const instance& instance);
  template<typename T>
  LUDO_API array_buffer<T>& data(instance& instance, const std::string& partition);
  template<typename T>
  LUDO_API const array_buffer<T>& data(const instance& instance, const std::string& partition);

  ///
  /// Determines if a buffer for a particular type of data exists within an instance.
  /// \param instance The instance to search.
  /// \param partition The name of the partition.
  /// \return True if the buffer exists, false otherwise.
  template<typename T>
  LUDO_API bool exists(const instance& instance);
  template<typename T>
  LUDO_API bool exists(const instance& instance, const std::string& partition);

  ///
  /// Retrieves the first element of a particular type of data within an instance.
  /// \param instance The instance containing the data.
  /// \param partition The name of the partition.
  /// \return The first element or nullptr if not found.
  template<typename T>
  LUDO_API T* first(instance& instance);
  template<typename T>
  LUDO_API const T* first(const instance& instance);
  template<typename T>
  LUDO_API T* first(instance& instance, const std::string& partition);
  template<typename T>
  LUDO_API const T* first(const instance& instance, const std::string& partition);

  ///
  /// Retrieves an element of a particular type of data within an instance.
  /// \param instance The instance containing the data.
  /// \param partition The name of the partition.
  /// \param id The ID of the element.
  /// \return The first element or nullptr if not found.
  template<typename T>
  LUDO_API T* get(instance& instance, uint64_t id);
  template<typename T>
  LUDO_API const T* get(const instance& instance, uint64_t id);
  template<typename T>
  LUDO_API T* get(instance& instance, const std::string& partition, uint64_t id);
  template<typename T>
  LUDO_API const T* get(const instance& instance, const std::string& partition, uint64_t id);

  ///
  /// Adds an element to the data of an instance.
  /// \param instance The instance to add the element to.
  /// \param init The initial state of the new element.
  /// \param partition The name of the partition.
  /// \return A pointer to the new element. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  template<typename T>
  LUDO_API T* add(instance& instance, const T& init, const std::string& partition = "default");

  ///
  /// Removes an element from the data of an instance.
  /// \param instance The instance to remove the element from.
  /// \param element The element to be removed (within the partition).
  /// \param partition The name of the partition.
  template<typename T>
  LUDO_API void remove(instance& instance, T* element, const std::string& partition);

  ///
  /// Allocates capacity for a heap of a particular type of data within an instance.
  /// \param instance The instance to allocate capacity within.
  /// \param capacity The maximum number of elements that can be contained within the heap.
  /// \return The allocated buffer.
  template<typename T>
  LUDO_API heap_buffer& allocate_heap(instance& instance, uint64_t capacity);

  ///
  /// Allocates capacity for a heap of a particular type of data within an instance in VRAM.
  /// \param instance The instance to allocate capacity within.
  /// \param capacity The maximum number of elements that can be contained within the heap.
  /// \param access_hint The type of access provided by the buffer.
  /// \return The allocated buffer.
  template<typename T>
  LUDO_API heap_buffer& allocate_heap_vram(instance& instance, uint64_t capacity, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates capacity for a heap of a particular type of data within an instance.
  /// \param instance The instance to deallocate capacity from.
  template<typename T>
  LUDO_API void deallocate_heap(instance& instance);

  ///
  /// Deallocates capacity for a heap of a particular type of data within an instance from VRAM.
  /// \param instance The instance to deallocate capacity from.
  template<typename T>
  LUDO_API void deallocate_heap_vram(instance& instance);

  ///
  /// Retrieves the heap buffer for a particular type of data within an instance.
  /// \param instance The instance containing the heap buffer.
  /// \return The heap buffer.
  template<typename T>
  LUDO_API heap_buffer& data_heap(instance& instance);
  template<typename T>
  LUDO_API const heap_buffer& data_heap(const instance& instance);

  template<typename T>
  LUDO_API std::string partitioned_buffer_key();

  template<typename T>
  LUDO_API std::string heap_buffer_key();
}

#include "data.hpp"

#endif // LUDO_DATA_H
