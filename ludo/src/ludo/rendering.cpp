/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "rendering.h"

namespace ludo
{
  uint8_t pixel_depth(const texture& texture)
  {
    auto component_count = uint8_t(0);
    if (texture.components == pixel_components::BGR || texture.components == pixel_components::RGB)
    {
      component_count = 3;
    }
    else if (texture.components == pixel_components::BGRA || texture.components == pixel_components::RGBA)
    {
      component_count = 4;
    }
    else if (texture.components == pixel_components::DEPTH)
    {
      component_count = 1;
    }
    else
    {
      assert(false && "unsupported components");
    }

    if (texture.datatype == pixel_datatype::UINT8)
    {
      return component_count;
    }
    else if (texture.datatype == pixel_datatype::FLOAT16)
    {
      return component_count * 2;
    }
    else if (texture.datatype == pixel_datatype::FLOAT32)
    {
      return component_count * 4;
    }
    else
    {
      assert(false && "unsupported datatype");
    }

    return 3;
  }
}
