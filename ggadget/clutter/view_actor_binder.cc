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
#include <string>
#include <vector>
#include <cairo.h>

#include <clutter/clutter.h>

#include "view_actor_binder.h"
#include <ggadget/common.h>
#include <ggadget/logger.h>
#include <ggadget/event.h>
#include <ggadget/main_loop_interface.h>
#include <ggadget/view_interface.h>
#include <ggadget/view_host_interface.h>
#include <ggadget/clip_region.h>
#include <ggadget/math_utils.h>
#include <ggadget/string_utils.h>
#include "cairo_canvas.h"
#include "cairo_graphics.h"
#include "key_convert.h"
#include "utilities.h"

#define GRAB_POINTER_EXPLICITLY
namespace ggadget {
namespace clutter {

// A small motion threshold to prevent a click with tiny mouse move from being
// treated as window move or resize.
static const double kDragThreshold = 3;

// Minimal interval between self draws.
static const unsigned int kSelfDrawInterval = 40;

class ViewActorBinder::Impl {
 public:
  Impl(ViewInterface *view,
       ViewHostInterface *host, ClutterActor *actor)
    : view_(view),
      host_(host),
      actor_(actor),
      handlers_(new gulong[kEventHandlersNum]),
      on_zoom_connection_(NULL),
      dbl_click_(false),
      focused_(false),
      button_pressed_(false),
#ifdef GRAB_POINTER_EXPLICITLY
      pointer_grabbed_(false),
#endif
      zoom_(1.0),
      mouse_down_x_(-1),
      mouse_down_y_(-1),
      mouse_down_hittest_(ViewInterface::HT_CLIENT),
      self_draw_(false),
      self_draw_timer_(0),
      last_self_draw_time_(0),
      freeze_updates_(false) {
    ASSERT(view);
    ASSERT(host);

    g_object_ref(G_OBJECT(actor_));

    for (size_t i = 0; i < kEventHandlersNum; ++i) {
      handlers_[i] = g_signal_connect(G_OBJECT(actor_),
                                      kEventHandlers[i].event,
                                      kEventHandlers[i].handler,
                                      this);
    }

    CairoGraphics *gfx= down_cast<CairoGraphics *>(view_->GetGraphics());
    ASSERT(gfx);

    zoom_ = gfx->GetZoom();
    on_zoom_connection_ = gfx->ConnectOnZoom(NewSlot(this, &Impl::OnZoom));
  }

  ~Impl() {
    view_ = NULL;

    if (self_draw_timer_) {
      g_source_remove(self_draw_timer_);
      self_draw_timer_ = 0;
    }

    for (size_t i = 0; i < kEventHandlersNum; ++i) {
      if (handlers_[i] > 0)
        g_signal_handler_disconnect(G_OBJECT(actor_), handlers_[i]);
      else
        DLOG("Handler %s was not connected.", kEventHandlers[i].event);
    }

    delete[] handlers_;
    handlers_ = NULL;

    if (on_zoom_connection_) {
      on_zoom_connection_->Disconnect();
      on_zoom_connection_ = NULL;
    }

    g_object_unref(G_OBJECT(actor_));
  }

  void OnZoom(double zoom) {
    zoom_ = zoom;
  }

