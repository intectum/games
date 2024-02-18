#include "sphere_ico.h"

namespace astrum
{
  void face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t index, uint32_t section_divisions, uint32_t divisions, const std::array<ludo::vec3, 3>& positions, uint8_t position_offset);
  uint32_t face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t divisions, const std::array<ludo::vec3, 3>& positions, uint8_t position_offset);
  uint32_t face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, const std::array<ludo::vec3, 3>& positions, uint8_t position_offset);
  void vertex(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_index, const ludo::vec3& position, uint8_t position_offset);

  void ico_section(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t index, uint32_t section_divisions, uint32_t divisions, uint8_t position_offset)
  {
    auto t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    auto positions = std::array<ludo::vec3, 20>
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

    for (auto& position : positions)
    {
      normalize(position);
    }

    auto sections_per_face = static_cast<uint32_t>(std::pow(4, section_divisions - 1));
    auto face_index = static_cast<uint32_t>(static_cast<float>(index) / static_cast<float>(sections_per_face));
    auto face_positions = std::array<ludo::vec3, 3>();

    assert(face_index >= 0 && face_index < 20);

    // 5 faces around point 0.
    if (face_index == 0) face_positions = { positions[0], positions[11], positions[5] };
    if (face_index == 1) face_positions = { positions[0], positions[5], positions[1] };
    if (face_index == 2) face_positions = { positions[0], positions[1], positions[7] };
    if (face_index == 3) face_positions = { positions[0], positions[7], positions[10] };
    if (face_index == 4) face_positions = { positions[0], positions[10], positions[11] };

    // 5 adjacent faces.
    if (face_index == 5) face_positions = { positions[1], positions[5], positions[9] };
    if (face_index == 6) face_positions = { positions[5], positions[11], positions[4] };
    if (face_index == 7) face_positions = { positions[11], positions[10], positions[2] };
    if (face_index == 8) face_positions = { positions[10], positions[7], positions[6] };
    if (face_index == 9) face_positions = { positions[7], positions[1], positions[8] };

    // 5 faces around point 3.
    if (face_index == 10) face_positions = { positions[3], positions[9], positions[4] };
    if (face_index == 11) face_positions = { positions[3], positions[4], positions[2] };
    if (face_index == 12) face_positions = { positions[3], positions[2], positions[6] };
    if (face_index == 13) face_positions = { positions[3], positions[6], positions[8] };
    if (face_index == 14) face_positions = { positions[3], positions[8], positions[9] };

    // 5 adjacent faces.
    if (face_index == 15) face_positions = { positions[4], positions[9], positions[5] };
    if (face_index == 16) face_positions = { positions[2], positions[4], positions[11] };
    if (face_index == 17) face_positions = { positions[6], positions[2], positions[10] };
    if (face_index == 18) face_positions = { positions[8], positions[6], positions[7] };
    if (face_index == 19) face_positions = { positions[9], positions[8], positions[1] };

    face(mesh, format, vertex_start, index % sections_per_face, section_divisions - 1, divisions - 1, face_positions, position_offset);
  }

  void face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t index, uint32_t section_divisions, uint32_t divisions, const std::array<ludo::vec3, 3>& positions, uint8_t position_offset)
  {
    if (section_divisions == 0)
    {
      face(mesh, format, vertex_start, divisions, { positions[0], positions[1], positions[2] }, position_offset);
      return;
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    auto sections_per_face = static_cast<uint32_t>(std::pow(4, section_divisions - 1));
    auto face_index = static_cast<uint32_t>(static_cast<float>(index) / static_cast<float>(sections_per_face));
    auto face_positions = std::array<ludo::vec3, 3>();

    assert(face_index >= 0 && face_index < 4);

    if (face_index == 0) face_positions = { positions[0], position_01, position_02 };
    if (face_index == 1) face_positions = { position_01, positions[1], position_12 };
    if (face_index == 2) face_positions = { position_02, position_12, positions[2] };
    if (face_index == 3) face_positions = { position_01, position_12, position_02 };

    face(mesh, format, vertex_start, index % sections_per_face, section_divisions - 1, divisions - 1, face_positions, position_offset);
  }

  uint32_t face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t divisions, const std::array<ludo::vec3, 3>& positions, uint8_t position_offset)
  {
    if (divisions == 0)
    {
      return face(mesh, format, vertex_start, { positions[0], positions[1], positions[2] }, position_offset);
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    vertex_start = face(mesh, format, vertex_start, divisions - 1, { positions[0], position_01, position_02 }, position_offset);
    vertex_start = face(mesh, format, vertex_start, divisions - 1, { position_01, positions[1], position_12 }, position_offset);
    vertex_start = face(mesh, format, vertex_start, divisions - 1, { position_02, position_12, positions[2] }, position_offset);
    vertex_start = face(mesh, format, vertex_start, divisions - 1, { position_01, position_12, position_02 }, position_offset);

    return vertex_start;
  }

  uint32_t face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, const std::array<ludo::vec3, 3>& positions, uint8_t position_offset)
  {
    vertex(mesh, format, vertex_start++, positions[0], position_offset);
    vertex(mesh, format, vertex_start++, positions[1], position_offset);
    vertex(mesh, format, vertex_start++, positions[2], position_offset);

    return vertex_start;
  }

  void vertex(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_index, const ludo::vec3& position, uint8_t position_offset)
  {
    auto vertex_byte_index = vertex_index * format.size;

    write(mesh.vertex_buffer, vertex_byte_index + position_offset, position);
    write(mesh.index_buffer, vertex_index * sizeof(uint32_t), vertex_index);
  }
}
