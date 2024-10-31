#version 460 core
#extension GL_ARB_bindless_texture : require

// Types

struct instance_t
{
  vec3 position;
  vec3 size;
  vec4 color;
};

struct point_t
{
  vec4 color;
};

// Inputs

in vec3 position;

// Buffers

layout(std430, binding = 2) buffer instance_layout
{
  instance_t instances[];
};

// Output

out point_t point;

const float min = -1.0;
const float max = 1.0;
const float size = max - min;

void main()
{
  instance_t instance = instances[gl_BaseInstance + gl_InstanceID];

  point.color = instance.color;

  float clip_x = min + instance.position.x / 1920.0 * size;
  float clip_y = min + instance.position.y / 1080.0 * size;
  float clip_width = instance.size.x / 1920.0 * size;
  float clip_height = instance.size.y / 1080.0 * size;

  float x = clip_x + clip_width * position.x;
  float y = clip_y + clip_height * position.y;

  gl_Position = vec4(x, y, 0.0, 1.0);
}
