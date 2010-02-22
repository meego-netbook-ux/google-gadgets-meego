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

#include "utilities.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <string>
#include <fontconfig/fontconfig.h>
#include <clutter/clutter.h>

#include <ggadget/common.h>
#include <ggadget/logger.h>
#include <ggadget/gadget.h>
#include <ggadget/gadget_consts.h>
#include <ggadget/messages.h>
#include <ggadget/permissions.h>
#include <ggadget/string_utils.h>
#include <ggadget/system_utils.h>
#include <ggadget/file_manager_interface.h>
#include <ggadget/file_manager_factory.h>
#include <ggadget/options_interface.h>
#include <ggadget/view.h>
#include <ggadget/view_interface.h>
#include <ggadget/xdg/desktop_entry.h>
#include <ggadget/xdg/utilities.h>

namespace ggadget {
namespace clutter {

void ShowAlertDialog(const char *title, const char *message) {
#if 0
  GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                             GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_INFO,
                                             GTK_BUTTONS_OK,
                                             "%s", message);
  GdkScreen *screen;
  gdk_display_get_pointer(gdk_display_get_default(), &screen, NULL, NULL, NULL);
  gtk_window_set_screen(GTK_WINDOW(dialog), screen);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(dialog), title);
  SetGadgetWindowIcon(GTK_WINDOW(dialog), NULL);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
#endif
}

ViewHostInterface::ConfirmResponse ShowConfirmDialog(const char *title,
                                                     const char *message,
                                                     bool cancel_button) {
#if 0
  GtkWidget *dialog;
  if (cancel_button) {
    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                    "%s", message);
    gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_NO, GTK_RESPONSE_NO,
                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                           GTK_STOCK_YES, GTK_RESPONSE_YES, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);
  } else {
    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
                                    "%s", message);
  }
  GdkScreen *screen;
  gdk_display_get_pointer(gdk_display_get_default(), &screen, NULL, NULL, NULL);
  gtk_window_set_screen(GTK_WINDOW(dialog), screen);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(dialog), title);
  SetGadgetWindowIcon(GTK_WINDOW(dialog), NULL);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  return result == GTK_RESPONSE_YES ? ViewHostInterface::CONFIRM_YES :
         result == GTK_RESPONSE_NO ? ViewHostInterface::CONFIRM_NO :
         cancel_button ? ViewHostInterface::CONFIRM_CANCEL :
                         ViewHostInterface::CONFIRM_NO;
#endif
  return ViewHostInterface::CONFIRM_YES;
}

std::string ShowPromptDialog(const char *title, const char *message,
                             const char *default_value) {
#if 0
  GtkWidget *dialog = gtk_dialog_new_with_buttons(
      title, NULL,
      static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR),
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OK, GTK_RESPONSE_OK,
      NULL);
  GdkScreen *screen;
  gdk_display_get_pointer(gdk_display_get_default(), &screen, NULL, NULL, NULL);
  gtk_window_set_screen(GTK_WINDOW(dialog), screen);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), TRUE);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  SetGadgetWindowIcon(GTK_WINDOW(dialog), NULL);

  GtkWidget *image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION,
                                              GTK_ICON_SIZE_DIALOG);
  GtkWidget *label = gtk_label_new(message);
  gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
  gtk_label_set_selectable(GTK_LABEL(label), TRUE);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 1.0);
  GtkWidget *entry = gtk_entry_new();
  if (default_value)
    gtk_entry_set_text(GTK_ENTRY(entry), default_value);
  gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

  GtkWidget *hbox = gtk_hbox_new(FALSE, 12);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 12);
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0);

  gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
  gtk_container_set_border_width(
      GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), 10);

  gtk_widget_show_all(dialog);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  std::string text;
  if (result == GTK_RESPONSE_OK)
    text = gtk_entry_get_text(GTK_ENTRY(entry));
  gtk_widget_destroy(dialog);
  return text;
#else
  return "FIXME!";
#endif
}

bool LoadFont(const char *filename) {
  FcConfig *config = FcConfigGetCurrent();
  bool success = FcConfigAppFontAddFile(config,
                   reinterpret_cast<const FcChar8 *>(filename));
  DLOG("LoadFont: %s %s", filename, success ? "success" : "fail");
  return success;
}

GdkPixbuf *LoadPixbufFromData(const std::string &data) {
  GdkPixbuf *pixbuf = NULL;
  GdkPixbufLoader *loader = NULL;
  GError *error = NULL;

  loader = gdk_pixbuf_loader_new();

  const guchar *ptr = reinterpret_cast<const guchar *>(data.c_str());
  if (gdk_pixbuf_loader_write(loader, ptr, data.size(), &error) &&
      gdk_pixbuf_loader_close(loader, &error)) {
    pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
    if (pixbuf) g_object_ref(pixbuf);
  }

  if (error) g_error_free(error);
  if (loader) g_object_unref(loader);

  return pixbuf;
}

