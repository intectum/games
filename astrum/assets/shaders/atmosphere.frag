#version 460 core
#extension GL_ARB_bindless_texture : require

// Based on this paper:
// https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering

// Constants

const float four_pi = 4.0 * 3.1416;

const uint in_scatter_sample_count = 10;
const uint optical_depth_sample_count = 10;
const float scale_height = 0.25;

const float scattering_strength = 10000000.0;
const vec3 scattering_constants = vec3(
  1.0 / pow(700.0, 4.0) * scattering_strength,
  1.0 / pow(530.0, 4.0) * scattering_strength,
  1.0 / pow(440.0, 4.0) * scattering_strength
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
  vec2 tex_coords;
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
  // TODO why is sampler2DShadow not working? AMD thing? I'm sure it was working before...
  //sampler2DShadow depth_sampler;
  sampler2D depth_sampler;
};

layout(std430, binding = 2) buffer program_layout
{
  sampler2D atmosphere_sampler;
  sampler2D blue_noise_sampler;
  planet_t planet;
};

// Output

out vec4 color;

float linear_depth()
{
  //float depth = texture(depth_sampler, vec3(point.tex_coords, 0.0));
  float depth = texture(depth_sampler, point.tex_coords).r;

  return camera.near_clipping_distance * camera.far_clipping_distance / (camera.far_clipping_distance + depth * (camera.near_clipping_distance - camera.far_clipping_distance));
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

vec2 square_tex_coord(vec2 tex_coord)
{
  float scale = 1280.0; // TODO window width
  float x = tex_coord.x * 1280; // TODO window width
  float y = tex_coord.y * 720; // TODO window height
  return vec2(x / scale, y / scale);
}

vec3 pixel_view_vector()
{
  vec4 ndc = vec4(vec3(point.tex_coords, 0.0) * 2.0 - 1.0, 1.0);

  mat4 camera_projection_inverse = inverse(camera.projection);
  vec4 view = camera_projection_inverse * ndc;
  return vec3(camera.view * vec4(view.xyz, 0.0));
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

// The altitude of a position normalized to the range [0,1] where 0 is at the planet surface and 1 is at the edge of the atmosphere.
float normalized_altitude(planet_t planet, vec3 position)
{
  float altitude = length(position - planet.position) - planet.radius;
  return altitude / (planet.atmosphere_radius - planet.radius);
}

// The atmospheric density at the (normalized) altitude given. Uses a lookup texture for better performance.
float atmospheric_density(float altitude)
{
  return texture(atmosphere_sampler, vec2(0.0, altitude)).r;
}

// The average atmospheric density along a ray (multiplied by the ray length). Uses a lookup texture for better performance.
float optical_depth(planet_t planet, vec3 ray_origin, vec3 ray_direction, float origin_altitude, float target_altitude)
{
  float angle = dot(normalize(planet.position - ray_origin), ray_direction) * 0.5 + 0.5;

  if (target_altitude < 1.0f)
  {
    bool target_below_origin = target_altitude < origin_altitude;

    // This effectively reverses the rays so that they do not go into the planet.
    angle = target_below_origin ? 1.0 - angle : angle;

    float depth = texture(atmosphere_sampler, vec2(angle, target_below_origin ? target_altitude : origin_altitude)).g * 512.0 * planet.radius;
    return depth - texture(atmosphere_sampler, vec2(angle, target_below_origin ? origin_altitude : target_altitude)).g * 512.0 * planet.radius;
  }

  return texture(atmosphere_sampler, vec2(angle, origin_altitude)).g * 512.0 * planet.radius;
}

// The amount of light scattered out (lost) along a ray.
vec3 out_scatter(planet_t planet, vec3 ray_origin, vec3 ray_direction, float origin_altitude, float target_altitude)
{
  return four_pi * scattering_constants * optical_depth(planet, ray_origin, ray_direction, origin_altitude, target_altitude);
}

// The amount of light scattered in (gained) along a ray.
vec3 in_scatter(planet_t planet, vec3 ray_origin, vec3 ray_direction, float ray_length)
{
  float camera_altitude = normalized_altitude(planet, camera.position);
  float step_size = ray_length / (in_scatter_sample_count + 1);

  // We'll just compute this once assuming the star is so far away that the rays from the star are essentially parallel.
  vec3 star_direction = normalize(lights[0].position - ray_origin);

  vec3 in_scattered_light = vec3(0.0, 0.0, 0.0);
  for (uint sample_index = 1; sample_index <= in_scatter_sample_count; sample_index++)
  {
    vec3 sample_position = ray_origin + ray_direction * sample_index * step_size;
    float sample_altitude = normalized_altitude(planet, sample_position);
    float sample_density = atmospheric_density(sample_altitude);

    vec3 star_ray_transmittance = out_scatter(planet, sample_position, star_direction, sample_altitude, 1.0);
    vec3 view_ray_transmittance = out_scatter(planet, sample_position, -ray_direction, sample_altitude, camera_altitude);

    in_scattered_light += sample_density * exp(-star_ray_transmittance - view_ray_transmittance) * step_size;
  }

  float angle = acos(dot(ray_direction, star_direction));

  // TODO add star light intensity
  return scattering_constants * rayleigh_phase(angle) * in_scattered_light;
}

void main()
{
  color = texture(color_sampler, point.tex_coords);

  vec3 view_direction = normalize(pixel_view_vector());
  vec2 view_ray_intersections = ray_sphere_intersection(camera.position, view_direction, planet.position, planet.atmosphere_radius);
  float atmosphere_depth = min(linear_depth() - view_ray_intersections[0], view_ray_intersections[1] - view_ray_intersections[0]);

  if (atmosphere_depth > 0.0)
  {
    vec3 near_position_in_atmosphere = camera.position + view_direction * view_ray_intersections[0];

    color += vec4(in_scatter(planet, near_position_in_atmosphere, view_direction, atmosphere_depth), 0.0);

    // Soften banding effect.
    vec4 blue_noise = texture(blue_noise_sampler, square_tex_coord(point.tex_coords) * dither_scale);
    blue_noise = (blue_noise - 0.5) * dither_strength;

    float brightness = (color.r + color.g + color.b) / 3;

    // Scale blue_noise by brightness as the pattern is very visible when the atmosphere effect is very dark
    color += blue_noise * brightness * 50.0;

    // TODO combine reflected light too...
  }
}
