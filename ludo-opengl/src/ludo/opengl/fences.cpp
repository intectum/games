/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <GL/glew.h>

#include "util.h"

namespace ludo
{
  const uint64_t timeout = 1000 * 1000 * 1000; // One second.

  fence create_fence()
  {
    auto fence = ludo::fence();

    auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); check_opengl_error();
    glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

    fence.id = reinterpret_cast<uint64_t>(sync);

    return fence;
  }

  void wait_for_fence(fence& fence)
  {
    if (!fence.id)
    {
      return;
    }

    auto sync = reinterpret_cast<__GLsync*>(fence.id);

    auto result = glClientWaitSync(sync, 0, timeout); check_opengl_error();
    assert(result != GL_TIMEOUT_EXPIRED && "fence was not signaled in time");

    glDeleteSync(sync); check_opengl_error();
    fence.id = 0;
  }
}
