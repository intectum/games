#include "ico_faces.h"

namespace astrum
{
  std::array<std::array<ludo::vec3, 3>, 20> ico_faces;
  bool built = false;

  const std::array<std::array<ludo::vec3, 3>, 20>& get_ico_faces()
  {
    if (!built)
    {
      auto t = (1.0f + std::sqrt(5.0f)) / 2.0f;

      auto ico_positions = std::array<ludo::vec3, 20>
      {{
        { -1.0f, t, 0.0f },
        { 1.0f, t, 0.0f },
        { -1.0f, -t, 0.0f },
        { 1.0f, -t, 0.0f },

        { 0.0f, -1.0f, t },
        { 0.0f, 1.0f, t },
        { 0.0f, -1.0f, -t },
        { 0.0f, 1.0f, -t },

        { t, 0.0f, -1.0f },
        { t, 0.0f, 1.0f },
        { -t, 0.0f, -1.0f },
        { -t, 0.0f, 1.0f },
      }};

      for (auto& ico_position : ico_positions)
      {
        normalize(ico_position);
      }

      ico_faces =
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

      built = true;
    }

    return ico_faces;
  }
}
