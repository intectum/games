/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <cstdint>
#include <utility>

namespace ludo
{
  struct arena
  {
    std::byte* start = nullptr;
    std::byte* next = nullptr;
    std::byte* end = nullptr;
  };

  arena allocate_arena(uint64_t size);

  std::byte* allocate(arena& arena, uint64_t size);

  template<typename T>
  std::pair<T*, T*> cast(arena& arena);

  template<typename T>
  std::pair<const T*, const T*> cast(const arena& arena);
}

#include "memory.hpp"