  static gboolean ButtonPressHandler(ClutterActor *actor,
                                     ClutterButtonEvent *event,
                                     gpointer user_data) {
    DLOG("ButtonPressHandler.");
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    EventResult result = EVENT_RESULT_UNHANDLED;

    impl->button_pressed_ = true;
    impl->host_->ShowTooltip("");

    if (!impl->focused_) {
      impl->focused_ = true;
      SimpleEvent e(Event::EVENT_FOCUS_IN);
      // Ignore the result.
      impl->view_->OnOtherEvent(e);
    }

    // This will break I suppose if there's more than one stage
    clutter_stage_set_key_focus (CLUTTER_STAGE(clutter_stage_get_default()),
                                 actor);

    int mod = ConvertClutterModifierToModifier(event->modifier_state);
    int button = event->button == 1 ? MouseEvent::BUTTON_LEFT :
                 event->button == 2 ? MouseEvent::BUTTON_MIDDLE :
                 event->button == 3 ? MouseEvent::BUTTON_RIGHT :
                                      MouseEvent::BUTTON_NONE;

    gfloat actor_x, actor_y;
    clutter_actor_transform_stage_point(actor, event->x,
                                        event->y,
                                        &actor_x, &actor_y);
    Event::Type type = Event::EVENT_INVALID;
    if (event->click_count == 1) {
      type = Event::EVENT_MOUSE_DOWN;
      impl->mouse_down_x_ = actor_x;
      impl->mouse_down_y_ = actor_y;
    } else if (event->click_count == 2) {
      impl->dbl_click_ = true;
      if (button == MouseEvent::BUTTON_LEFT)
        type = Event::EVENT_MOUSE_DBLCLICK;
      else if (button == MouseEvent::BUTTON_RIGHT)
        type = Event::EVENT_MOUSE_RDBLCLICK;
    }

    if (button != MouseEvent::BUTTON_NONE && type != Event::EVENT_INVALID) {
      MouseEvent e(type, (actor_x) / impl->zoom_,
                   (actor_y) / impl->zoom_,
                   0, 0, button, mod, event);

      result = impl->view_->OnMouseEvent(e);

      impl->mouse_down_hittest_ = impl->view_->GetHitTest();
      // If the View's hittest represents a special button, then handle it
      // here.
      if (result == EVENT_RESULT_UNHANDLED &&
          button == MouseEvent::BUTTON_LEFT &&
          type == Event::EVENT_MOUSE_DOWN) {
        if (impl->mouse_down_hittest_ == ViewInterface::HT_MENU) {
          impl->host_->ShowContextMenu(button);
        } else if (impl->mouse_down_hittest_ == ViewInterface::HT_CLOSE) {
          impl->host_->CloseView();
        }
        result = EVENT_RESULT_HANDLED;
      }
    }

    return result != EVENT_RESULT_UNHANDLED;
  }

  static gboolean ButtonReleaseHandler(ClutterActor *actor,
                                       ClutterButtonEvent *event,
                                       gpointer user_data) {
    DLOG("ButtonReleaseHandler.");
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    EventResult result = EVENT_RESULT_UNHANDLED;
    EventResult result2 = EVENT_RESULT_UNHANDLED;

    if (impl->button_pressed_ == false)
      return false;

    impl->button_pressed_ = false;
    impl->host_->ShowTooltip("");

#ifdef GRAB_POINTER_EXPLICITLY
    if (impl->pointer_grabbed_) {
      clutter_ungrab_pointer();
      impl->pointer_grabbed_ = false;
    }
#endif

    int mod = ConvertClutterModifierToModifier(event->modifier_state);
    int button = event->button == 1 ? MouseEvent::BUTTON_LEFT :
                 event->button == 2 ? MouseEvent::BUTTON_MIDDLE :
                 event->button == 3 ? MouseEvent::BUTTON_RIGHT :
                                      MouseEvent::BUTTON_NONE;

    gfloat actor_x, actor_y;
    clutter_actor_transform_stage_point(actor, (event->x),
                                        (event->y),
                                        &actor_x, &actor_y);
    if (button != MouseEvent::BUTTON_NONE) {
      MouseEvent e(Event::EVENT_MOUSE_UP,
                   (actor_x) / impl->zoom_,
                   (actor_y) / impl->zoom_,
                   0, 0, button, mod, event);
      result = impl->view_->OnMouseEvent(e);

      if (!impl->dbl_click_) {
        MouseEvent e2(button == MouseEvent::BUTTON_LEFT ?
                      Event::EVENT_MOUSE_CLICK : Event::EVENT_MOUSE_RCLICK,
                      (actor_x) / impl->zoom_,
                      (actor_y) / impl->zoom_,
                      0, 0, button, mod);
        result2 = impl->view_->OnMouseEvent(e2);
      } else {
        impl->dbl_click_ = false;
      }
    }

    impl->mouse_down_x_ = -1;
    impl->mouse_down_y_ = -1;
    impl->mouse_down_hittest_ = ViewInterface::HT_CLIENT;

    return result != EVENT_RESULT_UNHANDLED ||
           result2 != EVENT_RESULT_UNHANDLED;
  }

