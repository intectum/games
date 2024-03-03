/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <sstream>

#include "built_ins.h"

namespace ludo
{
  // TODO global?!
  std::unordered_map<std::string, uint64_t> render_programs;
  std::unordered_map<std::string, uint64_t> vertex_shaders;
  std::unordered_map<std::string, uint64_t> fragment_shaders;

  std::string key(const mesh_buffer_options& options);
  uint64_t add_render_program(instance& instance, const mesh_buffer_options& options);
  uint64_t add_vertex_shader(instance& instance, const mesh_buffer_options& options);
  uint64_t add_post_processing_vertex_shader(instance& instance);
  uint64_t add_fragment_shader(instance& instance, const mesh_buffer_options& options);
  void write_header(std::ostream& stream, const mesh_buffer_options& options);
  void write_types(std::ostream& stream, const mesh_buffer_options& options);
  void write_inputs(std::ostream& stream, const mesh_buffer_options& options);
  void write_buffers(std::ostream& stream, const mesh_buffer_options& options);
  void write_vertex_main(std::ostream& stream, const mesh_buffer_options& options);
  void write_lighting_functions(std::ostream& stream, const mesh_buffer_options& options);
  void write_fragment_main(std::ostream& stream, const mesh_buffer_options& options);

  uint64_t built_in_render_program(instance& instance, const mesh_buffer_options& options)
  {
    auto key = ludo::key(options);

    if (!render_programs.contains(key))
    {
      render_programs[key] = add_render_program(instance, options);
    }

    return render_programs[key];
  }

  uint64_t built_in_shader(instance& instance, const mesh_buffer_options& options, shader_type type)
  {
    auto key = ludo::key(options);

    if (type == shader_type::VERTEX)
    {
      if (!vertex_shaders.contains(key))
      {
        vertex_shaders[key] = add_vertex_shader(instance, options);
      }

      return vertex_shaders[key];
    }
    else if (type == shader_type::FRAGMENT)
    {
      if (!fragment_shaders.contains(key))
      {
        fragment_shaders[key] = add_fragment_shader(instance, options);
      }

      return fragment_shaders[key];
    }
    else
    {
      return 0;
    }
  }

  uint64_t post_processing_vertex_shader(instance& instance)
  {
    auto key = "post-processing";

    if (!vertex_shaders.contains(key))
    {
      vertex_shaders[key] = add_post_processing_vertex_shader(instance);
    }

    return vertex_shaders[key];
  }

  std::string key(const mesh_buffer_options& options)
  {
    auto key_stream = std::stringstream();
    key_stream << options.normals << "::" << options.colors << "::" << options.texture_count << "::" << options.bone_count;
    return key_stream.str();
  }

  uint64_t add_render_program(instance& instance, const mesh_buffer_options& options)
  {
    return add(
      instance,
      ludo::render_program
      {
        .vertex_shader_id = built_in_shader(instance, options, shader_type::VERTEX),
        .fragment_shader_id = built_in_shader(instance, options, shader_type::FRAGMENT)
      }
    )->id;
  }

  uint64_t add_vertex_shader(instance& instance, const mesh_buffer_options& options)
  {
    auto stream = std::stringstream();
    write_header(stream, options);
    write_types(stream, options);
    write_inputs(stream, options);
    write_buffers(stream, options);
    stream << std::endl;
    stream << "// Output" << std::endl;
    stream << std::endl;
    stream << "out point_t point;" << std::endl;
    write_vertex_main(stream, options);

    return add(instance, shader { .type = ludo::shader_type::VERTEX }, stream)->id;
  }

  uint64_t add_post_processing_vertex_shader(instance& instance)
  {
    auto options = mesh_buffer_options { .texture_count = 1 };

    auto stream = std::stringstream();
    write_header(stream, options);
    write_types(stream, options);
    write_inputs(stream, options);
    stream << std::endl;
    stream << "// Output" << std::endl;
    stream << std::endl;
    stream << "out point_t point;" << std::endl;

    stream <<
R"--(
void main()
{
  point.position = position;
  point.tex_coords[0] = tex_coord_0;

  gl_Position = vec4(position, 1.0);
}
)--";

