#include <iostream>

#include "parsing.h"

namespace fast_dom
{
  void parse_declaration_block(const str& src, str out[css_property_count])
  {
    auto pos = uint32_t(0);

    while (pos < src.length)
    {
      auto property_pos = pos;
      while (src.data[pos] != ':')
      {
        pos++;
      }

      auto property = str { .data = src.data + property_pos, .length = pos - property_pos };

      assert(src.data[pos] == ':');
      pos++;

      auto value_pos = pos;
      while (src.data[pos] != ';')
      {
        pos++;
      }

      auto value = str { .data = src.data + value_pos, .length = pos - value_pos };

      assert(src.data[pos] == ';');
      pos++;

      if (std::strncmp(property.data, "width", property.length) == 0)
      {
        out[css_property_width] = value;
      }
      else if (std::strncmp(property.data, "height", property.length) == 0)
      {
        out[css_property_height] = value;
      }
      else if (std::strncmp(property.data, "background-color", property.length) == 0)
      {
        out[css_property_background_color] = value;
      }
      else
      {
        char property_copy[128];
        std::strncpy(property_copy, property.data, property.length);
        property_copy[property.length] = 0;
        std::cout << "property " << property_copy << " not supported" << std::endl;
      }
    }
  }

  uint32_t parse_length(const str& src, uint32_t def)
  {
    if (!src.data)
    {
      return def;
    }

    if (std::strncmp(src.data + src.length - 2, "px", 2) == 0)
    {
      return std::strtoul(src.data, nullptr, 0);
    }

    std::cout << "unit not supported" << std::endl;
    return def;
  }

  ludo::vec4 parse_color(const str& src, const ludo::vec4& def)
  {
    if (!src.data)
    {
      return def;
    }

    if (std::strncmp(src.data, "blue", src.length) == 0)
    {
      return { 0.0f, 0.0f, 1.0f, 1.0f };
    }
    if (std::strncmp(src.data, "green", src.length) == 0)
    {
      return { 0.0f, 1.0f, 0.0f, 1.0f };
    }
    if (std::strncmp(src.data, "red", src.length) == 0)
    {
      return { 1.0f, 0.0f, 0.0f, 1.0f };
    }

    char color_copy[128];
    std::strncpy(color_copy, src.data, src.length);
    color_copy[src.length] = 0;
    std::cout << "format " << color_copy << " not supported" << std::endl;
    return def;
  }
}
