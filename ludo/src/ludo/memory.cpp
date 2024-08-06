/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <cstdlib>

#include "memory.h"

namespace ludo
{
  arena allocate_arena(uint64_t size)
  {
    auto data = static_cast<std::byte*>(malloc(size));

    return {
      .start = data,
      .next = data,
      .end = data + size
    };
  }

  std::byte* allocate(arena& arena, uint64_t size)
  {
    assert(arena.next + size <= arena.end && "not enough space");

    auto next = arena.next;
    arena.next += size;

    return next;
  }
}
