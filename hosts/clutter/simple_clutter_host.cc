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

#include <stdlib.h>
#include <clutter/clutter.h>
#include <string>
#include <map>

#include "simple_clutter_host.h"

#include <ggadget/common.h>
#include <ggadget/decorated_view_host.h>
#include <ggadget/floating_main_view_decorator.h>
#include <ggadget/docked_main_view_decorator.h>
#include <ggadget/popout_main_view_decorator.h>
#include <ggadget/details_view_decorator.h>
#include <ggadget/gadget.h>
#include <ggadget/gadget_consts.h>
#include <ggadget/gadget_manager_interface.h>
#include <ggadget/clutter/menu_builder.h>
#include <ggadget/clutter/single_view_host.h>
#include <ggadget/clutter/key_convert.h>
#include <ggadget/clutter/utilities.h>
#include <ggadget/locales.h>
#include <ggadget/messages.h>
#include <ggadget/logger.h>
#include <ggadget/script_runtime_manager.h>
#include <ggadget/view.h>
#include <ggadget/main_loop_interface.h>
#include <ggadget/file_manager_factory.h>
#include <ggadget/options_interface.h>
#include <ggadget/string_utils.h>
#include <ggadget/permissions.h>
#include <ggadget/math_utils.h>

#include "gadget_browser_host.h"

using namespace ggadget;
using namespace ggadget::clutter;

namespace ggadget {
DECLARE_VARIANT_PTR_TYPE(DecoratedViewHost);
}

namespace hosts {
namespace clutter {

static const char kOptionHotKey[] = "hotkey";
static const char kOptionGadgetsShown[] = "gadgets_shown";
static const char kOptionFontSize[] = "font_size";

static const int kMinFontSize = 4;
static const int kMaxFontSize = 16;


static gboolean SwitchAddButtonIcon (ClutterActor *actor,
                                     ClutterEvent *event,
                                     gpointer      user_data)
{
  if (event->type == CLUTTER_ENTER) {
    clutter_texture_set_from_file (CLUTTER_TEXTURE (actor),
                                   PIXMAP_DIR "control-hover.png", NULL);
  }else{
    clutter_texture_set_from_file (CLUTTER_TEXTURE (actor),
                                   PIXMAP_DIR "control.png", NULL);
  }
  return TRUE;
}

class SimpleClutterHost::Impl {
  struct GadgetInfo {
    GadgetInfo()
      : gadget(NULL), main(NULL),
        details_on_right(false) {
    }

    Gadget *gadget;

    SingleViewHost *main;
    SingleViewHost *details;
    DecoratedViewHost *main_decorator;

    bool details_on_right;
  };

 public:
  Impl(SimpleClutterHost *owner, OptionsInterface *options,
       double zoom, int view_debug_mode,
       Gadget::DebugConsoleConfig debug_console_config,
       int width, int height)
    : gadget_browser_host_(owner, view_debug_mode),
      owner_(owner),
      options_(options),
      zoom_(zoom),
      view_debug_mode_(view_debug_mode),
      debug_console_config_(debug_console_config),
      gadgets_shown_(true),
      font_size_(kDefaultFontSize),
      gadget_manager_(GetGadgetManager()),
      main_group_(NULL), gadget_group_(NULL), add_button_(NULL),
      trash_button_(NULL),
      stage_width (width), stage_height (height),
      trash_active_ (false)
  {
    ASSERT(gadget_manager_);
    ASSERT(options_);

    if (options_) {
      options_->GetInternalValue(
          kOptionGadgetsShown).ConvertToBool(&gadgets_shown_);
      options_->GetInternalValue(kOptionFontSize).ConvertToInt(&font_size_);
      font_size_ = std::min(kMaxFontSize, std::max(kMinFontSize, font_size_));
    }

    // Connect gadget related signals.
    gadget_manager_->ConnectOnNewGadgetInstance(
        NewSlot(this, &Impl::NewGadgetInstanceCallback));
    gadget_manager_->ConnectOnRemoveGadgetInstance(
        NewSlot(this, &Impl::RemoveGadgetInstanceCallback));

    // Initializes global permissions.
    // FIXME: Supports customizable global permissions.
    global_permissions_.SetGranted(Permissions::ALL_ACCESS, true);
  }

  ~Impl() {
    for (GadgetInfoMap::iterator it = gadgets_.begin();
         it != gadgets_.end(); ++it) {
      delete it->second.gadget;
    }
  }

