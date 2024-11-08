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

  void write_lod_types(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform) {
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

struct lod_t
{
  float low_detail_distance;
  float high_detail_distance;
};

struct instance_t
{
)--";

    if (!shared_transform) stream << "  mat4 transform;" << std::endl;

    stream <<
R"--(
  uint lod_index;
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

  void write_lod_inputs(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform) {
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

  void write_lod_buffers(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform) {
    stream <<
R"--(
// Buffers

layout(std430, binding = 0) buffer rendering_context_layout
{
  camera_t camera;
  uint light_count;
  light_t lights[];
};

layout(std430, binding = 1) buffer render_program_layout
{
)--";

    if (shared_transform) stream << "  mat4 transform;" << std::endl;

    stream <<
R"--(
    lod_t lods[];
};

layout(std430, binding = 2) buffer instance_layout
{
  instance_t instances[];
};
)--";
  }

  void write_lod_vertex_main(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform) {
    stream <<
R"--(
void main()
{
  instance_t instance = instances[gl_BaseInstance + gl_InstanceID];
  lod_t lod = lods[instance.lod_index];

)--";

    if (!shared_transform) stream << "  mat4 world_transform = instance.transform;" << std::endl;
    if (shared_transform) stream << "  mat4 world_transform = transform;" << std::endl;
    stream << std::endl;
    stream << "  vec4 world_position = world_transform * vec4(position, 1.0);" << std::endl;
    stream << "  vec4 low_detail_world_position = world_transform * vec4(low_detail_position, 1.0);" << std::endl;

    if (format.has_normal)
    {
      stream <<
R"--(
  mat4 world_rotation = world_transform;
  world_rotation[3][0] = 0.0;
  world_rotation[3][1] = 0.0;
  world_rotation[3][2] = 0.0;
  world_rotation[3][3] = 1.0;
  vec4 world_normal = world_rotation * vec4(normal, 1.0);
  vec4 low_detail_world_normal = world_rotation * vec4(low_detail_normal, 1.0);
)--";
    }

    stream << "  float distance = length(world_position.xyz - camera.position);" << std::endl;
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
    stream << "  gl_Position = camera.view_projection * interpolated_world_position;" << std::endl;
    stream << "}" << std::endl;
  }

  void write_lod_fragment_main(std::ostream& stream, const ludo::vertex_format& format, bool shared_transform)
  {
    stream <<
R"--(
void main()
{
  point_t local_point = point;

)--";

    if (format.has_normal) stream << "  local_point.color = apply_point_light(local_point, lights[0], camera);" << std::endl;

    stream <<
R"--(
  color = local_point.color;
  color.a = 1.0;
}
)--";
  }
}
