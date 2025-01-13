#include <cmath>

#include "../meshes/ico_faces.h"
#include "mesh.h"

namespace astrum
{
  void face(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, float radius, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t low_detail_divisions, uint32_t high_detail_divisions, const std::array<ludo::vec3, 3>& positions);
  void low_detail_face(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, const ludo::vertex_format& format, uint32_t divisions, const std::array<ludo::vec3, 3>& positions, const ludo::vec3& normal, const ludo::vec4& color);
  void high_detail_face(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, float radius, const ludo::vertex_format& format, uint32_t divisions, const std::array<ludo::vec3, 3>& positions);

  void terrain_mesh(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, float radius, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t index, uint32_t chunk_divisions, uint32_t low_detail_divisions, uint32_t high_detail_divisions)
  {
    auto chunks_per_ico_face = static_cast<uint32_t>(std::pow(4, chunk_divisions - 1));
    auto ico_face_index = static_cast<uint32_t>(static_cast<float>(index) / static_cast<float>(chunks_per_ico_face));

    assert(ico_face_index >= 0 && ico_face_index < 20);

    auto& ico_faces = get_ico_faces();

    terrain_mesh(mesh, indices, vertices, terrain_funcs, radius, low_detail_format, high_detail_format, write_low_detail_vertices, index % chunks_per_ico_face, chunk_divisions - 1, low_detail_divisions - 1, high_detail_divisions - 1, ico_faces[ico_face_index]);
  }

