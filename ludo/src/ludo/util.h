/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_UTIL_H
#define LUDO_UTIL_H

namespace ludo
{
  ///
  /// A range
  struct LUDO_API range
  {
    uint32_t start = 0;
    uint32_t count = 0;
  };
}

#endif // LUDO_UTIL_H
