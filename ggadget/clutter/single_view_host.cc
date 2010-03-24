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
  Copyright 2008-2010 Intel Corp..

  Authors:
  Iain Holmes <iain@linux.intel.com>
  Roger WANG <roger.wang@intel.com>
*/

#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>
#include <gdk/gdk.h>

#include <string>
#include <ggadget/gadget_consts.h>
#include <ggadget/file_manager_interface.h>
#include <ggadget/file_manager_factory.h>
#include <ggadget/logger.h>
#include <ggadget/options_interface.h>
#include <ggadget/gadget.h>
#include <ggadget/messages.h>
#include <ggadget/math_utils.h>
#include "single_view_host.h"
#include "view_actor_binder.h"
#include "cairo_graphics.h"
#include "menu_builder.h"
#include "tooltip.h"
#include "utilities.h"
#include "key_convert.h"

// It might not be necessary, because X server will grab the pointer
// implicitly when button is pressed.
// But using explicit mouse grabbing may avoid some issues by preventing some
// events from sending to client window when mouse is grabbed.
#define GRAB_POINTER_EXPLICITLY
namespace ggadget {
namespace clutter {
#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 100

static const double kMinimumZoom = 0.5;
static const double kMaximumZoom = 2.0;
static const int kStopMoveDragTimeout = 200;
static const char kMainViewWindowRole[] = "Google-Gadgets";

// Minimal interval between queue draws.
static const unsigned int kQueueDrawInterval = 40;

// Maximum live duration of queue draw timer.
static const uint64_t kQueueDrawTimerDuration = 1000;

class SingleViewHost::Impl {
 public:
  Impl(SingleViewHost *owner, ViewHostInterface::Type type,
       double zoom, int flags, int debug_mode, ClutterActor* main_group)
    : owner_(owner),
      view_(NULL),
      actor_(NULL),
      tooltip_(new Tooltip(kShowTooltipDelay, kHideTooltipDelay)),
      binder_(NULL),
      type_(type),
      initial_zoom_(zoom),
      flags_(flags),
      debug_mode_(debug_mode),
      stop_move_drag_source_(0),
      win_x_(0),
      win_y_(0),
      win_width_(0),
      win_height_(0),
      resize_view_zoom_(0),
      resize_view_width_(0),
      resize_view_height_(0),
      resize_win_x_(0),
      resize_win_y_(0),
      resize_win_width_(0),
      resize_win_height_(0),
      resize_button_(0),
      resize_mouse_x_(0),
      resize_mouse_y_(0),
      resize_width_mode_(0),
      resize_height_mode_(0),
      is_keep_above_(false),
      move_dragging_(false),
      drag_offset_x_(0),
      drag_offset_y_(0),
      enable_signals_(true),
      feedback_handler_(NULL),
      can_close_dialog_(false),
      main_group_ (main_group)
  {
    ASSERT(owner);
  }

  ~Impl() {
    Detach();
  }

  void Detach() {
    // To make sure that it won't be accessed anymore.
    view_ = NULL;

    delete feedback_handler_;
    feedback_handler_ = NULL;

    delete binder_;
    binder_ = NULL;

    if (actor_) {
      clutter_actor_destroy(actor_);
      actor_ = NULL;
    }
  }

