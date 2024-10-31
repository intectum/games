#pragma once

#include "./types.h"

namespace fast_dom
{
  uint32_t count_tags(const str& src);

  void parse_tags(const str& src, document& doc);

  uint32_t parse_tag(const str& src, uint32_t pos, tag_type& type_out, str& name_out, str& attribute_set_out);

  uint32_t parse_attribute(const str& src, uint32_t pos, attribute& out);
}
