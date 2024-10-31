#pragma once

#include <ludo/api.h>

#include "../util/str.h"
#include "./types.h"

namespace fast_dom
{
  void parse_declaration_block(const str& src, str out[css_property_count]);

  uint32_t parse_length(const str& src, uint32_t def = 0);

  ludo::vec4 parse_color(const str& src, const ludo::vec4& def = ludo::vec4_zero);
}