  void SetView(ViewInterface *view) {
    if (view_ == view)
      return;

    Detach();

    if (view == NULL) {
      on_view_changed_signal_();
      return;
    }

    view_ = view;
    if (type_ == ViewHostInterface::VIEW_HOST_OPTIONS) {
      // Options needs ok/cancel buttons added to it...
      actor_ = clutter_cairo_texture_new (DEFAULT_WIDTH, DEFAULT_HEIGHT);
      clutter_actor_set_reactive (actor_, TRUE);
    } else {
      // details and main view only need a toplevel window.
      // buttons of details view shall be provided by view decorator.
      actor_ = clutter_cairo_texture_new (DEFAULT_WIDTH, DEFAULT_HEIGHT);
      clutter_actor_set_reactive (actor_, TRUE);
    }

    g_signal_connect(G_OBJECT(actor_), "focus-in",
                     G_CALLBACK(FocusInHandler), this);
#ifdef _DEBUG
    g_signal_connect(G_OBJECT(actor_), "focus-out",
                     G_CALLBACK(FocusOutHandler), this);
#endif
    g_signal_connect(G_OBJECT(actor_), "enter-event",
                     G_CALLBACK(EnterNotifyHandler), this);
    g_signal_connect(G_OBJECT(actor_), "leave-event",
                     G_CALLBACK(EnterNotifyHandler), this);
    g_signal_connect(G_OBJECT(actor_), "show",
                     G_CALLBACK(ActorShowHandler), this);
    g_signal_connect_after(G_OBJECT(actor_), "hide",
                     G_CALLBACK(ActorHideHandler), this);

#if 0
    g_signal_connect(G_OBJECT(widget_), "size-request",
                     G_CALLBACK(WidgetSizeRequestHandler), this);

    g_signal_connect(G_OBJECT(fixed_), "size-allocate",
                     G_CALLBACK(FixedSizeAllocateHandler), this);
#endif
    binder_ = new ViewActorBinder(view_, owner_, actor_);
    on_view_changed_signal_();
  }

  void ViewCoordToNativeWidgetCoord(
      double x, double y, double *widget_x, double *widget_y) const {
    double zoom = view_->GetGraphics()->GetZoom();
    if (widget_x)
      *widget_x = x * zoom;
    if (widget_y)
      *widget_y = y * zoom;
  }

  void NativeWidgetCoordToViewCoord(double x, double y,
                                    double *view_x, double *view_y) const {
    double zoom = view_->GetGraphics()->GetZoom();
    if (zoom == 0) return;
    if (view_x) *view_x = x / zoom;
    if (view_y) *view_y = y / zoom;
  }

  void AdjustWindowSize() {
    ASSERT(view_);

    double zoom = view_->GetGraphics()->GetZoom();
    int width = static_cast<int>(ceil(view_->GetWidth() * zoom));
    int height = static_cast<int>(ceil(view_->GetHeight() * zoom));

    if (width < 1 || height < 1)
      return;

    gfloat width0, height0;

    DLOG ("AdjustWindowSize: view_: %p, width = %d, height = %d, actor = %p\n", view_, width, height, actor_);
    clutter_cairo_texture_set_surface_size (CLUTTER_CAIRO_TEXTURE (actor_), width, height);
    clutter_actor_get_size (actor_, &width0, &height0);
    DLOG ("actor new size: %d %d", (int)width0, (int)height0);
    QueueDraw();
  }

  void QueueResize() {
    // When doing resize drag, MotionNotifyHandler() is in charge of resizing
    // the window, so don't do it here.
    if (resize_width_mode_ == 0 && resize_height_mode_ == 0)
      AdjustWindowSize();
  }

  void QueueDraw() {
    binder_->QueueDraw();
  }

  void SetResizable(ViewInterface::ResizableMode mode) {
  }

  void SetCaption(const std::string &caption) {
  }

  void SetShowCaptionAlways(bool always) {
    // SingleViewHost will always show caption when window decorator is shown.
  }

  void SetCursor(ViewInterface::CursorType type, bool respect_hittest = true) {
    // Don't change cursor if it's in resize dragging mode.
    if (resize_width_mode_ || resize_height_mode_)
      return;
    ViewInterface::HitTest hittest =
      respect_hittest ? view_->GetHitTest() : ViewInterface::HT_CLIENT;
    GdkCursor *cursor = CreateCursor(type, hittest);
    GdkWindow *window;
    window = gdk_window_foreign_new (
              clutter_x11_get_stage_window (
                 CLUTTER_STAGE (clutter_actor_get_stage (actor_))));
    if (window) {
      gdk_window_set_cursor (window, cursor);
      g_object_unref (window);
    }
    if (cursor)
      gdk_cursor_unref(cursor);
  }

