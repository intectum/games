/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <algorithm>
#include <cassert>
#include <cstring>

#include "buffers.h"

namespace ludo
{
  template<typename T>
  T read(const buffer& buffer, uint64_t position)
  {
    assert(position >= 0 && position <= buffer.size - sizeof(T) && "position out of range");

    return *reinterpret_cast<T*>(buffer.data + position);
  }

  template<typename T>
  void write(buffer& buffer, uint64_t position, const T& value)
  {
    assert(position >= 0 && position <= buffer.size - sizeof(T) && "position out of range");

    *reinterpret_cast<T*>(buffer.data + position) = value;
  }

  template<typename T>
  array_buffer<T> allocate_array(heap_buffer& buffer, uint64_t capacity)
  {
    auto simple_child_buffer = allocate(buffer, capacity * sizeof(T));

    auto child_buffer = array_buffer<T>();
    child_buffer.data = simple_child_buffer.data;
    child_buffer.size = simple_child_buffer.size;

    return child_buffer;
  }

  template<typename T>
  partitioned_buffer<T> allocate_partitioned(heap_buffer& buffer, uint64_t capacity)
  {
    auto simple_child_buffer = allocate(buffer, capacity * sizeof(T));

    auto child_buffer = array_buffer<T>();
    child_buffer.data = simple_child_buffer.data;
    child_buffer.size = simple_child_buffer.size;

    return child_buffer;
  }

  template<typename T>
  void deallocate(heap_buffer& buffer, array_buffer<T>& child_buffer)
  {
    auto child_simple_buffer = ludo::buffer
    {
      .id = buffer.id,
      .data = child_buffer.data,
      .size = child_buffer.size
    };

    deallocate(buffer, child_simple_buffer);

    child_buffer.data = nullptr;
    child_buffer.size = 0;
    child_buffer.array_size = 0;
  }

  template<typename T>
  void deallocate(heap_buffer& buffer, partitioned_buffer<T>& child_buffer)
  {
    auto child_simple_buffer = ludo::buffer
    {
      .id = buffer.id,
      .data = child_buffer.data,
      .size = child_buffer.size
    };

    deallocate(buffer, child_simple_buffer);

    child_buffer.data = nullptr;
    child_buffer.size = 0;
    child_buffer.array_size = 0;
    child_buffer.partitions.clear();
  }

  template<typename T>
  T& array_buffer<T>::operator[](uint64_t index)
  {
    assert((index >= 0 && index < array_size) && "index out of range");

    return begin()[index];
  }

  template<typename T>
  const T& array_buffer<T>::operator[](uint64_t index) const
  {
    assert((index >= 0 && index < array_size) && "index out of range");

    return begin()[index];
  }

  template<typename T>
  T* array_buffer<T>::begin()
  {
    return reinterpret_cast<T*>(data);
  }

  template<typename T>
  const T* array_buffer<T>::begin() const
  {
    return reinterpret_cast<T*>(data);
  }

  template<typename T>
  const T* array_buffer<T>::cbegin() const
  {
    return reinterpret_cast<T*>(data);
  }

  template<typename T>
  T* array_buffer<T>::end()
  {
    return reinterpret_cast<T*>(data) + array_size;
  }

  template<typename T>
  const T* array_buffer<T>::end() const
  {
    return reinterpret_cast<T*>(data) + array_size;
  }

  template<typename T>
  const T* array_buffer<T>::cend() const
  {
    return reinterpret_cast<T*>(data) + array_size;
  }

  template<typename T>
  array_buffer<T> allocate_array(uint64_t capacity)
  {
    auto simple_buffer = allocate(capacity * sizeof(T));

    auto buffer = array_buffer<T>();
    buffer.id = simple_buffer.id;
    buffer.data = simple_buffer.data;
    buffer.size = simple_buffer.size;

    return buffer;
  }

  template<typename T>
  array_buffer<T> allocate_array_vram(uint64_t capacity, vram_buffer_access_hint access_hint)
  {
    auto simple_buffer = allocate_vram(capacity * sizeof(T), access_hint);

    auto buffer = array_buffer<T>();
    buffer.id = simple_buffer.id;
    buffer.data = simple_buffer.data;
    buffer.size = simple_buffer.size;

    return buffer;
  }

