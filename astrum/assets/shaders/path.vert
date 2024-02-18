#version 460
#extension GL_ARB_bindless_texture : require

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
};

// Inputs

in vec4 transform_columns[4];
in vec3 position;

// Buffers

layout(std430, binding = 0) buffer context_layout
{
  camera_t camera;
  uint light_count;
  light_t lights[];
};

layout(std430, binding = 2) buffer program_layout
{
  uint step_count;
  vec4 color[];
};

// Output

out point_t point;

void main()
{
  mat4 world_transform;
  world_transform[0] = transform_columns[0];
  world_transform[1] = transform_columns[1];
  world_transform[2] = transform_columns[2];
  world_transform[3] = transform_columns[3];

  vec4 world_position = world_transform * vec4(position, 1.0);
  uint index = gl_VertexID - gl_DrawID * step_count;

  point.position = world_position.xyz;
  point.color = vec4(color[gl_DrawID].rgb, 1.0f - (float(index) / step_count));

  gl_Position = camera.view_projection * world_position;
}
