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

#ifndef EXTENSIONS_LINUX_SYSTEM_FRAMEWORK_WIRELESS_CONNMAN_H__
#define EXTENSIONS_LINUX_SYSTEM_FRAMEWORK_WIRELESS_CONNMAN_H__

#include <ggadget/framework_interface.h>

#define CONNMAN_SERVICE         "org.moblin.connman"

#define CONNMAN_MANAGER_INTERFACE    CONNMAN_SERVICE ".Manager"
#define CONNMAN_SERVICE_INTERFACE    CONNMAN_SERVICE ".Service"
#define CONNMAN_CONNECTION_INTERFACE CONNMAN_SERVICE ".Connection"
#define CONNMAN_MANAGER_PATH        "/"

namespace ggadget {
namespace framework {

class WirelessAccessPointInterface;
namespace linux_system {

class WirelessConnman : public WirelessInterface {
 public:
  WirelessConnman();
  virtual ~WirelessConnman();

  virtual bool IsAvailable() const;
  virtual bool IsConnected() const;
  virtual bool EnumerationSupported() const;

  virtual int GetAPCount() const;
  virtual WirelessAccessPointInterface *GetWirelessAccessPoint(int index);

  virtual std::string GetName() const;
  virtual std::string GetNetworkName() const;
  virtual int GetSignalStrength() const;
  virtual void ConnectAP(const char *ap_name, Slot1<void, bool> *callback);
  virtual void DisconnectAP(const char *ap_name, Slot1<void, bool> *callback);

 private:
  class Impl;
  Impl *impl_;
  DISALLOW_EVIL_CONSTRUCTORS(WirelessConnman);
};

} // namespace linux_system
} // namespace framework
} // namespace ggadget

#endif // EXTENSIONS_LINUX_SYSTEM_FRAMEWORK_WIRELESS_CONNMAN_H__