  static gboolean KeyPressHandler(ClutterActor *actor,
                                  ClutterKeyEvent *event,
                                  gpointer user_data) {
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    EventResult result = EVENT_RESULT_UNHANDLED;
    EventResult result2 = EVENT_RESULT_UNHANDLED;

    impl->host_->ShowTooltip("");

    int mod = ConvertClutterModifierToModifier(event->modifier_state);
    unsigned int key_code = ConvertClutterKeyvalToKeyCode(event->keyval);
    if (key_code) {
      KeyboardEvent e(Event::EVENT_KEY_DOWN, key_code, mod, event);
      result = impl->view_->OnKeyEvent(e);
    } else {
      LOG("Unknown key: 0x%x", event->keyval);
    }

    guint32 key_char = 0;
    if ((event->modifier_state & (CLUTTER_CONTROL_MASK | CLUTTER_MOD1_MASK)) == 0) {
      if (key_code == KeyboardEvent::KEY_ESCAPE ||
          key_code == KeyboardEvent::KEY_RETURN ||
          key_code == KeyboardEvent::KEY_BACK ||
          key_code == KeyboardEvent::KEY_TAB) {
        // gdk_keyval_to_unicode doesn't support the above keys.
        key_char = key_code;
      } else {
        key_char = clutter_keysym_to_unicode(event->keyval);
      }
    } else if ((event->modifier_state & CLUTTER_CONTROL_MASK) &&
               key_code >= 'A' && key_code <= 'Z') {
      // Convert CTRL+(A to Z) to key press code for compatibility.
      key_char = key_code - 'A' + 1;
    }

    if (key_char) {
      // Send the char code in KEY_PRESS event.
      KeyboardEvent e2(Event::EVENT_KEY_PRESS, key_char, mod, event);
      result2 = impl->view_->OnKeyEvent(e2);
    }

    return result != EVENT_RESULT_UNHANDLED ||
           result2 != EVENT_RESULT_UNHANDLED;
  }

  static gboolean KeyReleaseHandler(ClutterActor *actor,
                                    ClutterKeyEvent *event,
                                    gpointer user_data) {
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    EventResult result = EVENT_RESULT_UNHANDLED;

    int mod = ConvertClutterModifierToModifier(event->modifier_state);
    unsigned int key_code = ConvertClutterKeyvalToKeyCode(event->keyval);
    if (key_code) {
      KeyboardEvent e(Event::EVENT_KEY_UP, key_code, mod, event);
      result = impl->view_->OnKeyEvent(e);
    } else {
      LOG("Unknown key: 0x%x", event->keyval);
    }

    return result != EVENT_RESULT_UNHANDLED;
  }

  void Redraw() {
    if (freeze_updates_)
      return;
    view_->Layout ();
    cairo_t *cr = NULL;

    const ClipRegion *view_region = view_->GetClipRegion();
    size_t count = view_region->GetRectangleCount();
    DLOG(" == actor update region == ");
    view_region->PrintLog();
    if (count) {
      Rectangle rect, union_rect;
      for (size_t i = 0; i < count; ++i) {
        rect = view_region->GetRectangle(i);
        if (zoom_ != 1.0) {
          rect.Zoom(zoom_);
          rect.Integerize(true);
        }
        union_rect.Union (rect);
      }
      cr = clutter_cairo_texture_create_region (CLUTTER_CAIRO_TEXTURE(actor_),
                                                union_rect.x, union_rect.y,
                                                union_rect.w, union_rect.h);

    }else{
      cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE(actor_));
    }
    if (!cr)
      return;

