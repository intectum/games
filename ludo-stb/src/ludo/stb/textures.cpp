/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <ludo/rendering.h>

namespace ludo
{
  texture load(const std::string& file_name)
  {
    auto stream = std::ifstream(file_name, std::ios::binary);

    return load(stream);
  }

  texture load(std::istream& stream)
  {
    stream.seekg(0, std::ios_base::end);
    auto stream_size = stream.tellg();
    auto stream_data = malloc(stream.tellg());
    stream.seekg(0);
    stream.read(static_cast<char*>(stream_data), stream_size);

    stbi_set_flip_vertically_on_load(true);

    auto x = int32_t(0);
    auto y = int32_t(0);
    auto channels_in_file = int32_t(0);
    auto data = stbi_load_from_memory(static_cast<stbi_uc*>(stream_data), static_cast<int32_t>(stream_size), &x, &y, &channels_in_file, 0);

    if (!data)
    {
      std::cout << "failed to load texture: " << stbi_failure_reason() << std::endl;
      return {};
    }

    assert(channels_in_file == 3 || channels_in_file == 4 && "unsupported pixel components");

    auto texture = ludo::texture
    {
      .components = channels_in_file == 4 ? pixel_components::RGBA : pixel_components::RGB,
      .width = uint32_t(x),
      .height = uint32_t(y)
    };
    ludo::init(texture);
    ludo::write(texture, reinterpret_cast<std::byte*>(data));

    stbi_image_free(data);
    free(stream_data);

    return texture;
  }
}
