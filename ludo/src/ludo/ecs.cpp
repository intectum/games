/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <iostream> // TODO remove
#include <thread>

#include "ecs.h"
#include "threading.h"

namespace ludo
{
  void run_unblocked_jobs(container& container, std::binary_semaphore& running_semaphore, std::mutex& data_mutex, std::vector<const job*>& pending_jobs, uint32_t& running_job_count, std::unordered_map<std::string, uint32_t>& component_read_counts, std::unordered_map<std::string, uint32_t>& component_write_counts);
  void enqueue_job(container& container, std::binary_semaphore& running_semaphore, std::mutex& data_mutex, std::vector<const job*>& pending_jobs, uint32_t& running_job_count, std::unordered_map<std::string, uint32_t>& component_read_counts, std::unordered_map<std::string, uint32_t>& component_write_counts, const job* job, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data);

  void init(container& container)
  {
    for (auto& archetype : container.archetypes)
    {
      for (auto& component_name : archetype.component_names)
      {
        auto component_iter = std::find_if(container.components.begin(), container.components.end(), [&](const component& component)
        {
          return component.name == component_name;
        });

        assert(component_iter != container.components.end() && "component not found");

        archetype.component_data.push_back(allocate_arena(archetype.capacity * component_iter->size));
      }
    }
  }

  void run(container& container, const std::vector<job>& jobs)
  {
    std::cout << "run!" << std::endl;

    auto running_semaphore = std::binary_semaphore(0);
    auto data_mutex = std::mutex();
    auto pending_jobs = std::vector<const job*>();
    for (auto& job : jobs)
    {
      pending_jobs.emplace_back(&job);
    }
    auto running_job_count = uint32_t(0);
    auto component_read_counts = std::unordered_map<std::string, uint32_t>();
    auto component_write_counts = std::unordered_map<std::string, uint32_t>();

    run_unblocked_jobs(container, running_semaphore, data_mutex, pending_jobs, running_job_count, component_read_counts, component_write_counts);

    running_semaphore.acquire();
  }

  void run_unblocked_jobs(container& container, std::binary_semaphore& running_semaphore, std::mutex& data_mutex, std::vector<const job*>& pending_jobs, uint32_t& running_job_count, std::unordered_map<std::string, uint32_t>& component_read_counts, std::unordered_map<std::string, uint32_t>& component_write_counts)
  {
    std::cout << "  run_unblocked_jobs" << std::endl;

    data_mutex.lock();
    for (auto job_iter = pending_jobs.begin(); job_iter != pending_jobs.end(); pending_jobs.erase(job_iter))
    {
      auto job = *job_iter;
      auto blocked = false;

      for (auto& read_component_name : job->read_component_names)
      {
        if (component_write_counts[read_component_name])
        {
          blocked = true;
          break;
        }
      }

      for (auto& write_component_name : job->write_component_names)
      {
        if (component_read_counts[write_component_name] || component_write_counts[write_component_name])
        {
          blocked = true;
          break;
        }
      }

      if (blocked)
      {
        break;
      }

      if (job->read_component_names.empty() && job->write_component_names.empty())
      {
        auto empty_component_data = std::vector<arena>();
        enqueue_job(container, running_semaphore, data_mutex, pending_jobs, running_job_count, component_read_counts, component_write_counts, job, empty_component_data, empty_component_data);
      }
      else
      {
        for (auto& archetype : container.archetypes)
        {
          auto read_component_data = std::vector<arena>();
          for (auto& read_component_name : job->read_component_names)
          {
            auto archetype_component_name_iter = std::find(archetype.component_names.begin(), archetype.component_names.end(), read_component_name);
            if (archetype_component_name_iter != archetype.component_names.end())
            {
              read_component_data.emplace_back(archetype.component_data[archetype_component_name_iter - archetype.component_names.begin()]);
            }
          }

          if (read_component_data.size() != job->read_component_names.size())
          {
            continue;
          }

          auto write_component_data = std::vector<arena>();
          for (auto& write_component_name : job->write_component_names)
          {
            auto archetype_component_name_iter = std::find(archetype.component_names.begin(), archetype.component_names.end(), write_component_name);
            if (archetype_component_name_iter != archetype.component_names.end())
            {
              write_component_data.emplace_back(archetype.component_data[archetype_component_name_iter - archetype.component_names.begin()]);
            }
          }

          if (write_component_data.size() != job->write_component_names.size())
          {
            continue;
          }

          for (auto& read_component_name : job->read_component_names)
          {
            component_read_counts[read_component_name]++;
          }
          for (auto& write_component_name : job->write_component_names)
          {
            component_write_counts[write_component_name]++;
          }

          enqueue_job(container, running_semaphore, data_mutex, pending_jobs, running_job_count, component_read_counts, component_write_counts, job, read_component_data, write_component_data);
        }
      }
    }
    data_mutex.unlock();
  }

  void enqueue_job(container& container, std::binary_semaphore& running_semaphore, std::mutex& data_mutex, std::vector<const job*>& pending_jobs, uint32_t& running_job_count, std::unordered_map<std::string, uint32_t>& component_read_counts, std::unordered_map<std::string, uint32_t>& component_write_counts, const job* job, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)
  {
    std::cout << "  enqueueing " << job << std::endl;
    running_job_count++;

    thread_pool_enqueue([=, &container, &running_semaphore, &data_mutex, &pending_jobs, &running_job_count, &component_read_counts, &component_write_counts]()
    {
      auto local_write_component_data = write_component_data;
      job->kernel(read_component_data, local_write_component_data);

      std::cout << "  completing " << job << std::endl;

      data_mutex.lock();
      running_job_count--;
      auto available_component_data_changed = false;
      for (auto& read_component_name : job->read_component_names)
      {
        component_read_counts[read_component_name]--;
        if (!component_read_counts[read_component_name])
        {
          available_component_data_changed = true;
        }
      }
      for (auto& write_component_name : job->write_component_names)
      {
        component_write_counts[write_component_name]--;
        if (!component_write_counts[write_component_name])
        {
          available_component_data_changed = true;
        }
      }
      data_mutex.unlock();


      if (pending_jobs.empty() && !running_job_count)
      {
        running_semaphore.release();
      }
      else if (available_component_data_changed)
      {
        run_unblocked_jobs(container, running_semaphore, data_mutex, pending_jobs, running_job_count, component_read_counts, component_write_counts);
      }
    });
  }
}
