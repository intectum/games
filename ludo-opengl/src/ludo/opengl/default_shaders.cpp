/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/animation.h>

#include "default_shaders.h"

namespace ludo
{
  std::stringstream default_vertex_shader_code(const vertex_format& format)
  {
    auto code = std::stringstream();
    write_header(code, format);
    code << std::endl;
    write_types(code, format);
    code << std::endl;
    write_inputs(code, format);
    code << std::endl;
    write_buffers(code, format);
    code << std::endl;
    code << "// Output" << std::endl;
    code << std::endl;
    code << "out point_t point;" << std::endl;
    code << "out sampler2D sampler;" << std::endl;
    code << std::endl;
    write_vertex_main(code, format);

    return code;
  }

  std::stringstream default_fragment_shader_code(const vertex_format& format)
  {
    auto code = std::stringstream();
    write_header(code, format);
    code << std::endl;
    write_types(code, format);
    code << std::endl;
    code << "// Input" << std::endl;
    code << std::endl;
    code << "in point_t point;" << std::endl;
    code << "in flat sampler2D sampler;" << std::endl;
    code << std::endl;
    write_buffers(code, format);
    code << std::endl;
    code << "// Output" << std::endl;
    code << std::endl;
    code << "out vec4 color;" << std::endl;
    write_lighting_functions(code, format);
    code << std::endl;
    write_fragment_main(code, format);

    return code;
  }

  void write_header(std::ostream& stream, const vertex_format& format)
  {
    stream << "#version 460 core" << std::endl;
    stream << "#extension GL_ARB_bindless_texture : require" << std::endl;
  }

