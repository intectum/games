#version 460
#extension GL_ARB_bindless_texture : require

// Based on this paper:
// https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering

// Constants

const float four_pi = 4.0 * 3.1416;

const int optical_depth_sample_count = 10;
const int scattering_sample_count = 10;
const float scale_height = 0.25;

const float scattering_strength = 2.5;
const vec4 scattering_constants = vec4(
  1.0 / pow(700.0, 4.0) * 1000000.0 * scattering_strength,
  1.0 / pow(530.0, 4.0) * 1000000.0 * scattering_strength,
  1.0 / pow(440.0, 4.0) * 1000000.0 * scattering_strength,
  1.0
);

const float dither_strength = 0.004;
const float dither_scale = 8.0;

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
  vec2 tex_coords[1];
};

struct planet_t
{
  vec3 position;
  float radius;
  float atmosphere_radius;
};

// Input

in point_t point;

// Buffers

layout(std430, binding = 0) buffer context_layout
{
  camera_t camera;
  uint light_count;
  light_t lights[];
};

layout(std430, binding = 1) buffer render_layout
{
  sampler2D color_sampler;
  sampler2DShadow depth_sampler;
};

layout(std430, binding = 2) buffer program_layout
{
  sampler2D blue_noise_sampler;
  planet_t planet;
};

// Output

out vec4 color;

float linear_depth(float depth)
{
  return camera.near_clipping_distance * camera.far_clipping_distance / (camera.far_clipping_distance + depth * (camera.near_clipping_distance - camera.far_clipping_distance));
}

vec2 square_tex_coord(vec2 tex_coord) {
  float scale = 1280.0;
  float x = tex_coord.x * 1280; // TODO
  float y = tex_coord.y * 720; // TODO
  return vec2(x / scale, y / scale);
}

vec2 ray_sphere_intersection(vec3 ray_origin, vec3 ray_direction, vec3 sphere_center, float sphere_radius)
{
/*
  // TODO get this method working with plus or minus sqrts?

  float ray_time_closest_to_sphere_center = dot(sphere_center - ray_origin, ray_direction);
  vec3 ray_point_closest_to_sphere_center = ray_origin + ray_direction * ray_time_closest_to_sphere_center;
  float ray_distance_to_sphere_center = length(sphere_center - ray_point_closest_to_sphere_center);

  if (ray_distance_to_sphere_center < sphere_radius)
  {
    float ray_intersection_offset = sqrt(sphere_radius * sphere_radius - ray_distance_to_sphere_center * ray_distance_to_sphere_center);
    float ray_time_near_intersection = ray_time_closest_to_sphere_center - ray_intersection_offset;
    float ray_time_far_intersection = ray_time_closest_to_sphere_center + ray_intersection_offset;
    return vec2(ray_time_near_intersection, ray_time_far_intersection);
  }
*/

  vec3 offset = ray_origin - sphere_center;
  float a = 1; // set to dot(ray_direction, ray_direction) if ray_direction might not be normalized.
  float b = 2 * dot(offset, ray_direction);
  float c = dot(offset, offset) - sphere_radius * sphere_radius;
  float d = b * b - 4 * a * c; // Discriminant from quadratic formula.

  // Number of intersections: 0 when d < 0; 1 when d = 0; 2 when d > 0.
  if (d > 0)
  {
    float s = sqrt(d);
    float distance_to_sphere_far = (-b + s) / (2 * a);

    if (distance_to_sphere_far >= 0)
    {
      float distance_to_sphere_near = max(0.0, (-b - s) / (2 * a));
      return vec2(distance_to_sphere_near, distance_to_sphere_far);
    }
  }

  return vec2(0.0);
}

// The amount of light scattered in a given angle by scattering.
float phase(float angle, float g)
{
  float g2 = g * g;
  float cangle = cos(angle);

  return ((3.0 * (1.0 - g2)) / (2.0 * (2.0 + g2))) * ((1.0 + cangle * cangle) / pow(1.0 + g2 - 2.0 * g * cangle, 2.0 / 3.0));
}

// The amount of light scattered in a given angle by Mie scattering (caused by large molecules in the atmosphere).
float mie_phase(float angle)
{
  // For Mie scattering, g is usually set between -0.75 and -0.999.
  return phase(angle, -0.85);
}

// The amount of light scattered in a given angle by Rayleigh scattering (caused by small molecules in the atmosphere).
float rayleigh_phase(float angle)
{
  float cangle = cos(angle);

  // Greatly simplified since g = 0 for Rayleigh scatting.
  return 0.75 * (1.0 + cangle * cangle);
}

