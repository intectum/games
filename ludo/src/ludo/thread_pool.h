/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <functional>

namespace ludo
{
  void thread_pool_start();

  ///
  /// Executes a task in the thread pool.
  /// \param task The task to execute.
  void thread_pool_enqueue(const std::function<void()>& task);
}
