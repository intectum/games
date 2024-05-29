#include "mesh.h"
#include "terrain_chunk.h"

namespace astrum
{
  void load_terrain_chunk(const terrain& terrain, float radius, uint32_t chunk_index, uint32_t lod_index, ludo::mesh& mesh)
  {
    auto& lowest_detail_lod = terrain.lods[0];

    auto& low_detail_lod = terrain.lods[lod_index > 0 ? lod_index - 1 : 0];
    auto& high_detail_lod = terrain.lods[lod_index];

    auto low_detail_format = terrain.format;
    low_detail_format.position_offset += terrain.format.size / 2;
    low_detail_format.normal_offset += terrain.format.size / 2;
    low_detail_format.color_offset += terrain.format.size / 2;

    terrain_mesh(terrain, radius, mesh, low_detail_format, terrain.format, true, chunk_index, lowest_detail_lod.level, low_detail_lod.level, high_detail_lod.level);
  }

  void init_terrain_chunk(const terrain& terrain, uint32_t chunk_index, ludo::render_mesh& render_mesh)
  {
    auto& chunk = terrain.chunks[chunk_index];
    auto is_highest_detail = chunk.lod_index == terrain.lods.size() - 1;
    auto& high_detail_lod = terrain.lods[chunk.lod_index];
    auto& higher_detail_lod = terrain.lods[is_highest_detail ? chunk.lod_index : chunk.lod_index + 1];

    auto max_distance = high_detail_lod.max_distance;
    auto min_distance = is_highest_detail ? 0.0f : higher_detail_lod.max_distance;
    auto distance_range = max_distance - min_distance;

    auto low_detail_distance = min_distance + distance_range * 0.66f;
    auto high_detail_distance = min_distance + distance_range * 0.33f;

    ludo::cast<float>(render_mesh.instance_buffer, 0) = low_detail_distance;
    ludo::cast<float>(render_mesh.instance_buffer, sizeof(float)) = high_detail_distance;
  }

  uint32_t terrain_chunk_lod_index(const terrain& terrain, uint32_t chunk_index, const std::vector<lod>& lods, const ludo::vec3& camera_position, const ludo::vec3& position)
  {
    auto& chunk = terrain.chunks[chunk_index];
    auto to_camera = camera_position - (position + chunk.center);
    auto to_camera_unit = to_camera;
    ludo::normalize(to_camera_unit);

    if (ludo::dot(to_camera_unit, chunk.normal) < -0.5f)
    {
      return uint32_t(0);
    }

    auto distance_to_camera = ludo::length(to_camera);

    for (auto variant_index = uint32_t(lods.size() - 1); variant_index < lods.size(); variant_index--)
    {
      if (distance_to_camera < lods[variant_index].max_distance)
      {
        return variant_index;
      }
    }

    return uint32_t(0);
  }
}