float atmospheric_density(planet_t planet, float altitude)
{
  float normalized_altitude = altitude / (planet.atmosphere_radius - planet.radius);
  float density = exp(-normalized_altitude / scale_height);

  // Ensure the density is 0.0 at the atmosphere radius.
  // Or maybe we can get a longer exp tail into the lower numbers? Would probably require a larger overall atmosphere though...
  //density *= 1.0 - normalized_altitude;

  return density;
}

// The average atmospheric density between 'from' and 'to'.
float optical_depth(planet_t planet, vec3 from, vec3 to)
{
  vec3 sample_position = from;
  vec3 sample_delta = (to - from) / (optical_depth_sample_count - 1);
  float sample_frequency = length(to - from) / (optical_depth_sample_count - 1);

  float depth = 0.0;
  for (int sample_index = 0; sample_index < optical_depth_sample_count; sample_index++)
  {
    float sample_altitude = length(sample_position - planet.position) - planet.radius;
    depth += atmospheric_density(planet, sample_altitude) * sample_frequency;
    sample_position += sample_delta;
  }

  return depth;
}

// The amount of light scattered out (lost) between 'from' and 'to'.
vec4 out_scatter(planet_t planet, vec3 from, vec3 to)
{
  return four_pi * scattering_constants * optical_depth(planet, from, to);
}

// The amount of light scattered in (gained) between 'from' and 'to'.
vec4 in_scatter(planet_t planet, vec3 from, vec3 to)
{
  vec3 view_ray = to - from;

  vec3 sample_position = from;
  vec3 sample_delta = view_ray / (scattering_sample_count - 1);
  float sample_frequency = length(view_ray) / (optical_depth_sample_count - 1);

  // We'll just compute this once assuming the star is so far away that the rays from the star are essentially parallel.
  vec3 star_direction = normalize(lights[0].position - sample_position);

  vec4 in_scattered_light = vec4(0.0, 0.0, 0.0, 1.0);
  for (int sample_index = 0; sample_index < scattering_sample_count; sample_index++)
  {
    float sample_altitude = length(sample_position - planet.position) - planet.radius;
    float sample_density = atmospheric_density(planet, sample_altitude);

    vec2 star_ray_intersections = ray_sphere_intersection(sample_position, star_direction, planet.position, planet.atmosphere_radius);
    float star_ray_length = star_ray_intersections[1] - star_ray_intersections[0];
    vec4 star_ray_transmittance = out_scatter(planet, sample_position, sample_position + star_direction * star_ray_length);

    vec4 view_ray_transmittance = out_scatter(planet, sample_position, camera.position);

    in_scattered_light += sample_density * exp(-star_ray_transmittance - view_ray_transmittance) * sample_frequency;
    sample_position += sample_delta;
  }

  float angle = acos(dot(normalize(view_ray), star_direction));

  in_scattered_light *= scattering_constants * rayleigh_phase(angle);

  // Soften banding effect.
  vec4 blue_noise = texture(blue_noise_sampler, square_tex_coord(point.tex_coords[0]) * dither_scale);
  blue_noise = (blue_noise - 0.5) * dither_strength;
  in_scattered_light += blue_noise;

  return in_scattered_light;
}

void main()
{
  color = texture(color_sampler, point.tex_coords[0]);

  float depth = linear_depth(texture(depth_sampler, vec3(point.tex_coords[0], 0.0)));

  mat4 camera_projection_inverse = inverse(camera.projection);
  vec3 view = vec3(camera_projection_inverse * vec4(point.tex_coords[0] * 2.0 - 1.0, 0.0, 1.0));
  view = vec3(camera.view * vec4(view, 0.0));
  vec3 ray_direction = normalize(view);

  vec2 intersections = ray_sphere_intersection(camera.position, ray_direction, planet.position, planet.atmosphere_radius);
  float atmosphere_depth = min(depth - intersections[0], intersections[1] - intersections[0]);

  if (atmosphere_depth > 0.0)
  {
    vec3 near_position_in_atmosphere = camera.position + ray_direction * intersections[0];
    vec3 far_position_in_atmosphere = camera.position + ray_direction * (intersections[0] + atmosphere_depth);
    color += in_scatter(planet, far_position_in_atmosphere, near_position_in_atmosphere);

    // TODO combine reflected light too...
  }
}
