/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "rendering_contexts.h"
#include "util.h"

namespace ludo
{
  template<>
  rendering_context* add(instance& instance, const rendering_context& init, const std::string& partition)
  {
    auto rendering_context = add(data<ludo::rendering_context>(instance), init, partition);
    rendering_context->id = next_id++;

    glewInit();

    // Sometimes glewInit() gives false negatives. Let's clear the OpenGL error, so it doesn't confuse us elsewhere.
    glGetError();

    // Ensure objects further from the viewpoint are not drawn over the top of closer objects. To assist multi
    // pass rendering, objects at the exact same distance can be rendered over (i.e. the object will be rendered
    // using the result of the last draw).
    glDepthFunc(GL_LEQUAL); check_opengl_error();
    glEnable(GL_DEPTH_TEST); check_opengl_error();

    // Only render the front (counter-clockwise) side of a polygon
    glEnable(GL_CULL_FACE); check_opengl_error();

    // Enable blending for rendering transparency
    glEnable(GL_BLEND); check_opengl_error();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); check_opengl_error();

    // Enable multisampling for anti-aliasing
    glEnable(GL_MULTISAMPLE); check_opengl_error();

    return rendering_context;
  }

  rendering_context* add(instance& instance, const rendering_context& init, uint32_t light_count, const std::string& partition)
  {
    auto rendering_context = add(instance, init, partition);

    auto camera_size = 224;
    auto light_size = 112;
    auto data_size = camera_size + 16 + light_count * light_size;

    rendering_context->shader_buffer = allocate_vram(data_size);

    // Default camera.
    set_camera(*rendering_context, camera
    {
      .view = mat4_identity,
      .projection = perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f)
    });

    write(rendering_context->shader_buffer, camera_size, light_count);

    // Lights that don't do anything...
    for (auto index = 0; index < light_count; index++)
    {
      set_light(*rendering_context, light
      {
        .ambient = vec4_zero,
        .diffuse = vec4_zero,
        .specular = vec4_zero,
        .position = vec3_zero,
        .direction = vec3_zero,
        .attenuation = vec3_zero,
        .strength = 0,
        .range = 0
      }, index);
    }

    return rendering_context;
  }

  template<>
  void remove<rendering_context>(instance& instance, rendering_context* element, const std::string& partition)
  {
    if (element->shader_buffer.size)
    {
      deallocate_vram(element->shader_buffer);
    }

    remove(data<rendering_context>(instance), element, partition);
  }

  camera get_camera(const rendering_context& rendering_context)
  {
    auto camera = ludo::camera();

    // In GLSL std430 buffer layout, data is packed into 16 byte aligned format.

    auto byte_index = 0;
    camera.near_clipping_distance = read<float>(rendering_context.shader_buffer, byte_index);
    byte_index += 4;
    camera.far_clipping_distance = read<float>(rendering_context.shader_buffer, byte_index);
    byte_index += 4;
    byte_index += 8; // Padding to 16 bytes
    camera.view = read<mat4>(rendering_context.shader_buffer, byte_index);
    byte_index += 64;
    camera.projection = read<mat4>(rendering_context.shader_buffer, byte_index);

    return camera;
  }

  void set_camera(rendering_context& rendering_context, const camera& camera)
  {
    auto view_inverse = camera.view;
    invert(view_inverse);
    auto view_projection = camera.projection * view_inverse;

    // In GLSL std430 buffer layout, data is packed into 16 byte aligned format.

    auto byte_index = 0;
    write(rendering_context.shader_buffer, byte_index, camera.near_clipping_distance);
    byte_index += 4;
    write(rendering_context.shader_buffer, byte_index, camera.far_clipping_distance);
    byte_index += 4;
    byte_index += 8; // Padding to 16 bytes
    write(rendering_context.shader_buffer, byte_index, camera.view);
    byte_index += 64;
    write(rendering_context.shader_buffer, byte_index, camera.projection);
    byte_index += 64;
    write(rendering_context.shader_buffer, byte_index, position(camera.view));
    byte_index += 12;
    byte_index += 4; // Padding to 16 bytes
    write(rendering_context.shader_buffer, byte_index, view_projection);
  }

  light get_light(const rendering_context& rendering_context, uint8_t index)
  {
    auto light = ludo::light();

    auto camera_size = 224;
    auto light_size = 112;

    assert(index >= 0 && index < read<uint32_t>(rendering_context.shader_buffer, camera_size) && "index out of bounds");

    // In GLSL std430 buffer layout, data is packed into 16 byte aligned format.

    auto byte_index = camera_size + 16 + index * light_size;
    light.ambient = read<vec4>(rendering_context.shader_buffer, byte_index);
    byte_index += 16;
    light.diffuse = read<vec4>(rendering_context.shader_buffer, byte_index);
    byte_index += 16;
    light.specular = read<vec4>(rendering_context.shader_buffer, byte_index);
    byte_index += 16;
    light.position = read<vec3>(rendering_context.shader_buffer, byte_index);
    byte_index += 12;
    byte_index += 4; // Padding to 16 bytes
    light.direction = read<vec3>(rendering_context.shader_buffer, byte_index);
    byte_index += 12;
    byte_index += 4; // Padding to 16 bytes
    light.attenuation = read<vec3>(rendering_context.shader_buffer, byte_index);
    byte_index += 12;
    light.strength = read<float>(rendering_context.shader_buffer, byte_index);
    byte_index += 4;
    light.range = read<float>(rendering_context.shader_buffer, byte_index);

    return light;
  }

  void set_light(rendering_context& rendering_context, const light& light, uint8_t index)
  {
    auto camera_size = 224;
    auto light_size = 112;

    assert(index >= 0 && index < read<uint32_t>(rendering_context.shader_buffer, camera_size) && "index out of bounds");

    // In GLSL std430 buffer layout, data is packed into 16 byte aligned format.

    auto byte_index = camera_size + 16 + index * light_size;
    write(rendering_context.shader_buffer, byte_index, light.ambient);
    byte_index += 16;
    write(rendering_context.shader_buffer, byte_index, light.diffuse);
    byte_index += 16;
    write(rendering_context.shader_buffer, byte_index, light.specular);
    byte_index += 16;
    write(rendering_context.shader_buffer, byte_index, light.position);
    byte_index += 12;
    byte_index += 4; // Padding to 16 bytes
    write(rendering_context.shader_buffer, byte_index, light.direction);
    byte_index += 12;
    byte_index += 4; // Padding to 16 bytes
    write(rendering_context.shader_buffer, byte_index, light.attenuation);
    byte_index += 12;
    write(rendering_context.shader_buffer, byte_index, light.strength);
    byte_index += 4;
    write(rendering_context.shader_buffer, byte_index, light.range);
  }

  void bind(const rendering_context& rendering_context)
  {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, rendering_context.shader_buffer.id); check_opengl_error();
  }
}
