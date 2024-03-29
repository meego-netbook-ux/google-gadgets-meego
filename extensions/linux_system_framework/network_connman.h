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

#ifndef EXTENSIONS_LINUX_SYSTEM_FRAMEWORK_CONNMAN_H__
#define EXTENSIONS_LINUX_SYSTEM_FRAMEWORK_CONNMAN_H__

#include <vector>
#include <string>
#include <ggadget/framework_interface.h>
#include <ggadget/scriptable_interface.h>
#include <ggadget/logger.h>
#include <ggadget/slot.h>
#include <ggadget/string_utils.h>
#include <ggadget/dbus/dbus_proxy.h>
#include <ggadget/dbus/dbus_result_receiver.h>
#include "wireless_connman.h"

namespace ggadget {
namespace framework {
namespace linux_system {

using namespace ggadget::dbus;

class NetworkConnman : public NetworkInterface {
 public:
  NetworkConnman();
  ~NetworkConnman();

  virtual bool IsOnline() {
    return is_online_;
  }
  virtual NetworkInterface::ConnectionType GetConnectionType() {
    return connection_type_;
  }
  virtual NetworkInterface::PhysicalMediaType GetPhysicalMediaType() {
    return physcial_media_type_;
  }
  virtual WirelessInterface *GetWireless() {
    return &wireless_;
  }
  static std::string GetConnInfo(ScriptableInterface *result);

 private:
  void OnSignal(const std::string &name, int argc, const Variant *argv);
  void Update();
  void UpdateConnInfo (ScriptableInterface *result);

 private:
  // true if using nm 0.7 or above, false if using nm 0.6.x
  bool is_new_api_;
  bool is_online_;
  NetworkInterface::ConnectionType connection_type_;
  NetworkInterface::PhysicalMediaType physcial_media_type_;

  DBusProxy *connman_;
  Connection *on_signal_connection_;
  WirelessConnman wireless_;

  DISALLOW_EVIL_CONSTRUCTORS(NetworkConnman);
};

} // namespace linux_system
} // namespace framework
} // namespace ggadget

#endif // EXTENSIONS_LINUX_SYSTEM_FRAMEWORK_CONNMAN_H__