  void terrain_mesh(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, float radius, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t index, uint32_t chunk_divisions, uint32_t low_detail_divisions, uint32_t high_detail_divisions, const std::array<ludo::vec3, 3>& positions)
  {
    if (chunk_divisions == 0)
    {
      assert(index >= 0 && index < 4);
      face(mesh, indices, vertices, terrain_funcs, radius, low_detail_format, high_detail_format, write_low_detail_vertices, low_detail_divisions, high_detail_divisions, positions);
      return;
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    auto chunks_per_face = static_cast<uint32_t>(std::pow(4, chunk_divisions - 1));
    auto face_index = static_cast<uint32_t>(static_cast<float>(index) / static_cast<float>(chunks_per_face));
    auto face_positions = std::array<ludo::vec3, 3>();

    assert(face_index >= 0 && face_index < 4);

    if (face_index == 0) face_positions = { positions[0], position_01, position_02 };
    if (face_index == 1) face_positions = { position_01, positions[1], position_12 };
    if (face_index == 2) face_positions = { position_02, position_12, positions[2] };
    if (face_index == 3) face_positions = { position_01, position_12, position_02 };

    terrain_mesh(mesh, indices, vertices, terrain_funcs, radius, low_detail_format, high_detail_format, write_low_detail_vertices, index % chunks_per_face, chunk_divisions - 1, low_detail_divisions - 1, high_detail_divisions - 1, face_positions);
  }

  void face(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, float radius, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t low_detail_divisions, uint32_t high_detail_divisions, const std::array<ludo::vec3, 3>& positions)
  {
    if (low_detail_divisions == 0)
    {
      if (write_low_detail_vertices)
      {
        auto height_0 = terrain_funcs.height(positions[0]);
        auto height_1 = terrain_funcs.height(positions[1]);
        auto height_2 = terrain_funcs.height(positions[2]);

        auto position_0 = positions[0] * radius * height_0;
        auto position_1 = positions[1] * radius * height_1;
        auto position_2 = positions[2] * radius * height_2;

        auto normal = ludo::cross(position_1 - position_0, position_2 - position_0);
        ludo::normalize(normal);

        auto color = terrain_funcs.color(positions[0][1], { height_0, height_1, height_2 }, ludo::dot(normal, positions[0]));

        auto temp_mesh = mesh;
        low_detail_face(mesh, indices, vertices, terrain_funcs, low_detail_format, high_detail_divisions, { position_0, position_1, position_2 }, normal, color);
        mesh = temp_mesh; // Do not keep updates to ranges
      }

      return high_detail_face(mesh, indices, vertices, terrain_funcs, radius, high_detail_format, high_detail_divisions, positions);
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    face(mesh, indices, vertices, terrain_funcs, radius, low_detail_format, high_detail_format, write_low_detail_vertices, low_detail_divisions - 1, high_detail_divisions - 1, { positions[0], position_01, position_02 });
    face(mesh, indices, vertices, terrain_funcs, radius, low_detail_format, high_detail_format, write_low_detail_vertices, low_detail_divisions - 1, high_detail_divisions - 1, { position_01, positions[1], position_12 });
    face(mesh, indices, vertices, terrain_funcs, radius, low_detail_format, high_detail_format, write_low_detail_vertices, low_detail_divisions - 1, high_detail_divisions - 1, { position_02, position_12, positions[2] });
    face(mesh, indices, vertices, terrain_funcs, radius, low_detail_format, high_detail_format, write_low_detail_vertices, low_detail_divisions - 1, high_detail_divisions - 1, { position_01, position_12, position_02 });
  }

  void low_detail_face(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, const ludo::vertex_format& format, uint32_t divisions, const std::array<ludo::vec3, 3>& positions, const ludo::vec3& normal, const ludo::vec4& color)
  {
    if (divisions == 0)
    {
      ludo::append_vertex(mesh, indices, vertices, format, positions[0], normal, color, { 0.0f, 0.0f }, false);
      ludo::append_vertex(mesh, indices, vertices, format, positions[1], normal, color, { 0.0f, 0.0f }, false);
      ludo::append_vertex(mesh, indices, vertices, format, positions[2], normal, color, { 0.0f, 0.0f }, false);

      return;
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;

    low_detail_face(mesh, indices, vertices, terrain_funcs, format, divisions - 1, { positions[0], position_01, position_02 }, normal, color);
    low_detail_face(mesh, indices, vertices, terrain_funcs, format, divisions - 1, { position_01, positions[1], position_12 }, normal, color);
    low_detail_face(mesh, indices, vertices, terrain_funcs, format, divisions - 1, { position_02, position_12, positions[2] }, normal, color);
    low_detail_face(mesh, indices, vertices, terrain_funcs, format, divisions - 1, { position_01, position_12, position_02 }, normal, color);
  }

  void high_detail_face(ludo::mesh& mesh, ludo::buffer& indices, ludo::buffer& vertices, const terrain_funcs& terrain_funcs, float radius, const ludo::vertex_format& format, uint32_t divisions, const std::array<ludo::vec3, 3>& positions)
  {
    if (divisions == 0)
    {
      auto height_0 = terrain_funcs.height(positions[0]);
      auto height_1 = terrain_funcs.height(positions[1]);
      auto height_2 = terrain_funcs.height(positions[2]);

      auto position_0 = positions[0] * radius * height_0;
      auto position_1 = positions[1] * radius * height_1;
      auto position_2 = positions[2] * radius * height_2;

      auto normal = ludo::cross(position_1 - position_0, position_2 - position_0);
      ludo::normalize(normal);

      auto color = terrain_funcs.color(positions[0][1], { height_0, height_1, height_2 }, ludo::dot(normal, positions[0]));

      ludo::append_vertex(mesh, indices, vertices, format, position_0, normal, color, { 0.0f, 0.0f }, false);
      ludo::append_vertex(mesh, indices, vertices, format, position_1, normal, color, { 0.0f, 0.0f }, false);
      ludo::append_vertex(mesh, indices, vertices, format, position_2, normal, color, { 0.0f, 0.0f }, false);

      return;
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    high_detail_face(mesh, indices, vertices, terrain_funcs, radius, format, divisions - 1, { positions[0], position_01, position_02 });
    high_detail_face(mesh, indices, vertices, terrain_funcs, radius, format, divisions - 1, { position_01, positions[1], position_12 });
    high_detail_face(mesh, indices, vertices, terrain_funcs, radius, format, divisions - 1, { position_02, position_12, positions[2] });
    high_detail_face(mesh, indices, vertices, terrain_funcs, radius, format, divisions - 1, { position_01, position_12, position_02 });
  }
}