#if 0
struct CursorTypeMapping {
  int type;
  GdkCursorType gdk_type;
};

// Ordering in this array must match the declaration in
// ViewInterface::CursorType.
static const CursorTypeMapping kCursorTypeMappings[] = {
  { ViewInterface::CURSOR_ARROW, GDK_LEFT_PTR}, // FIXME
  { ViewInterface::CURSOR_IBEAM, GDK_XTERM },
  { ViewInterface::CURSOR_WAIT, GDK_WATCH },
  { ViewInterface::CURSOR_CROSS, GDK_CROSS },
  { ViewInterface::CURSOR_UPARROW, GDK_CENTER_PTR },
  { ViewInterface::CURSOR_SIZE, GDK_SIZING },
  { ViewInterface::CURSOR_SIZENWSE, GDK_SIZING }, // FIXME
  { ViewInterface::CURSOR_SIZENESW, GDK_SIZING }, // FIXME
  { ViewInterface::CURSOR_SIZEWE, GDK_SB_H_DOUBLE_ARROW }, // FIXME
  { ViewInterface::CURSOR_SIZENS, GDK_SB_V_DOUBLE_ARROW }, // FIXME
  { ViewInterface::CURSOR_SIZEALL, GDK_FLEUR }, // FIXME
  { ViewInterface::CURSOR_NO, GDK_X_CURSOR },
  { ViewInterface::CURSOR_HAND, GDK_HAND2 },
  { ViewInterface::CURSOR_BUSY, GDK_WATCH }, // FIXME
  { ViewInterface::CURSOR_HELP, GDK_QUESTION_ARROW }
};

struct HitTestCursorTypeMapping {
  ViewInterface::HitTest hittest;
  GdkCursorType gdk_type;
};

static const HitTestCursorTypeMapping kHitTestCursorTypeMappings[] = {
  { ViewInterface::HT_LEFT, GDK_LEFT_SIDE },
  { ViewInterface::HT_RIGHT, GDK_RIGHT_SIDE },
  { ViewInterface::HT_TOP, GDK_TOP_SIDE },
  { ViewInterface::HT_BOTTOM, GDK_BOTTOM_SIDE },
  { ViewInterface::HT_TOPLEFT, GDK_TOP_LEFT_CORNER },
  { ViewInterface::HT_TOPRIGHT, GDK_TOP_RIGHT_CORNER },
  { ViewInterface::HT_BOTTOMLEFT, GDK_BOTTOM_LEFT_CORNER },
  { ViewInterface::HT_BOTTOMRIGHT, GDK_BOTTOM_RIGHT_CORNER }
};

GdkCursor *CreateCursor(int type, ViewInterface::HitTest hittest) {
  GdkCursorType gdk_type = GDK_ARROW;
  // Try match with hittest first.
  for (size_t i = 0; i < arraysize(kHitTestCursorTypeMappings); ++i) {
    if (kHitTestCursorTypeMappings[i].hittest == hittest) {
      gdk_type = kHitTestCursorTypeMappings[i].gdk_type;
      break;
    }
  }

  // No suitable mapping, try matching with cursor type.
  if (gdk_type == GDK_ARROW) {
    for (size_t i = 0; i < arraysize(kCursorTypeMappings); ++i) {
      if (kCursorTypeMappings[i].type == type) {
        gdk_type = kCursorTypeMappings[i].gdk_type;
        break;
      }
    }
  }

  DLOG("Create gtk cursor for type: %d, hittest: %d, gdk: %d",
       type, hittest, gdk_type);

  return gdk_cursor_new(gdk_type);
}
#endif

