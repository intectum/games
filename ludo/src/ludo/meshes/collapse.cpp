/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <limits>
#include <map>

#include "collapse.h"

namespace ludo
{
  struct collapsable_vertex
  {
    std::vector<uint32_t> mesh_vertex_indices;

    std::vector<uint32_t> face_indices;
    std::vector<uint32_t> adjacent_vertex_indices;

    bool collapsed = false;
    float collapse_cost = 0.0f;
    uint32_t collapse_to_vertex_index = 0;
  };

  struct face
  {
    vec3 normal;
  };

  void update_collapse_cost(const mesh& mesh, const vertex_format& format, const std::vector<face>& faces, std::vector<collapsable_vertex>& vertices, collapsable_vertex& vertex);
  float edge_collapse_cost(const mesh& mesh, const vertex_format& format, const std::vector<face>& faces, const collapsable_vertex& collapse_vertex, const collapsable_vertex& collapse_to_vertex);
  void collapse(mesh& mesh, const vertex_format& format, std::vector<face>& faces, std::vector<collapsable_vertex>& vertices, uint32_t collapse_index, uint32_t collapse_to_index);
  std::function<bool(const collapsable_vertex& vertex)> contains_mesh_vertex(uint32_t mesh_vertex);

  // Based on http://pomax.nihongoresources.com/downloads/PolygonReduction.pdf
  void collapse(mesh& mesh, const vertex_format& format, uint32_t iterations)
  {
    auto vertices = std::vector<collapsable_vertex>();
    vertices.reserve(mesh.vertex_buffer.size / format.size);

    for (auto vertex_index = uint32_t(0); vertex_index < vertices.capacity(); vertex_index++)
    {
      auto& position = cast<vec3>(mesh.vertex_buffer, vertex_index * format.size + format.position_offset);
      auto vertex_iter = std::find_if(vertices.begin(), vertices.end(), [&](const collapsable_vertex& vertex)
      {
        return cast<vec3>(mesh.vertex_buffer, vertex.mesh_vertex_indices[0] * format.size + format.position_offset) == position;
      });

      if (vertex_iter != vertices.end())
      {
        vertex_iter->mesh_vertex_indices.push_back(vertex_index);
        continue;
      }

      vertices.emplace_back(collapsable_vertex { .mesh_vertex_indices = {vertex_index } });
    }

    auto faces = std::vector<face>();
    faces.reserve(mesh.index_buffer.size / (3 * sizeof(uint32_t)));

    auto index_stream = stream(mesh.index_buffer);
    for (auto face_index = uint32_t(0); !ended(index_stream); face_index++)
    {
      auto indices = std::array<uint32_t, 3>
      {
        read<uint32_t>(index_stream),
        read<uint32_t>(index_stream),
        read<uint32_t>(index_stream)
      };

      auto positions = std::array<vec3, 3>
      {
        cast<vec3>(mesh.vertex_buffer, indices[0] * format.size + format.position_offset),
        cast<vec3>(mesh.vertex_buffer, indices[1] * format.size + format.position_offset),
        cast<vec3>(mesh.vertex_buffer, indices[2] * format.size + format.position_offset)
      };

      auto vertex_indices = std::array<uint32_t, 3>
      {
        uint32_t(std::find_if(vertices.begin(), vertices.end(), contains_mesh_vertex(indices[0])) - vertices.begin()),
        uint32_t(std::find_if(vertices.begin(), vertices.end(), contains_mesh_vertex(indices[1])) - vertices.begin()),
        uint32_t(std::find_if(vertices.begin(), vertices.end(), contains_mesh_vertex(indices[2])) - vertices.begin())
      };

      vertices[vertex_indices[0]].face_indices.push_back(face_index);
      vertices[vertex_indices[1]].face_indices.push_back(face_index);
      vertices[vertex_indices[2]].face_indices.push_back(face_index);

      for (auto vertex_index_0 = uint32_t(0); vertex_index_0 < 2; vertex_index_0++)
      {
        auto& adjacent_vertex_indices_0 = vertices[vertex_indices[vertex_index_0]].adjacent_vertex_indices;

        for (auto vertex_index_1 = vertex_index_0 + 1; vertex_index_1 < 3; vertex_index_1++)
        {
          auto& adjacent_vertex_indices_1 = vertices[vertex_indices[vertex_index_1]].adjacent_vertex_indices;

          if (std::find(adjacent_vertex_indices_0.begin(), adjacent_vertex_indices_0.end(), vertex_indices[vertex_index_1]) == adjacent_vertex_indices_0.end())
          {
            adjacent_vertex_indices_0.push_back(vertex_indices[vertex_index_1]);
            adjacent_vertex_indices_1.push_back(vertex_indices[vertex_index_0]);
          }
        }
      }

      auto normal = cross(positions[1] - positions[0], positions[2] - positions[0]);
      normalize(normal);

      faces.emplace_back(face { .normal = normal });
    }

    for (auto vertex_index = uint32_t(0); vertex_index < vertices.size(); vertex_index++)
    {
      update_collapse_cost(mesh, format, faces, vertices, vertices[vertex_index]);
    }

    for (auto iteration = uint32_t(0); iteration < iterations; iteration++)
    {
      auto collapse_cost = std::numeric_limits<float>::max();
      auto collapse_vertex_index = uint32_t(0);
      for (auto vertex_index = uint32_t(0); vertex_index < vertices.size(); vertex_index++)
      {
        auto& vertex = vertices[vertex_index];
        if (!vertex.collapsed && vertex.collapse_cost < collapse_cost)
        {
          auto minimum_adjacent_not_met = false;
          for (auto adjacent_vertex_index : vertex.adjacent_vertex_indices)
          {
            if (adjacent_vertex_index != vertex.collapse_to_vertex_index && vertices[adjacent_vertex_index].adjacent_vertex_indices.size() <= 3)
            {
              minimum_adjacent_not_met = true;
            }
          }

          if (minimum_adjacent_not_met)
          {
            continue;
          }

          collapse_cost = vertex.collapse_cost;
          collapse_vertex_index = vertex_index;
        }
      }

      if (collapse_cost < std::numeric_limits<float>::max())
      {
        collapse(mesh, format, faces, vertices, collapse_vertex_index, vertices[collapse_vertex_index].collapse_to_vertex_index);
      }
    }
  }