  void ShowTooltip(const std::string &tooltip) {
    DLOG("Disabled - SingleViewHost::ShowTooltip(%s)", tooltip.c_str());
    //tooltip_->Show(tooltip.c_str());
  }

  void ShowTooltipAtPosition(const std::string &tooltip, double x, double y) {
    ViewCoordToNativeWidgetCoord(x, y, &x, &y);
    int screen_x = static_cast<int>(x) + win_x_;
    int screen_y = static_cast<int>(y) + win_y_;
    DLOG("Disabled - SingleViewHost::ShowTooltipAtPosition(%s, %d, %d)",
         tooltip.c_str(), screen_x, screen_y);
    //tooltip_->ShowAtPosition(tooltip.c_str(), screen_x, screen_y);
  }

  bool ShowView(bool modal, int flags, Slot1<bool, int> *feedback_handler) {
    ASSERT(view_);

    delete feedback_handler_;
    feedback_handler_ = feedback_handler;

    // Adjust the window size just before showing the view, to make sure that
    // the window size has correct default size when showing.
    AdjustWindowSize();
    LoadWindowStates();
    clutter_container_add_actor (CLUTTER_CONTAINER (main_group_), actor_);
    clutter_actor_show(actor_);

    // Load window states again to make sure it's still correct
    // after the window is shown.
    LoadWindowStates();

    // Make sure the view is inside screen.
    //EnsureInsideScreen();

    return true;
  }

  void CloseView() {
    ASSERT(actor_);
    if (actor_)
      clutter_actor_hide(actor_);
  }

  void SetActorPosition(int x, int y) {
    ASSERT(actor_);
    if (actor_) {
      win_x_ = x;
      win_y_ = y;
      clutter_actor_set_position(actor_, x, y);
      SaveWindowStates(true, false);
    }
  }

  void SetKeepAbove(bool keep_above) {
  }

  std::string GetViewPositionOptionPrefix() {
    switch (type_) {
      case ViewHostInterface::VIEW_HOST_MAIN:
        return "main_view";
      case ViewHostInterface::VIEW_HOST_OPTIONS:
        return "options_view";
      case ViewHostInterface::VIEW_HOST_DETAILS:
        return "details_view";
      default:
        return "view";
    }
    return "";
  }

  void SaveWindowStates(bool save_position, bool save_keep_above) {
    if ((flags_ & RECORD_STATES) && view_ && view_->GetGadget()) {
      OptionsInterface *opt = view_->GetGadget()->GetOptions();
      std::string opt_prefix = GetViewPositionOptionPrefix();
      if (save_position) {
        opt->PutInternalValue((opt_prefix + "_x").c_str(),
                              Variant(win_x_));
        opt->PutInternalValue((opt_prefix + "_y").c_str(),
                              Variant(win_y_));
      }
      if (save_keep_above) {
        opt->PutInternalValue((opt_prefix + "_keep_above").c_str(),
                              Variant(is_keep_above_));
      }
    }
    // Don't save size and zoom information, it's conflict with view
    // decorator.
  }

  void LoadWindowStates() {
    if ((flags_ & RECORD_STATES) && view_ && view_->GetGadget()) {
      OptionsInterface *opt = view_->GetGadget()->GetOptions();
      std::string opt_prefix = GetViewPositionOptionPrefix();

      // Restore window position.
      Variant vx = opt->GetInternalValue((opt_prefix + "_x").c_str());
      Variant vy = opt->GetInternalValue((opt_prefix + "_y").c_str());
      int x, y;
      if (vx.ConvertToInt(&x) && vy.ConvertToInt(&y)) {
        win_x_ = x;
        win_y_ = y;
      } else {
        x = 0;
        y = 0;
      }

      // save anchor gravity, normally we keep the gravity at
      // north-west, but when loading states we happened to be in the
      // middle of animation for mapping effect.

      ClutterGravity g = clutter_actor_get_anchor_point_gravity (actor_);
      clutter_actor_set_anchor_point_from_gravity (actor_,
                                                   CLUTTER_GRAVITY_NORTH_WEST);
      clutter_actor_set_position(actor_, x, y);
      clutter_actor_move_anchor_point_from_gravity (actor_, g);
    }
    // Don't load size and zoom information, it's conflict with view
    // decorator.
  }

