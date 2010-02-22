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

#include <clutter/clutter.h>

#include <ggadget/main_loop_interface.h>
#include <ggadget/common.h>
#include <ggadget/logger.h>
#include "tooltip.h"

namespace ggadget {
namespace clutter {

class Tooltip::Impl {
 public:
  Impl(int show_timeout, int hide_timeout)
    : tooltip_(clutter_group_new()),
      label_(clutter_text_new()),
      show_timeout_(show_timeout),
      hide_timeout_(hide_timeout),
      show_timer_(0),
      hide_timer_(0) {
    ClutterColor color = { 0xff, 0xff, 0xb0, 0xaa };
    background_ = clutter_rectangle_new_with_color(&color);

    clutter_container_add_actor(CLUTTER_CONTAINER (tooltip_), background_);
    clutter_actor_show(background_);

    clutter_container_add_actor(CLUTTER_CONTAINER (tooltip_), label_);
    clutter_actor_show(label_);

    clutter_container_add_actor(CLUTTER_CONTAINER (clutter_stage_get_default()),
                                tooltip_);
  }

  ~Impl() {
    RemoveTimers();
    clutter_actor_destroy(tooltip_);
  }

  void AdjustAndShowWidget(int x, int y, bool center) {
    gfloat width, height;
    clutter_actor_get_size(tooltip_, &width, &height);

    if (center)
      x -= width / 2;

    y += 20;

    clutter_actor_set_position (tooltip_, x, y);
    clutter_actor_show_all(tooltip_);
  }

  bool DelayedShow(int watch_id) {
    int x = 0, y = 0;
    AdjustAndShowWidget(x, y, false);
    show_timer_ = 0;
    return false;
  }

  bool DelayedHide(int watch_id) {
    clutter_actor_hide(tooltip_);
    hide_timer_ = 0;
    return false;
  }

  void RemoveTimers() {
    if (show_timer_) {
      GetGlobalMainLoop()->RemoveWatch(show_timer_);
      show_timer_ = 0;
    }
    if (hide_timer_) {
      GetGlobalMainLoop()->RemoveWatch(hide_timer_);
      hide_timer_ = 0;
    }
  }

  static void
  set_text_and_size(Impl *impl,
                    const char *text) {
    clutter_text_set_text(CLUTTER_TEXT(impl->label_), text);

    gfloat width, height;
    clutter_actor_get_size(impl->label_, &width, &height);
    clutter_actor_set_size(impl->background_, width, height);
  }

  void Show(const char *tooltip) {
    Hide();
    if (tooltip && *tooltip) {
      set_text_and_size(this, tooltip);
      if (show_timeout_ > 0) {
        show_timer_ = GetGlobalMainLoop()->AddTimeoutWatch(
            show_timeout_,
            new WatchCallbackSlot(NewSlot(this, &Impl::DelayedShow)));
      } else {
        DelayedShow(0);
      }

      if (hide_timeout_ > 0) {
        hide_timer_ = GetGlobalMainLoop()->AddTimeoutWatch(
            hide_timeout_,
            new WatchCallbackSlot(NewSlot(this, &Impl::DelayedHide)));
      }
    }
  }

  void ShowAtPosition(const char *tooltip,
                      int x, int y) {
    Hide();
    if (tooltip && *tooltip) {
      set_text_and_size(this, tooltip);
      AdjustAndShowWidget(x, y, true);

      if (hide_timeout_ > 0) {
        hide_timer_ = GetGlobalMainLoop()->AddTimeoutWatch(
            hide_timeout_,
            new WatchCallbackSlot(NewSlot(this, &Impl::DelayedHide)));
      }
    }
  }

  void Hide() {
    RemoveTimers();
    clutter_actor_hide(tooltip_);
  }

  ClutterActor *tooltip_;
  ClutterActor *label_;
  ClutterActor *background_;
  int show_timeout_;
  int hide_timeout_;
  int show_timer_;
  int hide_timer_;
};

Tooltip::Tooltip(int show_timeout, int hide_timeout)
  : impl_(new Impl(show_timeout, hide_timeout)) {
}

Tooltip::~Tooltip() {
  delete impl_;
  impl_ = NULL;
}

void Tooltip::Show(const char *tooltip) {
  impl_->Show(tooltip);
}

void Tooltip::ShowAtPosition(const char *tooltip,
                             int x, int y) {
  impl_->ShowAtPosition(tooltip, x, y);
}

void Tooltip::Hide() {
  impl_->Hide();
}

} // namespace gtk
} // namespace ggadget
