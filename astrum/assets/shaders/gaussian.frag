#version 460 core
#extension GL_ARB_bindless_texture : require

// Constants

const float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

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
  bool horizontal;
};

// Output

out vec4 color;

void main()
{
  vec2 texel_size = 1.0 / textureSize(color_sampler, 0);

  vec3 color3 = texture(color_sampler, point.tex_coords).rgb * weights[0];

  if (horizontal)
  {
    for (int index = 1; index < 5; index++)
    {
      color3 += texture(color_sampler, point.tex_coords + vec2(texel_size.x * index, 0.0)).rgb * weights[index];
      color3 += texture(color_sampler, point.tex_coords - vec2(texel_size.x * index, 0.0)).rgb * weights[index];
    }
  }
  else
  {
    for (int index = 1; index < 5; index++)
    {
      color3 += texture(color_sampler, point.tex_coords + vec2(0.0, texel_size.y * index)).rgb * weights[index];
      color3 += texture(color_sampler, point.tex_coords - vec2(0.0, texel_size.y * index)).rgb * weights[index];
    }
  }

  color = vec4(color3, 1.0);
}