  bool ShowContextMenu(int button) {
    ASSERT(view_);
    DLOG("Show context menu.");

    g_print ("Show context menu\n");
    MenuBuilder menu_builder();

    return true;
  }


  static gboolean ResizeMotionNotifyHandler(ClutterActor *actor,
                                            ClutterMotionEvent *event,
                                            gpointer userdata) {
    Impl *impl = reinterpret_cast<Impl *>(userdata);
    if (impl->resize_width_mode_ || impl->resize_height_mode_) {
      int button = ConvertClutterModifierToButton(event->modifier_state);
      if (button == impl->resize_button_) {
        double original_width =
          impl->resize_view_width_ * impl->resize_view_zoom_;
        double original_height =
          impl->resize_view_height_ * impl->resize_view_zoom_;
        double delta_x = event->x - impl->resize_mouse_x_;
        double delta_y = event->y - impl->resize_mouse_y_;
        double width = original_width;
        double height = original_height;
        double new_width = width + impl->resize_width_mode_ * delta_x;
        double new_height = height + impl->resize_height_mode_ * delta_y;

        if (impl->view_->GetResizable() == ViewInterface::RESIZABLE_TRUE ||
            impl->view_->GetResizable() == ViewInterface::RESIZABLE_KEEP_RATIO) {
          double view_width = new_width / impl->resize_view_zoom_;
          double view_height = new_height / impl->resize_view_zoom_;

          if (impl->view_->OnSizing(&view_width, &view_height)) {
            impl->view_->SetSize(view_width, view_height);
            width = impl->view_->GetWidth() * impl->resize_view_zoom_;
            height = impl->view_->GetHeight() * impl->resize_view_zoom_;
          }
        } else if (impl->resize_view_width_ && impl->resize_view_height_) {
          double xzoom = new_width / impl->resize_view_width_;
          double yzoom = new_height / impl->resize_view_height_;
          double zoom = std::min(xzoom, yzoom);
          zoom = Clamp(zoom, kMinimumZoom, kMaximumZoom);

          impl->view_->GetGraphics()->SetZoom(zoom);
          width = impl->resize_view_width_ * zoom;
          height = impl->resize_view_height_ * zoom;
        }

        if (width != original_width || height != original_height) {
          delta_x = width - original_width;
          delta_y = height - original_height;
          int x = impl->resize_win_x_;
          int y = impl->resize_win_y_;
          if (impl->resize_width_mode_ == -1)
            x -= int(delta_x);
          if (impl->resize_height_mode_ == -1)
            y -= int(delta_y);
          int win_width = impl->resize_win_width_ + int(delta_x);
          int win_height = impl->resize_win_height_ + int(delta_y);

          // While the resize is in progress, use GL scaling to be speedy
          clutter_actor_set_position (impl->actor_, x, y);
          clutter_actor_set_size (impl->actor_, win_width, win_height);
        }
        return true;
      } else {
        impl->StopResizeDrag();
      }
    }

    return false;
  }

  static gboolean ResizeButtonReleaseHandler(ClutterActor *actor,
                                             ClutterButtonEvent *event,
                                             gpointer userdata) {
    Impl *impl = reinterpret_cast<Impl *>(userdata);

    g_signal_handler_disconnect(actor, impl->motion_handler_id_);
    g_signal_handler_disconnect(actor, impl->button_release_handler_id_);

    clutter_ungrab_pointer();

    impl->binder_->ThawUpdates();
    // Now the resize has finished, we redraw properly
    gfloat win_width, win_height;
    clutter_actor_get_size(impl->actor_, &win_width, &win_height);
    clutter_cairo_texture_set_surface_size (CLUTTER_CAIRO_TEXTURE(impl->actor_),
                                  win_width, win_height);

    impl->view_->MarkRedraw ();
    impl->binder_->Redraw();
    impl->StopResizeDrag();

    return false;
  }

