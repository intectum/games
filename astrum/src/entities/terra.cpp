#include <iostream>
#include <random>

#include <noise/noise.h>

#include "../constants.h"
#include "../terrain/mesh.h"
#include "../terrain/terrain.h"
#include "terra.h"

namespace astrum
{
  float terra_height(const ludo::vec3& position);
  ludo::vec4 terra_color(float longitude, const std::array<float, 3>& heights, float gradient);
  std::array<std::vector<tree>, tree_type_count> terra_tree(const terrain& terrain, float radius, uint32_t chunk_index);
  void terra_tree_internal(uint32_t divisions, noise::module::Perlin& perlin_forest, const std::array<ludo::vec3, 3>& face, std::vector<ludo::vec3>& positions);

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
    //noise += static_cast<float>(perlin_continent.GetValue(position[0], position[1], position[2])) * 0.05f;
    noise += ludo::perlin(position, 2.0f) * 0.05f;

    // Mountains
    auto mountain_mask = static_cast<float>(perlin_mountain_mask.GetValue(position[0], position[1], position[2]));
    if (noise > 0.0f && mountain_mask < 0.0f)
    {
      noise += (static_cast<float>(ridge_mountain.GetValue(position[0], position[1], position[2])) + 1.0f) * noise * std::pow(-mountain_mask, 2.0f) * 5.0f;
    }

    // Details
    //noise += static_cast<float>(perlin_detail.GetValue(position[0], position[1], position[2])) * 0.0005f;
    noise += ludo::perlin(position, 50.0f) * 0.0005f;

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

  // Poisson disc sampling based on https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf
  // did not work because it required you to keep track of all the previously planted trees
  // which in the case of a planet could be hundreds of thousands and memory was a problem (as well as speed).
  // Perhaps more sophisticated poisson disc sampling does not have these same limitations?
  // For now, placing them at regular intervals and applying some jitter to their positions seems adequate.
  std::array<std::vector<tree>, tree_type_count> terra_tree(const terrain& terrain, float radius, uint32_t chunk_index)
  {
    auto perlin_forest = noise::module::Perlin();
    perlin_forest.SetSeed(seed);
    perlin_forest.SetFrequency(2.0f);

    auto temp_mesh = ludo::mesh
    {
      .index_buffer = ludo::allocate(3 * sizeof(uint32_t)),
      .vertex_buffer = ludo::allocate(3 * sizeof(ludo::vec3))
    };

    terrain_mesh(terrain, radius, temp_mesh, ludo::vertex_format_p, ludo::vertex_format_p, false, chunk_index, 5, 5, 5);

    auto vertex_stream = ludo::stream(temp_mesh.vertex_buffer);
    auto face = std::array<ludo::vec3, 3>
    {
      read<ludo::vec3>(vertex_stream),
      read<ludo::vec3>(vertex_stream),
      read<ludo::vec3>(vertex_stream)
    };

    ludo::normalize(face[0]);
    ludo::normalize(face[1]);
    ludo::normalize(face[2]);

    ludo::deallocate(temp_mesh.index_buffer);
    ludo::deallocate(temp_mesh.vertex_buffer);

    auto positions = std::vector<ludo::vec3>();
    terra_tree_internal(6, perlin_forest, face, positions);

    auto trees = std::array<std::vector<tree>, tree_type_count>();
    for (auto& position : positions)
    {
      auto jitter = ludo::vec3
      {
        tree_distribution(tree_random) * 2.0f / radius,
        tree_distribution(tree_random) * 2.0f / radius,
        tree_distribution(tree_random) * 2.0f / radius
      };

      position += jitter;
      ludo::normalize(position);

      auto type = uint32_t(tree_distribution(tree_random) * tree_type_count);
      trees[type].emplace_back(tree
      {
        .position = position,
        .rotation = tree_distribution(tree_random) * ludo::two_pi,
        .scale = tree_distribution(tree_random) * 0.5f + 0.5f
      });
    }

    return trees;
  }

  void terra_tree_internal(uint32_t divisions, noise::module::Perlin& perlin_forest, const std::array<ludo::vec3, 3>& face, std::vector<ludo::vec3>& positions)
  {
    if (divisions == 0)
    {
      auto center = (face[0] + face[1] + face[2]) / 3.0f;

      auto height = terra_height(center);
      auto snow_min_height = (1.0f - std::pow(center[1], 20.0f)) * 1.08f;
      if (height < beach_max_height || height > snow_min_height)
      {
        return;
      }

      // Forests
      auto noise = static_cast<float>(perlin_forest.GetValue(center[0] + 1.0f, center[1] + 1.0f, center[2] + 1.0f));
      if (noise < 0.3333f)
      {
        return;
      }

      if (noise < 0.6666f)
      {
        auto tree_cover = (noise - 0.3333f) / 0.3333f / 10.0f;
        if (tree_distribution(tree_random) > tree_cover)
        {
          return;
        }
      }

      positions.push_back(center);
      return;
    }

    auto middle_positions = std::array<ludo::vec3, 3>
    {{
      (face[0] + face[1]) * 0.5f,
      (face[0] + face[2]) * 0.5f,
      (face[1] + face[2]) * 0.5f,
    }};

    normalize(middle_positions[0]);
    normalize(middle_positions[1]);
    normalize(middle_positions[2]);

    auto divided_faces = std::array<std::array<ludo::vec3, 3>, 4>
    {{
      {face[0], middle_positions[0], middle_positions[1] },
      {middle_positions[0], face[1], middle_positions[2] },
      {middle_positions[1], middle_positions[2], face[2] },
      {middle_positions[0], middle_positions[2], middle_positions[1] }
    }};

    for (auto face_index = uint32_t(0); face_index < divided_faces.size(); face_index++)
    {
      terra_tree_internal(divisions - 1, perlin_forest, divided_faces[face_index], positions);
    }
  }
}
