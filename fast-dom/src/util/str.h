#pragma once

#include <cstdint>

namespace fast_dom
{
  struct str
  {
    const char* data = nullptr;
    uint32_t length = 0;
  };
}
