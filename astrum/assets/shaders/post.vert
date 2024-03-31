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

// Inputs

in vec3 position;
in vec2 tex_coords;

// Output

out point_t point;

void main()
{
  point.position = position;
  point.tex_coords = tex_coords;

  gl_Position = vec4(position, 1.0);
}
