#include "mesh.h"

namespace astrum
{
  uint32_t face(const terrain& terrain, float radius, ludo::mesh& mesh, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t vertex_index, uint32_t low_detail_divisions, uint32_t high_detail_divisions, const std::array<ludo::vec3, 3>& positions);
  uint32_t low_detail_face(const terrain& terrain, ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_index, uint32_t divisions, const std::array<ludo::vec3, 3>& positions, const ludo::vec3& normal, const ludo::vec4& color);
  uint32_t high_detail_face(const terrain& terrain, float radius, ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_index, uint32_t divisions, const std::array<ludo::vec3, 3>& positions);

  void terrain_mesh(const terrain& terrain, float radius, ludo::mesh& mesh, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t index, uint32_t chunk_divisions, uint32_t low_detail_divisions, uint32_t high_detail_divisions)
  {
    auto chunks_per_ico_face = static_cast<uint32_t>(std::pow(4, chunk_divisions - 1));
    auto ico_face_index = static_cast<uint32_t>(static_cast<float>(index) / static_cast<float>(chunks_per_ico_face));

    assert(ico_face_index >= 0 && ico_face_index < 20);

    auto t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    auto ico_positions = std::array<ludo::vec3, 20>
    {
      ludo::vec3 { -1.0f, t, 0.0f },
      ludo::vec3 { 1.0f, t, 0.0f },
      ludo::vec3 { -1.0f, -t, 0.0f },
      ludo::vec3 { 1.0f, -t, 0.0f },

      ludo::vec3 { 0.0f, -1.0f, t },
      ludo::vec3 { 0.0f, 1.0f, t },
      ludo::vec3 { 0.0f, -1.0f, -t },
      ludo::vec3 { 0.0f, 1.0f, -t },

      ludo::vec3 { t, 0.0f, -1.0f },
      ludo::vec3 { t, 0.0f, 1.0f },
      ludo::vec3 { -t, 0.0f, -1.0f },
      ludo::vec3 { -t, 0.0f, 1.0f },
    };

    for (auto& ico_position : ico_positions)
    {
      normalize(ico_position);
    }

    std::array<std::array<ludo::vec3, 3>, 20> ico_faces =
    {{
      // 5 faces around point 0.
      { ico_positions[0], ico_positions[11], ico_positions[5] },
      { ico_positions[0], ico_positions[5], ico_positions[1] },
      { ico_positions[0], ico_positions[1], ico_positions[7] },
      { ico_positions[0], ico_positions[7], ico_positions[10] },
      { ico_positions[0], ico_positions[10], ico_positions[11] },

      // 5 adjacent faces.
      { ico_positions[1], ico_positions[5], ico_positions[9] },
      { ico_positions[5], ico_positions[11], ico_positions[4] },
      { ico_positions[11], ico_positions[10], ico_positions[2] },
      { ico_positions[10], ico_positions[7], ico_positions[6] },
      { ico_positions[7], ico_positions[1], ico_positions[8] },

      // 5 faces around point 3.
      { ico_positions[3], ico_positions[9], ico_positions[4] },
      { ico_positions[3], ico_positions[4], ico_positions[2] },
      { ico_positions[3], ico_positions[2], ico_positions[6] },
      { ico_positions[3], ico_positions[6], ico_positions[8] },
      { ico_positions[3], ico_positions[8], ico_positions[9] },

      // 5 adjacent faces.
      { ico_positions[4], ico_positions[9], ico_positions[5] },
      { ico_positions[2], ico_positions[4], ico_positions[11] },
      { ico_positions[6], ico_positions[2], ico_positions[10] },
      { ico_positions[8], ico_positions[6], ico_positions[7] },
      { ico_positions[9], ico_positions[8], ico_positions[1] }
    }};

    terrain_mesh(terrain, radius, mesh, low_detail_format, high_detail_format, write_low_detail_vertices, index % chunks_per_ico_face, chunk_divisions - 1, low_detail_divisions - 1, high_detail_divisions - 1, ico_faces[ico_face_index]);
  }