  void SetupUI() {
    main_group_ = clutter_group_new();
    clutter_set_motion_events_enabled (TRUE);

    gadget_browser_host_.SetGadgetGroup(main_group_);

    gadget_group_ = clutter_group_new();
    clutter_container_add_actor(CLUTTER_CONTAINER(main_group_), gadget_group_);
    clutter_actor_show(gadget_group_);

    trash_button_ =
      clutter_texture_new_from_file(PIXMAP_DIR "trash-normal.png", NULL);
    clutter_actor_set_reactive(trash_button_, true);

    clutter_actor_set_anchor_point_from_gravity (trash_button_,
                                                 CLUTTER_GRAVITY_SOUTH_WEST);
    clutter_container_add_actor(CLUTTER_CONTAINER(main_group_), trash_button_);
    clutter_actor_hide(trash_button_);

    add_button_ =
      clutter_texture_new_from_file(PIXMAP_DIR "control.png", NULL);
    clutter_container_add_actor(CLUTTER_CONTAINER(main_group_), add_button_);
    clutter_actor_set_reactive(add_button_, true);
    g_signal_connect (add_button_, "button-release-event",
                      G_CALLBACK (AddButtonClicked), this);

    g_signal_connect (add_button_, "enter-event",
                      G_CALLBACK (SwitchAddButtonIcon), NULL);
    g_signal_connect (add_button_, "leave-event",
                      G_CALLBACK (SwitchAddButtonIcon), NULL);

    clutter_actor_set_anchor_point_from_gravity (add_button_,
                                                 CLUTTER_GRAVITY_SOUTH_WEST);
    AdjustSize (stage_width, stage_height);
  }

  static void AddButtonClicked(ClutterActor *actor, ClutterButtonEvent *event,
                               gpointer userdata) {
    Impl *impl = reinterpret_cast<Impl *>(userdata);

    impl->gadget_manager_->ShowGadgetBrowserDialog(&impl->gadget_browser_host_);
  }

  static bool GetPermissionsDescriptionCallback(int permission,
                                                std::string *msg) {
    if (msg->length())
      msg->append("\n");
    msg->append("  ");
    msg->append(Permissions::GetDescription(permission));
    return true;
  }

  bool ConfirmGadget(int id, Permissions *permissions) {
    std::string path = gadget_manager_->GetGadgetInstancePath(id);
    std::string download_url, title, description;
    if (!gadget_manager_->GetGadgetInstanceInfo(id,
                                                GetSystemLocaleName().c_str(),
                                                NULL, &download_url,
                                                &title, &description))
      return false;

    // Get required permissions description.
    std::string permissions_msg;
    permissions->EnumerateAllRequired(
        NewSlot(GetPermissionsDescriptionCallback, &permissions_msg));

    g_print ("Title: %s\n", title.c_str());
    g_print ("Download: %s\n", download_url.c_str());
    g_print ("Description: %s\n", description.c_str());
    g_print ("Permissions: %s\n", permissions_msg.c_str());
#if 0
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "%s\n\n%s\n%s\n\n%s%s\n\n%s\n%s",
        GM_("GADGET_CONFIRM_MESSAGE"), title.c_str(), download_url.c_str(),
        GM_("GADGET_DESCRIPTION"), description.c_str(),
        GM_("GADGET_REQUIRED_PERMISSIONS"), permissions_msg.c_str());

    GdkScreen *screen;
    gdk_display_get_pointer(gdk_display_get_default(), &screen,
                            NULL, NULL, NULL);
    gtk_window_set_screen(GTK_WINDOW(dialog), screen);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(dialog), GM_("GADGET_CONFIRM_TITLE"));
    gtk_window_present(GTK_WINDOW(dialog));
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (result == GTK_RESPONSE_YES) {
      // TODO: Is it necessary to let user grant individual permissions
      // separately?
      permissions->GrantAllRequired();
      return true;
    }
    return false;
#else
    return true;
#endif
  }

  bool EnumerateGadgetInstancesCallback(int id) {
    if (!LoadGadgetInstance(id))
      gadget_manager_->RemoveGadgetInstance(id);
    // Return true to continue the enumeration.
    return true;
  }

  bool NewGadgetInstanceCallback(int id) {
    Permissions permissions;
    if (gadget_manager_->GetGadgetDefaultPermissions(id, &permissions)) {
      if (!permissions.HasUngranted() || ConfirmGadget(id, &permissions)) {
        // Save initial permissions.
        std::string options_name =
            gadget_manager_->GetGadgetInstanceOptionsName(id);
        OptionsInterface *options = CreateOptions(options_name.c_str());
        // Don't save required permissions.
        permissions.RemoveAllRequired();
        options->PutInternalValue(kPermissionsOption,
                                  Variant(permissions.ToString()));
        options->Flush();
        delete options;
        return LoadGadgetInstance(id);
      }
    } else {
      ShowAlertDialog(
          GM_("GOOGLE_GADGETS"),
          StringPrintf(
              GM_("GADGET_LOAD_FAILURE"),
              gadget_manager_->GetGadgetInstancePath(id).c_str()).c_str());
    }
    return false;
  }

