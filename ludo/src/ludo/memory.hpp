/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "memory.h"

namespace ludo
{
  template<typename T>
  std::pair<T*, T*> cast(arena& arena)
  {
    return { reinterpret_cast<T*>(arena.start), reinterpret_cast<T*>(arena.next) };
  }

  template<typename T>
  std::pair<const T*, const T*> cast(const arena& arena)
  {
    return { reinterpret_cast<const T*>(arena.start), reinterpret_cast<const T*>(arena.next) };
  }
}
