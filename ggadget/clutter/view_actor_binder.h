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

#ifndef GGADGET_CLUTTER_VIEW_WIDGET_BINDER_H__
#define GGADGET_CLUTTER_VIEW_WIDGET_BINDER_H__

#include <clutter/clutter.h>
#include <ggadget/common.h>

namespace ggadget {

class ViewHostInterface;
class ViewInterface;

namespace clutter {

/**
 * A class to bind a View with a ClutterActor.
 *
 * The specified View will be drawn on the specified ClutterActor,
 * and all events will be delegated to the View from the ClutterActor.
 *
 * The ViewWidgetBinder instance will take effect as soon as it is created,
 * unless any parameter is invalid.
 */
class ViewActorBinder {
 public:
  ViewActorBinder(ViewInterface *view,
                  ViewHostInterface *host, ClutterActor *actor);
  ~ViewActorBinder();

  int GetPointerX();
  int GetPointerY();

  void FreezeUpdates();
  void ThawUpdates();
  void Redraw();
  void QueueDraw();
 private:
  DISALLOW_EVIL_CONSTRUCTORS(ViewActorBinder);
  class Impl;
  Impl *impl_;
};

} // namespace clutter
} // namespace ggadget

#endif // GGADGET_CLUTTER_VIEW_ACTOR_BINDER_H__