  bool LoadGadgetInstance(int id) {
    bool result = false;
    std::string options = gadget_manager_->GetGadgetInstanceOptionsName(id);
    std::string path = gadget_manager_->GetGadgetInstancePath(id);
    if (options.length() && path.length()) {
      result =
          LoadGadget(path.c_str(), options.c_str(), id, false);
      DLOG("SimpleClutterHost: Load gadget %s, with option %s, %s",
           path.c_str(), options.c_str(), result ? "succeeded" : "failed");
    }
    return result;
  }

  Gadget* LoadGadget(const char *path, const char *options_name, int instance_id, bool) {
    if (gadgets_.find(instance_id) != gadgets_.end()) {
      // Gadget is already loaded.
      return gadgets_[instance_id].gadget;
    }

    Gadget *gadget = new Gadget(owner_, path, options_name, instance_id,
                                global_permissions_, debug_console_config_);
    GadgetInfoMap::iterator it = gadgets_.find(instance_id);

    if (!gadget->IsValid()) {
      LOG("Failed to load gadget %s", path);
      if (it != gadgets_.end()) {
        gadgets_.erase(it);
      }
      delete gadget;
      return NULL;
    }

    gadget->SetDisplayTarget(Gadget::TARGET_FLOATING_VIEW);
    gadget->GetMainView()->OnOtherEvent(SimpleEvent(Event::EVENT_UNDOCK));

    if (gadgets_shown_)
      gadget->ShowMainView();

    return gadget;
  }

  ViewHostInterface *NewViewHost(Gadget *gadget,
                                 ViewHostInterface::Type type) {
    ASSERT(gadget);
    int gadget_id = gadget->GetInstanceID();
    GadgetInfo *info = &gadgets_[gadget_id];
    ASSERT(info->gadget == NULL || info->gadget == gadget);
    info->gadget = gadget;

    SingleViewHost *svh = new SingleViewHost(type, zoom_, 0,
                                             view_debug_mode_, main_group_);

    if (type == ViewHostInterface::VIEW_HOST_OPTIONS) {
      return svh;
    }

    DecoratedViewHost *dvh;
    if (type == ViewHostInterface::VIEW_HOST_MAIN) {
      FloatingMainViewDecorator *view_decorator =
          new FloatingMainViewDecorator(svh, true);

      dvh = new DecoratedViewHost(view_decorator);
      ASSERT(!info->main);
      info->main = svh;
      info->main_decorator = dvh;

      svh->ConnectOnShowHide(
          NewSlot(this, &Impl::OnMainViewShowHideHandler, gadget_id));

      svh->ConnectOnBeginMoveDrag(
          NewSlot(this, &Impl::OnMainViewBeginMoveDragHandler, gadget_id));
      svh->ConnectOnEndMoveDrag(
          NewSlot(this, &Impl::OnMainViewEndMoveDragHandler, gadget_id));
      svh->ConnectOnMoved(
          NewSlot(this, &Impl::OnMainViewMovedHandler, gadget_id));


      view_decorator->ConnectOnClose(
          NewSlot(this, &Impl::OnCloseHandler, dvh));
      view_decorator->SetButtonVisible(MainViewDecoratorBase::POP_IN_OUT_BUTTON,
                                       false);
    } else {
      DetailsViewDecorator *view_decorator = new DetailsViewDecorator(svh);
      dvh = new DecoratedViewHost(view_decorator);
      ASSERT(info->main);
      ASSERT(!info->details);
      info->details = svh;

      svh->ConnectOnShowHide(
          NewSlot(this, &Impl::OnDetailsViewShowHideHandler, gadget_id));
#if 0
      svh->ConnectOnBeginResizeDrag(
          NewSlot(this, &Impl::OnDetailsViewBeginResizeHandler, gadget_id));
      svh->ConnectOnResized(
          NewSlot(this, &Impl::OnDetailsViewResizedHandler, gadget_id));
      svh->ConnectOnBeginMoveDrag(
          NewSlot(this, &Impl::OnDetailsViewBeginMoveHandler));
#endif
      view_decorator->ConnectOnClose(NewSlot(this, &Impl::OnCloseHandler, dvh));
    }

    ClutterActor *actor = svh->GetActor();
    clutter_container_add_actor (CLUTTER_CONTAINER (gadget_group_), actor);

    gfloat gadget_width, gadget_height;
    clutter_actor_get_size(actor, &gadget_width, &gadget_height);

    gfloat gadget_x, gadget_y;
    gadget_x = rand() % (int)(stage_width - gadget_width);
    gadget_y = rand() % (int)(stage_height - gadget_height);

    clutter_actor_set_position (actor, gadget_x, gadget_y);
    clutter_actor_show (actor);
    DLOG("NewViewHost: ADDED ACTOR(%p) x = %d, y = %d, width = %d, height = %d\n",
         actor,
         (int)gadget_x, (int)gadget_y, (int)gadget_width, (int)gadget_height);
    return dvh;
  }

