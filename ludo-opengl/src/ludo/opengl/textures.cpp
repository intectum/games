/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "textures.h"
#include "util.h"

namespace ludo
{
  GLint internal_pixel_format(const texture& texture);

  auto pixel_formats = std::unordered_map<pixel_components, GLenum>
  {
    { pixel_components::BGR, GL_BGR },
    { pixel_components::BGRA, GL_BGRA },
    { pixel_components::R, GL_RED },
    { pixel_components::RG, GL_RG },
    { pixel_components::RGB, GL_RGB },
    { pixel_components::RGBA, GL_RGBA },

    { pixel_components::DEPTH, GL_DEPTH_COMPONENT }
  };

  auto pixel_types = std::unordered_map<pixel_datatype, GLenum>
  {
    { pixel_datatype::UINT8, GL_UNSIGNED_BYTE },

    { pixel_datatype::FLOAT16, GL_HALF_FLOAT },
    { pixel_datatype::FLOAT32, GL_FLOAT }
  };

  void init(texture& texture, const texture_options& options)
  {
    auto name = GLuint();
    glGenTextures(1, &name); check_opengl_error();
    texture.id = name;

    if (options.samples > 1)
    {
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture.id); check_opengl_error();
      glTextureStorage2DMultisample(
        texture.id,
        options.samples,
        internal_pixel_format(texture),
        static_cast<GLsizei>(texture.width),
        static_cast<GLsizei>(texture.height),
        false
      ); check_opengl_error();
    }
    else
    {
      glBindTexture(GL_TEXTURE_2D, texture.id); check_opengl_error();
      glTextureStorage2D(
        texture.id,
        1,
        internal_pixel_format(texture),
        static_cast<GLsizei>(texture.width),
        static_cast<GLsizei>(texture.height)
      ); check_opengl_error();

      glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR); check_opengl_error();
      glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR); check_opengl_error();

      if (options.clamp)
      {
        glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); check_opengl_error();
        glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); check_opengl_error();
      }
    }
  }

  void de_init(texture& texture)
  {
    auto name = static_cast<GLuint>(texture.id);
    glDeleteTextures(1, &name); check_opengl_error();
    texture.id = 0;
  }

  std::vector<std::byte> read(const texture& texture)
  {
    auto data = std::vector<std::byte>(texture.width * texture.height * pixel_depth(texture));

    glGetTextureImage(
      texture.id,
      0,
      pixel_formats[texture.components],
      pixel_types[texture.datatype],
      static_cast<GLsizei>(data.size()),
      data.data()
    ); check_opengl_error();

    return data;
  }

  void write(texture& texture, const std::byte* data)
  {
    glTextureSubImage2D(
      texture.id,
      0,
      0,
      0,
      static_cast<GLsizei>(texture.width),
      static_cast<GLsizei>(texture.height),
      pixel_formats[texture.components],
      pixel_types[texture.datatype],
      data
    ); check_opengl_error();
  }

  uint64_t handle(const texture& texture)
  {
    auto handle = glGetTextureHandleARB(texture.id); check_opengl_error();
    auto resident = glIsTextureHandleResidentARB(handle); check_opengl_error();
    if (!resident)
    {
      glMakeTextureHandleResidentARB(handle); check_opengl_error();
    }

    return handle;
  }

  GLint internal_pixel_format(const texture& texture)
  {
    if (texture.components == pixel_components::BGR || texture.components == pixel_components::RGB)
    {
      if (texture.datatype == pixel_datatype::UINT8)
      {
        return GL_RGB8;
      }
      else if (texture.datatype == pixel_datatype::FLOAT16)
      {
        return GL_RGB16F;
      }
      else if (texture.datatype == pixel_datatype::FLOAT32)
      {
        return GL_RGB32F;
      }
    }
    else if (texture.components == pixel_components::BGRA || texture.components == pixel_components::RGBA)
    {
      if (texture.datatype == pixel_datatype::UINT8)
      {
        return GL_RGBA8;
      }
      else if (texture.datatype == pixel_datatype::FLOAT16)
      {
        return GL_RGBA16F;
      }
      else if (texture.datatype == pixel_datatype::FLOAT32)
      {
        return GL_RGBA32F;
      }
    }
    else if (texture.components == pixel_components::DEPTH)
    {
      if (texture.datatype == pixel_datatype::FLOAT32)
      {
        return GL_DEPTH_COMPONENT32F;
      }
    }
    else if (texture.components == pixel_components::R)
    {
      if (texture.datatype == pixel_datatype::UINT8)
      {
        return GL_R8;
      }
      else if (texture.datatype == pixel_datatype::FLOAT16)
      {
        return GL_R16F;
      }
      else if (texture.datatype == pixel_datatype::FLOAT32)
      {
        return GL_R32F;
      }
    }
    else if (texture.components == pixel_components::RG)
    {
      if (texture.datatype == pixel_datatype::UINT8)
      {
        return GL_RG8;
      }
      else if (texture.datatype == pixel_datatype::FLOAT16)
      {
        return GL_RG16F;
      }
      else if (texture.datatype == pixel_datatype::FLOAT32)
      {
        return GL_RG32F;
      }
    }
    else
    {
      assert(false && "unsupported components");
    }

    assert(false && "unsupported components/datatype combination");
    return GL_RGB8;
  }
}
