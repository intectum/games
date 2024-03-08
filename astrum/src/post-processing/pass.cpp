#include <fstream>

#include <ludo/opengl/built_ins.h>

#include "pass.h"
#include "util.h"

namespace astrum
{
  void add_pass(ludo::instance& inst, bool target_screen)
  {
    auto& frame_buffers = ludo::data<ludo::frame_buffer>(inst);
    auto& previous_frame_buffer = frame_buffers[frame_buffers.array_size - 1];

    if (target_screen)
    {
      ludo::add<ludo::script>(inst, [previous_frame_buffer](ludo::instance& inst)
      {
        auto& window = *ludo::first<ludo::window>(inst);
        ludo::blit(previous_frame_buffer, ludo::frame_buffer { .width = window.width, .height = window.height });
      });
    }
    else
    {
      auto frame_buffer = *add_post_processing_frame_buffer(inst, true);

      ludo::add<ludo::script>(inst, [previous_frame_buffer, frame_buffer](ludo::instance& inst)
      {
        ludo::blit(previous_frame_buffer, frame_buffer);
      });
    }
  }
}
