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
  Copyright 2008-2010 Intel Corp

  Authors:
  Iain Holmes <iain@linux.intel.com>
  Roger WANG <roger.wang@intel.com>
*/

#ifndef GGADGET_CLUTTER_UTILITIES_H__
#define GGADGET_CLUTTER_UTILITIES_H__

#include <string>
#include <clutter/clutter.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <ggadget/view_interface.h>
#include <ggadget/view_host_interface.h>
#include <ggadget/slot.h>

namespace ggadget {

class Gadget;

namespace clutter {

/**
 * @ingroup GtkLibrary
 * @{
 */

/**
 * Displays a message box containing the message string.
 *
 * @param title tile of the alert window.
 * @param message the alert message.
 */
void ShowAlertDialog(const char *title, const char *message);

/**
 * Displays a dialog containing the message string and Yes and No buttons.
 * @param title tile of the dialog.
 * @param message the message string.
 * @param cancel_button indicates whether to show the Cancel button.
 * @return which button is pressed. Note: if @a cancel_button is false,
 *     this function returns 0 if the user closes the dialog without pressing
 *     Yes or No button, to keep backward compatibility.
 */
ViewHostInterface::ConfirmResponse ShowConfirmDialog(const char *title,
                                                     const char *message,
                                                     bool cancel_button);

/**
 * Displays a dialog asking the user to enter text.
 * @param title tile of the dialog.
 * @param message the message string displayed before the edit box.
 * @param default_value the initial default value dispalyed in the edit box.
 * @return the user inputted text, or an empty string if user canceled the
 *     dialog.
 */
std::string ShowPromptDialog(const char *title, const char *message,
                             const char *default_value);

/** Load a given font into the application. */
bool LoadFont(const char *filename);

/**
 * Loads a GdkPixbuf object from raw image data.
 * @param data A string object containing the raw image data.
 * @return NULL on failure, a GdkPixbuf object otherwise.
 */
GdkPixbuf *LoadPixbufFromData(const std::string &data);

/**
 * Launches a desktop file.
 *
 * @param gadget The gadget which wants to launch the desktop file.
 * @param desktop_file The desktop file to be launched.
 * @return true if succeed.
 */
bool LaunchDesktopFile(const Gadget *gadget, const char *desktop_file);

/**
 * Opens a specified URL by system default application.
 *
 * Comparing to ggadget::xdg::OpenURL(), this function supports launching a
 * desktop file.
 *
 * @param gadget The gadget which wants to open the url, the permissions of
 *        this gadget will be checked to see if opening the url is allowed.
 *        If it's NULL, then only urls with http:// or https:// prefixes can
 *        be opened.
 * @param url The url to open.
 * @return true if succeed.
 */
bool OpenURL(const Gadget *gadget, const char *url);

void SetClutterDefaultGroup(ClutterActor *default_group);
ClutterActor *GetClutterDefaultGroup();
/** Gets current time in milliseconds. */
uint64_t GetCurrentTime();

/** @} */

} // namespace clutter
} // namespace ggadget

#endif // GGADGET_GTK_UTILITIES_H__
