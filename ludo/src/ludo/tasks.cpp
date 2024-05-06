/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <deque>
#include <mutex>
#include <semaphore>

#include "tasks.h"
#include "thread_pool.h"

namespace ludo
{
  struct background_task_queue
  {
    std::deque<task> tasks;
    std::vector<task_finalizer> finalizers;
    std::mutex mutex;
    std::binary_semaphore semaphore = std::binary_semaphore(1);
  };

  const auto background_task_queue_key = "ludo::background_task_queue";

  void execute(const task& task)
  {
    task()();
  }

  void series(const std::vector<task>& tasks)
  {
    auto finalizers = std::vector<task_finalizer>();
    for (auto& task : tasks)
    {
      execute(task);
    }
  }

  void parallel(const std::vector<task>& tasks, uint32_t max_thread_count)
  {
    auto thread_count = static_cast<uint32_t>(max_thread_count ? max_thread_count : tasks.size());
    auto thread_executing_semaphore = std::counting_semaphore(thread_count);

    auto futures = std::vector<std::future<task_finalizer>>();
    for (auto& task : tasks)
    {
      thread_executing_semaphore.acquire();
      futures.emplace_back(thread_pool_execute([&task, &thread_executing_semaphore]()
      {
        auto finalizer = task();

        thread_executing_semaphore.release();

        return finalizer;
      }));
    }

    for (auto& future : futures)
    {
      future.wait();
    }

    for (auto& future : futures)
    {
      future.get()();
    }
  }

  void divide_and_conquer(uint32_t range, const ranged_task& task, uint32_t max_thread_count)
  {
    auto thread_count = max_thread_count ? std::min(max_thread_count, thread_pool_available()) : thread_pool_available();
    assert(thread_count && "no threads available");

    auto range_per_thread = std::max(range / thread_count, uint32_t(1));

    auto futures = std::vector<std::future<task_finalizer>>();
    for (auto thread_index = 0; thread_index < thread_count; thread_index++)
    {
      auto start = thread_index * range_per_thread;
      auto end = thread_index == thread_count - 1 ? range : (thread_index + 1) * range_per_thread;

      // Could happen if the range is less than the thread count.
      if (start >= range)
      {
        break;
      }

      futures.emplace_back(thread_pool_execute([&task, start, end]()
      {
        return task(start, end);
      }));
    }

    for (auto& future : futures)
    {
      future.wait();
    }

    for (auto& future : futures)
    {
      future.get()();
    }
  }

  void enqueue_background_task(instance& instance, const task& task)
  {
    if (!instance.data.contains(background_task_queue_key))
    {
      instance.data[background_task_queue_key] = new background_task_queue();
    }

    auto queue = static_cast<background_task_queue*>(instance.data.at(background_task_queue_key));

    {
      auto lock = std::scoped_lock(queue->mutex);
      queue->tasks.emplace_back(task);
    }

    if (!queue->semaphore.try_acquire())
    {
      return;
    }

    auto thread = std::thread([queue]()
    {
      while (true)
      {
        auto task = ludo::task();

        {
          auto lock = std::scoped_lock(queue->mutex);
          if (queue->tasks.empty())
          {
            break;
          }

          task = queue->tasks.front();
          queue->tasks.pop_front();
        }

        auto finalize = task();

        {
          auto lock = std::scoped_lock(queue->mutex);
          queue->finalizers.emplace_back(finalize);
        }
      }

      queue->semaphore.release();
    });

    thread.detach();
  }

  void finalize_background_tasks(instance& instance)
  {
    if (!instance.data.contains(background_task_queue_key))
    {
      instance.data[background_task_queue_key] = new background_task_queue();
    }

    auto queue = static_cast<background_task_queue*>(instance.data.at(background_task_queue_key));

    auto lock = std::scoped_lock(queue->mutex);
    for (auto& finalizer : queue->finalizers)
    {
      finalizer();
    }

    queue->finalizers.clear();
  }
}