  void write_types(std::ostream& stream, const vertex_format& format)
  {
    stream << R"--(// Types

struct point_t
{
  vec3 position;
  vec3 normal;
  vec4 color;
  vec2 tex_coords;
};
)--";
  }

  void write_inputs(std::ostream& stream, const vertex_format& format)
  {
    stream << "// Inputs" << std::endl;
    stream << std::endl;
    stream << "in vec3 position;" << std::endl;
    if (format.has_normal) stream << "in vec3 normal;" << std::endl;
    if (format.has_color) stream << "in vec4 color;" << std::endl;
    if (format.has_texture_coordinate) stream << "in vec2 tex_coords;" << std::endl;
    if (format.has_bone_weights) stream << "in ivec4 bone_indices;" << std::endl;
    if (format.has_bone_weights) stream << "in vec4 bone_weights;" << std::endl;
  }

  void write_buffers(std::ostream& stream, const vertex_format& format)
  {
    stream << R"--(// Buffers

layout(std430, binding = 0) buffer camera_position_layout
{
  vec3 camera_positions[];
};

layout(std430, binding = 1) buffer camera_view_projection_layout
{
  mat4 camera_view_projections[];
};

layout(std430, binding = 10) buffer light_ambient_color_layout
{
  vec4 light_ambient_colors[];
};

layout(std430, binding = 11) buffer light_diffuse_color_layout
{
  vec4 light_diffuse_colors[];
};

layout(std430, binding = 12) buffer light_specular_color_layout
{
  vec4 light_specular_colors[];
};

layout(std430, binding = 13) buffer light_position_layout
{
  vec3 light_positions[];
};

layout(std430, binding = 14) buffer light_direction_layout
{
  vec3 light_directions[];
};

layout(std430, binding = 15) buffer light_attenuation_layout
{
  vec3 light_attenuations[];
};

layout(std430, binding = 16) buffer light_strength_layout
{
  float light_strengths[];
};

layout(std430, binding = 17) buffer light_range_layout
{
  float light_ranges[];
};

layout(std430, binding = 20) buffer transform_layout
{
  mat4 transforms[];
};

layout(std430, binding = 21) buffer sampler_layout
{
  sampler2D samplers[];
};

layout(std430, binding = 22) buffer bone_transform_layout
{
  mat4 bone_transforms[];
};
)--";
  }

  void write_vertex_main(std::ostream& stream, const vertex_format& format)
  {
    stream << "void main()" << std::endl;
    stream << "{" << std::endl;
    stream << "  int index = gl_BaseInstance + gl_InstanceID;" << std::endl;
    stream << "  mat4 world_transform = transforms[index];" << std::endl;
    stream << std::endl;
    if (format.has_bone_weights)
    {
      stream << "  // Animation" << std::endl;
      stream << "  uint bone_transforms_base_index = index * " << max_bones_per_armature << ";" << std::endl;
      stream << "  mat4 bone_transform = bone_transforms[bone_transforms_base_index + bone_indices[0]] * bone_weights[0];" << std::endl;
      stream << "  bone_transform += bone_transforms[bone_transforms_base_index + bone_indices[1]] * bone_weights[1];" << std::endl;
      stream << "  bone_transform += bone_transforms[bone_transforms_base_index + bone_indices[2]] * bone_weights[2];" << std::endl;
      stream << "  bone_transform += bone_transforms[bone_transforms_base_index + bone_indices[3]] * bone_weights[3];" << std::endl;
      stream << "  world_transform *= bone_transform;" << std::endl;
      stream << std::endl;
    }
    stream << "  vec4 world_position = world_transform * vec4(position, 1.0);" << std::endl;
    stream << std::endl;
    if (format.has_normal)
    {
      stream << "  mat4 world_rotation = world_transform;" << std::endl;
      stream << "  world_rotation[3][0] = 0.0;" << std::endl;
      stream << "  world_rotation[3][1] = 0.0;" << std::endl;
      stream << "  world_rotation[3][2] = 0.0;" << std::endl;
      stream << "  world_rotation[3][3] = 1.0;" << std::endl;
      stream << "  vec4 world_normal = world_rotation * vec4(normal, 1.0);" << std::endl;
    }
    stream << std::endl;
    stream << "  point.position = world_position.xyz;" << std::endl;
    stream << "  point.color = " << (format.has_color ? "color" : "vec4(1.0, 1.0, 1.0, 1.0)") << ";" << std::endl;
    if (format.has_normal) stream << "  point.normal = world_normal.xyz;" << std::endl;
    if (format.has_texture_coordinate) stream << "  point.tex_coords = tex_coords;" << std::endl;
    stream << std::endl;
    if (format.has_texture_coordinate) stream << "  sampler = samplers[index];" << std::endl;
    stream << std::endl;
    stream << "  gl_Position = camera.view_projection * world_position;" << std::endl;
    stream << "}" << std::endl;
  }

  void write_lighting_functions(std::ostream& stream, const vertex_format& format)
  {
    stream << R"--(// Lighting

vec4 apply_directional_light(
  point_t point,
  vec4 light_ambient_color,
  vec4 light_diffuse_color,
  vec4 light_specular_color,
  vec3 light_direction,
  float light_strength,
  vec3 camera_position
)
{
  // Add the ambient term.
  vec4 color = point.color * light_ambient_color;

  float diffuse_factor = dot(-light_direction, point.normal);
  if(diffuse_factor > 0.0)
  {
    // Add the diffuse term.
    color += diffuse_factor * point.color * light_diffuse_color;

    // Add the specular term.
    vec3 to_camera = normalize(camera_position - point.position);
    vec3 light_reflect = normalize(reflect(light_direction, point.normal));
    float specular_factor = dot(to_camera, light_reflect);

    specular_factor = pow(specular_factor, light_strength);
    if (specular_factor > 0.0)
    {
      color += specular_factor * light_specular_color;
    }
  }

  return color;
}

vec4 apply_point_light(
  point_t point,
  vec4 light_ambient_color,
  vec4 light_diffuse_color,
  vec4 light_specular_color,
  vec3 light_position,
  vec3 light_attenuation,
  float light_strength,
  float light_range,
  vec3 camera_position
)
{
  vec3 to_light = light_position - point.position;
  float distance_to_light = length(to_light);
  to_light /= distance_to_light;

  // If the point is out of range, do not light it.
  if(distance_to_light > light_range)
  {
    return vec4(0.0, 0.0, 0.0, 1.0);
  }

  // Add the ambient term.
  vec4 color = point.color * light_ambient_color;

  float diffuse_factor = dot(to_light, point.normal);
  if(diffuse_factor > 0.0)
  {
    // Add the diffuse term.
    color += diffuse_factor * point.color * light_diffuse_color;

    // Add the specular term.
    vec3 to_camera = normalize(camera_position - point.position);
    vec3 light_reflect = normalize(reflect(-to_light, point.normal));
    float specular_factor = dot(to_camera, light_reflect);

    specular_factor = pow(specular_factor, light_strength);
    if (specular_factor > 0.0)
    {
      color += specular_factor * light_specular_color;
    }
  }

  // Attenuate
  color /= dot(light_attenuation, vec3(1.0f, distance_to_light, distance_to_light * distance_to_light));

  return color;
}

vec4 apply_spot_light(
  point_t point,
  vec4 light_ambient_color,
  vec4 light_diffuse_color,
  vec4 light_specular_color,
  vec3 light_position,
  vec3 light_direction,
  vec3 light_attenuation,
  float light_strength,
  float light_range,
  vec3 camera_position
)
{
  vec3 to_light = normalize(light_position - point.position);
  float spot_factor = pow(max(dot(-to_light, light_direction), 0.0), light_strength);

  return apply_point_light(
    point,
    light_ambient_color,
    light_diffuse_color,
    light_specular_color,
    light_position,
    light_attenuation,
    light_strength,
    light_range,
    camera_position
  ) * spot_factor;
}
)--";
  }

  void write_fragment_main(std::ostream& stream, const vertex_format& format)
  {
    stream << "void main()" << std::endl;
    stream << "{" << std::endl;
    stream << "  point_t local_point = point;" << std::endl;
    stream << std::endl;
    if (format.has_texture_coordinate) stream << "  local_point.color = texture(sampler, point.tex_coords);" << std::endl;
    if (format.has_normal)
    {
      stream << "  local_point.color = apply_point_light(" << std::endl;
      stream << "    local_point," << std::endl;
      stream << "    light_ambient_colors[0]," << std::endl;
      stream << "    light_diffuse_colors[0]," << std::endl;
      stream << "    light_specular_colors[0]," << std::endl;
      stream << "    light_positions[0]," << std::endl;
      stream << "    light_attenuations[0]," << std::endl;
      stream << "    light_strengths[0]," << std::endl;
      stream << "    light_ranges[0]," << std::endl;
      stream << "    camera_positions[0]" << std::endl;
      stream << "  );" << std::endl;
    }
    stream << std::endl;
    stream << "  color = local_point.color;" << std::endl;
    stream << "  color.a = 1.0;" << std::endl;
    stream << "}" << std::endl;
  }
}
