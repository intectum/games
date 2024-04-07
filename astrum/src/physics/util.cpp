#include <cmath>

#include "../constants.h"
#include "util.h"

namespace astrum
{
  std::vector<ludo::contact> deepest_contacts(const std::vector<ludo::contact>& contacts)
  {
    auto deepest_contacts = std::vector<ludo::contact>();
    for (auto& contact : contacts)
    {
      auto deepest_contact_iter = std::find_if(deepest_contacts.begin(), deepest_contacts.end(), [&contact](const ludo::contact& deepest_contact)
      {
        return deepest_contact.body_b == contact.body_b;
      });

      if (deepest_contact_iter == deepest_contacts.end())
      {
        deepest_contacts.emplace_back(contact);
      }
      else if (contact.distance < (*deepest_contact_iter).distance)
      {
        *deepest_contact_iter = contact;
      }
    }

    return deepest_contacts;
  }

  float orbital_speed(float orbit_radius, float mass_of_larger_body)
  {
    return std::sqrt(gravitational_constant * mass_of_larger_body / orbit_radius);
  }
}
