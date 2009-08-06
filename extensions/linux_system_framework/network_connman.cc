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

#include <ggadget/string_utils.h>
#include <ggadget/main_loop_interface.h>
#include "network_connman.h"

namespace ggadget {
namespace framework {
namespace linux_system {

#define CONNMAN_STATE_ONLINE        "online"

NetworkConnman::NetworkConnman()
  : is_online_(true), // treats online by default
    connection_type_(CONNECTION_TYPE_802_3),
    physcial_media_type_(PHYSICAL_MEDIA_TYPE_UNSPECIFIED),
    connman_(NULL),
    on_signal_connection_(NULL) {
  connman_ = DBusProxy::NewSystemProxy(CONNMAN_SERVICE, CONNMAN_MANAGER_PATH,
                                       CONNMAN_MANAGER_INTERFACE);
  if (!connman_) {
    DLOG("Connman is not available.");
    return;
  }

  DLOG("Connman is used.");
  DBusStringReceiver result;
  if (connman_->CallMethod("GetState", true, kDefaultDBusTimeout,
                           result.NewSlot(), MESSAGE_TYPE_INVALID)) {
    is_online_ = (result.GetValue() == CONNMAN_STATE_ONLINE);
  }

  on_signal_connection_ = connman_->ConnectOnSignalEmit(
      NewSlot(this, &NetworkConnman::OnSignal));

  if (is_online_) {
    Update();
  } else {
    connection_type_ = CONNECTION_TYPE_UNKNOWN;
    physcial_media_type_ = PHYSICAL_MEDIA_TYPE_UNSPECIFIED;
  }
}

NetworkConnman::~NetworkConnman() {
  if (on_signal_connection_)
    on_signal_connection_->Disconnect();
  delete connman_;
}

std::string NetworkConnman::GetConnInfo(ScriptableInterface *result) {
  if (result) {
    ResultVariant value = result->GetProperty("DefaultTechnology");
    if (value.v().type() == Variant::TYPE_STRING) {
      return VariantValue<std::string>()(value.v());
    }
  }
  return std::string();
}

void NetworkConnman::OnSignal(const std::string &name, int argc, const Variant *argv) {
  DLOG("Got signal from connman: %s", name.c_str());
  bool need_update = false;

  if (name == "StateChanged") {
    std::string state;
    if (argc >= 1 && argv[0].ConvertToString(&state)) {
      is_online_ = (state == CONNMAN_STATE_ONLINE);
      DLOG("Network is %s.", is_online_ ? "online" : "offline");
      if (is_online_) {
        need_update = true;
      } else {
        connection_type_ = CONNECTION_TYPE_UNKNOWN;
        physcial_media_type_ = PHYSICAL_MEDIA_TYPE_UNSPECIFIED;
      }
    }
  } else if (name == "PropertyChanged") {
    need_update = is_online_;
  }
  if (need_update)
    Update();
}

void NetworkConnman::Update() {
  DLOG("Update network information.");
  DBusScriptableReceiver result;

  if (connman_->CallMethod("GetProperties",
                           true, kDefaultDBusTimeout, result.NewSlot(),
                           MESSAGE_TYPE_INVALID)) {

    std::string tech = GetConnInfo (result.GetValue());
    if (tech == "ethernet") {
            connection_type_ = CONNECTION_TYPE_802_3;
            physcial_media_type_ = PHYSICAL_MEDIA_TYPE_UNSPECIFIED;
    } else if (tech == "wifi") {
      connection_type_ = CONNECTION_TYPE_NATIVE_802_11;
      physcial_media_type_ = PHYSICAL_MEDIA_TYPE_NATIVE_802_11;
    } else {
      connection_type_ = CONNECTION_TYPE_UNKNOWN;
      physcial_media_type_ = PHYSICAL_MEDIA_TYPE_UNSPECIFIED;
    }
  }

  // Always return 802.3 type if the connection type is unknown.
  if (connection_type_ == CONNECTION_TYPE_UNKNOWN)
    connection_type_ = CONNECTION_TYPE_802_3;
}

} // namespace linux_system
} // namespace framework
} // namespace ggadget
