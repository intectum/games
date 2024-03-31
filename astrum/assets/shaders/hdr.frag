#version 460
#extension GL_ARB_bindless_texture : require

// Constants

const float exposure = 0.75;
const float gamma = 2.2;
const float gamma_inverse = 1.0 / gamma;

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
  vec3 color3 = texture(color_sampler, point.tex_coords).rgb;

  // Tone mapping (transform HDR to LDR).
  //color3 = color3 / (color3 + vec3(1.0)); // Reinhard
  color3 = vec3(1.0) - exp(-color3 * exposure); // Exposure

  // Gamma correction (correct for the monitor's gamma which tries to imitate how our eye perceives brightness).
  color3 = pow(color3, vec3(gamma_inverse));

  color = vec4(color3, 1.0);
}
