/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "mesh.h"
#include "metadata.h"

namespace astrum
{
  std::vector<ludo::vec3> build_positions(const terrain& terrain, float radius, uint32_t index, uint32_t patch_divisions, uint32_t divisions);

  void build_terrain_metadata(terrain& terrain, float radius)
  {
    auto& lowest_detail_lod = terrain.lods[0];

    auto patch_count = 20 * static_cast<uint32_t>(std::pow(4, lowest_detail_lod.level - 1));
    terrain.chunks.reserve(patch_count);

    for (auto patch_index = uint32_t(0); patch_index < patch_count; patch_index++)
    {
      auto patch_positions = build_positions(terrain, radius, patch_index, lowest_detail_lod.level, lowest_detail_lod.level);

      auto normal = ludo::cross(patch_positions[1] - patch_positions[0], patch_positions[2] - patch_positions[0]);
      ludo::normalize(normal);

      terrain.chunks.emplace_back(terrain_chunk
      {
        .center = (patch_positions[0] + patch_positions[1] + patch_positions[2]) / 3.0f,
        .normal = normal
      });
    }
  }

  void read_terrain_metadata(std::istream& stream, terrain& terrain)
  {
    auto chunk_count = uint64_t();
    stream.read(reinterpret_cast<char*>(&chunk_count), sizeof(uint64_t));
    terrain.chunks = std::vector<terrain_chunk>(chunk_count);
    for (auto& chunk : terrain.chunks)
    {
      stream.read(reinterpret_cast<char*>(&chunk.center), sizeof(ludo::vec3));
      stream.read(reinterpret_cast<char*>(&chunk.normal), sizeof(ludo::vec3));
    }
  }

  void write_terrain_metadata(std::ostream& stream, const terrain& terrain)
  {
    auto chunk_count = terrain.chunks.size();
    stream.write(reinterpret_cast<char*>(&chunk_count), sizeof(uint64_t));
    for (auto& chunk : terrain.chunks)
    {
      stream.write(reinterpret_cast<const char*>(&chunk.center), sizeof(ludo::vec3));
      stream.write(reinterpret_cast<const char*>(&chunk.normal), sizeof(ludo::vec3));
    }
  }

  std::vector<ludo::vec3> build_positions(const terrain& terrain, float radius, uint32_t index, uint32_t patch_divisions, uint32_t divisions)
  {
    auto patch_count = 20 * static_cast<uint32_t>(std::pow(4, patch_divisions - 1));
    auto vertex_count = 20 * 3 * static_cast<uint32_t>(std::pow(4, divisions - 1)) / patch_count;
    auto patch_positions = std::vector<ludo::vec3>(vertex_count);

    // TODO move out of this function! Slow!
    auto temp_mesh = ludo::mesh
    {
      .index_buffer = ludo::allocate(vertex_count * sizeof(uint32_t)),
      .vertex_buffer = ludo::allocate(vertex_count * sizeof(ludo::vec3))
    };

    terrain_mesh(terrain, radius, temp_mesh, ludo::vertex_format_p, ludo::vertex_format_p, false, index, patch_divisions, divisions, divisions);
    std::memcpy(patch_positions.data(), temp_mesh.vertex_buffer.data, vertex_count * sizeof(ludo::vec3));

    ludo::deallocate(temp_mesh.index_buffer);
    ludo::deallocate(temp_mesh.vertex_buffer);

    return patch_positions;
  }
}
