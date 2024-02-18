#ifndef ASTRUM_POST_PROCESSING_PASS_H
#define ASTRUM_POST_PROCESSING_PASS_H

#include <ludo/api.h>

namespace astrum
{
  void add_pass(ludo::instance& inst, bool target_screen = false);
}

#endif // ASTRUM_POST_PROCESSING_PASS_H
