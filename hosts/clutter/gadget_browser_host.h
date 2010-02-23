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

#ifndef HOSTS_CLUTTER_GADGET_BROWSER_HOST_H__
#define HOSTS_CLUTTER_GADGET_BROWSER_HOST_H__

#include <ggadget/gadget_consts.h>
#include <ggadget/host_interface.h>
#include <ggadget/clutter/single_view_host.h>
#include <ggadget/clutter/utilities.h>

#include <clutter/clutter.h>

namespace hosts {
namespace clutter {

// A special Host for Gadget browser to show browser in a decorated window.
class GadgetBrowserHost : public ggadget::HostInterface {
 public:
  GadgetBrowserHost(ggadget::HostInterface *owner, int view_debug_mode)
    : owner_(owner), view_debug_mode_(view_debug_mode) {
  }
  virtual ViewHostInterface *NewViewHost(Gadget *gadget,
                                         ViewHostInterface::Type type) {
    ggadget::clutter::SingleViewHost *svh = new ggadget::clutter::SingleViewHost
      (type, 1.0, 0, view_debug_mode_, main_group_);

    svh->ConnectOnShowHide
      (NewSlot (this, &hosts::clutter::GadgetBrowserHost::OnViewShowHideHandler, svh));

    return svh;
  }
  virtual Gadget *LoadGadget(const char *path, const char *options_name,
                             int instance_id, bool show_debug_console) {
    return owner_->LoadGadget(path, options_name, instance_id,
                              show_debug_console);
  }
  virtual void RemoveGadget(Gadget *gadget, bool save_data) {
    ggadget::GetGadgetManager()->RemoveGadgetInstance(gadget->GetInstanceID());
  }
  virtual bool LoadFont(const char *filename) {
    return owner_->LoadFont(filename);
  }
  virtual void Run() {}
//  virtual void ShowGadgetAboutDialog(Gadget *gadget) {
//    owner_->ShowGadgetAboutDialog(gadget);
//  }
  virtual void ShowGadgetDebugConsole(Gadget *gadget) {}
  virtual int GetDefaultFontSize() { return ggadget::kDefaultFontSize; }
  virtual bool OpenURL(const Gadget *gadget, const char *url) { return false; }

  void SetGadgetGroup(ClutterActor *group) { main_group_ = group; }

  virtual void Exit() {
    owner_->Exit();
  }

  void OnViewShowHideHandler(bool show, ggadget::clutter::SingleViewHost *svh) {
    ClutterActor *actor = svh->GetActor();
    if (actor) {
      //setting the position of gadget control to bottom left
      ClutterActor *stage = clutter_stage_get_default();
      gfloat stage_width, stage_height;
      clutter_actor_get_size (stage, &stage_width, &stage_height);
      clutter_actor_set_anchor_point_from_gravity (actor, CLUTTER_GRAVITY_SOUTH_WEST);
      clutter_actor_set_position (actor, 0, stage_height);

      //animation
      clutter_actor_set_scale (actor, 0.1, 0.1);

      clutter_actor_animate (actor, CLUTTER_EASE_IN_CUBIC, 300,
                             "scale-x", 1.0,
                             "scale-y", 1.0,
                             NULL);
    }
  }

 private:
  ggadget::HostInterface *owner_;
  ClutterActor *main_group_;
  int view_debug_mode_;
};

} // namespace clutter
} // namespace hosts

#endif // HOSTS_CLUTTER_GADGET_BROWSER_HOST_H__
