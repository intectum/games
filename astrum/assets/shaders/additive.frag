#version 460
#extension GL_ARB_bindless_texture : require

// Types

struct point_t
{
  vec3 position;
  vec3 normal;
  vec4 color;
  vec2 tex_coords[1];
};

// Input

in point_t point;

// Buffers

layout(std430, binding = 1) buffer node_layout
{
  sampler2D color_sampler_0;
  sampler2D color_sampler_1;
};

// Output

out vec4 color;

void main()
{
  color = texture(color_sampler_0, point.tex_coords[0]) + texture(color_sampler_1, point.tex_coords[0]);
}
