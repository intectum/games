/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "memory.h"

namespace ludo
{
  struct component
  {
    std::string name;
    uint32_t size = 0;
    bool vram = false;
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
    std::function<void(uint32_t entity_start, uint32_t entity_count, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)> kernel;
  };

  void init(container& container);

  template<typename component_1_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, uint32_t count = 1);

  template<typename component_1_t, typename component_2_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, uint32_t count = 1);

  template<typename component_1_t, typename component_2_t, typename component_3_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, uint32_t count = 1);

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, uint32_t count = 1);

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, uint32_t count = 1);

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, uint32_t count = 1);

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t, typename component_7_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, const component_7_t& component_7, uint32_t count = 1);

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t, typename component_7_t, typename component_8_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, const component_7_t& component_7, const component_8_t& component_8, uint32_t count = 1);

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t, typename component_7_t, typename component_8_t, typename component_9_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, const component_7_t& component_7, const component_8_t& component_8, const component_9_t& component_9, uint32_t count = 1);

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t, typename component_7_t, typename component_8_t, typename component_9_t, typename component_10_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, const component_7_t& component_7, const component_8_t& component_8, const component_9_t& component_9, const component_10_t& component_10, uint32_t count = 1);

  void run(std::vector<container>& containers, const std::vector<job>& jobs);
}

#include "ecs.hpp"