  void BeginResizeDrag(int button, ViewInterface::HitTest hittest) {
    // Determine the resize drag edge.
    resize_width_mode_ = 0;
    resize_height_mode_ = 0;
    if (hittest == ViewInterface::HT_LEFT) {
      resize_width_mode_ = -1;
    } else if (hittest == ViewInterface::HT_RIGHT) {
      resize_width_mode_ = 1;
    } else if (hittest == ViewInterface::HT_TOP) {
      resize_height_mode_ = -1;
    } else if (hittest == ViewInterface::HT_BOTTOM) {
      resize_height_mode_ = 1;
    } else if (hittest == ViewInterface::HT_TOPLEFT) {
      resize_height_mode_ = -1;
      resize_width_mode_ = -1;
    } else if (hittest == ViewInterface::HT_TOPRIGHT) {
      resize_height_mode_ = -1;
      resize_width_mode_ = 1;
    } else if (hittest == ViewInterface::HT_BOTTOMLEFT) {
      resize_height_mode_ = 1;
      resize_width_mode_ = -1;
    } else if (hittest == ViewInterface::HT_BOTTOMRIGHT) {
      resize_height_mode_ = 1;
      resize_width_mode_ = 1;
    } else {
      // Unsupported hittest;
      return;
    }

    if (on_begin_resize_drag_signal_(button, hittest)) {
      resize_width_mode_ = 0;
      resize_height_mode_ = 0;
      return;
    }

    resize_view_zoom_ = view_->GetGraphics()->GetZoom();
    resize_view_width_ = view_->GetWidth();
    resize_view_height_ = view_->GetHeight();

    clutter_actor_get_position(actor_, &win_x_, &win_y_);
    clutter_actor_get_size(actor_, &win_width_, &win_height_);
    resize_win_x_ = win_x_;
    resize_win_y_ = win_y_;
    resize_win_width_ = win_width_;
    resize_win_height_ = win_height_;
    resize_button_ = button;


    resize_mouse_x_ = binder_->GetPointerX();
    resize_mouse_y_ = binder_->GetPointerY();

    // For resize drag.
    motion_handler_id_ = g_signal_connect(G_OBJECT(actor_), "motion-event",
                                          G_CALLBACK(ResizeMotionNotifyHandler),
                                          this);
    button_release_handler_id_ = g_signal_connect(G_OBJECT(actor_),
                                                  "button-release-event",
                                                  G_CALLBACK(ResizeButtonReleaseHandler), this);

    // Raise the actor to the top
    clutter_actor_raise_top(actor_);

    binder_->FreezeUpdates();

    // Redirect everything to this actor
    clutter_grab_pointer(actor_);
  }

  void StopResizeDrag() {
    if (resize_width_mode_ || resize_height_mode_) {
      resize_width_mode_ = 0;
      resize_height_mode_ = 0;
      // QueueResize();
      on_end_resize_drag_signal_();
      SetCursor(ViewInterface::CURSOR_DEFAULT);
    }
  }

  static gboolean MotionNotifyHandler(ClutterActor *actor,
                                      ClutterMotionEvent *event,
                                      gpointer userdata) {
    Impl *impl = reinterpret_cast<Impl *>(userdata);

    if (impl->move_dragging_) {
      int x = event->x - impl->drag_offset_x_;
      int y = event->y - impl->drag_offset_y_;

      clutter_actor_set_position (impl->actor_, x, y);
      impl->on_moved_signal_ (x, y);
      impl->win_x_ = x;
      impl->win_y_ = y;
    }

    return false;
  }

  static gboolean ButtonReleaseHandler(ClutterActor *actor,
                                       ClutterButtonEvent *event,
                                       gpointer userdata) {
    Impl *impl = reinterpret_cast<Impl *>(userdata);

    g_signal_handler_disconnect(actor, impl->motion_handler_id_);
    g_signal_handler_disconnect(actor, impl->button_release_handler_id_);

    clutter_ungrab_pointer();
    if (impl->move_dragging_) {
      impl->move_dragging_ = false;
      impl->StopMoveDrag ();
    }
    if (impl->resize_width_mode_ || impl->resize_height_mode_) {
      impl->StopResizeDrag();
      return TRUE;
    }
    return FALSE;
  }

