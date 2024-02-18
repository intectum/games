/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_THREAD_POOL_H
#define LUDO_THREAD_POOL_H

#include <future>

#include "tasks.h"

namespace ludo
{
  ///
  /// Retrieves the number of available threads in the thread pool.
  /// \return The number of available threads in the thread pool.
  uint32_t thread_pool_available();

  ///
  /// Executes a task in a thread of the thread pool. If no threads are available it blocks until one becomes available.
  /// \param task The task to execute.
  /// \return A future whose result is the finalizer for the task.
  std::future<task_finalizer> thread_pool_execute(const task& task);
}

#endif // LUDO_THREAD_POOL_H
