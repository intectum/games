#include "sphere_ico.h"

namespace astrum
{
  uint32_t face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t divisions, uint8_t position_offset, const std::array<ludo::vec3, 3>& positions);
  uint32_t face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint8_t position_offset, const std::array<ludo::vec3, 3>& positions);
  void vertex(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_index, uint8_t position_offset, const ludo::vec3& position);

  void ico_section(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t index, uint32_t section_divisions, uint32_t divisions, uint8_t position_offset)
  {
    auto sections_per_face = static_cast<uint32_t>(std::pow(4, section_divisions - 1));
    auto face_index = static_cast<uint32_t>(static_cast<float>(index) / static_cast<float>(sections_per_face));

    assert(face_index >= 0 && face_index < 20);

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
    
    auto faces = std::array<std::array<ludo::vec3, 3>, 20>
    {{
      // 5 faces around point 0.
      { positions[0], positions[11], positions[5] },
      { positions[0], positions[5], positions[1] },
      { positions[0], positions[1], positions[7] },
      { positions[0], positions[7], positions[10] },
      { positions[0], positions[10], positions[11] },
      
      // 5 adjacent faces.
      { positions[1], positions[5], positions[9] },
      { positions[5], positions[11], positions[4] },
      { positions[11], positions[10], positions[2] },
      { positions[10], positions[7], positions[6] },
      { positions[7], positions[1], positions[8] },
      
      // 5 faces around point 3.
      { positions[3], positions[9], positions[4] },
      { positions[3], positions[4], positions[2] },
      { positions[3], positions[2], positions[6] },
      { positions[3], positions[6], positions[8] },
      { positions[3], positions[8], positions[9] },
      
      // 5 adjacent faces.
      { positions[4], positions[9], positions[5] },
      { positions[2], positions[4], positions[11] },
      { positions[6], positions[2], positions[10] },
      { positions[8], positions[6], positions[7] },
      { positions[9], positions[8], positions[1] }
    }};

    ico_section(mesh, format, vertex_start, index % sections_per_face, section_divisions - 1, divisions - 1, position_offset, faces[face_index]);
  }

  void ico_section(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t index, uint32_t section_divisions, uint32_t divisions, uint8_t position_offset, const std::array<ludo::vec3, 3>& positions)
  {
    if (section_divisions == 0)
    {
      assert(index >= 0 && index < 4);
      face(mesh, format, vertex_start, divisions, position_offset, positions);
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

    ico_section(mesh, format, vertex_start, index % sections_per_face, section_divisions - 1, divisions - 1, position_offset, face_positions);
  }

  uint32_t face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint32_t divisions, uint8_t position_offset, const std::array<ludo::vec3, 3>& positions)
  {
    if (divisions == 0)
    {
      return face(mesh, format, vertex_start, position_offset, positions);
    }

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    vertex_start = face(mesh, format, vertex_start, divisions - 1, position_offset, { positions[0], position_01, position_02 });
    vertex_start = face(mesh, format, vertex_start, divisions - 1, position_offset, { position_01, positions[1], position_12 });
    vertex_start = face(mesh, format, vertex_start, divisions - 1, position_offset, { position_02, position_12, positions[2] });
    vertex_start = face(mesh, format, vertex_start, divisions - 1, position_offset, { position_01, position_12, position_02 });

    return vertex_start;
  }

  uint32_t face(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_start, uint8_t position_offset, const std::array<ludo::vec3, 3>& positions)
  {
    vertex(mesh, format, vertex_start++, position_offset, positions[0]);
    vertex(mesh, format, vertex_start++, position_offset, positions[1]);
    vertex(mesh, format, vertex_start++, position_offset, positions[2]);

    return vertex_start;
  }

  void vertex(ludo::mesh& mesh, const ludo::vertex_format& format, uint32_t vertex_index, uint8_t position_offset, const ludo::vec3& position)
  {
    auto vertex_byte_index = vertex_index * format.size;

    write(mesh.vertex_buffer, vertex_byte_index + position_offset, position);
    write(mesh.index_buffer, vertex_index * sizeof(uint32_t), vertex_index);
  }
}
