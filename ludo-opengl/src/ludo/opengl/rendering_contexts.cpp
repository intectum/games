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

    rendering_context->shader_buffer = allocate_dual(data_size);

    // Default camera.
    set_camera(*rendering_context, camera
    {
      .view = mat4_identity,
      .projection = perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f)
    });

    cast<uint32_t>(rendering_context->shader_buffer.back, camera_size) = light_count;

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
    if (element->shader_buffer.back.size)
    {
      deallocate_dual(element->shader_buffer);
    }

    remove(data<rendering_context>(instance), element, partition);
  }

  camera get_camera(const rendering_context& rendering_context)
  {
    auto stream = ludo::stream(rendering_context.shader_buffer.back);
    auto camera = ludo::camera();

    camera.near_clipping_distance = read<float>(stream);
    camera.far_clipping_distance = read<float>(stream);
    stream.position += 8; // align 16
    camera.view = read<mat4>(stream);
    camera.projection = read<mat4>(stream);

    return camera;
  }

  void set_camera(rendering_context& rendering_context, const camera& camera)
  {
    auto stream = ludo::stream(rendering_context.shader_buffer.back);
    auto view_inverse = camera.view;
    invert(view_inverse);
    auto view_projection = camera.projection * view_inverse;

    write(stream, camera.near_clipping_distance);
    write(stream, camera.far_clipping_distance);
    stream.position += 8; // align 16
    write(stream, camera.view);
    write(stream, camera.projection);
    write(stream, position(camera.view));
    stream.position += 4; // align 16
    write(stream, view_projection);
  }

  light get_light(const rendering_context& rendering_context, uint8_t index)
  {
    auto light = ludo::light();

    auto camera_size = 224;
    auto light_size = 112;

    assert(index >= 0 && index < cast<uint32_t>(rendering_context.shader_buffer.back, camera_size) && "index out of bounds");

    auto stream = ludo::stream(rendering_context.shader_buffer.back, camera_size + 16 + index * light_size);
    light.ambient = read<vec4>(stream);
    light.diffuse = read<vec4>(stream);
    light.specular = read<vec4>(stream);
    light.position = read<vec3>(stream);
    stream.position += 4; // align 16
    light.direction = read<vec3>(stream);
    stream.position += 4; // align 16
    light.attenuation = read<vec3>(stream);
    light.strength = read<float>(stream);
    light.range = read<float>(stream);

    return light;
  }

  void set_light(rendering_context& rendering_context, const light& light, uint8_t index)
  {
    auto camera_size = 224;
    auto light_size = 112;

    assert(index >= 0 && index < cast<uint32_t>(rendering_context.shader_buffer.back, camera_size) && "index out of bounds");

    auto stream = ludo::stream(rendering_context.shader_buffer.back, camera_size + 16 + index * light_size);
    write(stream, light.ambient);
    write(stream, light.diffuse);
    write(stream, light.specular);
    write(stream, light.position);
    stream.position += 4; // align 16
    write(stream, light.direction);
    stream.position += 4; // align 16
    write(stream, light.attenuation);
    write(stream, light.strength);
    write(stream, light.range);
  }

  void bind(rendering_context& rendering_context)
  {
    push(rendering_context.shader_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, rendering_context.shader_buffer.front.id); check_opengl_error();
  }
}
