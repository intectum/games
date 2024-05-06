/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <cstring>

#include "arrays.h"

namespace ludo
{
  template<typename T>
  T& array<T>::operator[](uint64_t index)
  {
    assert(index >= 0 && index < length && "index out of range");

    return data[index];
  }

  template<typename T>
  const T& array<T>::operator[](uint64_t index) const
  {
    assert(index >= 0 && index < length && "index out of range");

    return data[index];
  }

  template<typename T>
  T* array<T>::begin()
  {
    return data;
  }

  template<typename T>
  const T* array<T>::begin() const
  {
    return data;
  }

  template<typename T>
  const T* array<T>::cbegin() const
  {
    return data;
  }

  template<typename T>
  T* array<T>::end()
  {
    return data + length;
  }

  template<typename T>
  const T* array<T>::end() const
  {
    return data + length;
  }

  template<typename T>
  const T* array<T>::cend() const
  {
    return data + length;
  }

  template<typename T>
  array<T> allocate_array(uint64_t capacity)
  {
    auto buffer = allocate(capacity * sizeof(T));

    auto array = ludo::array<T>();
    array.id = buffer.id;
    array.data = reinterpret_cast<T*>(buffer.data);
    array.capacity = capacity;

    return array;
  }

  template<typename T>
  array<T> allocate_array_vram(uint64_t capacity, vram_buffer_access_hint access_hint)
  {
    auto buffer = allocate_vram(capacity * sizeof(T), access_hint);

    auto array = ludo::array<T>();
    array.id = buffer.id;
    array.data = reinterpret_cast<T*>(buffer.data);
    array.capacity = capacity;

    return array;
  }

  template<typename T>
  void deallocate(array<T>& array)
  {
    auto buffer = ludo::buffer
    {
      .id = array.id,
      .data = reinterpret_cast<std::byte*>(array.data),
      .size = array.capacity * sizeof(T)
    };

    deallocate(buffer);

    array.data = nullptr;
    array.capacity = 0;
    array.length = 0;
  }

  template<typename T>
  void deallocate_vram(array<T>& array)
  {
    auto buffer = ludo::buffer
    {
      .id = array.id,
      .data = reinterpret_cast<std::byte*>(array.data),
      .size = array.capacity * sizeof(T)
    };

    deallocate_vram(buffer);

    array.data = nullptr;
    array.capacity = 0;
    array.length = 0;
  }

  template<typename T>
  T* add(array<T>& array, const T& init)
  {
    assert(array.capacity && "array not allocated (partitions of partitioned arrays cannot be modified directly)");
    assert(array.length < array.capacity && "array is full");

    auto element = array.data + array.length;
    std::uninitialized_copy(&init, &init + 1, element); // TODO is this the best idea?

    array.length++;

    return element;
  }

  template<typename T>
  void remove(array<T>& array, T* element)
  {
    assert(array.capacity && "array not allocated (partitions of partitioned arrays cannot be modified directly)");
    assert(element >= array.data && element < array.data + array.length && "element out of range");

    std::memmove(element, element + 1, (array.data + array.length - (element + 1)) * sizeof(T));

    array.length--;
  }

  template<typename T>
  void clear(array<T>& array)
  {
    assert(array.capacity && "array not allocated (partitions of partitioned arrays cannot be modified directly)");

    array.length = 0;
  }

  template<typename T>
  partitioned_array<T> allocate_partitioned_array(uint64_t capacity)
  {
    auto buffer = allocate(capacity * sizeof(T));

    auto array = partitioned_array<T>();
    array.id = buffer.id;
    array.data = reinterpret_cast<T*>(buffer.data);
    array.capacity = capacity;

    return array;
  }

  template<typename T>
  partitioned_array<T> allocate_partitioned_array_vram(uint64_t capacity, vram_buffer_access_hint access_hint)
  {
    auto buffer = allocate_vram(capacity * sizeof(T), access_hint);

    auto array = partitioned_array<T>();
    array.id = buffer.id;
    array.data = reinterpret_cast<T*>(buffer.data);
    array.capacity = capacity;

    return array;
  }

  template<typename T>
  void deallocate(partitioned_array<T>& array)
  {
    auto buffer = ludo::buffer
    {
      .id = array.id,
      .data = reinterpret_cast<std::byte*>(array.data),
      .size = array.capacity * sizeof(T)
    };

    deallocate(buffer);

    array.data = nullptr;
    array.capacity = 0;
    array.length = 0;
    array.partitions.clear();
  }

  template<typename T>
  void deallocate_vram(partitioned_array<T>& array)
  {
    auto buffer = ludo::buffer
    {
      .id = array.id,
      .data = reinterpret_cast<std::byte*>(array.data),
      .size = array.capacity * sizeof(T)
    };

    deallocate_vram(buffer);

    array.data = nullptr;
    array.capacity = 0;
    array.length = 0;
    array.partitions.clear();
  }

  template<typename T>
  T* add(partitioned_array<T>& array, const T& init, const std::string& partition)
  {
    assert(array.length < array.capacity && "array is full");

    auto partition_iter = find_or_create(array, partition);

    std::memmove(partition_iter->second.end() + 1, partition_iter->second.end(), (array.end() - partition_iter->second.end()) * sizeof(T));

    auto element = partition_iter->second.end();
    std::uninitialized_copy(&init, &init + 1, element);

    partition_iter->second.length++;

    std::for_each(partition_iter + 1, array.partitions.end(), [](std::pair<std::string, ludo::array<T>>& element)
    {
      element.second.data++;
    });

    array.length++;

    return element;
  }

  template<typename T>
  void remove(partitioned_array<T>& array, T* element, const std::string& partition)
  {
    auto partition_iter = find(array, partition);
    if (partition_iter == array.partitions.end())
    {
      return;
    }

    assert(element >= partition_iter->second.begin() && element < partition_iter->second.end() && "element out of range");

    std::memmove(element, element + 1, (array.data + array.length - (element + 1)) * sizeof(T));

    partition_iter->second.length--;

    std::for_each(partition_iter + 1, array.partitions.end(), [](std::pair<std::string, ludo::array<T>>& element)
    {
      element.second.data--;
    });

    array.length--;
  }

  template<typename T>
  void clear(partitioned_array<T>& array)
  {
    array.length = 0;
    array.partitions.clear();
  }

  template<typename T>
  typename std::vector<std::pair<std::string, array<T>>>::iterator find(partitioned_array<T>& array, const std::string& partition)
  {
    return std::find_if(array.partitions.begin(), array.partitions.end(), [&partition](const std::pair<std::string, ludo::array<T>>& element)
    {
      return element.first == partition;
    });
  }

  template<typename T>
  typename std::vector<std::pair<std::string, array<T>>>::const_iterator find(const partitioned_array<T>& array, const std::string& partition)
  {
    return std::find_if(array.partitions.begin(), array.partitions.end(), [&partition](const std::pair<std::string, ludo::array<T>>& element)
    {
      return element.first == partition;
    });
  }

  template<typename T>
  typename std::vector<std::pair<std::string, array<T>>>::iterator find_or_create(partitioned_array<T>& array, const std::string& partition)
  {
    auto partition_iter = find(array, partition);
    if (partition_iter == array.partitions.end())
    {
      auto partition_array = ludo::array<T>();
      partition_array.data = array.data + array.length;

      array.partitions.emplace_back(std::pair<std::string, ludo::array<T>> { partition, partition_array });

      return array.partitions.end() - 1;
    }

    return partition_iter;
  }
}
