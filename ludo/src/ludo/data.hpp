/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <typeinfo>

#include "algorithm.h"
#include "data.h"

namespace ludo
{
  template<typename T>
  partitioned_buffer<T>& allocate(instance& instance, uint64_t capacity)
  {
    auto key = partitioned_buffer_key<T>();
    instance.data[key] = new partitioned_buffer<T>(allocate_partitioned<T>(capacity));

    return *static_cast<partitioned_buffer<T>*>(instance.data[key]);
  }

  template<typename T>
  partitioned_buffer<T>& allocate_vram(instance& instance, uint64_t capacity, vram_buffer_access_hint access_hint)
  {
    auto key = partitioned_buffer_key<T>();
    instance.data[key] = new partitioned_buffer<T>(allocate_partitioned_vram<T>(capacity, access_hint));

    return *static_cast<partitioned_buffer<T>*>(instance.data[key]);
  }

  template<typename T>
  void deallocate(instance& instance)
  {
    auto& buffer = data<T>(instance);
    deallocate(buffer);

    instance.data.erase(partitioned_buffer_key<T>());
    delete &buffer;
  }

  template<typename T>
  void deallocate_vram(instance& instance)
  {
    auto& buffer = data<T>(instance);
    deallocate_vram(buffer);

    instance.data.erase(partitioned_buffer_key<T>());
    delete &buffer;
  }

  template<typename T>
  partitioned_buffer<T>& data(instance& instance)
  {
    return const_cast<partitioned_buffer<T>&>(data<T>(const_cast<const ludo::instance&>(instance)));
  }

  template<typename T>
  const partitioned_buffer<T>& data(const instance& instance)
  {
    assert(instance.data.find(partitioned_buffer_key<T>()) != instance.data.end() && "buffer not found");

    return *static_cast<partitioned_buffer<T>*>(instance.data.at(partitioned_buffer_key<T>()));
  }

  template<typename T>
  array_buffer<T>& data(instance& instance, const std::string& partition)
  {
    return find_or_create(data<T>(instance), partition)->second;
  }

  template<typename T>
  const array_buffer<T>& data(const instance& instance, const std::string& partition)
  {
    auto& buffer = data<T>(instance);
    auto partition_iter = find(buffer, partition);
    assert(partition_iter != buffer.partitions.end() && "partition not found");

    return partition_iter->second;
  }

  template<typename T>
  bool exists(const instance& instance)
  {
    return instance.data.contains(partitioned_buffer_key<T>());
  }

  template<typename T>
  bool exists(const instance& instance, const std::string& partition)
  {
    if (!exists<T>(instance))
    {
      return false;
    }

    auto& buffer = data<T>(instance);
    auto partition_iter = find(buffer, partition);
    return partition_iter != buffer.partitions.end();
  }

  template<typename T>
  T* first(instance& instance)
  {
    return const_cast<T*>(first<T>(const_cast<const ludo::instance&>(instance)));
  }

  template<typename T>
  const T* first(const instance& instance)
  {
    auto buffer_iter = instance.data.find(partitioned_buffer_key<T>());
    auto buffer = buffer_iter == instance.data.end() ? nullptr : static_cast<partitioned_buffer<T>*>(buffer_iter->second);
    if (buffer && buffer->array_size)
    {
      return buffer->begin();
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
    auto buffer_iter = instance.data.find(partitioned_buffer_key<T>());
    auto buffer = buffer_iter == instance.data.end() ? nullptr : static_cast<partitioned_buffer<T>*>(buffer_iter->second);
    if (buffer && buffer->array_size)
    {
      auto partition_iter = find(*buffer, partition);
      if (partition_iter != buffer->partitions.end() && partition_iter->second.array_size)
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
    auto& buffer = data<T>(instance);

    auto element = find_by_id(buffer.begin(), buffer.end(), id);
    if (element == buffer.end())
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
    auto& buffer = data<T>(instance, partition);

    auto element = find_by_id(buffer.begin(), buffer.end(), id);
    if (element == buffer.end())
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
    auto buffer_iter = instance.data.find(partitioned_buffer_key<T>());
    if (buffer_iter == instance.data.end())
    {
      return;
    }

    remove(*static_cast<partitioned_buffer<T>*>(buffer_iter->second), element, partition);
  }

  template<typename T>
  heap_buffer& allocate_heap(instance& instance, uint64_t capacity)
  {
    auto key = heap_buffer_key<T>();
    instance.data[key] = new heap_buffer(allocate_heap(capacity * sizeof(T)));

    return *static_cast<heap_buffer*>(instance.data[key]);
  }

  template<typename T>
  heap_buffer& allocate_heap_vram(instance& instance, uint64_t capacity, vram_buffer_access_hint access_hint)
  {
    auto key = heap_buffer_key<T>();
    instance.data[key] = new heap_buffer(allocate_heap_vram(capacity * sizeof(T), access_hint));

    return *static_cast<heap_buffer*>(instance.data[key]);
  }

  template<typename T>
  void deallocate_heap(instance& instance)
  {
    auto& buffer = data_heap<T>(instance);
    deallocate(buffer);

    instance.data.erase(heap_buffer_key<T>());
    delete &buffer;
  }

  template<typename T>
  void deallocate_heap_vram(instance& instance)
  {
    auto& buffer = data_heap<T>(instance);
    deallocate_vram(buffer);

    instance.data.erase(heap_buffer_key<T>());
    delete &buffer;
  }

  template<typename T>
  heap_buffer& data_heap(instance& instance)
  {
    return const_cast<heap_buffer&>(data_heap<T>(const_cast<const ludo::instance&>(instance)));
  }

  template<typename T>
  const heap_buffer& data_heap(const instance& instance)
  {
    assert(instance.data.find(heap_buffer_key<T>()) != instance.data.end() && "buffer not found");

    return *static_cast<heap_buffer*>(instance.data.at(heap_buffer_key<T>()));
  }

  template<typename T>
  std::string partitioned_buffer_key()
  {
    return std::string("ludo::partitioned_buffer::") + typeid(T).name();
  }

  template<typename T>
  std::string heap_buffer_key()
  {
    return std::string("ludo::heap_buffer::") + typeid(T).name();
  }
}
