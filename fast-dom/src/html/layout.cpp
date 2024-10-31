#include "layout.h"

namespace fast_dom
{
  void apply_layout(document& doc, uint32_t window_width, uint32_t window_height)
  {
    auto& parent_indices = doc.parent_indices;
    auto& styles = doc.styles;
    auto& positions = doc.positions;
    auto& sizes = doc.sizes;
    auto& layout_positions = doc.layout_positions;

    for (auto index = 0; index < doc.count; index++)
    {
      auto& parent_index = parent_indices[index];
      auto& style = styles[index];
      auto& position = positions[index];
      auto& size = sizes[index];
      auto& layout_position = layout_positions[index];

      size[0] = parse_length(style[fast_dom::css_property_width], index ? sizes[parent_index][0] : window_width);
      size[1] = parse_length(style[fast_dom::css_property_height]);

      if (index == 0)
      {
        position = ludo::vec3_zero;
      }
      else
      {
        auto& parent_layout_position = layout_positions[parent_index];
        position = parent_layout_position;
        parent_layout_position[1] += size[1];
      }

      layout_position = position;
    }
  }
}
