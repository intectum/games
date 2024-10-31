#version 460 core
#extension GL_ARB_bindless_texture : require

// Types

struct point_t
{
  vec4 color;
};

// Input

in point_t point;

// Output

out vec4 color;

void main()
{
  color = point.color;
}