  void BeginMoveDrag(int button) {
    DLOG("Start move dragging.");
    if (on_begin_move_drag_signal_(button))
      return;

    move_dragging_ = true;

    gfloat stage_x, stage_y;
    clutter_actor_get_position(actor_, &stage_x, &stage_y);
    drag_offset_x_ = binder_->GetPointerX() - stage_x;
    drag_offset_y_ = binder_->GetPointerY() - stage_y;

    // For move drag.
    motion_handler_id_ = g_signal_connect(G_OBJECT(actor_), "motion-event",
                                          G_CALLBACK(MotionNotifyHandler),
                                          this);
    button_release_handler_id_ = g_signal_connect(G_OBJECT(actor_),
                                                  "button-release-event",
                                                  G_CALLBACK(ButtonReleaseHandler), this);

    // Raise the actor to the top
    clutter_actor_raise_top(actor_);

    // Redirect everything to this actor
    clutter_grab_pointer(actor_);
  }

  void StopMoveDrag() {
    DLOG("Stop move dragging.");

    on_end_move_drag_signal_();
    SaveWindowStates (true, false);
  }

  void EnsureInsideScreen() {
//    GdkScreen *screen = gtk_widget_get_screen(window_);
//    int screen_width = gdk_screen_get_width(screen);
//    int screen_height = gdk_screen_get_height(screen);
//    int win_center_x = win_x_ + win_width_ / 2;
//    int win_center_y = win_y_ + win_width_ / 2;
//
//    if (win_center_x < 0 || win_center_x >= screen_width ||
//        win_center_y < 0 || win_center_y >= screen_height) {
//      DLOG("View is out of screen: sw: %d, sh: %d, x: %d, y: %d",
//           screen_width, screen_height, win_center_x, win_center_y);
//      win_x_ = (screen_width - win_width_) / 2;
//      win_y_ = (screen_height - win_height_) / 2;
//      gtk_window_move(GTK_WINDOW(window_), win_x_, win_y_);
//    }
  }

  // gtk signal handlers.
  static gboolean FocusInHandler(ClutterActor *actor,
                                 gpointer user_data) {
    DLOG("FocusInHandler(%p)", actor);
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    if (impl->move_dragging_)
      impl->StopMoveDrag();
    return FALSE;
  }

#ifdef _DEBUG
  static gboolean FocusOutHandler(ClutterActor *actor,
                                  gpointer user_data) {
    DLOG("FocusOutHandler(%p)", actor);
    return FALSE;
  }
#endif

  static gboolean EnterNotifyHandler(ClutterActor *actor,
                                     ClutterCrossingEvent *event,
                                     gpointer user_data) {
    DLOG("EnterNotifyHandler(%p)", actor);
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    if (event->type == CLUTTER_ENTER) {
      if (impl->move_dragging_)
        impl->StopMoveDrag();
      return FALSE;
    } else {
      impl->SetCursor (ViewInterface::CURSOR_DEFAULT, false);
      return FALSE;
    }
  }

  static void ActorShowHandler(ClutterActor *actor, gpointer user_data) {
    DLOG("View window is going to be shown.");
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    if (impl->view_ && impl->enable_signals_)
      impl->on_show_hide_signal_(true);
  }

  static void ActorHideHandler(ClutterActor* actor, gpointer user_data) {
    DLOG("View window is going to be hidden.");
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    if (impl->view_ && impl->enable_signals_) {
      impl->on_show_hide_signal_(false);

      if (impl->feedback_handler_ &&
          impl->type_ == ViewHostInterface::VIEW_HOST_DETAILS) {
        (*impl->feedback_handler_)(ViewInterface::DETAILS_VIEW_FLAG_NONE);
        delete impl->feedback_handler_;
        impl->feedback_handler_ = NULL;
      } else if (impl->type_ == ViewHostInterface::VIEW_HOST_MAIN &&
                 (impl->flags_ & REMOVE_ON_CLOSE) && impl->view_->GetGadget()) {
        impl->view_->GetGadget()->RemoveMe(true);
      }
    }
  }

