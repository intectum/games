/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

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
  T* find_by_id(T* begin, T* end, uint64_t id);
  template<typename T>
  const T* find_by_id(const T* begin, const T* end, uint64_t id);
}

#include "algorithm.hpp"
