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

#ifndef GGADGET_CLUTTER_MENU_BUILDER_H__
#define GGADGET_CLUTTER_MENU_BUILDER_H__
  
#include <map>
#include <string>
#include <vector>
#include <ggadget/common.h>
#include <ggadget/menu_interface.h>

namespace ggadget {
namespace clutter {

/**
 * @ingroup GtkLibrary
 * An implementation of @c MenuInterface for clutter based host.
 */
class MenuBuilder : public ggadget::MenuInterface {
 public:
  /**
   * Constructor.
   *
   * @param gtk_menu a GtkMenuShell instance, MenuBuilder doesn't own it.
   */
  MenuBuilder();
  virtual ~MenuBuilder();

  virtual void AddItem(const char *item_text, int style, int stock_icon,
                       Slot1<void, const char *> *handler, int priority);
  virtual void SetItemStyle(const char *item_text, int style);
  virtual MenuInterface *AddPopup(const char *popup_text, int priority);

  /** Checks if any item was added. */
  bool ItemAdded() const;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(MenuBuilder);
  class Impl;
  Impl *impl_;
};

} // namesapce clutter
} // namespace ggadget

#endif // GGADGET_CLUTTER_GTK_MENU_IMPL_H__
