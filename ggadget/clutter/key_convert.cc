/*
  Copyright 2008 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

/*
  Portions:
  Copyright 2008-2010 Intel Corp.

  Authors:
  Iain Holmes <iain@linux.intel.com>
  Roger WANG <roger.wang@intel.com>
*/

#include <algorithm>
#include <clutter/clutter.h>
#include <ggadget/event.h>
#include "key_convert.h"

using ggadget::KeyboardEvent;

namespace ggadget {
namespace clutter {

struct KeyvalKeyCode {
  guint clutter_keyval;
  unsigned int key_code;
};

static KeyvalKeyCode keyval_key_code_map[] = {
  { CLUTTER_Cancel,       KeyboardEvent::KEY_CANCEL },
  { CLUTTER_BackSpace,    KeyboardEvent::KEY_BACK },
  { CLUTTER_Tab,          KeyboardEvent::KEY_TAB },
  { CLUTTER_KP_Tab,       KeyboardEvent::KEY_TAB },
  { CLUTTER_ISO_Left_Tab, KeyboardEvent::KEY_TAB },  // Shift-TAB.
  { CLUTTER_Clear,        KeyboardEvent::KEY_CLEAR },
  { CLUTTER_Return,       KeyboardEvent::KEY_RETURN },
  { CLUTTER_KP_Enter,     KeyboardEvent::KEY_RETURN },
  { CLUTTER_Shift_L,      KeyboardEvent::KEY_SHIFT },
  { CLUTTER_Shift_R,      KeyboardEvent::KEY_SHIFT },
  { CLUTTER_Control_L,    KeyboardEvent::KEY_CONTROL },
  { CLUTTER_Control_R,    KeyboardEvent::KEY_CONTROL },
  { CLUTTER_Alt_L,        KeyboardEvent::KEY_ALT },
  { CLUTTER_Alt_R,        KeyboardEvent::KEY_ALT },
  { CLUTTER_Pause,        KeyboardEvent::KEY_PAUSE },
  { CLUTTER_Caps_Lock,    KeyboardEvent::KEY_CAPITAL },
  { CLUTTER_Escape,       KeyboardEvent::KEY_ESCAPE },
  { CLUTTER_space,        KeyboardEvent::KEY_SPACE },
  { CLUTTER_KP_Space,     KeyboardEvent::KEY_SPACE },
  { CLUTTER_Page_Up,      KeyboardEvent::KEY_PAGE_UP },
  { CLUTTER_KP_Page_Up,   KeyboardEvent::KEY_PAGE_UP },
  { CLUTTER_Page_Down,    KeyboardEvent::KEY_PAGE_DOWN },
  { CLUTTER_KP_Page_Down, KeyboardEvent::KEY_PAGE_DOWN },
  { CLUTTER_End,          KeyboardEvent::KEY_END },
  { CLUTTER_KP_End,       KeyboardEvent::KEY_END },
  { CLUTTER_Home,         KeyboardEvent::KEY_HOME },
  { CLUTTER_KP_Home,      KeyboardEvent::KEY_HOME },
  { CLUTTER_Left,         KeyboardEvent::KEY_LEFT },
  { CLUTTER_KP_Left,      KeyboardEvent::KEY_LEFT },
  { CLUTTER_Up,           KeyboardEvent::KEY_UP },
  { CLUTTER_KP_Up,        KeyboardEvent::KEY_UP },
  { CLUTTER_Right,        KeyboardEvent::KEY_RIGHT },
  { CLUTTER_KP_Right,     KeyboardEvent::KEY_RIGHT },
  { CLUTTER_Down,         KeyboardEvent::KEY_DOWN },
  { CLUTTER_KP_Down,      KeyboardEvent::KEY_DOWN },
  { CLUTTER_Select,       KeyboardEvent::KEY_SELECT },
  { CLUTTER_Print,        KeyboardEvent::KEY_PRINT },
  { CLUTTER_Execute,      KeyboardEvent::KEY_EXECUTE },
  { CLUTTER_Insert,       KeyboardEvent::KEY_INSERT },
  { CLUTTER_KP_Insert,    KeyboardEvent::KEY_INSERT },
  { CLUTTER_Delete,       KeyboardEvent::KEY_DELETE },
  { CLUTTER_KP_Delete,    KeyboardEvent::KEY_DELETE },
  { CLUTTER_Help,         KeyboardEvent::KEY_HELP },
  { CLUTTER_Menu,         KeyboardEvent::KEY_CONTEXT_MENU },
  { CLUTTER_exclam,       '1' },
  { CLUTTER_at,           '2' },
  { CLUTTER_numbersign,   '3' },
  { CLUTTER_dollar,       '4' },
  { CLUTTER_percent,      '5' },
  { CLUTTER_asciicircum,  '6' },
  { CLUTTER_ampersand,    '7' },
  { CLUTTER_asterisk,     '8' },
  { CLUTTER_parenleft,    '9' },
  { CLUTTER_parenright,   '0' },
  { CLUTTER_colon,        KeyboardEvent::KEY_COLON },
  { CLUTTER_semicolon,    KeyboardEvent::KEY_COLON },
  { CLUTTER_plus,         KeyboardEvent::KEY_PLUS },
  { CLUTTER_equal,        KeyboardEvent::KEY_PLUS },
  { CLUTTER_comma,        KeyboardEvent::KEY_COMMA },
  { CLUTTER_less,         KeyboardEvent::KEY_COMMA },
  { CLUTTER_minus,        KeyboardEvent::KEY_MINUS },
  { CLUTTER_underscore,   KeyboardEvent::KEY_MINUS },
  { CLUTTER_period,       KeyboardEvent::KEY_PERIOD },
  { CLUTTER_greater,      KeyboardEvent::KEY_PERIOD },
  { CLUTTER_slash,        KeyboardEvent::KEY_SLASH },
  { CLUTTER_question,     KeyboardEvent::KEY_SLASH },
  { CLUTTER_grave,        KeyboardEvent::KEY_GRAVE },
  { CLUTTER_asciitilde,   KeyboardEvent::KEY_GRAVE },
  { CLUTTER_bracketleft,  KeyboardEvent::KEY_BRACKET_LEFT },
  { CLUTTER_braceleft,    KeyboardEvent::KEY_BRACKET_LEFT },
  { CLUTTER_backslash,    KeyboardEvent::KEY_BACK_SLASH },
  { CLUTTER_bar,          KeyboardEvent::KEY_BACK_SLASH },
  { CLUTTER_bracketright, KeyboardEvent::KEY_BRACKET_RIGHT },
  { CLUTTER_braceright,   KeyboardEvent::KEY_BRACKET_RIGHT },
  { CLUTTER_quotedbl,     KeyboardEvent::KEY_QUOTE },
  { CLUTTER_apostrophe,   KeyboardEvent::KEY_QUOTE },
  { CLUTTER_0,            '0' },
  { CLUTTER_1,            '1' },
  { CLUTTER_2,            '2' },
  { CLUTTER_3,            '3' },
  { CLUTTER_4,            '4' },
  { CLUTTER_5,            '5' },
  { CLUTTER_6,            '6' },
  { CLUTTER_7,            '7' },
  { CLUTTER_8,            '8' },
  { CLUTTER_9,            '9' },
  { CLUTTER_0,            '0' },
  { CLUTTER_A,            'A' },
  { CLUTTER_B,            'B' },
  { CLUTTER_C,            'C' },
  { CLUTTER_D,            'D' },
  { CLUTTER_E,            'E' },
  { CLUTTER_F,            'F' },
  { CLUTTER_G,            'G' },
  { CLUTTER_H,            'H' },
  { CLUTTER_I,            'I' },
  { CLUTTER_J,            'J' },
  { CLUTTER_K,            'K' },
  { CLUTTER_L,            'L' },
  { CLUTTER_M,            'M' },
  { CLUTTER_N,            'N' },
  { CLUTTER_O,            'O' },
  { CLUTTER_P,            'P' },
  { CLUTTER_Q,            'Q' },
  { CLUTTER_R,            'R' },
  { CLUTTER_S,            'S' },
  { CLUTTER_T,            'T' },
  { CLUTTER_U,            'U' },
  { CLUTTER_V,            'V' },
  { CLUTTER_W,            'W' },
  { CLUTTER_X,            'X' },
  { CLUTTER_Y,            'Y' },
  { CLUTTER_Z,            'Z' },
  { CLUTTER_a,            'A' },
  { CLUTTER_b,            'B' },
  { CLUTTER_c,            'C' },
  { CLUTTER_d,            'D' },
  { CLUTTER_e,            'E' },
  { CLUTTER_f,            'F' },
  { CLUTTER_g,            'G' },
  { CLUTTER_h,            'H' },
  { CLUTTER_i,            'I' },
  { CLUTTER_j,            'J' },
  { CLUTTER_k,            'K' },
  { CLUTTER_l,            'L' },
  { CLUTTER_m,            'M' },
  { CLUTTER_n,            'N' },
  { CLUTTER_o,            'O' },
  { CLUTTER_p,            'P' },
  { CLUTTER_q,            'Q' },
  { CLUTTER_r,            'R' },
  { CLUTTER_s,            'S' },
  { CLUTTER_t,            'T' },
  { CLUTTER_u,            'U' },
  { CLUTTER_v,            'V' },
  { CLUTTER_w,            'W' },
  { CLUTTER_x,            'X' },
  { CLUTTER_y,            'Y' },
  { CLUTTER_z,            'Z' },
  { CLUTTER_KP_0,         KeyboardEvent::KEY_NUMPAD0 },
  { CLUTTER_KP_1,         KeyboardEvent::KEY_NUMPAD1 },
  { CLUTTER_KP_2,         KeyboardEvent::KEY_NUMPAD2 },
  { CLUTTER_KP_3,         KeyboardEvent::KEY_NUMPAD3 },
  { CLUTTER_KP_4,         KeyboardEvent::KEY_NUMPAD4 },
  { CLUTTER_KP_5,         KeyboardEvent::KEY_NUMPAD5 },
  { CLUTTER_KP_6,         KeyboardEvent::KEY_NUMPAD6 },
  { CLUTTER_KP_7,         KeyboardEvent::KEY_NUMPAD7 },
  { CLUTTER_KP_8,         KeyboardEvent::KEY_NUMPAD8 },
  { CLUTTER_KP_9,         KeyboardEvent::KEY_NUMPAD9 },
  { CLUTTER_KP_Multiply,  KeyboardEvent::KEY_MULTIPLY },
  { CLUTTER_KP_Add,       KeyboardEvent::KEY_ADD },
  { CLUTTER_KP_Separator, KeyboardEvent::KEY_SEPARATOR },
  { CLUTTER_KP_Subtract,  KeyboardEvent::KEY_SUBTRACT },
  { CLUTTER_KP_Decimal,   KeyboardEvent::KEY_DECIMAL },
  { CLUTTER_KP_Divide,    KeyboardEvent::KEY_DIVIDE },
  { CLUTTER_F1,           KeyboardEvent::KEY_F1 },
  { CLUTTER_KP_F1,        KeyboardEvent::KEY_F1 },
  { CLUTTER_F2,           KeyboardEvent::KEY_F2 },
  { CLUTTER_KP_F2,        KeyboardEvent::KEY_F2 },
  { CLUTTER_F3,           KeyboardEvent::KEY_F3 },
  { CLUTTER_KP_F3,        KeyboardEvent::KEY_F3 },
  { CLUTTER_F4,           KeyboardEvent::KEY_F4 },
  { CLUTTER_KP_F4,        KeyboardEvent::KEY_F4 },
  { CLUTTER_F5,           KeyboardEvent::KEY_F5 },
  { CLUTTER_F6,           KeyboardEvent::KEY_F6 },
  { CLUTTER_F7,           KeyboardEvent::KEY_F7 },
  { CLUTTER_F8,           KeyboardEvent::KEY_F8 },
  { CLUTTER_F9,           KeyboardEvent::KEY_F9 },
  { CLUTTER_F10,          KeyboardEvent::KEY_F10 },
  { CLUTTER_F11,          KeyboardEvent::KEY_F11 },
  { CLUTTER_F12,          KeyboardEvent::KEY_F12 },
  { CLUTTER_F13,          KeyboardEvent::KEY_F13 },
  { CLUTTER_F14,          KeyboardEvent::KEY_F14 },
  { CLUTTER_F15,          KeyboardEvent::KEY_F15 },
  { CLUTTER_F16,          KeyboardEvent::KEY_F16 },
  { CLUTTER_F17,          KeyboardEvent::KEY_F17 },
  { CLUTTER_F18,          KeyboardEvent::KEY_F18 },
  { CLUTTER_F19,          KeyboardEvent::KEY_F19 },
  { CLUTTER_F20,          KeyboardEvent::KEY_F20 },
  { CLUTTER_F21,          KeyboardEvent::KEY_F21 },
  { CLUTTER_F22,          KeyboardEvent::KEY_F22 },
  { CLUTTER_F23,          KeyboardEvent::KEY_F23 },
  { CLUTTER_F24,          KeyboardEvent::KEY_F24 },
  { CLUTTER_Num_Lock,     KeyboardEvent::KEY_NUMLOCK },
  { CLUTTER_Scroll_Lock,  KeyboardEvent::KEY_SCROLL },
  { 0, 0 }, // Guard entry for checking the sorted state of this table.
};

static inline bool KeyvalCompare(const KeyvalKeyCode &v1,
                                 const KeyvalKeyCode &v2) {
  return v1.clutter_keyval < v2.clutter_keyval;
}

unsigned int ConvertClutterKeyvalToKeyCode(guint keyval) {
  if (keyval_key_code_map[0].clutter_keyval != 0) {
    std::sort(keyval_key_code_map,
              keyval_key_code_map + arraysize(keyval_key_code_map),
              KeyvalCompare);
  }

  ASSERT(keyval_key_code_map[0].clutter_keyval == 0);
  KeyvalKeyCode key = { keyval, 0 };
  const KeyvalKeyCode *pos = std::lower_bound(
      keyval_key_code_map, keyval_key_code_map + arraysize(keyval_key_code_map),
      key, KeyvalCompare);
  ASSERT(pos);
  return pos->clutter_keyval == keyval ? pos->key_code : 0;
}

int ConvertClutterModifierToModifier(guint state) {
  int mod = Event::MOD_NONE;
  if (state & CLUTTER_SHIFT_MASK) {
    mod |= Event::MOD_SHIFT;
  }
  if (state & CLUTTER_CONTROL_MASK) {
    mod |= Event::MOD_CONTROL;
  }
  if (state & CLUTTER_MOD1_MASK) {
    mod |= Event::MOD_ALT;
  }
  return mod;
}

int ConvertClutterModifierToButton(guint state) {
  int button = MouseEvent::BUTTON_NONE;
  if (state & CLUTTER_BUTTON1_MASK) {
    button |= MouseEvent::BUTTON_LEFT;
  }
  if (state & CLUTTER_BUTTON2_MASK) {
    button |= MouseEvent::BUTTON_MIDDLE;
  }
  if (state & CLUTTER_BUTTON3_MASK) {
    button |= MouseEvent::BUTTON_RIGHT;
  }
  return button;
}

} // namespace clutter
} // namespace ggadget