  template<typename T>
  void deallocate(array_buffer<T>& buffer)
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
    buffer.array_size = 0;
  }

  template<typename T>
  void deallocate_vram(array_buffer<T>& buffer)
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
    buffer.array_size = 0;
  }

  template<typename T>
  T* add(array_buffer<T>& buffer, const T& init)
  {
    assert(buffer.size && "buffer not allocated (partitions of partitioned buffers cannot be added to directly)");
    assert(buffer.array_size < buffer.size / sizeof(T) && "buffer is full");

    auto element = buffer.end();
    std::uninitialized_copy(&init, &init + 1, element);

    buffer.array_size++;

    return element;
  }

  template<typename T>
  void remove(array_buffer<T>& buffer, T* element)
  {
    assert(buffer.size && "buffer not allocated (partitions of partitioned buffers cannot be removed from directly)");
    assert((element >= buffer.begin() && element < buffer.end()) && "element out of range");

    std::memmove(element, element + 1, (buffer.end() - (element + 1)) * sizeof(T));

    buffer.array_size--;
  }

  template<typename T>
  void clear(array_buffer<T>& buffer)
  {
    assert(buffer.size && "buffer not allocated (partitions of partitioned buffers cannot be cleared directly)");

    buffer.array_size = 0;
  }

  template<typename T>
  partitioned_buffer<T> allocate_partitioned(uint64_t capacity)
  {
    auto simple_buffer = allocate(capacity * sizeof(T));

    auto buffer = partitioned_buffer<T>();
    buffer.id = simple_buffer.id;
    buffer.data = simple_buffer.data;
    buffer.size = simple_buffer.size;

    return buffer;
  }

  template<typename T>
  partitioned_buffer<T> allocate_partitioned_vram(uint64_t capacity, vram_buffer_access_hint access_hint)
  {
    auto simple_buffer = allocate_vram(capacity * sizeof(T), access_hint);

    auto buffer = partitioned_buffer<T>();
    buffer.id = simple_buffer.id;
    buffer.data = simple_buffer.data;
    buffer.size = simple_buffer.size;

    return buffer;
  }

  template<typename T>
  void deallocate(partitioned_buffer<T>& buffer)
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
    buffer.array_size = 0;
  }

  template<typename T>
  void deallocate_vram(partitioned_buffer<T>& buffer)
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
    buffer.array_size = 0;
  }

  template<typename T>
  T* add(partitioned_buffer<T>& buffer, const T& init, const std::string& partition)
  {
    assert(buffer.array_size < buffer.size / sizeof(T) && "buffer is full");

    auto partition_iter = find_or_create(buffer, partition);

    std::memmove(partition_iter->second.end() + 1, partition_iter->second.end(), (buffer.end() - partition_iter->second.end()) * sizeof(T));

    auto element = partition_iter->second.end();
    std::uninitialized_copy(&init, &init + 1, element);

    partition_iter->second.array_size++;

    std::for_each(partition_iter + 1, buffer.partitions.end(), [](std::pair<std::string, array_buffer<T>>& element)
    {
      auto cast_data = reinterpret_cast<T*>(element.second.data);
      cast_data++;

      element.second.data = reinterpret_cast<std::byte*>(cast_data);
    });

    buffer.array_size++;

    return element;
  }

  template<typename T>
  void remove(partitioned_buffer<T>& buffer, T* element, const std::string& partition)
  {
    auto partition_iter = find(buffer, partition);
    if (partition_iter == buffer.partitions.end())
    {
      return;
    }

    assert((element >= partition_iter->second.begin() && element < partition_iter->second.end()) && "element out of range");

    std::memmove(element, element + 1, (buffer.end() - (element + 1)) * sizeof(T));

    partition_iter->second.array_size--;

    std::for_each(partition_iter + 1, buffer.partitions.end(), [](std::pair<std::string, array_buffer<T>>& element)
    {
      auto cast_data = reinterpret_cast<T*>(element.second.data);
      cast_data--;

      element.second.data = reinterpret_cast<std::byte*>(cast_data);
    });

    buffer.array_size--;
  }

  template<typename T>
  void clear(partitioned_buffer<T>& buffer)
  {
    buffer.array_size = 0;
    buffer.partitions.clear();
  }

  template<typename T>
  typename std::vector<std::pair<std::string, array_buffer<T>>>::iterator find(partitioned_buffer<T>& buffer, const std::string& partition)
  {
    return std::find_if(buffer.partitions.begin(), buffer.partitions.end(), [&partition](const std::pair<std::string, array_buffer<T>>& element)
    {
      return element.first == partition;
    });
  }

  template<typename T>
  typename std::vector<std::pair<std::string, array_buffer<T>>>::const_iterator find(const partitioned_buffer<T>& buffer, const std::string& partition)
  {
    return std::find_if(buffer.partitions.begin(), buffer.partitions.end(), [&partition](const std::pair<std::string, array_buffer<T>>& element)
    {
      return element.first == partition;
    });
  }

  template<typename T>
  typename std::vector<std::pair<std::string, array_buffer<T>>>::iterator find_or_create(partitioned_buffer<T>& buffer, const std::string& partition)
  {
    auto partition_iter = find(buffer, partition);
    if (partition_iter == buffer.partitions.end())
    {
      auto partition_buffer = array_buffer<T>();
      partition_buffer.data = reinterpret_cast<std::byte*>(buffer.end());
      partition_buffer.size = 0;
      partition_buffer.array_size = 0;

      buffer.partitions.emplace_back(std::pair<std::string, array_buffer<T>> { partition, partition_buffer });

      return buffer.partitions.end() - 1;
    }

    return partition_iter;
  }
}
