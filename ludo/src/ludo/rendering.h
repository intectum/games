/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_RENDERING_H
#define LUDO_RENDERING_H

#include "core.h"
#include "data.h"
#include "math/mat.h"
#include "math/vec.h"
#include "windowing.h"

namespace ludo
{
  ///
  /// A rendering context.
  struct LUDO_API rendering_context
  {
    uint64_t id = 0; ///< The ID of the render context.

    buffer data_buffer; ///< A buffer containing data available to all render programs within the context.
  };

  ///
  /// A set of options for rendering.
  struct LUDO_API render_options
  {
    uint64_t frame_buffer_id = 0; ///< The ID of the frame buffer to render to. An ID of 0 will render to the window.
    uint64_t window_id = 0; ///< The ID of the window to render to. An ID of 0 will render to the first window in the instance.

    std::vector<uint64_t> mesh_ids; ///< The IDs of the meshes to render. Takes precedence over linear octrees. For better performance, add these meshes to the instance first.
    std::vector<uint64_t> linear_octree_ids; ///< The IDs of the linear octrees containing meshes to render.

    uint64_t render_program_id; ///< The IDs of the render program to use. Takes precedence over render programs specified by mesh buffers.

    buffer data_buffer; ///< A buffer containing data available to all render programs.
  };

  ///
  /// A render program consisting of a set of shaders.
  struct LUDO_API render_program
  {
    uint64_t id = 0; ///< The ID of the render program.
    uint64_t vertex_shader_id = 0; ///< The ID of the vertex shader.
    uint64_t geometry_shader_id = 0; ///< The ID of the geometry shader.
    uint64_t fragment_shader_id = 0; ///< The ID of the fragment shader.

    buffer data_buffer; ///< A buffer containing data available to the render program.
  };

  ///
  /// The type of a shader.
  enum class shader_type
  {
    VERTEX, ///< Executes once per vertex. Outputs a transformed vertex.
    GEOMETRY, ///< Executes once per vertex (after the vertex shader). Capable of outputting several vertices.
    FRAGMENT ///< Executes once per fragment/pixel. Outputs color.
  };

  ///
  /// A shader program that runs on the GPU.
  struct LUDO_API shader
  {
    uint64_t id = 0; ///< The ID of the shader.
    shader_type type = shader_type::VERTEX; ///< The type of the shader. TODO remove this and add it as a parameter to the add() function?
  };

  ///
  /// A frame buffer.
  struct LUDO_API frame_buffer
  {
    uint64_t id = 0; ///< The ID of the frame buffer.

    uint32_t width = 0; ///< The width of the frame buffer.
    uint32_t height = 0; ///< The height of the frame buffer.

    std::vector<uint64_t> color_texture_ids; ///< The textures to write colors to.
    uint64_t depth_texture_id = 0; ///< The texture to write depths to.
    uint64_t stencil_texture_id = 0; ///< The texture to write stencils to.
  };

  ///
  /// The components of a pixel.
  enum class pixel_components
  {
    BGR, ///< [blue,green,red].
    BGRA, ///< [blue,green,red,alpha].
    RGB, ///< [red,green,blue].
    RGBA, ///< [red,green,blue,alpha].

    DEPTH
  };

  ///
  /// The datatype of a component within a pixel.
  enum class pixel_datatype
  {
    UINT8,

    FLOAT16,
    FLOAT32,
  };

  ///
  /// A texture.
  struct LUDO_API texture
  {
    uint64_t id = 0; ///< The ID of the texture.

    pixel_components components = pixel_components::RGB; ///< The pixel components of the texture.
    pixel_datatype datatype = pixel_datatype::UINT8; ///< The pixel datatype of the texture.
    uint32_t width = 0; ///< The width of the texture.
    uint32_t height = 0; ///< The height of the texture.
  };

  ///
  /// A set of options for building a texture.
  struct LUDO_API texture_options
  {
    bool clamp = false; ///< Determines if texture coordinates outside the range [0,1] should be clamped to that range instead of repeating.
    uint8_t samples = 1; ///< Samples per pixel. Specifying more than 1 sample results in a 'multisample' texture.
  };

  ///
  /// A camera.
  struct LUDO_API camera
  {
    float near_clipping_distance = 0.1f; ///< The distance from the camera to the near clipping plane.
    float far_clipping_distance = 1000.0f; ///< The distance from the camera to the far clipping plane.

    mat4 view; ///< The viewpoint of the camera (position and rotation).
    mat4 projection; ///< The projection transformation e.g. orthogonal or perspective.
  };

  ///
  /// A light source.
  struct LUDO_API light
  {
    vec4 ambient; ///< The ambient light provided by this light.
    vec4 diffuse; ///< The diffuse light provided by this light.
    vec4 specular; ///< The specular light provided by this light.

    vec3 position; ///< The position of this light source.
    vec3 direction; ///< The direction in which this light is pointed.
    vec3 attenuation; ///< The rate at which the intensity of the light provided by this light will decrease at longer range. The first element of the vector is the linear dropoff, the second is the square dropoff and the third is the cubic dropoff.
    float strength = 1; ///< The strength of this light.
    float range = 1000; ///< The distance that the light will reach.
  };

