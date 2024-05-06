/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <typeinfo>

#include "../algorithm.h"
#include "data.h"

namespace ludo
{
  template<typename T>
  partitioned_array<T>& allocate(instance& instance, uint64_t capacity)
  {
    auto key = partitioned_array_key<T>();
    instance.data[key] = new partitioned_array<T>(allocate_partitioned_array<T>(capacity));

    return *static_cast<partitioned_array<T>*>(instance.data[key]);
  }

  template<typename T>
  partitioned_array<T>& allocate_vram(instance& instance, uint64_t capacity, vram_buffer_access_hint access_hint)
  {
    auto key = partitioned_array_key<T>();
    instance.data[key] = new partitioned_array<T>(allocate_partitioned_vram<T>(capacity, access_hint));

    return *static_cast<partitioned_array<T>*>(instance.data[key]);
  }

  template<typename T>
  void deallocate(instance& instance)
  {
    auto& array = data<T>(instance);
    deallocate(array);

    instance.data.erase(partitioned_array_key<T>());
    delete &array;
  }

  template<typename T>
  void deallocate_vram(instance& instance)
  {
    auto& array = data<T>(instance);
    deallocate_vram(array);

    instance.data.erase(partitioned_array_key<T>());
    delete &array;
  }

  template<typename T>
  partitioned_array<T>& data(instance& instance)
  {
    return const_cast<partitioned_array<T>&>(data<T>(const_cast<const ludo::instance&>(instance)));
  }

  template<typename T>
  const partitioned_array<T>& data(const instance& instance)
  {
    assert(instance.data.find(partitioned_array_key<T>()) != instance.data.end() && "array not found");

    return *static_cast<partitioned_array<T>*>(instance.data.at(partitioned_array_key<T>()));
  }

  template<typename T>
  array<T>& data(instance& instance, const std::string& partition)
  {
    return find_or_create(data<T>(instance), partition)->second;
  }

  template<typename T>
  const array<T>& data(const instance& instance, const std::string& partition)
  {
    auto& array = data<T>(instance);
    auto partition_iter = find(array, partition);
    assert(partition_iter != array.partitions.end() && "partition not found");

    return partition_iter->second;
  }

  template<typename T>
  bool exists(const instance& instance)
  {
    return instance.data.contains(partitioned_array_key<T>());
  }

  template<typename T>
  bool exists(const instance& instance, const std::string& partition)
  {
    if (!exists<T>(instance))
    {
      return false;
    }

    auto& array = data<T>(instance);
    auto partition_iter = find(array, partition);
    return partition_iter != array.partitions.end();
  }

  template<typename T>
  T* first(instance& instance)
  {
    return const_cast<T*>(first<T>(const_cast<const ludo::instance&>(instance)));
  }

  template<typename T>
  const T* first(const instance& instance)
  {
    auto array_iter = instance.data.find(partitioned_array_key<T>());
    auto array = array_iter == instance.data.end() ? nullptr : static_cast<partitioned_array<T>*>(array_iter->second);
    if (array && array->length)
    {
      return array->begin();
    }

    return nullptr;
  }

  template<typename T>
  T* first(instance& instance, const std::string& partition)
  {
    return const_cast<T*>(first<T>(const_cast<const ludo::instance&>(instance), partition));
  }

  template<typename T>
  const T* first(const instance& instance, const std::string& partition)
  {
    auto array_iter = instance.data.find(partitioned_array_key<T>());
    auto array = array_iter == instance.data.end() ? nullptr : static_cast<partitioned_array<T>*>(array_iter->second);
    if (array && array->length)
    {
      auto partition_iter = find(*array, partition);
      if (partition_iter != array->partitions.end() && partition_iter->second.length)
      {
        return partition_iter->second.begin();
      }
    }

    return nullptr;
  }

  template<typename T>
  T* get(instance& instance, uint64_t id)
  {
    return const_cast<T*>(get<T>(const_cast<const ludo::instance&>(instance), id));
  }

  template<typename T>
  const T* get(const instance& instance, uint64_t id)
  {
    auto& array = data<T>(instance);

    auto element = find_by_id(array.begin(), array.end(), id);
    if (element == array.end())
    {
      return nullptr;
    }

    return element;
  }

  template<typename T>
  T* get(instance& instance, const std::string& partition, uint64_t id)
  {
    return const_cast<T*>(get<T>(const_cast<const ludo::instance&>(instance), partition, id));
  }

  template<typename T>
  const T* get(const instance& instance, const std::string& partition, uint64_t id)
  {
    auto& array = data<T>(instance, partition);

    auto element = find_by_id(array.begin(), array.end(), id);
    if (element == array.end())
    {
      return nullptr;
    }

    return element;
  }

  template<typename T>
  T* add(instance& instance, const T& init, const std::string& partition)
  {
    auto element = add(data<T>(instance), init, partition);
    element->id = next_id++;

    return element;
  }

  template<typename T>
  void remove(instance& instance, T* element, const std::string& partition)
  {
    auto array_iter = instance.data.find(partitioned_array_key<T>());
    if (array_iter == instance.data.end())
    {
      return;
    }

    remove(*static_cast<partitioned_array<T>*>(array_iter->second), element, partition);
  }

  template<typename T>
  std::string partitioned_array_key()
  {
    return std::string("ludo::partitioned_array::") + typeid(T).name();
  }
}
