/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <functional>
#include <vector>

#include "memory.h"

namespace ludo
{
  struct component
  {
    std::string name;
    uint32_t size = 0;
  };

  struct archetype
  {
    std::string name;
    std::vector<std::string> component_names;
    std::vector<arena> component_data;
    uint32_t capacity = 0;
    uint32_t count = 0;
  };

  struct container
  {
    std::vector<component> components;
    std::vector<archetype> archetypes;
  };

  struct job
  {
    std::vector<std::string> read_component_names;
    std::vector<std::string> write_component_names;
    std::function<void(const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)> kernel;
  };

  void init(container& container);

  template<typename component_1_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1);

  template<typename component_1_t, typename component_2_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2);

  template<typename component_1_t, typename component_2_t, typename component_3_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3);

  void run(container& container, const std::vector<job>& jobs);
}

#include "ecs.hpp"
