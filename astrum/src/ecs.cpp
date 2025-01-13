#include "ecs.h"

namespace astrum
{
  ludo::archetype* get_archetype(ludo::container& container, const std::string& archetype_name)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const ludo::archetype& archetype)
    {
      return archetype.name == archetype_name;
    });
    if (archetype_iter == container.archetypes.end()) return nullptr;

    return &*archetype_iter;
  }

  const ludo::archetype* get_archetype(const ludo::container& container, const std::string& archetype_name)
  {
    return get_archetype(const_cast<ludo::container&>(container), archetype_name);
  }

  ludo::arena* get_components(ludo::container& container, const std::string& archetype_name, const std::string& component_name)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const ludo::archetype& archetype)
    {
      return archetype.name == archetype_name;
    });
    if (archetype_iter == container.archetypes.end()) return nullptr;

    auto component_iter = std::find(archetype_iter->component_names.begin(), archetype_iter->component_names.end(), component_name);
    if (component_iter == archetype_iter->component_names.end()) return nullptr;

    auto component_index = &*component_iter - &archetype_iter->component_names.front();
    return &archetype_iter->component_data[component_index];
  }

  const ludo::arena* get_components(const ludo::container& container, const std::string& archetype_name, const std::string& component_name)
  {
    return get_components(const_cast<ludo::container&>(container), archetype_name, component_name);
  }

  uint32_t count_components(ludo::container& container, const std::string& component_name)
  {
    auto count = uint32_t(0);

    for (auto& archetype : container.archetypes)
    {
      auto component_iter = std::find(archetype.component_names.begin(), archetype.component_names.end(), component_name);
      if (component_iter == archetype.component_names.end()) continue;

      count += archetype.count;
    }

    return count;
  }

  uint32_t count_components(ludo::container& container, const std::string& archetype_name, const std::string& component_name)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const ludo::archetype& archetype)
    {
      return archetype.name == archetype_name;
    });
    if (archetype_iter == container.archetypes.end()) return 0;

    auto component_iter = std::find(archetype_iter->component_names.begin(), archetype_iter->component_names.end(), component_name);
    if (component_iter == archetype_iter->component_names.end()) return 0;

    return archetype_iter->count;
  }
}
