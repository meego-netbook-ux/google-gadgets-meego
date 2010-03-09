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

#include <ggadget/logger.h>
#include <ggadget/common.h>
#include <ggadget/slot.h>
#include "menu_builder.h"

namespace ggadget {
namespace clutter {

class MenuBuilder::Impl {
 public:
  Impl() : item_added_(false) {
    g_print ("Created menu\n");
  }
  ~Impl() {
  }

  // Windows version uses '&' as the mnemonic indicator, and this has been
  // taken as the part of the Gadget API.
  static std::string ConvertWindowsStyleMnemonics(const char *text) {
    std::string result;
    if (text) {
      while (*text) {
        switch (*text) {
          case '&': result += '_'; break;
          case '_': result += "__";
          default: result += *text;
        }
        text++;
      }
    }
    return result;
  }


  void AddMenuItem(const char *text, int style, int stock_icon,
                   ggadget::Slot1<void, const char *> *handler,
                   int priority) {
    g_print ("Added item: %s\n", text);
  }

  void SetItemStyle(const char *text, int style) {
  }

  static void DestroyMenuBuilderCallback(gpointer data) {
    delete reinterpret_cast<MenuBuilder *>(data);
  }

  MenuInterface *AddPopup(const char *text, int priority) {
    MenuBuilder *submenu = NULL;
    return submenu;
  }

  bool item_added_;
};

MenuBuilder::MenuBuilder()
    : impl_(new Impl()) {
  DLOG("Create MenuBuilder.");
}

MenuBuilder::~MenuBuilder() {
  DLOG("Destroy MenuBuilder.");
  delete impl_;
  impl_ = NULL;
}

void MenuBuilder::AddItem(const char *item_text, int style, int stock_icon,
                          Slot1<void, const char *> *handler, int priority) {
  impl_->AddMenuItem(item_text, style, stock_icon, handler, priority);
}

void MenuBuilder::SetItemStyle(const char *item_text, int style) {
  impl_->SetItemStyle(item_text, style);
}

ggadget::MenuInterface *MenuBuilder::AddPopup(const char *popup_text,
                                              int priority) {
  return impl_->AddPopup(popup_text, priority);
}

bool MenuBuilder::ItemAdded() const {
  return impl_->item_added_;
}

} // namespace clutter
} // namespace ggadget
