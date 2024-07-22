/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <sstream>

#include "animation.h"
#include "data/data.h"
#include "math/mat.h"
#include "math/vec.h"
#include "meshes.h"
#include "util.h"

namespace ludo
{
  ///
  /// A fence.
  struct fence
  {
    uint64_t id = 0; ///< A unique identifier.
  };

  ///
  /// A rendering context.
  struct rendering_context
  {
    uint64_t id = 0; ///< A unique identifier.

    ludo::fence fence; ///< A fence used to determine when rendering is complete.

    double_buffer shader_buffer; ///< The data available to all render programs within the context.
  };

  ///
  /// A camera.
  struct camera
  {
    float near_clipping_distance = 0.1f; ///< The distance to the near clipping plane.
    float far_clipping_distance = 1000.0f; ///< The distance to the far clipping plane.

    mat4 view; ///< The viewpoint.
    mat4 projection; ///< The projection e.g. orthogonal or perspective.
  };

  ///
  /// A light source.
  struct light
  {
    vec4 ambient; ///< The ambient light.
    vec4 diffuse; ///< The diffuse light.
    vec4 specular; ///< The specular light.

    vec3 position; ///< The position.
    vec3 direction; ///< The direction (ignored for point lights).
    vec3 attenuation; ///< The rate at which the intensity of the light will decrease by. It is of the form [<linear dropoff>,<square dropoff>,<cubic dropoff>].
    float strength = 1; ///< The strength.
    float range = 1000; ///< The distance that the light will reach.
  };

  ///
  /// A program that executes a render pipeline.
  struct render_program
  {
    uint64_t id = 0; ///< A unique identifier.

    mesh_primitive primitive = mesh_primitive::TRIANGLE_LIST; ///< The primitive to render.
    vertex_format format; ///< The vertex format.

    buffer command_buffer; ///< The commands to be executed.
    double_buffer shader_buffer; ///< The data available to this render program.

    buffer instance_buffer_front; ///< The committed instance data.
    heap instance_buffer_back; ///< The instance data.
    uint32_t instance_size = 0; ///< The size (in bytes) of the instance data per instance.

    range active_commands; ///< The active commands.

    bool push_on_bind = true; // TODO revise
  };

  ///
  /// An instanced mesh.
  struct render_mesh
  {
    uint64_t id = 0; ///< A unique identifier.
    uint64_t render_program_id = 0; ///< The render program used to draw this mesh.

    range instances = { 0, 1 }; ///< The instances.
    range indices; ///< The indices.
    range vertices; ///< The vertices.

    buffer instance_buffer; ///< The instance data.
    uint32_t instance_size = 0; ///< The size (in bytes) of the instance data per instance.
  };

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
  /// Starts a rendering transaction. Waits for the previous transaction to complete.
  /// Must be called before any calls to commit_render_commands(...).
  /// \param rendering_context The rendering context.
  /// \param render_programs The render programs that can be used in the transaction.
  void start_render_transaction(rendering_context& rendering_context, array<render_program>& render_programs);

  ///
  /// Commits render commands to the GPU.
  /// \param rendering_context The rendering context.
  /// \param render_programs The render programs to commit transactions for.
  /// \param render_commands The render commands to sample from.
  /// \param indices The indices to sample from.
  /// \param vertices The vertices to sample from.
  void commit_render_commands(rendering_context& rendering_context, array<render_program>& render_programs, const heap& render_commands, const heap& indices, const heap& vertices);

  ///
  /// Commits the rendering transaction.
  /// \param rendering_context The rendering context.
  void commit_render_transaction(rendering_context& rendering_context);

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
  /// Initializes a rendering context.
  /// The shader buffer will be of the form <camera><light_count><light_0>...<light_n>
  /// \param rendering_context The rendering context.
  /// \param light_count The number of lights the rendering context can contain.
  void init(rendering_context& rendering_context, uint32_t light_count);

  ///
  /// De-initializes a rendering context.
  /// \param rendering_context The rendering context.
  void de_init(rendering_context& rendering_context);

  ///
  /// Retrieves a camera from a rendering context.
  /// \param rendering_context The rendering context.
  /// \return The camera.
  camera get_camera(const rendering_context& rendering_context);

  ///
  /// Sets a camera of a rendering context.
  /// \param rendering_context The rendering context.
  /// \param camera The camera.
  void set_camera(rendering_context& rendering_context, const camera& camera);

  ///
  /// Retrieves a light from a rendering context.
  /// \param rendering_context The rendering context.
  /// \param index The index of the light.
  /// \return The light.
  light get_light(const rendering_context& rendering_context, uint32_t index);

  ///
  /// Sets a light of a rendering context.
  /// \param rendering_context The rendering context.
  /// \param light The light.
  /// \param index The index of the light.
  void set_light(rendering_context& rendering_context, const light& light, uint32_t index);

  ///
  /// Build the planes of a camera's view frustum.
  /// Planes have their normals pointing into the view frustum.
  /// \param camera The camera.
  /// \return The planes.
  std::array<vec4, 6> frustum_planes(const camera& camera);

  ///
  /// Initializes a render program.
  /// \param render_program The render program.
  /// \param format The vertex format to build the render program for.
  /// \param render_commands The render commands to allocate from.
  /// \param instance_capacity The maximum number of instances.
  /// \param init_instances Determines if the instance buffers should be initialized.
  void init(render_program& render_program, const vertex_format& format, heap& render_commands, uint32_t instance_capacity);

