/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <queue>
#include <thread>

#include "thread_pool.h"

namespace ludo
{
  static auto queue = std::queue<std::function<void()>>();
  static auto mutex = std::mutex();
  static auto semaphore = std::counting_semaphore(0);

  void thread_pool_start()
  {
    static auto threads = std::vector<std::thread>();
    while (threads.size() < std::thread::hardware_concurrency())
    {
      threads.emplace_back([]()
      {
        while (true)
        {
          semaphore.acquire();

          mutex.lock();
          auto task = queue.front();
          queue.pop();
          mutex.unlock();

          task();
        }
      });
    }
  }

  void thread_pool_enqueue(const std::function<void()>& task)
  {
    mutex.lock();
    queue.push(task);
    mutex.unlock();

    semaphore.release();
  }
}
