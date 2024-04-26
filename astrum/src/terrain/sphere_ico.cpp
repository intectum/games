#include "sphere_ico.h"

namespace astrum
{
  void find_sphere_ico_chunks(std::unordered_map<uint32_t, std::array<ludo::vec3, 3>>& sections, uint32_t cumulative_index, uint32_t section_divisions, const std::function<bool(const std::array<ludo::vec3, 3>& triangle)>& test, const std::array<ludo::vec3, 3>& positions);

  std::unordered_map<uint32_t, std::array<ludo::vec3, 3>> find_sphere_ico_chunks(uint32_t section_divisions, const std::function<bool(const std::array<ludo::vec3, 3>& triangle)>& test)
  {
    std::unordered_map<uint32_t, std::array<ludo::vec3, 3>> sections;

    auto sections_per_face = static_cast<uint32_t>(std::pow(4, section_divisions - 1));
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

    for (auto index = 0; index < ico_faces.size(); index++)
    {
      if (test(ico_faces[index]))
      {
        find_sphere_ico_chunks(sections, index * sections_per_face, section_divisions - 1, test, ico_faces[index]);
      }
    }

    return sections;
  }

  void find_sphere_ico_chunks(std::unordered_map<uint32_t, std::array<ludo::vec3, 3>>& sections, uint32_t cumulative_index, uint32_t section_divisions, const std::function<bool(const std::array<ludo::vec3, 3>& triangle)>& test, const std::array<ludo::vec3, 3>& positions)
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
        find_sphere_ico_chunks(sections, cumulative_index + index * sections_per_face, section_divisions - 1, test, sub_faces[index]);
      }
    }
  }
}
