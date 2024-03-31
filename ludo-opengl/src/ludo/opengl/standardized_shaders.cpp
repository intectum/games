/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/animation.h>

#include "standardized_shaders.h"

namespace ludo
{
  void write_header(std::ostream& stream, const vertex_format_options& options)
  {
    stream <<
R"--(
#version 460
#extension GL_ARB_bindless_texture : require
)--";
  }

  void write_types(std::ostream& stream, const vertex_format_options& options)
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

struct instance_t
{
  mat4 transform;
)--";

  if (options.texture) stream << "  sampler2D sampler;" << std::endl;

  stream <<
R"--(
};

struct point_t
{
  vec3 position;
  vec3 normal;
  vec4 color;
  vec2 tex_coords;
};
)--";
  }

  void write_inputs(std::ostream& stream, const vertex_format_options& options)
  {
    stream <<
R"--(
// Inputs

in vec3 position;
)--";
    if (options.normals) stream << "in vec3 normal;" << std::endl;
    if (options.colors) stream << "in vec4 color;" << std::endl;
    if (options.texture) stream << "in vec2 tex_coords;" << std::endl;
    if (options.bones) stream << "in ivec4 bone_indices;" << std::endl;
    if (options.bones) stream << "in vec4 bone_weights;" << std::endl;
  }

  void write_buffers(std::ostream& stream, const vertex_format_options& options)
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

// binding = 1 is reserved for the render_layout

// binding = 2 is reserved for the render_program_layout

layout(std430, binding = 3) buffer instance_layout
{
  instance_t instances[];
};

layout(std430, binding = 4) buffer animation_layout
{
  mat4 bone_transforms[];
};
)--";
  }

  void write_vertex_main(std::ostream& stream, const vertex_format_options& options)
  {
    stream <<
R"--(
void main()
{
  instance_t instance = instances[gl_DrawID];
  mat4 world_transform = instance.transform;
)--";

    if (options.bones)
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
    stream << "  point.color = " << (options.colors ? "color" : "vec4(1.0, 1.0, 1.0, 1.0)") << ";" << std::endl;
    if (options.normals) stream << "  point.normal = world_normal.xyz;" << std::endl;
    if (options.texture) stream << "  point.tex_coords = tex_coords;" << std::endl;
    stream << std::endl;
    if (options.texture) stream << "  sampler = instance.sampler;" << std::endl;
    stream << std::endl;
    stream << "  gl_Position = camera.view_projection * world_position;" << std::endl;
    stream << "}" << std::endl;
  }

  void write_lighting_functions(std::ostream& stream, const vertex_format_options& options)
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

  void write_fragment_main(std::ostream& stream, const vertex_format_options& options)
  {
    stream <<
R"--(
void main()
{
  point_t local_point = point;

)--";

    if (options.texture) stream << "  local_point.color = texture(sampler, point.tex_coords);" << std::endl;
    if (options.normals) stream << "  local_point.color = apply_point_light(local_point, lights[0], camera);" << std::endl;

    stream <<
R"--(
  color = local_point.color;
  color.a = 1.0;
}
)--";
    }
}