  void update_collapse_cost(const mesh& mesh, const vertex_format& format, const std::vector<face>& faces, std::vector<collapsable_vertex>& vertices, collapsable_vertex& vertex)
  {
    vertex.collapse_cost = std::numeric_limits<float>::max();
    for (auto adjacent_vertex_index : vertex.adjacent_vertex_indices)
    {
      auto collapse_cost = edge_collapse_cost(mesh, format, faces, vertex, vertices[adjacent_vertex_index]);
      if (collapse_cost < vertex.collapse_cost)
      {
        vertex.collapse_cost = collapse_cost;
        vertex.collapse_to_vertex_index = adjacent_vertex_index;
      }
    }
  }

  float edge_collapse_cost(const mesh& mesh, const vertex_format& format, const std::vector<face>& faces, const collapsable_vertex& collapse_vertex, const collapsable_vertex& collapse_to_vertex)
  {
    std::vector<uint32_t> shared_face_indices;
    std::vector<uint32_t> collapse_only_face_indices;
    for (auto collapse_face_index : collapse_vertex.face_indices)
    {
      auto shared = false;
      for (auto collapse_to_face_index : collapse_to_vertex.face_indices)
      {
        if (collapse_to_face_index == collapse_face_index)
        {
          shared_face_indices.push_back(collapse_face_index);
          shared = true;
          break;
        }
      }

      if (!shared)
      {
        collapse_only_face_indices.push_back(collapse_face_index);
      }
    }

    // Use the collapse-adjacent face facing most away from the shared-adjacent faces to determine our curvature term
    auto curvature = 0.0f;
    for (auto collapse_only_face_index : collapse_only_face_indices)
    {
      auto minimum_curvature = 1.0f;
      for (auto shared_face_index : shared_face_indices)
      {
        auto dot = ludo::dot(faces[collapse_only_face_index].normal, faces[shared_face_index].normal) * -1.0f;
        minimum_curvature = std::min(minimum_curvature, dot * 0.5f + 0.5f);
      }

      curvature = std::max(curvature, minimum_curvature);
    }

    auto collapse_position = cast<vec3>(mesh.vertex_buffer, collapse_vertex.mesh_vertex_indices[0] * format.size + format.position_offset);
    auto collapse_to_position = cast<vec3>(mesh.vertex_buffer, collapse_to_vertex.mesh_vertex_indices[0] * format.size + format.position_offset);

    return length(collapse_to_position - collapse_position) * curvature;
  }

