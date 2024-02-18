/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_ALGORITHM_H
#define LUDO_ALGORITHM_H

#include <vector>

#include "core.h"

namespace ludo
{
  ///
  /// Finds the first element in the range [begin, end) with the given id.
  /// \param begin The start of the range to search.
  /// \param end The end of the range to search.
  /// \param id The id to search for.
  /// \return The first matching element or end if it is not found.
  template<typename T>
  LUDO_API T* find_by_id(T* begin, T* end, uint64_t id);
  template<typename T>
  LUDO_API const T* find_by_id(const T* begin, const T* end, uint64_t id);
}

#include "algorithm.hpp"

#endif // LUDO_ALGORITHM_H
