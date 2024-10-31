#pragma once

#include "../css/parsing.h"

namespace fast_dom
{
  struct attribute
  {
    str name;
    str value;
    uint32_t length = 0;
  };

  enum tag_type
  {
    tag_type_opening,
    tag_type_closing,
    tag_type_void
  };

  struct document
  {
    str* names;
    str* attribute_sets;
    uint32_t* parent_indices;
    str** styles;
    ludo::vec3* positions;
    ludo::vec3* sizes;
    ludo::vec3* layout_positions;

    uint32_t count = 0;
  };
}