    cairo_operator_t op = cairo_get_operator(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, op);

    CairoCanvas *canvas = new CairoCanvas(cr,
                                          view_->GetGraphics()->GetZoom(),
                                          view_->GetWidth(),
                                          view_->GetHeight());
    DLOG ("redraw");
    view_->Draw(canvas);

    canvas->Destroy();
    cairo_destroy(cr);
  }

  static gboolean MotionNotifyHandler(ClutterActor *actor,
                                      ClutterMotionEvent *event,
                                      gpointer user_data) {
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    int button = ConvertClutterModifierToButton(event->modifier_state);
    int mod = ConvertClutterModifierToModifier(event->modifier_state);

    gfloat actor_x, actor_y;
    clutter_actor_transform_stage_point(actor, (event->x),
                                        (event->y),
                                        &actor_x, &actor_y);
    MouseEvent e(Event::EVENT_MOUSE_MOVE,
                 (actor_x) / impl->zoom_,
                 (actor_y) / impl->zoom_,
                 0, 0, button, mod);
#ifdef GRAB_POINTER_EXPLICITLY
    if (button != MouseEvent::BUTTON_NONE && !impl->pointer_grabbed_) {
      // Grab the cursor to prevent losing events.
      clutter_grab_pointer(actor);
      impl->pointer_grabbed_ = true;
    }
#endif

    EventResult result = impl->view_->OnMouseEvent(e);

    if (result == EVENT_RESULT_UNHANDLED && button != MouseEvent::BUTTON_NONE &&
        impl->mouse_down_x_ >= 0 && impl->mouse_down_y_ >= 0 &&
        (std::abs((actor_x) - impl->mouse_down_x_) > kDragThreshold ||
         std::abs((actor_y) - impl->mouse_down_y_) > kDragThreshold ||
         impl->mouse_down_hittest_ != ViewInterface::HT_CLIENT)) {
      impl->button_pressed_ = false;
      // Send fake mouse up event to the view so that we can start to drag
      // the window. Note: no mouse click event is sent in this case, to prevent
      // unwanted action after window move.
      MouseEvent e(Event::EVENT_MOUSE_UP,
                   (actor_x) / impl->zoom_,
                   (actor_y) / impl->zoom_,
                   0, 0, button, mod);

      // Ignore the result of this fake event.
      impl->view_->OnMouseEvent(e);

      ViewInterface::HitTest hittest = impl->mouse_down_hittest_;
      bool resize_drag = false;
      // Determine the resize drag edge.
      if (hittest == ViewInterface::HT_LEFT ||
          hittest == ViewInterface::HT_RIGHT ||
          hittest == ViewInterface::HT_TOP ||
          hittest == ViewInterface::HT_BOTTOM ||
          hittest == ViewInterface::HT_TOPLEFT ||
          hittest == ViewInterface::HT_TOPRIGHT ||
          hittest == ViewInterface::HT_BOTTOMLEFT ||
          hittest == ViewInterface::HT_BOTTOMRIGHT) {
        resize_drag = true;
      }

#ifdef GRAB_POINTER_EXPLICITLY
      // ungrab the pointer before starting move/resize drag.
      if (impl->pointer_grabbed_) {
        clutter_ungrab_pointer();
        impl->pointer_grabbed_ = false;
      }
#endif

      impl->mouse_down_stage_x_ = event->x;
      impl->mouse_down_stage_y_ = event->y;

      if (resize_drag) {
        impl->host_->BeginResizeDrag(button, hittest);
      } else {
        impl->host_->BeginMoveDrag(button);
      }

      impl->mouse_down_x_ = -1;
      impl->mouse_down_y_ = -1;
      impl->mouse_down_hittest_ = ViewInterface::HT_CLIENT;
      impl->mouse_down_stage_x_ = 0;
      impl->mouse_down_stage_y_ = 0;
    }

    return result != EVENT_RESULT_UNHANDLED;
  }

