#include <cassert>
#include <cmath>
#include <cstdint>
#include <immintrin.h>

#include "noise.h"

namespace ludo
{
  vec3 fade(const vec3& t);
  float lerp(float t, float a, float b);
  float grad(uint32_t hash, float x, float y, float z);

  // Based on https://cs.nyu.edu/~perlin/noise
  // References:
  // - https://libnoise.sourceforge.net
  // - https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-5-implementing-improved-perlin-noise
  // - https://developer.nvidia.com/gpugems/gpugems2/part-iii-high-quality-rendering/chapter-26-implementing-improved-perlin-noise
  // - https://rtouti.github.io/graphics/perlin-noise-algorithm
  float perlin(const vec3& position, float frequency, const uint32_t permutation[512])
  {
    auto scaled_position = position * frequency;

    auto floor_position = vec3 { std::floor(scaled_position[0]), std::floor(scaled_position[1]), std::floor(scaled_position[2]) };

    // Find the unit cube that contains the inputs
    auto x_i = uint32_t(floor_position[0]) & 255;
    auto y_i = uint32_t(floor_position[1]) & 255;
    auto z_i = uint32_t(floor_position[2]) & 255;

    // Find the point relative to the unit cube
    auto relative_position = scaled_position - floor_position;

    // Find fade values for the relative point
    auto fade_position = fade(relative_position);

    // Hash coordinates of the 8 unit cube corners
    auto A = permutation[x_i] + y_i;
    auto AA = permutation[A] + z_i;
    auto AB = permutation[A + 1] + z_i;
    auto B = permutation[x_i + 1] + y_i;
    auto BA = permutation[B] + z_i;
    auto BB = permutation[B + 1] + z_i;

    // Add blended results from the 8 unit cube corners
    return lerp(fade_position[2], lerp(fade_position[1], lerp(fade_position[0], grad(permutation[AA], relative_position[0], relative_position[1], relative_position[2]),
                                               grad(permutation[BA], relative_position[0] - 1, relative_position[1], relative_position[2])),
                                  lerp(fade_position[0], grad(permutation[AB], relative_position[0], relative_position[1] - 1, relative_position[2]),
                                               grad(permutation[BB], relative_position[0] - 1, relative_position[1] - 1, relative_position[2]))),
                     lerp(fade_position[1], lerp(fade_position[0], grad(permutation[AA + 1], relative_position[0], relative_position[1], relative_position[2] - 1),
                                               grad(permutation[BA + 1], relative_position[0] - 1, relative_position[1], relative_position[2] - 1)),
                                  lerp(fade_position[0], grad(permutation[AB + 1], relative_position[0], relative_position[1] - 1, relative_position[2] - 1),
                                               grad(permutation[BB + 1], relative_position[0] - 1, relative_position[1] - 1, relative_position[2] - 1))));
  }

  vec3 fade(const vec3& t)
  {
    //return t * t * (3.0 - 2.0 * t);

    vec3 result =
    {
      t[0] * t[0] * (3.0f - 2.0f * t[0]),
      t[1] * t[1] * (3.0f - 2.0f * t[1]),
      t[2] * t[2] * (3.0f - 2.0f * t[2]),
    };

    //return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);

    /*vec3 result =
    {
      t[0] * t[0] * t[0] * (t[0] * (t[0] * 6.0f - 15.0f) + 10.0f),
      t[1] * t[1] * t[1] * (t[1] * (t[1] * 6.0f - 15.0f) + 10.0f),
      t[2] * t[2] * t[2] * (t[2] * (t[2] * 6.0f - 15.0f) + 10.0f),
    };*/

    return result;

    /*__m128 t_wide = _mm_load_ps(t.data());
    __m128 fade_wide = _mm_mul_ps(t_wide, _mm_set1_ps(6.0f));
    fade_wide = _mm_sub_ps(fade_wide, _mm_set1_ps(15.0f));
    fade_wide = _mm_mul_ps(fade_wide, t_wide);
    fade_wide = _mm_add_ps(fade_wide, _mm_set1_ps(10.0f));
    fade_wide = _mm_mul_ps(fade_wide, t_wide);
    fade_wide = _mm_mul_ps(fade_wide, t_wide);
    fade_wide = _mm_mul_ps(fade_wide, t_wide);

    vec4 result_wide;
    _mm_store_ps(result_wide.data(), fade_wide);

    return vec3(result_wide);*/

    /*assert(near(result[0], result_wide[0]));
    assert(near(result[1], result_wide[1]));
    assert(near(result[2], result_wide[2]));*/
  }

  float lerp(float t, float a, float b)
  {
    return a + t * (b - a);
  }

  float grad(uint32_t hash, float x, float y, float z)
  {
    auto h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
    auto u = h < 8 ? x : y;                 // INTO 12 GRADIENT DIRECTIONS.
    auto v = h < 4 ? y : h == 12 || h == 14 ? x : z;

    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
  }
}
