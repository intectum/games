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
  std::deque<task>& background_queue(instance& instance);
  std::vector<task_finalizer>& background_queue_finalizers(instance& instance);
  std::mutex& background_mutex(instance& instance);
  std::binary_semaphore& background_semaphore(instance& instance);

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

  void enqueue_background(instance& instance, const task& task)
  {
    auto& queue = background_queue(instance);
    auto& queue_finalizers = background_queue_finalizers(instance);
    auto& mutex = background_mutex(instance);
    auto& semaphore = background_semaphore(instance);

    {
      auto lock = std::scoped_lock(mutex);
      queue.emplace_back(task);
    }

    if (!semaphore.try_acquire())
    {
      return;
    }

    auto thread = std::thread([&queue, &queue_finalizers, &mutex, &semaphore]()
    {
      while (true)
      {
        auto task = ludo::task();

        {
          auto lock = std::scoped_lock(mutex);
          if (queue.empty())
          {
            break;
          }

          task = queue.front();
          queue.pop_front();
        }

        auto finalize = task();

        {
          auto lock = std::scoped_lock(mutex);
          queue_finalizers.emplace_back(finalize);
        }
      }

      semaphore.release();
    });

    thread.detach();
  }

  void finalize_background(instance& instance)
  {
    auto& queue_finalizers = background_queue_finalizers(instance);
    auto& mutex = background_mutex(instance);

    auto lock = std::scoped_lock(mutex);
    for (auto& finalizer : queue_finalizers)
    {
      finalizer();
    }

    queue_finalizers.clear();
  }

  std::deque<task>& background_queue(instance& instance)
  {
    auto name = "ludo::background::queue";

    if (instance.data.find(name) == instance.data.end())
    {
      instance.data[name] = new std::deque<task>();
    }

    return *static_cast<std::deque<task>*>(instance.data.at(name));
  }

  std::vector<task_finalizer>& background_queue_finalizers(instance& instance)
  {
    auto name = "ludo::background::queue-finalizers";

    if (instance.data.find(name) == instance.data.end())
    {
      instance.data[name] = new std::vector<task_finalizer>();
    }

    return *static_cast<std::vector<task_finalizer>*>(instance.data.at(name));
  }

  std::mutex& background_mutex(instance& instance)
  {
    auto name = "ludo::background::mutex";

    if (instance.data.find(name) == instance.data.end())
    {
      instance.data[name] = new std::mutex();
    }

    return *static_cast<std::mutex*>(instance.data.at(name));
  }

  std::binary_semaphore& background_semaphore(instance& instance)
  {
    auto name = "ludo::background::semaphore";

    if (instance.data.find(name) == instance.data.end())
    {
      instance.data[name] = new std::binary_semaphore(1);
    }

    return *static_cast<std::binary_semaphore*>(instance.data.at(name));
  }
}