    return add(instance, shader { .type = ludo::shader_type::VERTEX }, stream)->id;
  }

  uint64_t add_fragment_shader(instance& instance, const mesh_buffer_options& options)
  {
    auto stream = std::stringstream();
    write_header(stream, options);
    write_types(stream, options);
    stream << std::endl;
    stream << "// Input" << std::endl;
    stream << std::endl;
    stream << "in point_t point;" << std::endl;
    write_buffers(stream, options);
    stream << std::endl;
    stream << "// Output" << std::endl;
    stream << std::endl;
    stream << "out vec4 color;" << std::endl;
    write_lighting_functions(stream, options);
    write_fragment_main(stream, options);

    return add(instance, shader { .type = ludo::shader_type::FRAGMENT }, stream)->id;
  }

  void write_header(std::ostream& stream, const mesh_buffer_options& options)
  {
    stream <<
R"--(
#version 460
#extension GL_ARB_bindless_texture : require
)--";
  }

  void write_types(std::ostream& stream, const mesh_buffer_options& options)
  {
    stream <<
R"--(
// Types

struct camera_t
{
  float near_clipping_distance;
  float far_clipping_distance;

  mat4 view;
  mat4 projection;

  vec3 position;
  mat4 view_projection;
};

struct light_t
{
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  vec3 position;
  vec3 direction;
  vec3 attenuation;
  float strength;
  float range;
};

struct point_t
{
  vec3 position;
  vec3 normal;
  vec4 color;
)--";

    if (options.texture_count) stream << "  vec2 tex_coords[" << static_cast<uint32_t>(options.texture_count) << "];" << std::endl;
    stream << "};" << std::endl;
  }

  void write_inputs(std::ostream& stream, const mesh_buffer_options& options)
  {
    stream <<
R"--(
// Inputs

in vec4 transform_columns[4];
in vec3 position;
)--";
    if (options.normals) stream << "in vec3 normal;" << std::endl;
    if (options.colors) stream << "in vec4 color;" << std::endl;
    for (auto index = 0; index < options.texture_count; index++)
    {
      stream << "in vec2 tex_coord_" << index << ";" << std::endl;
    }
    if (options.bone_count) stream << "in ivec4 bone_indices;" << std::endl;
    if (options.bone_count) stream << "in vec4 bone_weights;" << std::endl;
  }

  void write_buffers(std::ostream& stream, const mesh_buffer_options& options)
  {
    stream <<
R"--(
// Buffers

layout(std430, binding = 0) buffer context_layout
{
  camera_t camera;
  uint light_count;
  light_t lights[];
};
)--";

    if (options.texture_count)
    {
      stream <<
R"--(
layout(std430, binding = 3) buffer texture_layout
{
  sampler2D samplers[)--" << static_cast<uint32_t>(options.texture_count) << R"--(];
};
)--";
    }

    if (options.bone_count)
    {
      stream <<
R"--(
layout(std430, binding = 4) buffer animation_layout
{
  mat4 bone_transforms[];
};
)--";
    }
  }

  void write_vertex_main(std::ostream& stream, const mesh_buffer_options& options)
  {
    stream <<
R"--(
void main()
{
  mat4 world_transform;
  world_transform[0] = transform_columns[0];
  world_transform[1] = transform_columns[1];
  world_transform[2] = transform_columns[2];
  world_transform[3] = transform_columns[3];
)--";

    if (options.bone_count)
    {
      stream <<
R"--(
  // Animation
  mat4 bone_transform = bone_transforms[bone_indices[0]] * bone_weights[0];
  bone_transform += bone_transforms[bone_indices[1]] * bone_weights[1];
  bone_transform += bone_transforms[bone_indices[2]] * bone_weights[2];
  bone_transform += bone_transforms[bone_indices[3]] * bone_weights[3];
  world_transform *= bone_transform;
)--";
    }

    stream << std::endl;
    stream << "  vec4 world_position = world_transform * vec4(position, 1.0);" << std::endl;

    if (options.normals)
    {
      stream <<
R"--(
  mat4 world_rotation = world_transform;
  world_rotation[3][0] = 0.0;
  world_rotation[3][1] = 0.0;
  world_rotation[3][2] = 0.0;
  world_rotation[3][3] = 1.0;
  vec4 world_normal = world_rotation * vec4(normal, 1.0);
)--";
    }

    stream << std::endl;
    stream << "  point.position = world_position.xyz;" << std::endl;
    if (options.normals) stream << "  point.normal = world_normal.xyz;" << std::endl;
    stream << "  point.color = " << (options.colors ? "color" : "vec4(1.0, 1.0, 1.0, 1.0)") << ";" << std::endl;
    for (auto index = 0; index < options.texture_count; index++)
    {
      stream << "  point.tex_coords[" << index << "] = tex_coord_" << index << ";" << std::endl;
    }
    stream << std::endl;
    stream << "  gl_Position = camera.view_projection * world_position;" << std::endl;
    stream << "}" << std::endl;
  }

  void write_lighting_functions(std::ostream& stream, const mesh_buffer_options& options)
  {
    stream <<
R"--(
// Lighting

vec4 apply_directional_light(point_t point, light_t light, camera_t camera)
{
  // Add the ambient term.
  vec4 color = point.color * light.ambient;

  float diffuse_factor = dot(-light.direction, point.normal);
  if(diffuse_factor > 0.0)
  {
    // Add the diffuse term.
    color += diffuse_factor * point.color * light.diffuse;

    // Add the specular term.
    vec3 to_camera = normalize(camera.position - point.position);
    vec3 light_reflect = normalize(reflect(light.direction, point.normal));
    float specular_factor = dot(to_camera, light_reflect);

    specular_factor = pow(specular_factor, light.strength);
    if (specular_factor > 0.0)
    {
      color += specular_factor * light.specular;
    }
  }

  return color;
}

vec4 apply_point_light(point_t point, light_t light, camera_t camera)
{
  vec3 to_light = light.position - point.position;
  float distance_to_light = length(to_light);
  to_light /= distance_to_light;

  // If the point is out of range, do not light it.
  if(distance_to_light > light.range)
  {
    return vec4(0.0, 0.0, 0.0, 1.0);
  }

  // Add the ambient term.
  vec4 color = point.color * light.ambient;

  float diffuse_factor = dot(to_light, point.normal);
  if(diffuse_factor > 0.0)
  {
    // Add the diffuse term.
    color += diffuse_factor * point.color * light.diffuse;

    // Add the specular term.
    vec3 to_camera = normalize(camera.position - point.position);
    vec3 light_reflect = normalize(reflect(-to_light, point.normal));
    float specular_factor = dot(to_camera, light_reflect);

    specular_factor = pow(specular_factor, light.strength);
    if (specular_factor > 0.0)
    {
      color += specular_factor * light.specular;
    }
  }

  // Attenuate
  color /= dot(light.attenuation, vec3(1.0f, distance_to_light, distance_to_light * distance_to_light));

  return color;
}

vec4 apply_spot_light(point_t point, light_t light, camera_t camera)
{
  vec3 to_light = normalize(light.position - point.position);

  return apply_point_light(point, light, camera) *
         pow(max(dot(-to_light, light.direction), 0.0), light.strength);
}
)--";
  }

  void write_fragment_main(std::ostream& stream, const mesh_buffer_options& options)
  {
    stream <<
R"--(
void main()
{
  point_t local_point = point;
  )--" << (options.texture_count ? "local_point.color = texture(samplers[0], point.tex_coords[0]);" : "") << R"--(

  color = )--" << (options.normals ? "apply_point_light(local_point, lights[0], camera);" : "local_point.color;") << R"--(

  color.a = 1.0;
}
)--";
  }
}
