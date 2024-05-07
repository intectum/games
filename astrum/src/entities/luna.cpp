#include <random>

#include <libnoise/noise.h>

#include "../constants.h"
#include "../terrain/terrain.h"
#include "luna.h"

namespace astrum
{
  float luna_height(const ludo::vec3& position);
  ludo::vec4 luna_color(float longitude, const std::array<float, 3>& heights, float gradient);
  std::vector<tree> luna_tree(const terrain& terrain, float radius, uint32_t chunk_index);

  const auto seed = 123456;
  const auto crater_count = 100;
  const auto crater_min_radius = 0.05f;
  const auto crater_max_radius = 0.09f;
  const auto crater_rim_width = 0.9f;
  const auto crater_rim_steepness = 0.5f;
  const auto crater_floor_height = -0.8f;

  auto crater_random = std::mt19937(seed);
  std::uniform_real_distribution<float> crater_position_distribution(-1.0f, 1.0f);
  std::uniform_real_distribution<float> crater_radius_distribution(crater_min_radius, crater_max_radius);

  struct crater
  {
    ludo::vec3 position = ludo::vec3_zero;
    float radius;
  };

  float cavity(float x);
  float rim(float x);
  float floor(float x);
  float crater_func(float x);
  float bias(float x, float bias);

  auto craters = std::vector<crater>();

  void add_luna(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity)
  {
    auto celestial_body = ludo::add(
      inst,
      astrum::celestial_body
      {
        .name = "luna",
        .radius = luna_radius,
        .mass = luna_mass,
      },
      "celestial-bodies"
    );

    ludo::add(
      inst,
      point_mass
      {
        .mass = celestial_body->mass,
        .transform = initial_transform,
        .linear_velocity = initial_velocity
      },
    "celestial-bodies"
    );

    add_terrain(
      inst,
      terrain
      {
        .format = ludo::vertex_format_pn,
        .lods = luna_lods,
        .height_func = luna_height,
        .color_func = luna_color,
        .tree_func = luna_tree
      },
      *celestial_body,
    "celestial-bodies"
    );
  }

  float luna_height(const ludo::vec3& position)
  {
    auto perlin = noise::module::Perlin();
    perlin.SetSeed(seed);
    perlin.SetFrequency(50.0f);

    auto noise = 0.0f;

    // Details
    noise += static_cast<float>(perlin.GetValue(position[0], position[1], position[2])) * 0.001f;

    noise = std::max(noise, 0.0f);

    //Craters
    if (craters.empty())
    {
      craters.reserve(crater_count);
      for (auto index = 0; index < crater_count; index++)
      {
        auto crater_position = ludo::vec3 { crater_position_distribution(crater_random), crater_position_distribution(crater_random), crater_position_distribution(crater_random) };
        ludo::normalize(crater_position);

        craters.emplace_back(crater { .position = crater_position, .radius = bias(crater_radius_distribution(crater_random), 0.1f) });
      }
    }

    for (auto& crater : craters)
    {
      auto x = ludo::length(position - crater.position) / crater.radius;
      if (x > 2.0f)
      {
        continue;
      }

      noise += crater_func(x) * crater.radius;
    }

    return 1.0f + noise;
  }

  ludo::vec4 luna_color(float longitude, const std::array<float, 3>& heights, float gradient)
  {
    return ludo::vec4_one;
  }

  std::vector<tree> luna_tree(const terrain& terrain, float radius, uint32_t chunk_index)
  {
    return {};
  }

  float cavity(float x)
  {
    return x * x - 1.0f;
  };

  float rim(float x)
  {
    x = std::abs(x) - 1.0f - crater_rim_width;
    return crater_rim_steepness * x * x;
  };

  float floor(float x)
  {
    return crater_floor_height;
  };

  float crater_func(float x)
  {
    return std::max(std::min(cavity(x), rim(x)), floor(x));
  };

  float bias(float x, float bias)
  {
    // Make input feel more linear.
    auto k = std::pow(1.0f - bias, 3.0f);

    return (x * k) / (x * k - x + 1.0f);
  };
}
