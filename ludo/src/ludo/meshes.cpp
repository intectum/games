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

      format.components.emplace_back('b', max_bone_weights_per_vertex);
      format.size += max_bone_weights_per_vertex * (sizeof(uint32_t) + sizeof(float));
    }

    return format;
  }

  void load(mesh& mesh, buffer& indices, buffer& vertices, const std::string& file_name)
  {
    auto stream = std::ifstream(file_name, std::ios::binary);
    return load(mesh, indices, vertices, stream);
  }

  void load(mesh& mesh, buffer& indices, buffer& vertices, std::istream& stream)
  {
    load_metadata(mesh, stream);
    stream.read(reinterpret_cast<char*>(indices.data), mesh.indices.count * sizeof(uint32_t));
    stream.read(reinterpret_cast<char*>(vertices.data), mesh.vertices.count * mesh.vertex_size);
  }

  void load_metadata(mesh& mesh, const std::string& file_name)
  {
    auto stream = std::ifstream(file_name);
    return load_metadata(mesh, stream);
  }

  void load_metadata(mesh& mesh, std::istream& stream)
  {
    stream.read(reinterpret_cast<char*>(&mesh.indices.count), sizeof(uint32_t));
    stream.read(reinterpret_cast<char*>(&mesh.vertices.count), sizeof(uint32_t));
    stream.read(reinterpret_cast<char*>(&mesh.vertex_size), sizeof(uint32_t));
  }

  void save(const mesh& mesh, const buffer& indices, const buffer& vertices, const std::string& file_name)
  {
    auto stream = std::ofstream(file_name, std::ios::binary);
    save(mesh, indices, vertices, stream);
  }

  void save(const mesh& mesh, const buffer& indices, const buffer& vertices, std::ostream& stream)
  {
    stream.write(reinterpret_cast<const char*>(&mesh.indices.count), sizeof(uint32_t));
    stream.write(reinterpret_cast<const char*>(&mesh.vertices.count), sizeof(uint32_t));
    stream.write(reinterpret_cast<const char*>(&mesh.vertex_size), sizeof(uint32_t));
    stream.write(reinterpret_cast<const char*>(indices.data), mesh.indices.count * sizeof(uint32_t));
    stream.write(reinterpret_cast<const char*>(vertices.data), mesh.vertices.count * mesh.vertex_size);
  }
}