  ///
  /// Renders meshes within the given instance.
  /// If no meshes or linear octrees are specified in the options:
  /// - Renders all linear octrees, or if there are no linear octrees:
  /// - Renders all meshes
  /// Where linear octrees are used, meshes must have bounds that can be contained within an octant of the linear octree.
  /// \param instance The instance containing the meshes to render. Must contain a rendering_context.
  /// \param options The options used to render.
  LUDO_API void render(instance& instance, const render_options& options = {});

  ///
  /// Waits for rendering to complete.
  LUDO_API void wait_for_render();

  template<>
  LUDO_API rendering_context* add(instance& instance, const rendering_context& init, const std::string& partition);

  ///
  /// Adds a rendering context to the data of an instance.
  /// Allocates a data buffer based on the options provided.
  /// The data buffer will be of the form: <camera><light_count><light_0>...<light_n>
  /// \param instance The instance to add the rendering context to.
  /// \param init The initial state of the new rendering context.
  /// \param light_count The number of lights the rendering context can contain.
  /// \param partition The name of the partition.
  /// \return A pointer to the new rendering context. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  LUDO_API rendering_context* add(instance& instance, const rendering_context& init, uint32_t light_count, const std::string& partition = "default");

  template<>
  LUDO_API void remove<rendering_context>(instance& instance, rendering_context* element, const std::string& partition);

  ///
  /// Retrieves a camera from a rendering context.
  /// The data buffer must be of the form of that created via the add(context& context, const rendering_context& init, uint32_t light_count, const std::string& partition) function.
  /// \param rendering_context The rendering context to retrieve the camera from.
  /// \return The camera.
  LUDO_API camera get_camera(const rendering_context& rendering_context);

  ///
  /// Sets a camera of a rendering context.
  /// The data buffer must be of the form of that created via the add(context& context, const rendering_context& init, uint32_t light_count, const std::string& partition) function.
  /// \param rendering_context The rendering context to set the camera of.
  /// \param camera The camera.
  LUDO_API void set_camera(rendering_context& rendering_context, const camera& camera);

  ///
  /// Retrieves a light from a rendering context.
  /// The data buffer must be of the form of that created via the add(context& context, const rendering_context& init, uint32_t light_count, const std::string& partition) function.
  /// \param rendering_context The rendering context to retrieve the light from.
  /// \param index The index of the light within the rendering context.
  /// \return The light.
  LUDO_API light get_light(const rendering_context& rendering_context, uint8_t index);

  ///
  /// Sets a light of a rendering context.
  /// The data buffer must be of the form of that created via the add(context& context, const rendering_context& init, uint32_t light_count, const std::string& partition) function.
  /// \param rendering_context The rendering context to set the light of.
  /// \param light The light.
  /// \param index The index of the light within the rendering context.
  LUDO_API void set_light(rendering_context& rendering_context, const light& light, uint8_t index);

  template<>
  LUDO_API render_program* add(instance& instance, const render_program& init, const std::string& partition);

  template<>
  LUDO_API void remove<render_program>(instance& instance, render_program* element, const std::string& partition);

  template<>
  LUDO_API shader* add(instance& instance, const shader& init, const std::string& partition);

  ///
  /// Adds a shader to the data of an instance.
  /// \param instance The instance to add the shader to.
  /// \param init The initial state of the new shader.
  /// \param code The source code of the shader.
  /// \param partition The name of the partition.
  /// \return A pointer to the new shader. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  LUDO_API shader* add(instance& instance, const shader& init, std::istream& code, const std::string& partition = "default");

  template<>
  LUDO_API void remove<shader>(instance& instance, shader* element, const std::string& partition);

  template<>
  LUDO_API frame_buffer* add(instance& instance, const frame_buffer& init, const std::string& partition);

  template<>
  LUDO_API void remove<frame_buffer>(instance& instance, frame_buffer* element, const std::string& partition);

  ///
  /// Copies the content of the source frame buffer to the dest frame buffer.
  /// \param source The frame buffer to copy content from.
  /// \param dest The frame buffer to copy content to.
  void blit(const frame_buffer& source, const frame_buffer& dest);

  template<>
  LUDO_API texture* add(instance& instance, const texture& init, const std::string& partition);

  ///
  /// Adds a texture to the data of an instance.
  /// \param instance The instance to add the texture to.
  /// \param init The initial state of the new texture.
  /// \param options The options used to initialize the texture.
  /// \param partition The name of the partition.
  /// \return A pointer to the new texture. This pointer is not guaranteed to remain valid after subsequent additions/removals.
  LUDO_API texture* add(instance& instance, const texture& init, const texture_options& options, const std::string& partition = "default");

  template<>
  LUDO_API void remove<texture>(instance& instance, texture* element, const std::string& partition);

  ///
  /// Reads data from a texture.
  /// \param texture The texture.
  /// \return The texture data.
  LUDO_API std::vector<std::byte> read(const texture& texture);

  ///
  /// Writes data to a texture.
  /// \param texture The texture.
  /// \param data The texture data.
  LUDO_API void write(texture& texture, const std::byte* data);

  template<>
  LUDO_API void write(buffer& buffer, uint64_t position, const texture& value);

  ///
  /// Determines the size (in bytes) of a pixel in a texture.
  /// \param texture The texture.
  /// \return The size (in bytes) of a pixel in a texture.
  uint8_t pixel_depth(const texture& texture);
}

#endif // LUDO_RENDERING_H