  static gboolean ScrollHandler(ClutterActor *actor,
                                ClutterScrollEvent *event,
                                gpointer user_data) {
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    int delta_x = 0, delta_y = 0;
    if (event->direction == CLUTTER_SCROLL_UP) {
      delta_y = MouseEvent::kWheelDelta;
    } else if (event->direction == CLUTTER_SCROLL_DOWN) {
      delta_y = -MouseEvent::kWheelDelta;
    } else if (event->direction == CLUTTER_SCROLL_LEFT) {
      delta_x = MouseEvent::kWheelDelta;
    } else if (event->direction == CLUTTER_SCROLL_RIGHT) {
      delta_x = -MouseEvent::kWheelDelta;
    }

    gfloat actor_x, actor_y;
    clutter_actor_transform_stage_point(actor, event->x,
                                        event->y,
                                        &actor_x, &actor_y);
    MouseEvent e(Event::EVENT_MOUSE_WHEEL,
                 (actor_x) / impl->zoom_,
                 (actor_y) / impl->zoom_,
                 delta_x, delta_y,
                 ConvertClutterModifierToButton(event->modifier_state),
                 ConvertClutterModifierToModifier(event->modifier_state));
    return impl->view_->OnMouseEvent(e) != EVENT_RESULT_UNHANDLED;
  }

  static gboolean LeaveNotifyHandler(ClutterActor *actor,
                                     ClutterCrossingEvent *event,
                                     gpointer user_data) {
    Impl *impl = reinterpret_cast<Impl *>(user_data);

    // Don't send mouse out event if the mouse is grabbed.
    if (impl->button_pressed_)
      return FALSE;

    impl->host_->ShowTooltip("");

    gfloat actor_x, actor_y;
    clutter_actor_transform_stage_point(actor, (event->x),
                                        (event->y),
                                        &actor_x, &actor_y);
    MouseEvent e(Event::EVENT_MOUSE_OUT,
                 (actor_x) / impl->zoom_,
                 (actor_y) / impl->zoom_, 0, 0,
                 MouseEvent::BUTTON_NONE,
                 Event::MOD_NONE);
    return impl->view_->OnMouseEvent(e) != EVENT_RESULT_UNHANDLED;
  }

  static gboolean EnterNotifyHandler(ClutterActor *actor,
                                     ClutterCrossingEvent *event,
                                     gpointer user_data) {
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    impl->host_->ShowTooltip("");

    gfloat actor_x, actor_y;
    clutter_actor_transform_stage_point(actor, (event->x),
                                        (event->y),
                                        &actor_x, &actor_y);
    MouseEvent e(Event::EVENT_MOUSE_OVER,
                 (actor_x) / impl->zoom_,
                 (actor_y) / impl->zoom_, 0, 0,
                 MouseEvent::BUTTON_NONE,
                 Event::MOD_NONE);
    return impl->view_->OnMouseEvent(e) != EVENT_RESULT_UNHANDLED;
  }

  static gboolean FocusInHandler(ClutterActor *actor,
                                 gpointer user_data) {
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    if (!impl->focused_) {
      impl->focused_ = true;
      SimpleEvent e(Event::EVENT_FOCUS_IN);
      return impl->view_->OnOtherEvent(e) != EVENT_RESULT_UNHANDLED;
    }
    return FALSE;
  }

  static gboolean FocusOutHandler(ClutterActor *actor,
                                  gpointer user_data) {
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    if (impl->focused_) {
      impl->focused_ = false;
      SimpleEvent e(Event::EVENT_FOCUS_OUT);
#ifdef GRAB_POINTER_EXPLICITLY
      // Ungrab the pointer if the focus is lost.
      if (impl->pointer_grabbed_) {
        clutter_ungrab_pointer();
        impl->pointer_grabbed_ = false;
      }
#endif
      return impl->view_->OnOtherEvent(e) != EVENT_RESULT_UNHANDLED;
    }
    return FALSE;
  }

