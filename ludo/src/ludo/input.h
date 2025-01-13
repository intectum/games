/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

namespace ludo
{
  ///
  /// The different states a physical button can be in
  enum button_state
  {
    button_state_none, ///< The button is inactive
    button_state_down, ///< The button was just pressed
    button_state_hold, ///< The button is being held down
    button_state_up ///< The button was just released
  };

  ///
  /// The buttons on a keyboard
  enum keyboard_button
  {
    keyboard_button_zero,
    keyboard_button_one,
    keyboard_button_two,
    keyboard_button_three,
    keyboard_button_four,
    keyboard_button_five,
    keyboard_button_six,
    keyboard_button_seven,
    keyboard_button_eight,
    keyboard_button_nine,

    keyboard_button_a,
    keyboard_button_b,
    keyboard_button_c,
    keyboard_button_d,
    keyboard_button_e,
    keyboard_button_f,
    keyboard_button_g,
    keyboard_button_h,
    keyboard_button_i,
    keyboard_button_j,
    keyboard_button_k,
    keyboard_button_l,
    keyboard_button_m,
    keyboard_button_n,
    keyboard_button_o,
    keyboard_button_p,
    keyboard_button_q,
    keyboard_button_r,
    keyboard_button_s,

    keyboard_button_t,
    keyboard_button_u,
    keyboard_button_v,
    keyboard_button_w,
    keyboard_button_x,
    keyboard_button_y,
    keyboard_button_z,

    keyboard_button_f1,
    keyboard_button_f2,
    keyboard_button_f3,
    keyboard_button_f4,
    keyboard_button_f5,
    keyboard_button_f6,
    keyboard_button_f7,
    keyboard_button_f8,
    keyboard_button_f9,
    keyboard_button_f10,
    keyboard_button_f11,
    keyboard_button_f12,

    keyboard_button_apostrophe,
    keyboard_button_arrow_down,
    keyboard_button_arrow_left,
    keyboard_button_arrow_right,
    keyboard_button_arrow_up,
    keyboard_button_asterisk,
    keyboard_button_at,
    keyboard_button_backslash,
    keyboard_button_backspace,
    keyboard_button_colon,
    keyboard_button_comma,
    keyboard_button_del,
    keyboard_button_end,
    keyboard_button_enter,
    keyboard_button_escape,
    keyboard_button_equals,
    keyboard_button_grave,
    keyboard_button_home,
    keyboard_button_insert,
    keyboard_button_left_alt,
    keyboard_button_left_bracket,
    keyboard_button_left_ctrl,
    keyboard_button_left_shift,
    keyboard_button_minus,
    keyboard_button_page_down,
    keyboard_button_page_up,
    keyboard_button_period,
    keyboard_button_right_alt,
    keyboard_button_right_bracket,
    keyboard_button_right_ctrl,
    keyboard_button_right_shift,
    keyboard_button_semicolon,
    keyboard_button_slash,
    keyboard_button_space,
    keyboard_button_tab,
    keyboard_button_underline,
    keyboard_button_count
  };

  ///
  /// The buttons on a mouse
  enum mouse_button
  {
    mouse_button_middle,
    mouse_button_left,
    mouse_button_right,
    mouse_button_count
  };
}
