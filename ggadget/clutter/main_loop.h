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

#ifndef GGADGET_CLUTTER_MAIN_LOOP_H__
#define GGADGET_CLUTTER_MAIN_LOOP_H__
#include <ggadget/main_loop_interface.h>

namespace ggadget {
namespace clutter {

/**
 * @ingroup GtkLibrary
 * It's is a wrapper around gtk's main loop functions to implement
 * MainLoopInterface interface.
 */
class MainLoop : public MainLoopInterface {
 public:
  MainLoop();
  virtual ~MainLoop();
  virtual int AddIOReadWatch(int fd, WatchCallbackInterface *callback);
  virtual int AddIOWriteWatch(int fd, WatchCallbackInterface *callback);
  virtual int AddTimeoutWatch(int interval, WatchCallbackInterface *callback);
  virtual WatchType GetWatchType(int watch_id);
  virtual int GetWatchData(int watch_id);
  virtual void RemoveWatch(int watch_id);
  // This function just call g_main_loop_run(). So you can use either g_main_loop_run() or
  // this function.
  virtual void Run();
  // This function just call g_main_context_iteration(). So you can use either
  // g_main_context_iteration() or this function.
  virtual bool DoIteration(bool may_block);
  // This function just call g_main_loop_quit().
  virtual void Quit();
  virtual bool IsRunning() const;
  virtual uint64_t GetCurrentTime() const;
  virtual bool IsMainThread() const;
  virtual void WakeUp();

 private:
  class Impl;
  Impl *impl_;
  DISALLOW_EVIL_CONSTRUCTORS(MainLoop);
};

} // namespace clutter
} // namespace ggadget
#endif  // GGADGET_CLUTTER_MAIN_LOOP_H__