  void collapse(mesh& mesh, const vertex_format& format, std::vector<face>& faces, std::vector<collapsable_vertex>& vertices, uint32_t collapse_index, uint32_t collapse_to_index)
  {
    auto& collapse_vertex = vertices[collapse_index];
    auto& collapse_to_vertex = vertices[collapse_to_index];

    collapse_to_vertex.mesh_vertex_indices.insert(collapse_to_vertex.mesh_vertex_indices.end(), collapse_vertex.mesh_vertex_indices.begin(), collapse_vertex.mesh_vertex_indices.end());

    for (auto collapse_face_index : collapse_vertex.face_indices)
    {
      auto collapse_face_iter = std::find(collapse_to_vertex.face_indices.begin(), collapse_to_vertex.face_indices.end(), collapse_face_index);
      if (collapse_face_iter == collapse_to_vertex.face_indices.end())
      {
        collapse_to_vertex.face_indices.push_back(collapse_face_index);

        auto index_stream = stream(mesh.index_buffer, collapse_face_index * (3 * sizeof(uint32_t)));
        auto indices = std::array<uint32_t, 3>
        {
          read<uint32_t>(index_stream),
          read<uint32_t>(index_stream),
          read<uint32_t>(index_stream)
        };

        auto positions = std::array<vec3, 3>
        {
          cast<vec3>(mesh.vertex_buffer, indices[0] * format.size + format.position_offset),
          cast<vec3>(mesh.vertex_buffer, indices[1] * format.size + format.position_offset),
          cast<vec3>(mesh.vertex_buffer, indices[2] * format.size + format.position_offset),
        };

        auto normal = cross(positions[1] - positions[0], positions[2] - positions[0]);
        normalize(normal);

        faces[collapse_face_index].normal = normal;
      }
      else
      {
        collapse_to_vertex.face_indices.erase(collapse_face_iter);
      }
    }

    for (auto collapse_adjacent_vertex_index : collapse_vertex.adjacent_vertex_indices)
    {
      auto& collapse_adjacent_vertex = vertices[collapse_adjacent_vertex_index];

      auto collapse_vertex_iter = std::find(collapse_adjacent_vertex.adjacent_vertex_indices.begin(), collapse_adjacent_vertex.adjacent_vertex_indices.end(), collapse_index);
      collapse_adjacent_vertex.adjacent_vertex_indices.erase(collapse_vertex_iter);

      if (collapse_adjacent_vertex_index != collapse_to_index)
      {
        auto collapse_adjacent_vertex_iter = std::find(collapse_to_vertex.adjacent_vertex_indices.begin(), collapse_to_vertex.adjacent_vertex_indices.end(), collapse_adjacent_vertex_index);
        if (collapse_adjacent_vertex_iter == collapse_to_vertex.adjacent_vertex_indices.end())
        {
          collapse_to_vertex.adjacent_vertex_indices.push_back(collapse_adjacent_vertex_index);
          collapse_adjacent_vertex.adjacent_vertex_indices.push_back(collapse_to_index);
        }
      }

      update_collapse_cost(mesh, format, faces, vertices, collapse_adjacent_vertex);
    }

    auto& collapse_to_position = cast<vec3>(mesh.vertex_buffer, collapse_to_vertex.mesh_vertex_indices[0] * format.size + format.position_offset);
    auto collapse_to_color = format.has_color ? cast<vec4>(mesh.vertex_buffer, collapse_to_vertex.mesh_vertex_indices[0] * format.size + format.color_offset) : vec4();
    auto collapse_to_texture_coordinate = format.has_texture_coordinate ? cast<vec2>(mesh.vertex_buffer, collapse_to_vertex.mesh_vertex_indices[0] * format.size + format.texture_coordinate_offset): vec2();
    for (auto mesh_vertex_index : collapse_vertex.mesh_vertex_indices)
    {
      cast<vec3>(mesh.vertex_buffer, mesh_vertex_index * format.size + format.position_offset) = collapse_to_position;
      // We're not updating normals at the moment, any attempt I made just looked worse...
      if (format.has_color)
      {
        cast<vec4>(mesh.vertex_buffer, mesh_vertex_index * format.size + format.color_offset) = collapse_to_color;
      }
      if (format.has_texture_coordinate)
      {
        cast<vec2>(mesh.vertex_buffer, mesh_vertex_index * format.size + format.texture_coordinate_offset) = collapse_to_texture_coordinate;
      }
    }

    collapse_vertex.collapsed = true;
  }

  const vec3& position(const mesh& mesh, const vertex_format& format, const collapsable_vertex& vertex)
  {
    return cast<vec3>(mesh.vertex_buffer, vertex.mesh_vertex_indices[0] * format.size + format.position_offset);
  }

  std::function<bool(const collapsable_vertex& vertex)> contains_mesh_vertex(uint32_t mesh_vertex)
  {
    return [=] (const collapsable_vertex& vertex)
    {
      return std::find(vertex.mesh_vertex_indices.begin(), vertex.mesh_vertex_indices.end(), mesh_vertex) != vertex.mesh_vertex_indices.end();
    };
  }
}
