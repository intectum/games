#include <cassert>
#include <iostream>

#include "parsing.h"

namespace fast_dom
{
  uint32_t count_tags(const str& src)
  {
    auto count = uint32_t(0);

    for (auto pos = uint32_t(0); pos < src.length; pos++)
    {
      if (src.data[pos] == '<' && src.data[pos + 1] != '/')
      {
        count++;
      }
    }

    return count;
  }

  void parse_tags(const str& src, document& doc)
  {
    auto& names = doc.names;
    auto& attribute_sets = doc.attribute_sets;
    auto& parent_indices = doc.parent_indices;
    auto& display_outers = doc.display_outers;

    uint32_t pos = 0;
    uint32_t tag_index = 0;
    uint32_t parent_tag_index = 0;
    while (pos < src.length)
    {
      tag_type type;
      str name;
      str attribute_set;
      pos += parse_tag(src, pos, type, name, attribute_set);

      if (type == fast_dom::tag_type_closing)
      {
        parent_tag_index = parent_indices[parent_tag_index];
        continue;
      }

      assert(tag_index < doc.count);
      names[tag_index] = name;
      attribute_sets[tag_index] = attribute_set;
      parent_indices[tag_index] = parent_tag_index;

      if (std::strncmp(name.data, "span", name.length) == 0)
      {
        display_outers[tag_index] = display_outer_inline;
      }

      if (type == fast_dom::tag_type_opening)
      {
        parent_tag_index = tag_index;
      }

      tag_index++;
    }
  }

  uint32_t parse_tag(const str& src, uint32_t pos, tag_type& type_out, str& name_out, str& attribute_set_out)
  {
    auto start_pos = pos;

    assert(src.data[pos] == '<');
    pos++;

    if (src.data[pos] == '/')
    {
      type_out = tag_type_closing;
      pos++;
    }

    auto name_pos = pos;
    while (src.data[pos] != ' ' && src.data[pos] != '/' && src.data[pos] != '>') // TODO support additional whitespace - preprocess to remove excess?
    {
      pos++;
    }

    name_out = {.data = src.data + name_pos, .length = pos - name_pos};

    if (src.data[pos] == ' ')
    {
      pos++;
    }

    auto attribute_set_pos = pos;
    while (src.data[pos] != '/' && src.data[pos] != '>')
    {
      pos++;
    }

    attribute_set_out = {.data = src.data + attribute_set_pos, .length = pos - attribute_set_pos};

    if (src.data[pos] == '/')
    {
      type_out = tag_type_void;
      pos++;
    }

    assert(src.data[pos] == '>');
    pos++;

    return pos - start_pos;
  }

  uint32_t parse_attribute(const str& src, uint32_t pos, attribute& out)
  {
    auto start_pos = pos;

    auto name_pos = pos;
    while (src.data[pos] != '=' && src.data[pos] != ' ' && src.data[pos] != '/' && src.data[pos] != '>')
    {
      pos++;
    }

    out.name = str { .data = src.data + name_pos, .length = pos - name_pos };

    if (src.data[pos] == ' ' || src.data[pos] == '/' || src.data[pos] == '>')
    {
      return pos - start_pos;
    }

    assert(src.data[pos] == '=');
    pos++;

    // TODO in some cases quotes are not required
    auto single_quoted = src.data[pos] == '\'';
    auto double_quoted = src.data[pos] == '"';
    assert(single_quoted || double_quoted);
    pos++;

    auto value_pos = pos;
    while ((single_quoted && src.data[pos] != '\'') || (double_quoted && src.data[pos] != '"'))
    {
      pos++;
    }

    out.value = str { .data = src.data + value_pos, .length = pos - value_pos };

    assert((single_quoted && src.data[pos] == '\'') || (double_quoted && src.data[pos] == '"'));
    pos++;

    return pos - start_pos;
  }
}
