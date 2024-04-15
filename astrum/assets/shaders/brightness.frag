#version 460
#extension GL_ARB_bindless_texture : require

// Types

struct point_t
{
  vec3 position;
  vec3 normal;
  vec4 color;
  vec2 tex_coords;
};

// Input

in point_t point;

// Buffers

layout(std430, binding = 1) buffer render_layout
{
  sampler2D color_sampler;
};

// Output

out vec4 color;

void main()
{
  vec4 base_color = texture(color_sampler, point.tex_coords);

  // Luma conversion
  // https://en.wikipedia.org/wiki/Grayscale#Luma_coding_in_video_systems
  float brightness = dot(base_color.rgb, vec3(0.2126, 0.7152, 0.0722));

  color = clamp(base_color * brightness / 3.0, 0.0, 1.0);
}
