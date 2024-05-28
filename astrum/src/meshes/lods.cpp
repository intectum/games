#include "lods.h"

namespace astrum
{
  std::vector<ludo::mesh> build_lod_meshes(const ludo::mesh& source, const ludo::vertex_format& format, ludo::heap& indices, ludo::heap& vertices, const std::vector<uint32_t>& iterations)
  {
    std::vector<ludo::mesh> lod_meshes;

    auto lod_format = format;
    lod_format.components.insert(lod_format.components.end(), format.components.begin(), format.components.end());
    lod_format.size *= 2;
    lod_format.position_offset += format.size;
    lod_format.normal_offset += format.size;
    lod_format.color_offset += format.size;
    lod_format.texture_coordinate_offset += format.size;

    auto temp_mesh = ludo::mesh();
    temp_mesh.index_buffer = ludo::allocate(source.index_buffer.size);
    temp_mesh.vertex_buffer = ludo::allocate(source.vertex_buffer.size);

    for (auto lod_iterations : iterations)
    {
      auto counts = ludo::clean(
        temp_mesh,
        lod_meshes.empty() ? source : lod_meshes[lod_meshes.size() - 1],
        format,
        lod_meshes.empty() ? format : lod_format
      );

      auto lod_mesh = ludo::mesh();
      ludo::init(lod_mesh, indices, vertices, counts.first, counts.second, lod_format.size);
      std::memcpy(lod_mesh.index_buffer.data, temp_mesh.index_buffer.data, lod_mesh.index_buffer.size);
      for (auto vertex_index = uint32_t(0); vertex_index < counts.second; vertex_index++)
      {
        auto vertex_position = vertex_index * format.size;

        std::memcpy(
          lod_mesh.vertex_buffer.data + vertex_position * 2,
          temp_mesh.vertex_buffer.data + vertex_position,
          format.size
        );
        std::memcpy(
          lod_mesh.vertex_buffer.data + vertex_position * 2 + format.size,
          temp_mesh.vertex_buffer.data + vertex_position,
          format.size
        );
      }

      ludo::collapse(lod_mesh, lod_format, lod_iterations);

      lod_meshes.push_back(lod_mesh);
    }

    ludo::deallocate(temp_mesh.index_buffer);
    ludo::deallocate(temp_mesh.vertex_buffer);

    return lod_meshes;
  }

  uint32_t find_lod_index(const std::vector<lod>& lods, const ludo::vec3& camera_position, const ludo::vec3& target_position, const ludo::vec3& target_normal)
  {
    auto to_camera = camera_position - target_position;
    auto to_camera_unit = to_camera;
    ludo::normalize(to_camera_unit);

    // TODO make this part of frustum culling instead?
    if (ludo::dot(to_camera_unit, target_normal) < -0.5f)
    {
      return uint32_t(0);
    }

    auto distance_to_camera = ludo::length(camera_position - target_position);

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
