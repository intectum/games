/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <algorithm>

#include "algorithm.h"

namespace ludo
{
  template<typename T>
  T* find_by_id(T* begin, T* end, uint64_t id)
  {
    return const_cast<T*>(find_by_id<T>(const_cast<const T*>(begin), const_cast<const T*>(end), id));
  }

  template<typename T>
  const T* find_by_id(const T* begin, const T* end, uint64_t id)
  {
    return std::find_if(begin, end, [id](const T& value)
    {
      return value.id == id;
    });
  }
}
