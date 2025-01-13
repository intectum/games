#pragma once

#include <ludo/api.h>

namespace astrum
{
  ludo::archetype* get_archetype(ludo::container& container, const std::string& archetype_name);
  const ludo::archetype* get_archetype(const ludo::container& container, const std::string& archetype_name);

  ludo::arena* get_components(ludo::container& container, const std::string& archetype_name, const std::string& component_name);
  const ludo::arena* get_components(const ludo::container& container, const std::string& archetype_name, const std::string& component_name);

  uint32_t count_components(ludo::container& container, const std::string& component_name);

  uint32_t count_components(ludo::container& container, const std::string& archetype_name, const std::string& component_name);

  template<typename T>
  T* find_component(ludo::container& container, const std::string& component_name, uint64_t id);
  template<typename T>
  const T* find_component(const ludo::container& container, const std::string& component_name, uint64_t id);
}

#include "ecs.hpp"
