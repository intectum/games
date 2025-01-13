/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/opengl/default_shaders.h>

#include "lod_shaders.h"

namespace astrum
{
  void write_lod_types(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform);
  void write_lod_inputs(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform);
  void write_lod_buffers(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform);
  void write_lod_vertex_main(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform);
  void write_lod_fragment_main(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform);

  std::stringstream lod_vertex_shader_code(const ludo::vertex_format& format, bool shared_transform)
  {
    auto code = std::stringstream();
    write_header(code, format);
    write_lod_types(code, format, shared_transform);
    write_lod_inputs(code, format, shared_transform);
    write_lod_buffers(code, format, shared_transform);
    code << std::endl;
    code << "// Output" << std::endl;
    code << std::endl;
    code << "out point_t point;" << std::endl;
    code << "out sampler2D sampler;" << std::endl;
    write_lod_vertex_main(code, format, shared_transform);

    return code;
  }

  std::stringstream lod_fragment_shader_code(const ludo::vertex_format& format, bool shared_transform)
  {
    auto code = std::stringstream();
    write_header(code, format);
    write_lod_types(code, format, shared_transform);
    code << std::endl;
    code << "// Input" << std::endl;
    code << std::endl;
    code << "in point_t point;" << std::endl;
    code << "in flat sampler2D sampler;" << std::endl;
    write_lod_buffers(code, format, shared_transform);
    code << std::endl;
    code << "// Output" << std::endl;
    code << std::endl;
    code << "out vec4 color;" << std::endl;
    write_lighting_functions(code, format);
    write_lod_fragment_main(code, format, shared_transform);

    return code;
  }

  void write_lod_types(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform)
  {
    stream <<
R"--(
// Types

struct lod_t
{
  float low_detail_distance;
  float high_detail_distance;
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

  void write_lod_inputs(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform)
  {
    stream <<
R"--(
// Inputs

in vec3 position;
)--";

    if (format.has_normal) stream << "in vec3 normal;" << std::endl;
    if (format.has_color) stream << "in vec4 color;" << std::endl;
    stream << "in vec3 low_detail_position;" << std::endl;
    if (format.has_normal) stream << "in vec3 low_detail_normal;" << std::endl;
    if (format.has_color) stream << "in vec4 low_detail_color;" << std::endl;
  }

  void write_lod_buffers(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform)
  {
    stream <<
R"--(
// Buffers

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

layout(std430, binding = 21) buffer lod_layout
{
  lod_t lods[];
};

layout(std430, binding = 22) buffer lod_index_layout
{
  uint lod_indices[];
};
)--";
  }

  void write_lod_vertex_main(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform)
  {
    stream << "void main()" << std::endl;
    stream << "{" << std::endl;
    stream << "  int index = gl_BaseInstance + gl_InstanceID;" << std::endl;
    if (!shared_transform) stream << "  mat4 world_transform = transforms[index];" << std::endl;
    if (shared_transform) stream << "  mat4 world_transform = transforms[0];" << std::endl;
    stream << "  lod_t lod = lods[lod_indices[index]];" << std::endl;
    stream << std::endl;
    stream << "  vec4 world_position = world_transform * vec4(position, 1.0);" << std::endl;
    stream << "  vec4 low_detail_world_position = world_transform * vec4(low_detail_position, 1.0);" << std::endl;
    stream << std::endl;
    if (format.has_normal)
    {
      stream << "  mat4 world_rotation = world_transform;" << std::endl;
      stream << "  world_rotation[3][0] = 0.0;" << std::endl;
      stream << "  world_rotation[3][1] = 0.0;" << std::endl;
      stream << "  world_rotation[3][2] = 0.0;" << std::endl;
      stream << "  world_rotation[3][3] = 1.0;" << std::endl;
      stream << "  vec4 world_normal = world_rotation * vec4(normal, 1.0);" << std::endl;
      stream << "  vec4 low_detail_world_normal = world_rotation * vec4(low_detail_normal, 1.0);" << std::endl;
      stream << std::endl;
    }
    stream << "  float distance = length(world_position.xyz - camera_positions[0]);" << std::endl;
    stream << "  float interpolation_range = lod.low_detail_distance - lod.high_detail_distance;" << std::endl;
    stream << "  float interpolation_time = clamp((distance - lod.high_detail_distance) / interpolation_range, 0.0, 1.0);" << std::endl;
    stream << "  vec4 interpolated_world_position = world_position + (low_detail_world_position - world_position) * interpolation_time;" << std::endl;
    if (format.has_normal) stream << "  vec4 interpolated_world_normal = world_normal + (low_detail_world_normal - world_normal) * interpolation_time;" << std::endl;
    if (format.has_normal) stream << "  normalize(interpolated_world_normal);" << std::endl;
    if (format.has_color) stream << "  vec4 interpolated_color = color + (low_detail_color - color) * interpolation_time;" << std::endl;
    stream << std::endl;
    stream << "  point.position = interpolated_world_position.xyz;" << std::endl;
    if (format.has_normal) stream << "  point.normal = interpolated_world_normal.xyz;" << std::endl;
    stream << "  point.color = " << (format.has_color ? "interpolated_color" : "vec4(1.0, 1.0, 1.0, 1.0)") << ";" << std::endl;
    stream << std::endl;
    stream << "  gl_Position = camera_view_projections[0] * interpolated_world_position;" << std::endl;
    stream << "}" << std::endl;
  }

  void write_lod_fragment_main(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform)
  {
    stream << "void main()" << std::endl;
    stream << "{" << std::endl;
    stream << "  point_t local_point = point;" << std::endl;
    stream << std::endl;
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
