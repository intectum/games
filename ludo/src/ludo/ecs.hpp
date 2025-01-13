/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cstring>

namespace ludo
{
  template<typename component_t>
  void add_component(container& container, archetype& archetype, uint32_t component_index, const component_t& component, uint32_t count);

  template<typename component_1_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->count + count <= archetype_iter->capacity && "not enough space");
    assert(archetype_iter->component_names.size() == 1 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);

    archetype_iter->count += count;
  }

  template<typename component_1_t, typename component_2_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->count + count <= archetype_iter->capacity && "not enough space");
    assert(archetype_iter->component_names.size() == 2 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);
    add_component(container, *archetype_iter, 1, component_2, count);

    archetype_iter->count += count;
  }

  template<typename component_1_t, typename component_2_t, typename component_3_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 3 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);
    add_component(container, *archetype_iter, 1, component_2, count);
    add_component(container, *archetype_iter, 2, component_3, count);

    archetype_iter->count += count;
  }

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 4 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);
    add_component(container, *archetype_iter, 1, component_2, count);
    add_component(container, *archetype_iter, 2, component_3, count);
    add_component(container, *archetype_iter, 3, component_4, count);

    archetype_iter->count += count;
  }

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 5 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);
    add_component(container, *archetype_iter, 1, component_2, count);
    add_component(container, *archetype_iter, 2, component_3, count);
    add_component(container, *archetype_iter, 3, component_4, count);
    add_component(container, *archetype_iter, 4, component_5, count);

    archetype_iter->count += count;
  }

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 6 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);
    add_component(container, *archetype_iter, 1, component_2, count);
    add_component(container, *archetype_iter, 2, component_3, count);
    add_component(container, *archetype_iter, 3, component_4, count);
    add_component(container, *archetype_iter, 4, component_5, count);
    add_component(container, *archetype_iter, 5, component_6, count);

    archetype_iter->count += count;
  }

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t, typename component_7_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, const component_7_t& component_7, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 7 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);
    add_component(container, *archetype_iter, 1, component_2, count);
    add_component(container, *archetype_iter, 2, component_3, count);
    add_component(container, *archetype_iter, 3, component_4, count);
    add_component(container, *archetype_iter, 4, component_5, count);
    add_component(container, *archetype_iter, 5, component_6, count);
    add_component(container, *archetype_iter, 6, component_7, count);

    archetype_iter->count += count;
  }

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t, typename component_7_t, typename component_8_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, const component_7_t& component_7, const component_8_t& component_8, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 8 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);
    add_component(container, *archetype_iter, 1, component_2, count);
    add_component(container, *archetype_iter, 2, component_3, count);
    add_component(container, *archetype_iter, 3, component_4, count);
    add_component(container, *archetype_iter, 4, component_5, count);
    add_component(container, *archetype_iter, 5, component_6, count);
    add_component(container, *archetype_iter, 6, component_7, count);
    add_component(container, *archetype_iter, 7, component_8, count);

    archetype_iter->count += count;
  }

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t, typename component_7_t, typename component_8_t, typename component_9_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, const component_7_t& component_7, const component_8_t& component_8, const component_9_t& component_9, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 9 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);
    add_component(container, *archetype_iter, 1, component_2, count);
    add_component(container, *archetype_iter, 2, component_3, count);
    add_component(container, *archetype_iter, 3, component_4, count);
    add_component(container, *archetype_iter, 4, component_5, count);
    add_component(container, *archetype_iter, 5, component_6, count);
    add_component(container, *archetype_iter, 6, component_7, count);
    add_component(container, *archetype_iter, 7, component_8, count);
    add_component(container, *archetype_iter, 8, component_9, count);

    archetype_iter->count += count;
  }

  template<typename component_1_t, typename component_2_t, typename component_3_t, typename component_4_t, typename component_5_t, typename component_6_t, typename component_7_t, typename component_8_t, typename component_9_t, typename component_10_t>
  void add(container& container, const std::string& archetype_name, const component_1_t& component_1, const component_2_t& component_2, const component_3_t& component_3, const component_4_t& component_4, const component_5_t& component_5, const component_6_t& component_6, const component_7_t& component_7, const component_8_t& component_8, const component_9_t& component_9, const component_10_t& component_10, uint32_t count)
  {
    auto archetype_iter = std::find_if(container.archetypes.begin(), container.archetypes.end(), [&](const archetype& archetype)
    {
      return archetype.name == archetype_name;
    });

    assert(archetype_iter != container.archetypes.end() && "archetype not found");
    assert(archetype_iter->component_names.size() == 10 && "incorrect number of components");

    add_component(container, *archetype_iter, 0, component_1, count);
    add_component(container, *archetype_iter, 1, component_2, count);
    add_component(container, *archetype_iter, 2, component_3, count);
    add_component(container, *archetype_iter, 3, component_4, count);
    add_component(container, *archetype_iter, 4, component_5, count);
    add_component(container, *archetype_iter, 5, component_6, count);
    add_component(container, *archetype_iter, 6, component_7, count);
    add_component(container, *archetype_iter, 7, component_8, count);
    add_component(container, *archetype_iter, 8, component_9, count);
    add_component(container, *archetype_iter, 9, component_10, count);

    archetype_iter->count += count;
  }

  template<typename component_t>
  void add_component(container& container, archetype& archetype, uint32_t component_index, const component_t& component, uint32_t count)
  {
    auto component_iter = std::find_if(container.components.begin(), container.components.end(), [&](const ludo::component& component)
    {
      return component.name == archetype.component_names[component_index];
    });

    assert(component_iter != container.components.end() && "component not found");
    assert(sizeof(component) == component_iter->size && "incorrect component size");

    auto data = allocate(archetype.component_data[component_index], count * component_iter->size);
    for (auto index = 0; index < count; index++)
    {
      std::memcpy(data + index * component_iter->size, &component, component_iter->size);
    }
  }
}
