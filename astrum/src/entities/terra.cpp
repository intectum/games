#include <random>

#include <libnoise/noise.h>

#include "../constants.h"
#include "../terrain/terrain.h"
#include "terra.h"

namespace astrum
{
  const auto max_tree_count_per_section = 100;
  const auto beach_max_height = 1.0001f;

  auto seed = 123456;
  auto tree_random = std::mt19937(seed);
  std::uniform_real_distribution<float> tree_distribution(0.0f, 1.0f);

  void add_terra(ludo::instance& inst, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity)
  {
    auto celestial_body = ludo::add(
      inst,
      astrum::celestial_body
      {
        .name = "terra",
        .radius = terra_radius,
        .mass = terra_mass,
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
        .format = ludo::vertex_format_pnc,
        .lods = terra_lods,
        .height_func = terra_height,
        .color_func = terra_color,
        .tree_func = terra_tree
      },
      *celestial_body,
      "celestial-bodies"
    );
  }

  float terra_height(const ludo::vec3& position)
  {
    auto perlin_continent = noise::module::Perlin();
    perlin_continent.SetSeed(seed);
    perlin_continent.SetFrequency(2.0f);

    auto perlin_detail = noise::module::Perlin();
    perlin_detail.SetSeed(seed);
    perlin_detail.SetFrequency(50.0f);

    auto perlin_mountain_mask = noise::module::Perlin();
    perlin_mountain_mask.SetSeed(seed);

    auto ridge_mountain = noise::module::RidgedMulti();
    ridge_mountain.SetSeed(seed);

    auto noise = 0.0f;

    // Continents
    noise += static_cast<float>(perlin_continent.GetValue(position[0], position[1], position[2])) * 0.05f;

    // Mountains
    auto mountain_mask = static_cast<float>(perlin_mountain_mask.GetValue(position[0], position[1], position[2]));
    if (noise > 0.0f && mountain_mask < 0.0f)
    {
      noise += (static_cast<float>(ridge_mountain.GetValue(position[0], position[1], position[2])) + 1.0f) * noise * std::pow(-mountain_mask, 2.0f) * 5.0f;
    }

    // Details
    noise += static_cast<float>(perlin_detail.GetValue(position[0], position[1], position[2])) * 0.0005f;

    return 1.0f + std::max(noise, 0.0f);
  }

  ludo::vec4 terra_color(float longitude, const std::array<float, 3>& heights, float gradient)
  {
    auto color = ludo::vec4_zero;

    auto snow_min_height = (1.0f - std::pow(longitude, 20.0f)) * 1.08f;

    // Oceans
    if (heights[0] == 1.0f && heights[1] == 1.0f && heights[2] == 1.0f)
    {
      color = ludo::vec4 { 0.0f, 0.5f, 1.0f, 1.0f };
    }
    // Beaches
    else if (heights[0] < beach_max_height && heights[1] < beach_max_height && heights[2] < beach_max_height)
    {
      color = ludo::vec4 { 0.95f, 0.9f, 0.7f, 1.0f };
    }
    // Snow
    else if (heights[0] > snow_min_height && heights[1] > snow_min_height && heights[2] > snow_min_height)
    {
      color = ludo::vec4 { 1.0f, 1.0f, 1.0f, 1.0f };
    }
    // Cliffs
    else if (gradient < 0.4f)
    {
      color = ludo::vec4 { 0.25f, 0.25, 0.25f, 1.0f };
    }
    // Banks
    else if (gradient < 0.6f)
    {
      color = ludo::vec4 { 0.4f, 0.35f, 0.2f, 1.0f };
    }
    // Grass
    else
    {
      color = ludo::vec4 { 0.3f, 0.5f, 0.3f, 1.0f };
    }

    return color;
  }

  std::vector<tree> terra_tree(uint32_t patch_index)
  {
    auto perlin_forest = noise::module::Perlin();
    perlin_forest.SetSeed(seed);
    perlin_forest.SetFrequency(2.0f);

    auto temp_mesh = ludo::mesh
    {
      .index_buffer = ludo::allocate(3 * sizeof(uint32_t)),
      .vertex_buffer = ludo::allocate(3 * sizeof(ludo::vec3))
    };

    //terrain_chunk(temp_mesh, ludo::vertex_format_p, 0, chunk_index, 5, 5, 0); TODO!

    auto positions = std::vector<ludo::vec3>(3);
    std::memcpy(positions.data(), temp_mesh.vertex_buffer.data, 3 * sizeof(ludo::vec3));

    ludo::deallocate(temp_mesh.index_buffer);
    ludo::deallocate(temp_mesh.vertex_buffer);

    auto trees = std::vector<tree>();
    for (auto tree_index = 0; tree_index < max_tree_count_per_section; tree_index++)
    {
      auto position_barycentric = ludo::vec3 { tree_distribution(tree_random), tree_distribution(tree_random), tree_distribution(tree_random) };
      auto position = position_barycentric[0] * positions[0] + position_barycentric[1] * positions[1] + position_barycentric[2] * positions[2];
      ludo::normalize(position);

      auto height = terra_height(position);
      auto snow_min_height = (1.0f - std::pow(position[1], 20.0f)) * 1.08f;
      if (height < beach_max_height || height > snow_min_height)
      {
        continue;
      }

      // Forests
      auto noise = static_cast<float>(perlin_forest.GetValue(position[0] + 1.0f, position[1] + 1.0f, position[2] + 1.0f));

      if (noise > tree_distribution(tree_random))
      {
        trees.emplace_back(tree { .position = position, .rotation = tree_distribution(tree_random) * ludo::two_pi, .scale = tree_distribution(tree_random) / 2.0f + 0.5f });
      }
    }

    return trees;
  }
}