  SingleViewHost *owner_;
  ViewInterface *view_;
  ClutterActor *actor_;
  guint motion_handler_id_;
  guint button_release_handler_id_;

  Tooltip *tooltip_;
  ViewActorBinder *binder_;

  ViewHostInterface::Type type_;
  double initial_zoom_;
  int flags_;
  int debug_mode_;

  int stop_move_drag_source_;

  gfloat win_x_;
  gfloat win_y_;
  gfloat win_width_;
  gfloat win_height_;

  // For resize drag
  double resize_view_zoom_;
  double resize_view_width_;
  double resize_view_height_;

  int resize_win_x_;
  int resize_win_y_;
  int resize_win_width_;
  int resize_win_height_;

  int resize_button_;
  gdouble resize_mouse_x_;
  gdouble resize_mouse_y_;

  // -1 to resize left, 1 to resize right.
  int resize_width_mode_;
  // -1 to resize top, 1 to resize bottom.
  int resize_height_mode_;
  // End of resize drag variants.

  bool is_keep_above_;
  bool move_dragging_;
  int drag_offset_x_;
  int drag_offset_y_;

  bool enable_signals_;

  Slot1<bool, int> *feedback_handler_;
  bool can_close_dialog_; // Only useful when a model dialog is
                          // running.

  ClutterActor* main_group_;

  Signal0<void> on_view_changed_signal_;
  Signal1<void, bool> on_show_hide_signal_;

  Signal2<bool, int, int> on_begin_resize_drag_signal_;
  Signal2<void, int, int> on_resized_signal_;
  Signal0<void> on_end_resize_drag_signal_;

  Signal1<bool, int> on_begin_move_drag_signal_;
  Signal2<void, int, int> on_moved_signal_;
  Signal0<void> on_end_move_drag_signal_;

