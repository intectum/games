#include <btBulletDynamicsCommon.h>

#include "../ecs.h"
#include "../entities/spaceships.h"
#include "game.h"
#include "map.h"
#include "person.h"
#include "spaceship.h"

namespace astrum
{
  void perform_person_action(ludo::container& container, game_controls& game_controls);
  void perform_spaceship_action(ludo::container& container, game_controls& game_controls);

  void control_game(const ludo::instance& inst, std::vector<ludo::container>& containers, const ludo::window& window, game_controls& game_controls, map_controls& map_controls, ludo::vec3& camera_position, ludo::quat& camera_rotation)
  {
    auto action = window.keyboard_button_states.at(ludo::keyboard_button_f) == ludo::button_state_up;
    auto map = window.keyboard_button_states.at(ludo::keyboard_button_m) == ludo::button_state_up;

    if (map)
    {
      if (game_controls.mode == game_controls::mode::map)
      {
        std::swap(game_controls.mode, game_controls.previous_mode);
      }
      else
      {
        game_controls.previous_mode = game_controls.mode;
        game_controls.mode = game_controls::mode::map;
      }
    }

    if (game_controls.mode == game_controls::mode::person)
    {
      if (action)
      {
        perform_person_action(containers[0], game_controls);
      }

      control_person(inst, containers[0], window, game_controls.person_index, camera_position, camera_rotation);
      return;
    }

    if (game_controls.mode == game_controls::mode::spaceship)
    {
      if (action)
      {
        perform_spaceship_action(containers[0], game_controls);
      }

      control_spaceship(containers[0], window, game_controls.spaceship_index, camera_position, camera_rotation);
      return;
    }

    if (game_controls.mode == game_controls::mode::map)
    {
      control_map(inst, containers, window, map_controls, camera_position, camera_rotation);
    }
  }

  void perform_person_action(ludo::container& container, game_controls& game_controls)
  {
    auto person_kinematic_bodies = reinterpret_cast<btRigidBody*>(
      get_components(
        container,
        "person",
        "kinematic_body"
      )->data
    );

    auto spaceship_count = get_archetype(container, "spaceship")->count;
    auto spaceship_ghost_bodies = reinterpret_cast<btRigidBody*>(
      get_components(
        container,
        "spaceship",
        "ghost_body"
      )->data
    );

    auto& person_kinematic_body = person_kinematic_bodies[game_controls.person_index];

    // TODO enter spaceship
    /*for (auto spaceship_index = uint32_t(0); spaceship_index < spaceship_count; spaceship_index++)
    {
      for (auto& contact : ludo::contacts(physics_context, spaceship_ghost_bodies[spaceship_index]))
      {
        if (contact.body_b_id == person_kinematic_body)
        {
          enter_spaceship(container, game_controls.person_index, spaceship_index);

          game_controls.previous_mode = game_controls.mode;
          game_controls.mode = game_controls::mode::spaceship;
          game_controls.spaceship_index = spaceship_index;

          return;
        }
      }
    }*/
  }

  void perform_spaceship_action(ludo::container& container, game_controls& game_controls)
  {
    exit_spaceship(container, game_controls.person_index, game_controls.spaceship_index);

    game_controls.previous_mode = game_controls.mode;
    game_controls.mode = game_controls::mode::person;
  }
}
