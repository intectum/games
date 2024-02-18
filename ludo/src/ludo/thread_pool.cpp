/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "logging.h"
#include "thread_pool.h"

namespace ludo
{
  const auto thread_pool_size = std::thread::hardware_concurrency();

  auto thread_pool_executing = std::atomic_uint(0);
  auto thread_pool_semaphore = std::counting_semaphore(thread_pool_size);

  uint32_t thread_pool_available()
  {
    return thread_pool_size - thread_pool_executing.load();
  }

  std::future<task_finalizer> thread_pool_execute(const task& task)
  {
    while (!thread_pool_semaphore.try_acquire_for(std::chrono::duration(std::chrono::seconds(1))))
    {
      log_warning("ludo", "waiting for thread pool availability...");
    }
    thread_pool_executing++;

    return std::async(std::launch::async, [task]()
    {
      auto finalizer = task();

      thread_pool_executing--;
      thread_pool_semaphore.release();

      return finalizer;
    });
  }
}
