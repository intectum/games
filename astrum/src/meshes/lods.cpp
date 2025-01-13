#include "lods.h"

namespace astrum
{
  std::vector<ludo::mesh> build_lod_meshes(const ludo::mesh& src, ludo::arena& indices, ludo::arena& vertices, const ludo::vertex_format& format, const std::vector<uint32_t>& iterations)
  {
    std::vector<ludo::mesh> lod_meshes;

    auto lod_format = format;
    lod_format.components.insert(lod_format.components.end(), format.components.begin(), format.components.end());
    lod_format.size *= 2;
    lod_format.position_offset += format.size;
    lod_format.normal_offset += format.size;
    lod_format.color_offset += format.size;
    lod_format.texture_coordinate_offset += format.size;

    // TODO
    /*auto temp_mesh = ludo::mesh();
    auto temp_indices = ludo::allocate_buffer(src.indices.count * sizeof(uint32_t));
    auto temp_vertices = ludo::allocate_buffer(src.vertices.count * format.size);

    for (auto lod_iterations : iterations)
    {
      auto counts = ludo::clean(
        temp_mesh,
        temp_indices,
        temp_vertices,
        lod_meshes.empty() ? src : lod_meshes[lod_meshes.size() - 1],
        lod_meshes.empty() ? src : lod_meshes[lod_meshes.size() - 1],
        lod_meshes.empty() ? src : lod_meshes[lod_meshes.size() - 1],
        format,
        lod_meshes.empty() ? format : lod_format
      );

      auto lod_mesh = ludo::mesh();
      ludo::mesh_alloc(lod_mesh, counts.first, counts.second, lod_format.size);
      std::memcpy(lod_mesh.indices.start, temp_mesh.indices.start, source.indices.end - source.indices.start);
      for (auto vertex_index = uint32_t(0); vertex_index < counts.second; vertex_index++)
      {
        auto vertex_position = vertex_index * format.size;

        std::memcpy(
          lod_mesh.vertices.start + vertex_position * 2,
          temp_mesh.vertices.start + vertex_position,
          format.size
        );
        std::memcpy(
          lod_mesh.vertices.start + vertex_position * 2 + format.size,
          temp_mesh.vertices.start + vertex_position,
          format.size
        );
      }

      ludo::collapse(lod_mesh, lod_format, lod_iterations);

      lod_meshes.push_back(lod_mesh);
    }

    ludo::free_buffer(temp_indices);
    ludo::free_buffer(temp_vertices);*/

    return lod_meshes;
  }

  uint32_t find_lod_index(const std::vector<lod>& lods, const ludo::vec3& camera_position, const ludo::vec3& target_position, const ludo::vec3& target_normal)
  {
    auto to_camera = camera_position - target_position;

    // TODO make this part of frustum culling instead?
    auto to_camera_unit = to_camera;
    ludo::normalize(to_camera_unit);
    if (ludo::dot(to_camera_unit, target_normal) < -0.5f)
    {
      return 0;
    }

    auto distance_to_camera = ludo::length(to_camera);
    for (auto index = uint32_t(lods.size() - 1); index < lods.size(); index--)
    {
      if (distance_to_camera < lods[index].max_distance)
      {
        return index;
      }
    }

    return 0;
  }
}
