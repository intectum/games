/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cstring>

namespace ludo
{
  template<typename component_t>
  void add_component(container& container, uint32_t archetype_index, uint32_t component_index, const component_t& component);

  template<typename component_1_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 1 && "incorrect number of components");

    add_component(container, archetype_iter - container.archetypes.begin(), 0, component_1);

    archetype_iter->count++;
  }

  template<typename component_1_t, typename component_2_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 2 && "incorrect number of components");

    add_component(container, archetype_iter - container.archetypes.begin(), 0, component_1);
    add_component(container, archetype_iter - container.archetypes.begin(), 1, component_2);

    archetype_iter->count++;
  }

  template<typename component_1_t, typename component_2_t, typename component_3_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 3 && "incorrect number of components");

    add_component(container, archetype_iter - container.archetypes.begin(), 0, component_1);
    add_component(container, archetype_iter - container.archetypes.begin(), 1, component_2);
    add_component(container, archetype_iter - container.archetypes.begin(), 2, component_3);

    archetype_iter->count++;
  }


  template<typename component_t>
  void add_component(container& container, uint32_t archetype_index, uint32_t component_index, const component_t& component)
  {
    auto component_iter = std::find_if(container.components.begin(), container.components.end(), [&](const ludo::component& component)
    {
      return component.name == container.archetypes[archetype_index].component_names[component_index];
    });

    assert(component_iter != container.components.end() && "component not found");
    assert(sizeof(component) == component_iter->size && "incorrect component size");

    auto data = allocate(container.archetypes[archetype_index].component_data[component_index], component_iter->size);
    std::memcpy(data, &component, component_iter->size);
  }
}
