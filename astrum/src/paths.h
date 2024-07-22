#pragma once

#include <ludo/api.h>

namespace astrum
{
  void add_prediction_paths(ludo::instance& inst, const std::vector<ludo::vec4>& colors);

  void update_prediction_paths(ludo::instance& inst);
}
