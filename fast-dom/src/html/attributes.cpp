#include <iostream>

#include "attributes.h"
#include "parsing.h"

namespace fast_dom
{
  void parse_attributes(document& doc)
  {
    auto& attribute_sets = doc.attribute_sets;
    auto& styles = doc.styles;

    for (auto index = 0; index < doc.count; index++)
    {
      auto& attribute_set = attribute_sets[index];
      auto& style = styles[index];

      auto pos = uint32_t(0);
      while (pos < attribute_set.length)
      {
        if (attribute_set.data[pos] == ' ')
        {
          pos++;
        }

        attribute attribute;
        pos += parse_attribute(attribute_set, pos, attribute);

        if (std::strncmp(attribute.name.data, "style", attribute.name.length) == 0)
        {
          parse_declaration_block(attribute.value, style);
        }
        else
        {
          char name_copy[128];
          std::strncpy(name_copy, attribute.name.data, attribute.name.length);
          name_copy[attribute.name.length] = 0;
          std::cout << "attribute " << name_copy << " not supported" << std::endl;
        }
      }
    }
  }
}
