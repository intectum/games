/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <GL/glew.h>

#include <ludo/rendering.h>

#include "util.h"

namespace ludo
{
  const uint64_t timeout = 1000 * 1000 * 1000; // One second.

  void init(fence& fence)
  {
    auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); check_opengl_error();
    glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

    fence.id = reinterpret_cast<uint64_t>(sync);
  }

  void de_init(fence& fence)
  {
    glDeleteSync(reinterpret_cast<__GLsync*>(fence.id)); check_opengl_error();
    fence.id = 0;
  }

  void wait(fence& fence)
  {
    assert(fence.id && "fence not initialized");

    auto sync = reinterpret_cast<__GLsync*>(fence.id);

    auto result = glClientWaitSync(sync, 0, timeout); check_opengl_error();
    assert(result != GL_TIMEOUT_EXPIRED && "fence was not signaled in time");

    de_init(fence);
  }
}
