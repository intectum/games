#include "ecs.h"

namespace astrum
{
  template<typename T>
  T* find_component(ludo::container& container, const std::string& component_name, uint64_t id)
  {
    for (auto& archetype : container.archetypes)
    {
      auto component_name_iter = std::find(archetype.component_names.begin(), archetype.component_names.end(), component_name);
      if (component_name_iter != archetype.component_names.end())
      {
        auto component_index = component_name_iter - archetype.component_names.begin();
        auto component_start = static_cast<T*>(archetype.component_data[component_index].data);

        for (auto index = 0; index < archetype.count; index++)
        {
          if (component_start[index].id == id) return &component_start[index];
        }
      }
    }

    return nullptr;
  }

  template<typename T>
  const T* find_component(const ludo::container& container, const std::string& component_name, uint64_t id)
  {
    return find_component<T>(const_cast<ludo::container&>(container), component_name, id);
  }
}
