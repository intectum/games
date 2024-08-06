/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <chrono>
#include <queue>
#include <thread>

#include "threading.h"

namespace ludo
{
  static auto threads = std::vector<std::thread>();
  static auto thread_pool_running = false;
  static auto queue = std::queue<std::function<void()>>();
  static auto mutex = std::mutex();
  static auto semaphore = std::counting_semaphore(0);

  void thread_pool_start()
  {
    while (threads.size() < std::thread::hardware_concurrency())
    {
      threads.emplace_back([]()
      {
        while (thread_pool_running)
        {
          if (semaphore.try_acquire_for(std::chrono::milliseconds(100)))
          {
            mutex.lock();
            auto task = queue.front();
            queue.pop();
            mutex.unlock();

            task();
          }
        }
      });

      thread_pool_running = true;
    }
  }

  void thread_pool_stop()
  {
    thread_pool_running = false;
    for (auto& thread : threads)
    {
      thread.join();
    }
    threads.clear();
  }

  void thread_pool_enqueue(const std::function<void()>& task)
  {
    mutex.lock();
    queue.push(task);
    mutex.unlock();

    semaphore.release();
  }
}
