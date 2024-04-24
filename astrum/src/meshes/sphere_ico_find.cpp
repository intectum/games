#include "sphere_ico.h"

namespace astrum
{
  void face(std::unordered_map<uint32_t, std::array<ludo::vec3, 3>>& sections, uint32_t cumulative_index, uint32_t section_divisions, const std::function<bool(const std::array<ludo::vec3, 3>& triangle)>& test, const std::array<ludo::vec3, 3>& positions);

  std::unordered_map<uint32_t, std::array<ludo::vec3, 3>> find_ico_sections(uint32_t section_divisions, const std::function<bool(const std::array<ludo::vec3, 3>& triangle)>& test)
  {
    std::unordered_map<uint32_t, std::array<ludo::vec3, 3>> sections;

    auto sections_per_face = static_cast<uint32_t>(std::pow(4, section_divisions - 1));

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

    auto ico_faces = std::array<std::array<ludo::vec3, 3>, 20>
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

    for (auto index = 0; index < ico_faces.size(); index++)
    {
      if (test(ico_faces[index]))
      {
        face(sections, index * sections_per_face, section_divisions - 1, test, ico_faces[index]);
      }
    }

    return sections;
  }

  void face(std::unordered_map<uint32_t, std::array<ludo::vec3, 3>>& sections, uint32_t cumulative_index, uint32_t section_divisions, const std::function<bool(const std::array<ludo::vec3, 3>& triangle)>& test, const std::array<ludo::vec3, 3>& positions)
  {
    if (section_divisions == 0)
    {
      sections[cumulative_index] = positions;
      return;
    }

    auto sections_per_face = static_cast<uint32_t>(std::pow(4, section_divisions - 1));

    auto position_01 = (positions[0] + positions[1]) * 0.5f;
    auto position_02 = (positions[0] + positions[2]) * 0.5f;
    auto position_12 = (positions[1] + positions[2]) * 0.5f;
    normalize(position_01);
    normalize(position_02);
    normalize(position_12);

    auto sub_faces = std::array<std::array<ludo::vec3, 3>, 4>
    {{
      { positions[0], position_01, position_02 },
      { position_01, positions[1], position_12 },
      { position_02, position_12, positions[2] },
      { position_01, position_12, position_02 }
    }};

    for (auto index = 0; index < sub_faces.size(); index++)
    {
      if (test(sub_faces[index]))
      {
        face(sections, cumulative_index + index * sections_per_face, section_divisions - 1, test, sub_faces[index]);
      }
    }
  }
}
