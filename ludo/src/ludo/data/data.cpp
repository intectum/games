/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>

#include "data.h"

namespace ludo
{
  heap& allocate_heap(instance& instance, const std::string& name, uint64_t size)
  {
    auto key = heap_key(name);
    instance.data[key] = new heap(allocate_heap(size));

    return *static_cast<heap*>(instance.data[key]);
  }

  heap& allocate_heap_vram(instance& instance, const std::string& name, uint64_t size, vram_buffer_access_hint access_hint)
  {
    auto key = heap_key(name);
    instance.data[key] = new heap(allocate_heap_vram(size, access_hint));

    return *static_cast<heap*>(instance.data[key]);
  }

  void deallocate_heap(instance& instance, const std::string& name)
  {
    auto& heap = data_heap(instance, name);
    deallocate(heap);

    instance.data.erase(heap_key(name));
    delete &heap;
  }

  void deallocate_heap_vram(instance& instance, const std::string& name)
  {
    auto& heap = data_heap(instance, name);
    deallocate_vram(heap);

    instance.data.erase(heap_key(name));
    delete &heap;
  }

  heap& data_heap(instance& instance, const std::string& name)
  {
    assert(instance.data.find(heap_key(name)) != instance.data.end() && "heap not found");

    return *static_cast<heap*>(instance.data.at(heap_key(name)));
  }

  const heap& data_heap(const instance& instance, const std::string& name)
  {
    assert(instance.data.find(heap_key(name)) != instance.data.end() && "heap not found");

    return *static_cast<const heap*>(instance.data.at(heap_key(name)));
  }

  std::string heap_key(const std::string& name)
  {
    return std::string("ludo::heap::") + name;
  }
}
