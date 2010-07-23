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

#ifndef HOSTS_CLUTTER_SIMPLE_CLUTTER_HOST_H__
#define HOSTS_CLUTTER_SIMPLE_CLUTTER_HOST_H__

#include "config.h"

#include <string>
#include <clutter/clutter.h>

#include <ggadget/gadget.h>
#include <ggadget/host_interface.h>
#include <ggadget/options_interface.h>

#ifdef HAVE_MPL
#include <meego-panel/mpl-panel-clutter.h>
#include <meego-panel/mpl-panel-common.h>
#endif

namespace hosts {
namespace clutter {

using ggadget::Gadget;
using ggadget::HostInterface;
using ggadget::ViewHostInterface;;
using ggadget::OptionsInterface;

class SimpleClutterHost : public ggadget::HostInterface {
 public:
  SimpleClutterHost(OptionsInterface *options, double zoom,
                    int view_debug_mode,
                    Gadget::DebugConsoleConfig debug_console_config,
                    int width, int height);
  virtual ~SimpleClutterHost();
  virtual ViewHostInterface *NewViewHost(Gadget *gadget,
                                         ViewHostInterface::Type type);
  virtual void RemoveGadget(Gadget *gadget, bool save_data);
  virtual bool LoadFont(const char *filename);
  virtual void ShowGadgetAboutDialog(Gadget *gadget);
  virtual void ShowGadgetDebugConsole(Gadget *gadget);
  virtual int GetDefaultFontSize();
  virtual bool OpenURL(const Gadget *gadget, const char *url);
  virtual Gadget* LoadGadget(const char *path, const char *options_name, int instance_id, bool); 

  void Exit();
  void AdjustSize (int width, int height);
  static gboolean ExitCallback (gpointer that);

  ClutterActor *GetActor();
#ifdef HAVE_MPL
  void SetMutterPanel (MplPanelClient* panel) {panel_ = panel;}
 private:
  MplPanelClient* panel_;
#endif
 private:
  class Impl;
  Impl *impl_;
  DISALLOW_EVIL_CONSTRUCTORS(SimpleClutterHost);
};

} // namespace clutter
} // namespace hosts

#endif // HOSTS_CLUTTER_SIMPLE_CLUTTER_HOST_H__
