/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/data/buffers.h>
#include <ludo/testing.h>

#include "buffers.h"

namespace ludo
{
  void test_buffers()
  {
    test_group("buffers");

    auto buffer = allocate(sizeof(int32_t) * 10);
    test_not_equal<void*>("buffer: allocate (data)", buffer.data, nullptr);
    test_equal("buffer: allocate (size)", buffer.size, sizeof(int32_t) * 10);

    deallocate(buffer);
    test_equal<void*>("buffer: deallocate (data)", buffer.data, nullptr);
    test_equal("buffer: deallocate (size)", buffer.size, 0ul);
  }
}
