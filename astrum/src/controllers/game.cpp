#include "../entities/spaceships.h"
#include "game.h"
#include "map.h"
#include "person.h"
#include "spaceship.h"
#include "types.h"

namespace astrum
{
  void perform_person_action(ludo::instance& inst, game_controls& game_controls);
  void perform_spaceship_action(ludo::instance& inst, game_controls& game_controls);

  void control_game(ludo::instance& inst)
  {
    auto& window = *ludo::first<ludo::window>(inst);
    auto& game_controls = *ludo::first<astrum::game_controls>(inst);

    auto action = window.active_keyboard_button_states[ludo::keyboard_button::F] == ludo::button_state::UP;
    auto map = window.active_keyboard_button_states[ludo::keyboard_button::M] == ludo::button_state::UP;

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
        perform_person_action(inst, game_controls);
      }

      control_person(inst, game_controls.person_index);
      return;
    }

    if (game_controls.mode == game_controls::mode::spaceship)
    {
      if (action)
      {
        perform_spaceship_action(inst, game_controls);
      }

      control_spaceship(inst, game_controls.spaceship_index);
      return;
    }

    if (game_controls.mode == game_controls::mode::map)
    {
      control_map(inst);
    }
  }

  void perform_person_action(ludo::instance& inst, game_controls& game_controls)
  {
    auto& person_kinematic_body = ludo::data<ludo::kinematic_body>(inst, "people")[game_controls.person_index];
    auto& spaceship_ghost_bodies = ludo::data<ludo::ghost_body>(inst, "spaceships");

    for (auto spaceship_index = uint32_t(0); spaceship_index < spaceship_ghost_bodies.length; spaceship_index++)
    {
      for (auto& contact : ludo::contacts(inst, spaceship_ghost_bodies[spaceship_index]))
      {
        if (contact.body_b == &person_kinematic_body)
        {
          enter_spaceship(inst, game_controls.person_index, spaceship_index);

          game_controls.previous_mode = game_controls.mode;
          game_controls.mode = game_controls::mode::spaceship;
          game_controls.spaceship_index = spaceship_index;

          return;
        }
      }
    }
  }

  void perform_spaceship_action(ludo::instance& inst, game_controls& game_controls)
  {
    exit_spaceship(inst, game_controls.person_index, game_controls.spaceship_index);

    game_controls.previous_mode = game_controls.mode;
    game_controls.mode = game_controls::mode::person;
  }
}