bool LaunchDesktopFile(const Gadget *gadget, const char *desktop_file) {
#if 0
  ASSERT(desktop_file);

  ggadget::xdg::DesktopEntry desktop_entry(desktop_file);

  // Use ggadget::xdg::OpenURL() to open a link.
  if (desktop_entry.GetType() == ggadget::xdg::DesktopEntry::LINK) {
    return OpenURL(gadget, desktop_entry.GetURL().c_str());
  }

  if (desktop_entry.GetType() != ggadget::xdg::DesktopEntry::APPLICATION) {
    DLOG("Invalid desktop file: %s", desktop_file);
    return false;
  }

  gint argc = 0;
  gchar **argv = NULL;
  GError *error = NULL;
  std::string command = desktop_entry.GetExecCommand(0, NULL);

  // Parse command line first, to make sure it's correct.
  if (!g_shell_parse_argv(command.c_str(), &argc, &argv, &error)) {
    if (error) g_error_free(error);
    if (argv) g_strfreev(argv);
    DLOG("Failed to parse command line: %s", command.c_str());
    return false;
  }

  if (error) {
    g_error_free(error);
    error = NULL;
  }

#if defined(HAVE_STARTUP_NOTIFICATION)
  SnDisplay *sn_display = NULL;
  SnLauncherContext *sn_context = NULL;

  if (desktop_entry.SupportStartupNotify()) {
    sn_display = sn_display_new(clutter_x11_get_default_display(),
                                StartupNotifyErrorTrapPush,
                                StartupNotifyErrorTrapPop);
    if (sn_display) {
      sn_context =
        sn_launcher_context_new(sn_display, clutter_x11_get_default_screen());

      std::string name = desktop_entry.GetName();
      sn_launcher_context_set_description(sn_context, name.c_str());
      sn_launcher_context_set_name(sn_context, name.c_str());
      sn_launcher_context_set_binary_name(sn_context, argv[0]);

#if 0
      int workspace = GetCurrentDesktopOfScreen(screen);
      sn_launcher_context_set_workspace(sn_context, workspace);
#endif

      std::string wmclass = desktop_entry.GetStartupWMClass();
      if (wmclass.length())
        sn_launcher_context_set_wmclass(sn_context, wmclass.c_str());
      std::string icon = desktop_entry.GetIcon();
      if (icon.length())
        sn_launcher_context_set_icon_name(sn_context, icon.c_str());

      sn_launcher_context_initiate(sn_context, g_get_prgname(), argv[0],
                                   gtk_get_current_event_time());
    }
  }
#endif

  GSpawnFlags flags = static_cast<GSpawnFlags>(
      G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL |
      G_SPAWN_SEARCH_PATH);
  GSpawnChildSetupFunc setup_func = NULL;
  gpointer user_data = NULL;
#if defined(HAVE_STARTUP_NOTIFICATION) && defined(GDK_WINDOWING_X11)
  if (sn_context) {
    setup_func = reinterpret_cast<GSpawnChildSetupFunc>(
        sn_launcher_context_setup_child_process);
    user_data = reinterpret_cast<gpointer>(sn_context);
  }
#endif

  std::string working_dir = desktop_entry.GetWorkingDirectory();
  // Ignore relative or invalid path.
  struct stat dir_stat;
  if (!IsAbsolutePath(working_dir.c_str()) ||
      stat(working_dir.c_str(), &dir_stat) != 0 || !S_ISDIR(dir_stat.st_mode))
    working_dir = GetHomeDirectory();

  bool result =
      gdk_spawn_on_screen(screen,
                          working_dir.length() ? working_dir.c_str() : NULL,
                          argv, NULL, flags, setup_func, user_data,
                          NULL, &error);

  if (error) {
    if (!result)
      DLOG("Error when launching %s (%s): %s",
           desktop_file, command.c_str(), error->message);
    g_error_free(error);
  }
  if (argv)
    g_strfreev(argv);
#if defined(HAVE_STARTUP_NOTIFICATION)
  if (sn_context) {
    if (result) {
      g_timeout_add(kStartupNotifyTimeout,
                    StartupNotifyTimeoutHandler, sn_context);
    } else {
      sn_launcher_context_complete(sn_context);
      sn_launcher_context_unref(sn_context);
    }
  }

  if (sn_display)
    sn_display_unref(sn_display);
#endif
  return result;
#endif
  return false;
}

bool OpenURL(const Gadget *gadget, const char *url) {
  ASSERT(url && *url);

  Permissions default_permissions;
  default_permissions.SetRequired(Permissions::NETWORK, true);
  default_permissions.GrantAllRequired();

  const Permissions *permissions =
      gadget ? gadget->GetPermissions() : &default_permissions;

  std::string path;
  if (IsAbsolutePath(url)) {
    path = url;
  } else if (IsValidFileURL(url)) {
    path = DecodeURL(url + arraysize(kFileUrlPrefix) - 1);
  }

  if (path.length()) {
    if (!permissions->IsRequiredAndGranted(Permissions::ALL_ACCESS)) {
      LOG("No permission to open a local file: %s", url);
      return false;
    }
    std::string mime = ggadget::xdg::GetFileMimeType(path.c_str());
    if (mime == ggadget::xdg::kDesktopEntryMimeType)
      return LaunchDesktopFile(gadget, path.c_str());
  }

  return ggadget::xdg::OpenURL(*permissions, url);
}

static ClutterActor *default_group_ = NULL;
void SetClutterDefaultGroup(ClutterActor *default_group) {
  default_group_ = default_group;
}


uint64_t GetCurrentTime() {
  GTimeVal tv;
  g_get_current_time(&tv);
  return static_cast<uint64_t>(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

ClutterActor *GetClutterDefaultGroup() {
  return default_group_;
}
} // namespace clutter
} // namespace ggadget
