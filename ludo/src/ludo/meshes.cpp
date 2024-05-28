/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <fstream>

#include "animation.h"
#include "meshes.h"

namespace ludo
{
  vertex_format format(bool normal, bool color, bool texture_coordinate, bool bone_weights)
  {
    auto format = vertex_format();

    format.components.emplace_back(std::pair { 'p', 3 });
    format.size += 3 * sizeof(float);

    if (normal)
    {
      format.has_normal = true;
      format.normal_offset = format.size;

      format.components.emplace_back(std::pair { 'n', 3 });
      format.size += 3 * sizeof(float);
    }

    if (color)
    {
      format.has_color = true;
      format.color_offset = format.size;

      format.components.emplace_back(std::pair { 'c', 4 });
      format.size += 4 * sizeof(float);
    }

    if (texture_coordinate)
    {
      format.has_texture_coordinate = true;
      format.texture_coordinate_offset = format.size;

      format.components.emplace_back(std::pair { 't', 2 });
      format.size += 2 * sizeof(float);
    }

    if (bone_weights)
    {
      format.has_bone_weights = true;
      format.bone_weights_offset = format.size;

      format.components.emplace_back(std::pair { 'b', max_bone_weights_per_vertex });
      format.size += max_bone_weights_per_vertex * (sizeof(uint32_t) + sizeof(float));
    }

    return format;
  }

  void init(mesh& mesh, heap& indices, heap& vertices, uint32_t index_count, uint32_t vertex_count, uint8_t vertex_size)
  {
    mesh.id = next_id++;

    mesh.index_buffer = allocate(indices, index_count * sizeof(uint32_t));
    mesh.vertex_buffer = allocate(vertices, vertex_count * vertex_size, vertex_size);
    mesh.vertex_size = vertex_size;
  }

  void de_init(mesh& mesh, heap& indices, heap& vertices)
  {
    mesh.id = 0;

    if (mesh.index_buffer.data)
    {
      deallocate(indices, mesh.index_buffer);
    }

    if (mesh.vertex_buffer.data)
    {
      deallocate(vertices, mesh.vertex_buffer);
    }
  }

  mesh load(const std::string& file_name, heap& indices, heap& vertices)
  {
    auto stream = std::ifstream(file_name);

    return load(stream, indices, vertices);
  }

  mesh load(std::istream& stream, heap& indices, heap& vertices)
  {
    auto mesh = ludo::mesh();
    mesh.id = next_id++;

    stream.read(reinterpret_cast<char*>(&mesh.index_buffer.size), sizeof(uint64_t));
    mesh.index_buffer = allocate(indices, mesh.index_buffer.size);
    stream.read(reinterpret_cast<char*>(mesh.index_buffer.data), static_cast<int64_t>(mesh.index_buffer.size));

    stream.read(reinterpret_cast<char*>(&mesh.vertex_size), sizeof(uint32_t));

    stream.read(reinterpret_cast<char*>(&mesh.vertex_buffer.size), sizeof(uint64_t));
    mesh.vertex_buffer = allocate(vertices, mesh.vertex_buffer.size, mesh.vertex_size);
    stream.read(reinterpret_cast<char*>(mesh.vertex_buffer.data), static_cast<int64_t>(mesh.vertex_buffer.size));

    return mesh;
  }

  void save(const mesh& mesh, const std::string& file_name)
  {
    auto stream = std::ofstream(file_name);

    save(mesh, stream);
  }

  void save(const mesh& mesh, std::ostream& stream)
  {
    stream.write(reinterpret_cast<const char*>(&mesh.index_buffer.size), sizeof(uint64_t));
    stream.write(reinterpret_cast<const char*>(mesh.index_buffer.data), static_cast<int64_t>(mesh.index_buffer.size));

    stream.write(reinterpret_cast<const char*>(&mesh.vertex_size), sizeof(uint32_t));

    stream.write(reinterpret_cast<const char*>(&mesh.vertex_buffer.size), sizeof(uint64_t));
    stream.write(reinterpret_cast<const char*>(mesh.vertex_buffer.data), static_cast<int64_t>(mesh.vertex_buffer.size));
  }

  std::pair<uint32_t, uint32_t> mesh_counts(const std::string& file_name)
  {
    auto stream = std::ifstream(file_name);

    return mesh_counts(stream);
  }

  std::pair<uint32_t, uint32_t> mesh_counts(std::istream& stream)
  {
    auto index_buffer_size = uint64_t(0);
    stream.read(reinterpret_cast<char*>(&index_buffer_size), sizeof(uint64_t));
    stream.seekg(static_cast<int64_t>(index_buffer_size), std::ios_base::cur);

    auto vertex_size = uint32_t(0);
    stream.read(reinterpret_cast<char*>(&vertex_size), sizeof(uint32_t));

    auto vertex_buffer_size = uint64_t(0);
    stream.read(reinterpret_cast<char*>(&vertex_buffer_size), sizeof(uint64_t));

    return { index_buffer_size / sizeof(uint32_t), vertex_buffer_size / vertex_size };
  }
}
