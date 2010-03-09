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

#ifndef GGADGET_CLUTTER_KEY_CONVERT_H__
#define GGADGET_CLUTTER_KEY_CONVERT_H__

#include <glib.h>

namespace ggadget {
namespace clutter {

/**
 * @ingroup GtkLibrary
 * @{
 */

/**
 * Convert a Clutter keyval to a key code accepted by @c ggadget::KeyboardEvent
 * whose type is @c ggadget::Event::EVENT_KEY_DOWN or
 * @c ggadget::Event::EVENT_KEY_UP).
 */
unsigned int ConvertClutterKeyvalToKeyCode(guint keyval);

/**
 * Convert a ClutterModifierType to a value understood by the modifier fields in
 * Event classes.
 */
int ConvertClutterModifierToModifier(guint state);

/**
 * Convert a ClutterModifierType to a value understood by the button fields in
 * Event classes.
 */
int ConvertClutterModifierToButton(guint state);

/** @} */

} // namespace clutter
} // namespace ggadget

#endif  // GGADGET_CLUTTER_KEY_CONVERT_H__
