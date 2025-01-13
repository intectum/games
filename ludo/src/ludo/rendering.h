/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <sstream>

#include "math/mat.h"
#include "math/vec.h"
#include "meshes.h"

namespace ludo
{
  ///
  /// A fence.
  struct fence
  {
    uint64_t id = 0; ///< A unique identifier.
  };

  ///
  /// A program that executes a render pipeline.
  struct render_program
  {
    uint64_t id = 0; ///< A unique identifier.

    mesh_primitive primitive = mesh_primitive::TRIANGLE_LIST; ///< The primitive to render.
    vertex_format format; ///< The vertex format.
  };

  struct render_command;

  ///
  /// A frame buffer.
  struct frame_buffer
  {
    uint64_t id = 0; ///< A unique identifier. 0 will render to the window.

    uint32_t width = 0; ///< The width.
    uint32_t height = 0; ///< The height.

    std::vector<uint64_t> color_texture_ids; ///< The color textures.
    uint64_t depth_texture_id = 0; ///< The depth texture.
    uint64_t stencil_texture_id = 0; ///< The stencil texture.
  };

  ///
  /// The components of a pixel.
  enum class pixel_components
  {
    BGR, ///< [blue,green,red].
    BGRA, ///< [blue,green,red,alpha].
    R, ///< [red].
    RG, ///< [red,green].
    RGB, ///< [red,green,blue].
    RGBA, ///< [red,green,blue,alpha].

    DEPTH ///< Depth only.
  };

  ///
  /// The datatype of a component within a pixel.
  enum class pixel_datatype
  {
    UINT8, ///< 8bit unsigned int

    FLOAT16, ///< 16bit float
    FLOAT32, ///< 32bit float
  };

  ///
  /// A texture.
  struct texture
  {
    uint64_t id = 0; ///< A unique identifier.

    pixel_components components = pixel_components::RGB; ///< The pixel components.
    pixel_datatype datatype = pixel_datatype::UINT8; ///< The pixel datatype.
    uint32_t width = 0; ///< The width.
    uint32_t height = 0; ///< The height.
  };

  ///
  /// A set of options for building a texture.
  struct texture_options
  {
    bool clamp = false; ///< Determines if texture coordinates outside the range [0,1] should be clamped to that range instead of repeating.
    uint8_t samples = 1; ///< Samples per pixel. Specifying more than 1 sample results in a 'multisample' texture.
  };

  ///
  /// Executes render commands on the GPU.
  /// \param render_program The render program to run.
  /// \param start The index of the first render command to execute.
  /// \param count The number of render commands to execute.
  void render(const render_program& render_program, uint32_t start, uint32_t count);

  ///
  /// Initializes a fence.
  /// \param fence The fence.
  void init(fence& fence);

  ///
  /// De-initializes a fence.
  /// \param fence The fence.
  void de_init(fence& fence);

  ///
  /// Waits for a fence.
  /// \param fence The fence.
  void wait(fence& fence);

  ///
  /// Initializes rendering.
  void init_rendering();

  ///
  /// Binds commands.
  /// \id The ID of the command buffer to bind.
  void bind_commands(uint32_t id);

  ///
  /// Binds indices.
  /// \id The ID of the index buffer to bind.
  void bind_indices(uint32_t id);

  ///
  /// Binds vertices.
  /// \id The ID of the vertex buffer to bind.
  void bind_vertices(uint32_t id);

  ///
  /// Binds data.
  /// \position The position to bind to.
  /// \id The ID of the data buffer to bind.
  void bind_data(uint32_t position, uint32_t id);

  ///
  /// Build the view projection matrix of a camera.
  /// \param view The view matrix of the camera.
  /// \param projection The projection matrix of the camera.
  /// \return The view projection matrix.
  mat4 view_projection(const mat4& view, const mat4& projection);

  ///
  /// Build the planes of a camera's view frustum.
  /// Planes have their normals pointing into the view frustum.
  /// \param view_projection The view projection matrix of the camera.
  /// \return The planes.
  std::array<vec4, 6> frustum_planes(const mat4& view_projection);

  ///
  /// Initializes a render program.
  /// \param render_program The render program.
  /// \param format The vertex format to build the render program for.
  void init(render_program& render_program, const vertex_format& format);

  ///
  /// Initializes a render program.
  /// \param render_program The render program.
  /// \param vertex_shader_file_name The name of the file containing the vertex shader source code.
  /// \param fragment_shader_file_name The name of the file containing the fragment shader source code.
  void init(render_program& render_program, const std::string& vertex_shader_file_name, const std::string& fragment_shader_file_name);

  ///
  /// Initializes a render program.
  /// \param render_program The render program.
  /// \param vertex_shader_code The vertex shader source code.
  /// \param fragment_shader_code The fragment shader source code.
  void init(render_program& render_program, std::istream& vertex_shader_code, std::istream& fragment_shader_code);

  ///
  /// De-initializes a render program and reclaims the render commands.
  /// \param render_program The render program.
  void de_init(render_program& render_program);

  ///
  /// Sets the current render program.
  /// \param render_program The render program.
  void use(const render_program& render_program);

  ///
  /// Builds the default vertex shader code for a vertex format.
  /// \param format The vertex format.
  /// \return The vertex shader code.
  std::stringstream default_vertex_shader_code(const vertex_format& format);

  ///
  /// Builds the default fragment shader code for a vertex format.
  /// \param format The vertex format.
  /// \return The fragment shader code.
  std::stringstream default_fragment_shader_code(const vertex_format& format);

  ///
  /// Initializes a frame buffer.
  /// \param frame_buffer The frame buffer.
  void init(frame_buffer& frame_buffer);

  ///
  /// De-initializes a frame buffer.
  /// \param frame_buffer The frame buffer.
  void de_init(frame_buffer& frame_buffer);

  ///
  /// Sets the current frame buffer.
  /// \param frame_buffer The frame buffer.
  void use(const frame_buffer& frame_buffer);

  ///
  /// Sets the current frame buffer and clears its textures.
  /// \param frame_buffer The frame buffer.
  /// \param color The color to clear the color texture with.
  void use_and_clear(const frame_buffer& frame_buffer, const vec4& color = vec4 { 0.0f, 0.0f, 0.0f, 1.0f });

  ///
  /// Copies the content of the source frame buffer to the dest frame buffer.
  /// \param source The frame buffer to copy content from.
  /// \param dest The frame buffer to copy content to.
  void blit(const frame_buffer& source, const frame_buffer& dest);

  ///
  /// Initializes a texture.
  /// \param texture The texture.
  /// \param options The options used to initialize the texture.
  void init(texture& texture, const texture_options& options = {});

  ///
  /// De-initializes a texture.
  /// \param texture The texture.
  void de_init(texture& texture);

  ///
  /// Loads a texture from an image file.
  /// \param file_name The name of the file containing the texture data.
  /// \return The texture.
  texture load(const std::string& file_name);

  ///
  /// Loads a texture from a stream.
  /// \param stream The texture data.
  /// \return The texture.
  texture load(std::istream& stream);

  ///
  /// Reads data from a texture.
  /// \param texture The texture.
  /// \return The texture data.
  std::vector<std::byte> read(const texture& texture);

  ///
  /// Writes data to a texture.
  /// \param texture The texture.
  /// \param data The texture data.
  void write(texture& texture, const std::byte* data);

  ///
  /// Determines the size (in bytes) of a pixel in a texture.
  /// \param texture The texture.
  /// \return The size (in bytes) of a pixel in a texture.
  uint8_t pixel_depth(const texture& texture);
}