  static gboolean SelfDrawHandler(gpointer user_data) {
    Impl *impl = reinterpret_cast<Impl *>(user_data);
    impl->self_draw_timer_ = 0;
    impl->Redraw();
    return FALSE;
  }

  ViewInterface *view_;
  ViewHostInterface *host_;
  ClutterActor *actor_;
  gulong *handlers_;
  Connection *on_zoom_connection_;
  bool dbl_click_;
  bool focused_;
  bool button_pressed_;
#ifdef GRAB_POINTER_EXPLICITLY
  bool pointer_grabbed_;
#endif
  double zoom_;
  double mouse_down_x_;
  double mouse_down_y_;
  int mouse_down_stage_x_;
  int mouse_down_stage_y_;
  ViewInterface::HitTest mouse_down_hittest_;

  bool self_draw_;
  guint self_draw_timer_;
  uint64_t last_self_draw_time_;
  bool freeze_updates_;

  struct EventHandlerInfo {
    const char *event;
    void (*handler)(void);
  };

  static EventHandlerInfo kEventHandlers[];
  static const size_t kEventHandlersNum;
};

ViewActorBinder::Impl::EventHandlerInfo
ViewActorBinder::Impl::kEventHandlers[] = {
  { "button-press-event", G_CALLBACK(ButtonPressHandler) },
  { "button-release-event", G_CALLBACK(ButtonReleaseHandler) },
  { "enter-event", G_CALLBACK(EnterNotifyHandler) },
  /*  { "expose-event", G_CALLBACK(ExposeHandler) },*/
  { "focus-in", G_CALLBACK(FocusInHandler) },
  { "focus-out", G_CALLBACK(FocusOutHandler) },
  { "key-press-event", G_CALLBACK(KeyPressHandler) },
  { "key-release-event", G_CALLBACK(KeyReleaseHandler) },
  { "leave-event", G_CALLBACK(LeaveNotifyHandler) },
  { "motion-event", G_CALLBACK(MotionNotifyHandler) },
  { "scroll-event", G_CALLBACK(ScrollHandler) },
};

const size_t ViewActorBinder::Impl::kEventHandlersNum =
  arraysize(ViewActorBinder::Impl::kEventHandlers);

ViewActorBinder::ViewActorBinder(ViewInterface *view,
                                   ViewHostInterface *host, ClutterActor *actor)
  : impl_(new Impl(view, host, actor)) {
}

ViewActorBinder::~ViewActorBinder() {
  delete impl_;
  impl_ = NULL;
}

void ViewActorBinder::Redraw() {
  impl_->Redraw();
}

void ViewActorBinder::QueueDraw() {
  if (!impl_->self_draw_timer_) {
    uint64_t current_time = GetCurrentTime();
    if (current_time - impl_->last_self_draw_time_ >= kSelfDrawInterval) {
      impl_->self_draw_timer_ =
          g_idle_add_full(CLUTTER_PRIORITY_REDRAW, Impl::SelfDrawHandler,
                          impl_, NULL);
    } else {
      impl_->self_draw_timer_ =
          g_timeout_add(kSelfDrawInterval -
                        (current_time - impl_->last_self_draw_time_),
                        Impl::SelfDrawHandler, impl_);
    }
  }
}

int ViewActorBinder::GetPointerX() {
  return impl_->mouse_down_stage_x_;
}

int ViewActorBinder::GetPointerY() {
  return impl_->mouse_down_stage_y_;
}

void ViewActorBinder::FreezeUpdates() {
  impl_->freeze_updates_ = true;
}

void ViewActorBinder::ThawUpdates() {
  impl_->freeze_updates_ = false;
}

} // namespace clutter
} // namespace ggadget