  ///
  /// Initializes a render program.
  /// \param render_program The render program.
  /// \param vertex_shader_file_name The name of the file containing the vertex shader source code.
  /// \param fragment_shader_file_name The name of the file containing the fragment shader source code.
  /// \param render_commands The render commands to allocate from.
  /// \param instance_capacity The maximum number of instances.
  /// \param init_instances Determines if the instance buffers should be initialized.
  void init(render_program& render_program, const std::string& vertex_shader_file_name, const std::string& fragment_shader_file_name, heap& render_commands, uint32_t instance_capacity);

  ///
  /// Initializes a render program.
  /// \param render_program The render program.
  /// \param vertex_shader_code The vertex shader source code.
  /// \param fragment_shader_code The fragment shader source code.
  /// \param render_commands The render commands to allocate from.
  /// \param instance_capacity The maximum number of instances.
  /// \param init_instances Determines if the instance buffers should be initialized.
  void init(render_program& render_program, std::istream& vertex_shader_code, std::istream& fragment_shader_code, heap& render_commands, uint32_t instance_capacity);

  ///
  /// Initializes a render program.
  /// \param render_program The render program.
  /// \param vertex_shader_code The vertex shader source code.
  /// \param fragment_shader_code The fragment shader source code.
  void init(render_program& render_program, std::istream& vertex_shader_code, std::istream& fragment_shader_code);

  ///
  /// De-initializes a render program and reclaims the render commands.
  /// \param render_program The render program.
  /// \param render_commands The render commands to reclaim to.
  void de_init(render_program& render_program, heap& render_commands);

  ///
  /// Commits the state of a render program to the front buffer.
  /// \param render_program The render program.
  void commit(render_program& render_program);

  ///
  /// Sets the current render program.
  /// \param render_program The render program.
  void use(render_program& render_program);

  ///
  /// Adds a render command to the render program's command buffer and increments the active command count.
  /// \param render_program The render program.
  /// \param render_mesh The render mesh.
  void add_render_command(render_program& render_program, const render_mesh& render_mesh);

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
  /// Initializes a render mesh.
  /// \param render_mesh The render mesh.
  void init(render_mesh& render_mesh);

  ///
  /// Initializes a render mesh by connecting to a render program and a mesh, then initializes it's instances.
  /// See init_instances for the assumed instance format.
  /// \param render_mesh The render mesh.
  /// \param render_program The render program.
  /// \param mesh The mesh.
  /// \param indices The indices the mesh was allocated from. TODO remove from mesh so this is not needed?
  /// \param vertices The vertices the mesh was allocated from. TODO remove from mesh so this is not needed?
  /// \param instance_capacity The maximum number of instances.
  void init(render_mesh& render_mesh, render_program& render_program, const mesh& mesh, const heap& indices, const heap& vertices, uint32_t instance_capacity);

  ///
  /// De-initializes a render mesh.
  /// \param render_mesh The render mesh.
  void de_init(render_mesh& render_mesh);

  ///
  /// Connects a render mesh to a render program.
  /// \param render_mesh The render mesh.
  /// \param render_program The render program.
  /// \param instance_capacity The maximum number of instances.
  void connect(render_mesh& render_mesh, render_program& render_program, uint32_t instance_capacity);

  ///
  /// Disconnects a render mesh from a render program.
  /// \param render_mesh The render mesh.
  /// \param render_program The render program.
  void disconnect(render_mesh& render_mesh, render_program& render_program);

  ///
  /// Connects a render mesh to a mesh. The render mesh should be connected to a render program first.
  /// \param render_mesh The render mesh.
  /// \param mesh The mesh.
  /// \param indices The indices the mesh was allocated from. TODO remove from mesh so this is not needed?
  /// \param vertices The vertices the mesh was allocated from. TODO remove from mesh so this is not needed?
  void connect(render_mesh& render_mesh, const mesh& mesh, const heap& indices, const heap& vertices);

  ///
  /// Initializes the instances of a render mesh.
  /// Instances are assumed to be of the form <transform>[<texture>][<bone transforms>]
  /// \param render_mesh The render mesh.
  /// \param mesh The mesh.
  /// \param instance_count The number of instances to initialize.
  void init_instances(render_mesh& render_mesh, const mesh& mesh, uint32_t instance_count);

  ///
  /// Retrieves the transform from a render mesh.
  /// \param render_mesh The transform to retrieve.
  /// \param instance_index The index of the instance to retrieve the transform for.
  /// \return The transform.
  mat4& instance_transform(render_mesh& render_mesh, uint32_t instance_index = 0);
  const mat4& instance_transform(const render_mesh& render_mesh, uint32_t instance_index = 0);

  ///
  /// Sets the texture of a render mesh.
  /// \param render_mesh The render mesh.
  /// \param texture The texture.
  /// \param instance_index The index of the instance to set the texture for.
  void set_instance_texture(render_mesh& render_mesh, const texture& texture, uint32_t instance_index = 0);

  ///
  /// Retrieves the bone transforms from a render mesh.
  /// \param render_mesh The render mesh.
  /// \param instance_index The index of the instance to retrieve the bone transforms for.
  /// \return The bone transforms.
  ludo::mat4* instance_bone_transforms(render_mesh& render_mesh, uint32_t instance_index = 0);
  const ludo::mat4* instance_bone_transforms(const render_mesh& render_mesh, uint32_t instance_index = 0);

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

  void de_init(texture& texture);

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