  static const unsigned int kShowTooltipDelay = 500;
  static const unsigned int kHideTooltipDelay = 4000;
};

SingleViewHost::SingleViewHost(ViewHostInterface::Type type,
                               double zoom, int flags, int debug_mode,
                               ClutterActor* main_group)
  : impl_(new Impl(this, type, zoom, flags, debug_mode, main_group)) {
}

SingleViewHost::~SingleViewHost() {
  DLOG("SingleViewHost Dtor: %p", this);
  delete impl_;
  impl_ = NULL;
}

ViewHostInterface::Type SingleViewHost::GetType() const {
  return impl_->type_;
}

void SingleViewHost::Destroy() {
  delete this;
}

void SingleViewHost::SetView(ViewInterface *view) {
  impl_->SetView(view);
}

ViewInterface *SingleViewHost::GetView() const {
  return impl_->view_;
}

void *SingleViewHost::GetNativeWidget() const {
  return impl_->actor_;
}

void SingleViewHost::ViewCoordToNativeWidgetCoord(
    double x, double y, double *widget_x, double *widget_y) const {
  impl_->ViewCoordToNativeWidgetCoord(x, y, widget_x, widget_y);
}

void SingleViewHost::NativeWidgetCoordToViewCoord(
    double x, double y, double *view_x, double *view_y) const {
  impl_->NativeWidgetCoordToViewCoord(x, y, view_x, view_y);
}

void SingleViewHost::QueueDraw() {
  impl_->QueueDraw();
}

void SingleViewHost::QueueResize() {
  impl_->QueueResize();
}

void SingleViewHost::EnableInputShapeMask(bool enable) {
}

void SingleViewHost::SetResizable(ViewInterface::ResizableMode mode) {
  impl_->SetResizable(mode);
}

void SingleViewHost::SetCaption(const std::string &caption) {
  impl_->SetCaption(caption);
}

void SingleViewHost::SetShowCaptionAlways(bool always) {
  impl_->SetShowCaptionAlways(always);
}

void SingleViewHost::SetCursor(ViewInterface::CursorType type) {
  impl_->SetCursor(type);
}

void SingleViewHost::ShowTooltip(const std::string &tooltip) {
  impl_->ShowTooltip(tooltip);
}

void SingleViewHost::ShowTooltipAtPosition(const std::string &tooltip,
                                           double x, double y) {
  impl_->ShowTooltipAtPosition(tooltip, x, y);
}

bool SingleViewHost::ShowView(bool modal, int flags,
                              Slot1<bool, int> *feedback_handler) {
  return impl_->ShowView(modal, flags, feedback_handler);
}

void SingleViewHost::CloseView() {
  impl_->CloseView();
}

bool SingleViewHost::ShowContextMenu(int button) {
  return impl_->ShowContextMenu(button);
}

void SingleViewHost::BeginResizeDrag(int button,
                                     ViewInterface::HitTest hittest) {
  impl_->BeginResizeDrag(button, hittest);
}

void SingleViewHost::BeginMoveDrag(int button) {
  impl_->BeginMoveDrag(button);
}

void SingleViewHost::Alert(const ViewInterface *view, const char *message) {
  ShowAlertDialog(view->GetCaption().c_str(), message);
}

ViewHostInterface::ConfirmResponse SingleViewHost::Confirm(
    const ViewInterface *view, const char *message, bool cancel_button) {
  return ShowConfirmDialog(view->GetCaption().c_str(), message, cancel_button);
}

std::string SingleViewHost::Prompt(const ViewInterface *view,
                                   const char *message,
                                   const char *default_value) {
  return ShowPromptDialog(view->GetCaption().c_str(), message, default_value);
}

int SingleViewHost::GetDebugMode() const {
  return impl_->debug_mode_;
}

GraphicsInterface *SingleViewHost::NewGraphics() const {
  return new CairoGraphics(impl_->initial_zoom_);
}

ClutterActor *SingleViewHost::GetActor() const {
  return impl_->actor_;
}

void SingleViewHost::GetActorPosition(int *x, int *y) const {
  gfloat x0, y0;
  clutter_actor_get_position (impl_->actor_, &x0, &y0);
  if (x) *x = (int)x0;
  if (y) *y = (int)y0;
}

void SingleViewHost::SetActorPosition(int x, int y) {
  impl_->SetActorPosition(x, y);
}

void SingleViewHost::GetActorSize(int *width, int *height) const {
  gfloat width0, height0;
  clutter_actor_get_size (impl_->actor_, &width0, &height0);
  if (width) *width = (int)width0;
  if (height) *height = (int)height0;
}

bool SingleViewHost::IsKeepAbove() const {
  return impl_->is_keep_above_;
}

void SingleViewHost::SetKeepAbove(bool keep_above) {
  impl_->SetKeepAbove(keep_above);
}

bool SingleViewHost::IsVisible() const {
  return CLUTTER_ACTOR_IS_VISIBLE(impl_->actor_);
}

Connection *SingleViewHost::ConnectOnViewChanged(Slot0<void> *slot) {
  return impl_->on_view_changed_signal_.Connect(slot);
}

Connection *SingleViewHost::ConnectOnShowHide(Slot1<void, bool> *slot) {
  return impl_->on_show_hide_signal_.Connect(slot);
}

Connection *SingleViewHost::ConnectOnBeginMoveDrag(Slot1<bool, int> *slot) {
  return impl_->on_begin_move_drag_signal_.Connect(slot);
}

Connection *SingleViewHost::ConnectOnMoved(Slot2<void, int, int> *slot) {
  return impl_->on_moved_signal_.Connect(slot);
}

Connection *SingleViewHost::ConnectOnEndMoveDrag(Slot0<void> *slot) {
  return impl_->on_end_move_drag_signal_.Connect(slot);
}

} // namespace clutter
} // namespace ggadget
