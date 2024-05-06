/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_TASKS_H
#define LUDO_TASKS_H

#include <deque>
#include <functional>
#include <mutex>
#include <semaphore>

#include "core.h"

namespace ludo
{
  ///
  /// A function used to finalize a task.
  /// This is typically executed at a controlled time to avoid conflicts between threads.
  using task_finalizer = std::function<void()>;

  ///
  /// A task.
  /// Tasks are simply functions that return "finalize" functions.
  using task = std::function<task_finalizer()>;

  ///
  /// A ranged task.
  /// Tasks are simply functions that return "finalize" functions.
  using ranged_task = std::function<task_finalizer(uint32_t start, uint32_t end)>;

  ///
  /// Executes a task immediately on the current thread including finalization.
  /// \param task The task to execute.
  LUDO_API void execute(const task& task);

  ///
  /// Executes tasks in series and blocks this thread until they are all complete.
  /// \param task The tasks to execute.
  LUDO_API void series(const std::vector<task>& tasks);

  ///
  /// Executes tasks in parallel and blocks this thread until they are all complete.
  /// \param task The tasks to execute.
  /// \param max_thread_count The maximum number of threads to execute in parallel. Zero indicates that there is no
  /// maximum. At most, all the available threads from the thread pool will be used.
  LUDO_API void parallel(const std::vector<task>& tasks, uint32_t max_thread_count = 0);

  ///
  /// Executes a task in parallel, providing a sub-range to each instance of the task.
  /// \param range The full range to be divided between the instances of the task i.e. [0, range).
  /// \param task A task to execute for each sub-range.
  /// \param max_thread_count The maximum number of threads to execute in parallel. Zero indicates that there is no
  /// maximum. At most, all the available threads from the thread pool will be used.
  LUDO_API void divide_and_conquer(uint32_t range, const ranged_task& task, uint32_t max_thread_count = 0);

  ///
  /// Enqueues a task to be executed in the background. finalize_background_tasks(instance& instance) must be called to
  /// perform finalization.
  /// \param instance The instance in which the task will be enqueued.
  /// \param task The task to enqueue.
  LUDO_API void enqueue_background_task(instance& instance, const task& task);

  ///
  /// Finalizes all tasks that have completed execution in the background but are not yet finalized.
  /// \param instance The instance in which the tasks were executed.
  LUDO_API void finalize_background_tasks(instance& instance);
}

#endif // LUDO_TASKS_H
