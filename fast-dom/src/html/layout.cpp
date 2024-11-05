#include <algorithm>

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
    auto& display_outers = doc.display_outers;

    auto break_heights = new uint32_t[doc.count]; // TODO arena

    for (auto index = 0; index < doc.count; index++)
    {
      auto& parent_index = parent_indices[index];
      auto& style = styles[index];
      auto& position = positions[index];
      auto& size = sizes[index];
      auto& layout_position = layout_positions[index];
      auto& display_outer = display_outers[index];

      size[0] = (float) parse_length(style[fast_dom::css_property_width], index ? (uint32_t) sizes[parent_index][0] : window_width);
      size[1] = (float) parse_length(style[fast_dom::css_property_height]);

      if (index == 0)
      {
        position = ludo::vec3_zero;
      }
      else
      {
        auto& parent_position = positions[parent_index];
        auto& parent_size = sizes[parent_index];
        auto& parent_layout_position = layout_positions[parent_index];
        auto& parent_break_height = break_heights[parent_index];

        auto break_line = [&]()
        {
          parent_layout_position[1] += (float) parent_break_height;
          if (parent_layout_position[0] > 0)
          {
            parent_layout_position[0] = parent_position[0];
          }
        };

        if (display_outer == display_outer_block)
        {
          break_line();

          position = parent_layout_position;
          parent_break_height = (uint32_t) size[1];
        }
        else
        {
          if (size[0] > parent_size[0] - parent_layout_position[0])
          {
            break_line();
            parent_break_height = 0;
          }

          position = parent_layout_position;
          parent_layout_position[0] += size[0];
          parent_break_height = std::max(parent_break_height, (uint32_t) size[1]);
        }
      }

      layout_position = position;
    }

    delete[] break_heights;
  }
}