  void terrain_mesh(const terrain& terrain, float radius, ludo::mesh& mesh, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t index, uint32_t chunk_divisions, uint32_t low_detail_divisions, uint32_t high_detail_divisions, const std::array<ludo::vec3, 3>& positions)
  {
    if (chunk_divisions == 0)
    {
      assert(index >= 0 && index < 4);
      face(terrain, radius, mesh, low_detail_format, high_detail_format, write_low_detail_vertices, 0, low_detail_divisions, high_detail_divisions, positions);
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

    terrain_mesh(terrain, radius, mesh, low_detail_format, high_detail_format, write_low_detail_vertices, index % chunks_per_face, chunk_divisions - 1, low_detail_divisions - 1, high_detail_divisions - 1, face_positions);
  }

  uint32_t face(const terrain& terrain, float radius, ludo::mesh& mesh, const ludo::vertex_format& low_detail_format, const ludo::vertex_format& high_detail_format, bool write_low_detail_vertices, uint32_t vertex_index, uint32_t low_detail_divisions, uint32_t high_detail_divisions, const std::array<ludo::vec3, 3>& positions)
  {
    if (low_detail_divisions == 0)
    {
      if (write_low_detail_vertices)
      {
        auto height_0 = terrain.height_func(positions[0]);
        auto height_1 = terrain.height_func(positions[1]);
        auto height_2 = terrain.height_func(positions[2]);

        auto position_0 = positions[0] * radius * height_0;
        auto position_1 = positions[1] * radius * height_1;
        auto position_2 = positions[2] * radius * height_2;

        auto normal = ludo::cross(position_1 - position_0, position_2 - position_0);
        ludo::normalize(normal);

        auto color = terrain.color_func(positions[0][1], { height_0, height_1, height_2 }, ludo::dot(normal, positions[0]));

        auto end_vertex_index = low_detail_face(terrain, mesh, low_detail_format, vertex_index, high_detail_divisions, { position_0, position_1, position_2 }, normal, color);
        ludo::colorize(mesh, low_detail_format, vertex_index, end_vertex_index - vertex_index, color);
      }

      return high_detail_face(terrain, radius, mesh, high_detail_format, vertex_index, high_detail_divisions, positions);
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    vertex_index = face(terrain, radius, mesh, low_detail_format, high_detail_format, write_low_detail_vertices, vertex_index, low_detail_divisions - 1, high_detail_divisions - 1, { positions[0], position_01, position_02 });
    vertex_index = face(terrain, radius, mesh, low_detail_format, high_detail_format, write_low_detail_vertices, vertex_index, low_detail_divisions - 1, high_detail_divisions - 1, { position_01, positions[1], position_12 });
    vertex_index = face(terrain, radius, mesh, low_detail_format, high_detail_format, write_low_detail_vertices, vertex_index, low_detail_divisions - 1, high_detail_divisions - 1, { position_02, position_12, positions[2] });
    vertex_index = face(terrain, radius, mesh, low_detail_format, high_detail_format, write_low_detail_vertices, vertex_index, low_detail_divisions - 1, high_detail_divisions - 1, { position_01, position_12, position_02 });

    return vertex_index;
  }

  uint32_t low_detail_face(const terrain& terrain, ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_index, uint32_t divisions, const std::array<ludo::vec3, 3>& positions, const ludo::vec3& normal, const ludo::vec4& color)
  {
    if (divisions == 0)
    {
      auto i = vertex_index;
      auto v = vertex_index;
      ludo::write_vertex(mesh, format, i, v, positions[0], normal, { 0.0f, 0.0f }, false);
      ludo::write_vertex(mesh, format, i, v, positions[1], normal, { 0.0f, 0.0f }, false);
      ludo::write_vertex(mesh, format, i, v, positions[2], normal, { 0.0f, 0.0f }, false);

      return vertex_index + 3;
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;

    vertex_index = low_detail_face(terrain, mesh, format, vertex_index, divisions - 1, { positions[0], position_01, position_02 }, normal, color);
    vertex_index = low_detail_face(terrain, mesh, format, vertex_index, divisions - 1, { position_01, positions[1], position_12 }, normal, color);
    vertex_index = low_detail_face(terrain, mesh, format, vertex_index, divisions - 1, { position_02, position_12, positions[2] }, normal, color);
    vertex_index = low_detail_face(terrain, mesh, format, vertex_index, divisions - 1, { position_01, position_12, position_02 }, normal, color);

    return vertex_index;
  }

  uint32_t high_detail_face(const terrain& terrain, float radius, ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_index, uint32_t divisions, const std::array<ludo::vec3, 3>& positions)
  {
    if (divisions == 0)
    {
      auto height_0 = terrain.height_func(positions[0]);
      auto height_1 = terrain.height_func(positions[1]);
      auto height_2 = terrain.height_func(positions[2]);

      auto position_0 = positions[0] * radius * height_0;
      auto position_1 = positions[1] * radius * height_1;
      auto position_2 = positions[2] * radius * height_2;

      auto normal = ludo::cross(position_1 - position_0, position_2 - position_0);
      ludo::normalize(normal);

      auto color = terrain.color_func(positions[0][1], { height_0, height_1, height_2 }, ludo::dot(normal, positions[0]));

      auto i = vertex_index;
      auto v = vertex_index;
      ludo::write_vertex(mesh, format, i, v, position_0, normal, { 0.0f, 0.0f }, false);
      ludo::write_vertex(mesh, format, i, v, position_1, normal, { 0.0f, 0.0f }, false);
      ludo::write_vertex(mesh, format, i, v, position_2, normal, { 0.0f, 0.0f }, false);
      ludo::colorize(mesh, format, vertex_index, 3, color);

      return vertex_index + 3;
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    vertex_index = high_detail_face(terrain, radius, mesh, format, vertex_index, divisions - 1, { positions[0], position_01, position_02 });
    vertex_index = high_detail_face(terrain, radius, mesh, format, vertex_index, divisions - 1, { position_01, positions[1], position_12 });
    vertex_index = high_detail_face(terrain, radius, mesh, format, vertex_index, divisions - 1, { position_02, position_12, positions[2] });
    vertex_index = high_detail_face(terrain, radius, mesh, format, vertex_index, divisions - 1, { position_01, position_12, position_02 });

    return vertex_index;
  }
}
