/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "util.h"

namespace ludo
{
  auto pixel_formats = std::unordered_map<pixel_format, GLenum>
  {
    { pixel_format::BGR, GL_BGR },
    { pixel_format::BGR_HDR, GL_BGR },
    { pixel_format::BGRA, GL_BGRA },
    { pixel_format::BGRA_HDR, GL_BGRA },
    { pixel_format::RGB, GL_RGB },
    { pixel_format::RGB_HDR, GL_RGB },
    { pixel_format::RGBA, GL_RGBA },
    { pixel_format::RGBA_HDR, GL_RGBA },

    { pixel_format::DEPTH, GL_DEPTH_COMPONENT }
  };

  auto internal_pixel_formats = std::unordered_map<pixel_format, GLint>
  {
    { pixel_format::BGR, GL_RGB8 },
    { pixel_format::BGR_HDR, GL_RGB16F },
    { pixel_format::BGRA, GL_RGBA8 },
    { pixel_format::BGRA_HDR, GL_RGBA16F },
    { pixel_format::RGB, GL_RGB8 },
    { pixel_format::RGB_HDR, GL_RGB16F },
    { pixel_format::RGBA, GL_RGBA8 },
    { pixel_format::RGBA_HDR, GL_RGBA16F },

    { pixel_format::DEPTH, GL_DEPTH_COMPONENT32F }
  };

  auto srgb_internal_pixel_formats = std::unordered_map<pixel_format, GLint>
  {
    { pixel_format::BGR, GL_SRGB8 },
    { pixel_format::BGRA, GL_SRGB8_ALPHA8 },
    { pixel_format::RGB, GL_SRGB8 },
    { pixel_format::RGBA, GL_SRGB8_ALPHA8 }
  };

  auto pixel_depths = std::unordered_map<pixel_format, GLuint>
  {
    { pixel_format::BGR, 3 },
    { pixel_format::BGR_HDR, 6 },
    { pixel_format::BGRA, 4 },
    { pixel_format::BGRA_HDR, 8 },
    { pixel_format::RGB, 3 },
    { pixel_format::RGB_HDR, 6 },
    { pixel_format::RGBA, 4 },
    { pixel_format::RGBA_HDR, 8 },

    { pixel_format::DEPTH, 4 }
  };

  auto texture_handles = std::unordered_map<uint64_t, uint64_t>();

  template<>
  texture* add(instance& instance, const texture& init, const std::string& partition)
  {
    return add(instance, init, {}, partition);
  }

  texture* add(instance& instance, const texture& init, const texture_options& options, const std::string& partition)
  {
    auto texture = add(data<ludo::texture>(instance), init, partition);

    auto name = GLuint();
    glGenTextures(1, &name); check_opengl_error();
    texture->id = name;

    if (options.samples > 1)
    {
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture->id); check_opengl_error();
      glTextureStorage2DMultisample(
        texture->id,
        options.samples,
        internal_pixel_formats[texture->format],
        static_cast<GLsizei>(texture->width),
        static_cast<GLsizei>(texture->height),
        false
      ); check_opengl_error();
    }
    else
    {
      glBindTexture(GL_TEXTURE_2D, texture->id); check_opengl_error();
      glTextureParameteri(texture->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR); check_opengl_error();
      glTextureParameteri(texture->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR); check_opengl_error();

      if (options.clamp)
      {
        glTextureParameteri(texture->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); check_opengl_error();
        glTextureParameteri(texture->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); check_opengl_error();
      }
    }

    return texture;
  }

  template<>
  void remove<texture>(instance& instance, texture* element, const std::string& partition)
  {
    auto name = static_cast<GLuint>(element->id);

    glDeleteTextures(1, &name); check_opengl_error();

    remove(data<texture>(instance), element, partition);
  }

  std::vector<std::byte> read(const texture& texture, bool convert_srgb)
  {
    auto data = std::vector<std::byte>(texture.width * texture.height * pixel_depths[texture.format]);
    auto type = texture.format == pixel_format::DEPTH ? GL_FLOAT : GL_UNSIGNED_BYTE;
    auto internal_format = convert_srgb ? srgb_internal_pixel_formats[texture.format] : internal_pixel_formats[texture.format];

    glGetTextureImage(texture.id, 0, internal_format, type, static_cast<GLsizei>(data.size()), data.data()); check_opengl_error();

    return data;
  }

  void write(texture& texture, const std::byte* data, bool convert_srgb)
  {
    auto type = texture.format == pixel_format::DEPTH ? GL_FLOAT : GL_UNSIGNED_BYTE;
    auto internal_format = convert_srgb ? srgb_internal_pixel_formats[texture.format] : internal_pixel_formats[texture.format];

    glTextureImage2DEXT(
      texture.id,
      GL_TEXTURE_2D,
      0,
      internal_format,
      static_cast<GLsizei>(texture.width),
      static_cast<GLsizei>(texture.height),
      0,
      pixel_formats[texture.format],
      type,
      data
    ); check_opengl_error();
  }

  template<>
  void write(buffer& buffer, uint64_t position, const texture& value)
  {
    if (!texture_handles.contains(value.id))
    {
      auto handle = glGetTextureHandleARB(value.id); check_opengl_error();
      glMakeTextureHandleResidentARB(handle); check_opengl_error();

      texture_handles[value.id] = handle;
    }

    write(buffer, position, texture_handles[value.id]);
  }
}