  void RemoveGadget(Gadget *gadget, bool save_data) {
    ASSERT(gadget);
    int id = gadget->GetInstanceID();

    // If RemoveGadgetInstance() returns false, then means this instance is not
    // installed by gadget manager.
    if (!gadget_manager_->RemoveGadgetInstance(id))
      RemoveGadgetInstanceCallback(id);
  }

  void RemoveGadgetInstanceCallback(int instance_id) {
    GadgetInfoMap::iterator it = gadgets_.find(instance_id);
    if (it != gadgets_.end()) {
      delete it->second.gadget;
      gadgets_.erase(it);
    } else {
      LOG("Can't find gadget instance %d", instance_id);
    }
  }

  void LoadGadgets() {
    gadget_manager_->EnumerateGadgetInstances(
        NewSlot(this, &Impl::EnumerateGadgetInstancesCallback));
  }

  void OnCloseHandler(DecoratedViewHost *decorated) {
    ViewInterface *child = decorated->GetView();
    Gadget *gadget = child ? child->GetGadget() : NULL;

    ASSERT(gadget);
    if (!gadget) return;

    GadgetInfo *info = &gadgets_[gadget->GetInstanceID()];

    switch (decorated->GetType()) {
    case ViewHostInterface::VIEW_HOST_MAIN:
      if (decorated == info->main_decorator) {
        gadget->RemoveMe(true);
      }
      break;
    case ViewHostInterface::VIEW_HOST_DETAILS:
      gadget->CloseDetailsView();
      break;
    default:
      ASSERT_M(false, ("Invalid decorator type."));
    }
  }

  bool OnMainViewBeginMoveDragHandler(int button, int gadget_id) {
    clutter_actor_show(trash_button_);
    clutter_actor_hide(add_button_);
    trash_active_ = false;
    return false;
  }

  void OnMainViewMovedHandler(int x, int y, int gadget_id) {
    GadgetInfoMap::iterator it = gadgets_.find(gadget_id);
    if (it != gadgets_.end()) {
      int x, y, w, h;
      ClutterGeometry geo;

      it->second.main->GetActorPosition(&x, &y);
      it->second.main->GetActorSize(&w, &h);
      clutter_actor_get_geometry (trash_button_, &geo);

      Rectangle rect(x, y, w, h), rect0 (geo.x, geo.y, geo.width, geo.height);

      if (rect0.Intersect(rect)) {
        if (!trash_active_) {
          clutter_texture_set_from_file (CLUTTER_TEXTURE (trash_button_),
                                         PIXMAP_DIR "trash-colored.png", NULL);
          trash_active_ = true;
        }
      }else{
        if (trash_active_) {
          clutter_texture_set_from_file (CLUTTER_TEXTURE (trash_button_),
                                         PIXMAP_DIR "trash-normal.png", NULL);
          trash_active_ = false;
        }
      }
    }
  }

  void OnMainViewEndMoveDragHandler(int gadget_id) {
    GadgetInfoMap::iterator it = gadgets_.find(gadget_id);
    if (it != gadgets_.end()) {
      int x, y, w, h;
      ClutterGeometry geo;

      it->second.main->GetActorPosition(&x, &y);
      it->second.main->GetActorSize(&w, &h);
      clutter_actor_get_geometry (trash_button_, &geo);

      Rectangle rect(x, y, w, h), rect0 (geo.x, geo.y, geo.width, geo.height);

      if (rect0.Intersect(rect)) {
        //RemoveGadget (it->second.gadget, true);
        it->second.gadget->RemoveMe (true);
      }
    }

    clutter_actor_show(add_button_);
    clutter_actor_hide(trash_button_);
  }

  void OnPopOutHandler(DecoratedViewHost *decorated) {
  }

  void OnPopInHandler(DecoratedViewHost *decorated) {
  }

  void AdjustViewHostPosition(GadgetInfo *info) {
    ASSERT(info && info->main && info->main_decorator);
    int x, y;
    int width, height;
    info->main->GetActorPosition(&x, &y);
    info->main->GetActorSize(&width, &height);

#if 0
    if (info->details && info->details->IsVisible()) {
      int details_width, details_height;
      info->details->GetActorSize(&details_width, &details_height);

      info->details->SetActorPosition(x - details_width, y);
    }
#endif
  }

  void OnMainViewShowHideHandler(bool show, int gadget_id) {
    GadgetInfoMap::iterator it = gadgets_.find(gadget_id);
    if (it != gadgets_.end()) {
      if (show) {
        AdjustViewHostPosition(&it->second);
      } else {
#if 0
        if (it->second.details) {
          // The details view won't be shown again.
          it->second.details->CloseView();
          it->second.details = NULL;
        }
#endif
      }
    }
  }

  void OnDetailsViewShowHideHandler(bool show, int gadget_id) {
    GadgetInfoMap::iterator it = gadgets_.find(gadget_id);
    if (it != gadgets_.end() && it->second.details) {
      if (show) {
        AdjustViewHostPosition(&it->second);
      } else {
        // The same details view will never shown again.
        it->second.details = NULL;
      }
    }
  }

  void AdjustSize (int width, int height) {
    stage_width = width;
    stage_height = height;

    if (main_group_)
      clutter_actor_set_size(main_group_, stage_width, stage_height);
    if (gadget_group_)
      clutter_actor_set_size(gadget_group_, stage_width, stage_height);
    if (add_button_)
      clutter_actor_set_position(add_button_, 0, stage_height);
    if (trash_button_)
      clutter_actor_set_position(trash_button_, 0, stage_height);
  }

  typedef std::map<int, GadgetInfo> GadgetInfoMap;
  GadgetInfoMap gadgets_;

  GadgetBrowserHost gadget_browser_host_;
  SimpleClutterHost *owner_;
  OptionsInterface *options_;

  double zoom_;
  int view_debug_mode_;
  Gadget::DebugConsoleConfig debug_console_config_;
  bool gadgets_shown_;
  bool transparent_;
  int font_size_;

  GadgetManagerInterface *gadget_manager_;
  ClutterActor *main_group_;
  ClutterActor *gadget_group_;
  ClutterActor *add_button_, *trash_button_;
  int stage_width, stage_height;
  bool trash_active_;

  Permissions global_permissions_;
};

SimpleClutterHost::SimpleClutterHost(OptionsInterface *options, double zoom,
                                     int view_debug_mode,
                                     Gadget::DebugConsoleConfig debug_console_config,
                                     int width, int height)
  : impl_(new Impl(this, options, zoom, view_debug_mode,
                   debug_console_config, width, height)) {
  impl_->SetupUI();
  impl_->LoadGadgets();
}

SimpleClutterHost::~SimpleClutterHost() {
  delete impl_;
  impl_ = NULL;
}

ViewHostInterface *SimpleClutterHost::NewViewHost(Gadget *gadget,
                                                  ViewHostInterface::Type type) {
  return impl_->NewViewHost(gadget, type);
}

void SimpleClutterHost::RemoveGadget(Gadget *gadget, bool save_data) {
  return impl_->RemoveGadget(gadget, save_data);
}

bool SimpleClutterHost::LoadFont(const char *filename) {
  return ggadget::clutter::LoadFont(filename);
}

void SimpleClutterHost::ShowGadgetAboutDialog(ggadget::Gadget *gadget) {
//  ggadget::clutter::ShowGadgetAboutDialog(gadget);
}

void SimpleClutterHost::ShowGadgetDebugConsole(Gadget *gadget) {
  //  impl_->ShowGadgetDebugConsole(gadget);
}

int SimpleClutterHost::GetDefaultFontSize() {
  return impl_->font_size_;
}

bool SimpleClutterHost::OpenURL(const Gadget *gadget, const char *url) {
  return ggadget::clutter::OpenURL(gadget, url);
}

ClutterActor *SimpleClutterHost::GetActor() {
  return impl_->main_group_;
}

Gadget* SimpleClutterHost::LoadGadget(const char *path, const char *options_name, int instance_id, bool debug) {
  return impl_->LoadGadget(path, options_name, instance_id, debug);
}

void SimpleClutterHost::Exit () {
  g_timeout_add (200, (GSourceFunc)&SimpleClutterHost::ExitCallback, (gpointer)this);
}

void SimpleClutterHost::AdjustSize (int width, int height) {
  impl_->AdjustSize (width, height);
}

gboolean SimpleClutterHost::ExitCallback (gpointer that) {
  return FALSE;
}

} // namespace clutter
} // namespace hosts
