/*
  Copyright 2009 Intel Corp.

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

#include "wireless_connman.h"

#include <algorithm>
#include <cstddef>
#include <time.h>

#include <ggadget/dbus/dbus_proxy.h>
#include <ggadget/dbus/dbus_result_receiver.h>
#include <ggadget/scriptable_interface.h>
#include <ggadget/scriptable_array.h>
#include <ggadget/string_utils.h>
#include <ggadget/slot.h>

namespace ggadget {
namespace framework {
namespace linux_system {

using namespace ggadget::dbus;

class WirelessConnman::Impl {
  class ConnmanAP : public WirelessAccessPointInterface {

    Impl* impl_;
    std::string ap_path_;
    DBusProxy* service_;

    std::string name_, state_, mode_;
    int strength_;

  void GetServicesInfo (ScriptableInterface *result) {
    if (result) {
      ResultVariant State    = result->GetProperty("State");
      ResultVariant Name     = result->GetProperty("Name");
      ResultVariant Mode     = result->GetProperty("Mode");
      ResultVariant Strength = result->GetProperty("Strength");

      if (Strength.v().type() != Variant::TYPE_INT64 ||
          State.v().type() != Variant::TYPE_STRING ||
          Name.v().type() != Variant::TYPE_STRING ||
          Mode.v().type() != Variant::TYPE_STRING )
        return;

      state_    = VariantValue<std::string>()(State.v());
      name_     = VariantValue<std::string>()(Name.v());
      mode_     = VariantValue<std::string>()(Mode.v());
      strength_ = (int)VariantValue<char>()(Strength.v());

    }
  }
  public:
    ConnmanAP (Impl* impl, const std::string& path)
      :impl_(impl), ap_path_(path), service_(NULL), strength_(0) {
      service_ = DBusProxy::NewSystemProxy (CONNMAN_SERVICE, path,
                                            CONNMAN_SERVICE_INTERFACE);
      DBusScriptableReceiver result;
      if (service_->CallMethod("GetProperties",
                              true, kDefaultDBusTimeout, result.NewSlot(),
                              MESSAGE_TYPE_INVALID)) {
        GetServicesInfo (result.GetValue());
      }
    }
    virtual ~ConnmanAP() {
      if (service_)
        delete service_;
    }
    virtual void Destroy() {
      delete this;
    }
    virtual std::string GetName() const {
      return name_;
    }
    virtual Type GetType() const {
      if (mode_ == "adhoc")
        return WIRELESS_TYPE_INDEPENDENT;
      else if (mode_ == "managed")
        return WIRELESS_TYPE_INFRASTRUCTURE;
      else
        return WIRELESS_TYPE_ANY;
    }
    virtual int GetSignalStrength() const {
      return strength_;
    }
    virtual void Connect(Slot1<void, bool> *callback) {
      impl_->Connect(ap_path_, callback);
    }
    virtual void Disconnect(Slot1<void, bool> *callback) {
      impl_->Connect(ap_path_, callback, false);
    }
  };

 public:
  Impl()
    : connman_(NULL), on_signal_connection_(NULL), connected_connection_(NULL),
      on_signal_manager_(NULL)
  {

    connman_ = DBusProxy::NewSystemProxy(CONNMAN_SERVICE, CONNMAN_MANAGER_PATH,
                                         CONNMAN_MANAGER_INTERFACE);
    if (!connman_) {
      DLOG("Connman is not available.");
      return;
    }

    DLOG("Connman is used.");
    on_signal_manager_ = connman_->ConnectOnSignalEmit(
        NewSlot(this, &Impl::OnSignal));

    UpdateWirelessInfo();
  }
  ~Impl() {
    if (on_signal_connection_)
      on_signal_connection_->Disconnect();
    if (on_signal_manager_)
      on_signal_manager_->Disconnect();
    if (connected_connection_)
      delete connected_connection_;
    if (connman_)
      delete connman_;
  }
  bool IsAvailable() {
    return available_;
  }
  bool IsConnected() {
    return connected_;
  }
  bool EnumerationSupported() {
    return ap_paths_.size() > 0;
  }
  int GetAPCount() {
    return ap_paths_.size();
  }
  WirelessAccessPointInterface* GetWirelessAccessPoint(int index) {
    if (index >= 0 && index < static_cast<int>(ap_index_.size())) {
      return new ConnmanAP(this, ap_index_[index]);
    }
    return NULL;
  }
  std::string GetName() {
    return connected_wifi_interface_;
  }
  std::string GetNetworkName() {
    return connected_wifi_name_;
  }
  int GetSignalStrength() {
    return connected_wifi_strength_;
  }

  void ConnectAP(const char *ap_name, Slot1<void, bool> *callback) {
    StringMap::iterator it = ap_paths_.find (ap_name);
    if (it == ap_paths_.end()) {
      //we don't have that AP
      if (callback) {
        (*callback)(false);
        delete callback;
      }
    }else{
      if (connected_wifi_name_ == ap_name) {
        //we already connected to that
        if (callback) {
          (*callback)(true);
          delete callback;
        }
      }else{
        //do the real connection
        Connect (it->second, callback);
      }
    }
  }

  void DisconnectAP(const char *ap_name, Slot1<void, bool> *callback) {
    StringMap::iterator it = ap_paths_.find (ap_name);
    if (it == ap_paths_.end()) {
      //we don't have that AP
      if (callback) {
        (*callback)(false);
        delete callback;
      }
    }else{
      //do the real disconnection
      Connect (it->second, callback, false);
    }
  }

 private:
  void Connect (const std::string& ap_path, Slot1<void, bool> *callback, bool connect=true) {
    DBusProxy* service = DBusProxy::NewSystemProxy(CONNMAN_SERVICE, ap_path,
                                                   CONNMAN_SERVICE_INTERFACE);

    const char* command = connect ? "Connect" : "Disconnect";

    //we give 120 secs for connect
    if (service->CallMethod(command,
                            true, 120000, NULL,
                            MESSAGE_TYPE_INVALID)) {
      if (callback) {
        (*callback)(true);
        delete callback;
      }
    }else{
      if (callback) {
        (*callback)(false);
        delete callback;
      }
    }
    delete service;
  }

  bool CallbackAvailTech(int i, const Variant &elm) {
    if (elm.type() == Variant::TYPE_STRING) {
      std::string tech = VariantValue<std::string>()(elm);
      if (tech == "wifi") {
        available_ = true;
        return false;
      }
    }
    return true;
  }

  bool CallbackConnectedTech(int i, const Variant &elm) {
    if (elm.type() == Variant::TYPE_STRING) {
      std::string tech = VariantValue<std::string>()(elm);
      if (tech == "wifi") {
        connected_ = true;
        return false;
      }
    }
    return true;
  }

  bool CallbackConnections(int i, const Variant &elm) {
    if (elm.type() == Variant::TYPE_STRING) {
      std::string path = VariantValue<std::string>()(elm);
      DBusProxy* connection = DBusProxy::NewSystemProxy(CONNMAN_SERVICE, path,
                                                        CONNMAN_CONNECTION_INTERFACE);
      DBusScriptableReceiver result;
      if (connection->CallMethod("GetProperties",
                               true, kDefaultDBusTimeout, result.NewSlot(),
                               MESSAGE_TYPE_INVALID)) {
        ScriptableInterface* obj = result.GetValue();
        if (obj) {
          ResultVariant Strength = obj->GetProperty("Strength");
          ResultVariant Name     = obj->GetProperty("Interface");
          ResultVariant Type     = obj->GetProperty("Type");

          std::string name, type;
          int strength;
          if (Strength.v().type() != Variant::TYPE_INT64 ||
              Name.v().type() != Variant::TYPE_STRING ||
              Type.v().type() != Variant::TYPE_STRING ) {
            delete connection;
            return true;
          }

          strength = (int)VariantValue<char>()(Strength.v());
          name     = VariantValue<std::string>()(Name.v());
          type     = VariantValue<std::string>()(Type.v());

          if (type == "wifi") {
            connected_wifi_interface_ = name;
            connected_wifi_strength_  = strength;

            if (connected_connection_) {
              if (on_signal_connection_)
                on_signal_connection_->Disconnect();
              delete connected_connection_;
            }

            connected_connection_ = connection;
            //NOTE: we use the same signal handler for manager and connection
            on_signal_connection_ = connection->ConnectOnSignalEmit (NewSlot(this, &Impl::OnSignal));

            return false;
          }
        }
      }
      delete connection;
    }
    return true;
  }

  bool CallbackAPs(int i, const Variant &elm) {
    if (elm.type() == Variant::TYPE_STRING) {
      std::string path = VariantValue<std::string>()(elm);

      DBusProxy* service = DBusProxy::NewSystemProxy(CONNMAN_SERVICE, path,
                                                     CONNMAN_SERVICE_INTERFACE);
      DBusScriptableReceiver result;
      if (service->CallMethod("GetProperties",
                              true, kDefaultDBusTimeout, result.NewSlot(),
                              MESSAGE_TYPE_INVALID)) {
        ScriptableInterface* obj = result.GetValue();
        if (obj) {
          ResultVariant Name   = obj->GetProperty("Name");
          ResultVariant Type   = obj->GetProperty("Type");
          ResultVariant State  = obj->GetProperty("State");

          std::string name, type, state;

          if ( Name.v().type() != Variant::TYPE_STRING ||
              State.v().type() != Variant::TYPE_STRING ||
               Type.v().type() != Variant::TYPE_STRING ) {
            delete service;
            return true;
          }

          name     = VariantValue<std::string>()(Name.v());
          type     = VariantValue<std::string>()(Type.v());
          state    = VariantValue<std::string>()(State.v());

          if (type == "wifi") {
            ap_paths_[name] = path;
            ap_index_.push_back (path);
            if (state == "ready")
              connected_wifi_name_ = name;
          }
        }
      }
      delete service;
    }
    return true;
  }

  void GetWirelessAvail (ScriptableInterface *result) {
    if (result) {
      ResultVariant value = result->GetProperty("AvailableTechnologies");
      if (value.v().type() == Variant::TYPE_SCRIPTABLE) {
        ScriptableInterface *ptr =
          VariantValue<ScriptableInterface*>()(value.v());
        if (ptr) {
          ptr->EnumerateElements ( NewSlot (this, &Impl::CallbackAvailTech) );
        }
      }
    }
  }

  void GetWirelessStatus (ScriptableInterface *result) {
    if (result) {
      ResultVariant value = result->GetProperty("ConnectedTechnologies");
      if (value.v().type() == Variant::TYPE_SCRIPTABLE) {
        ScriptableInterface *ptr =
          VariantValue<ScriptableInterface*>()(value.v());
        if (ptr) {
          ptr->EnumerateElements ( NewSlot (this, &Impl::CallbackConnectedTech) );
        }
      }
    }
  }

  void GetWirelessConnections (ScriptableInterface *result) {
    if (result) {
      ResultVariant value = result->GetProperty("Connections");
      if (value.v().type() == Variant::TYPE_SCRIPTABLE) {
        ScriptableInterface *ptr =
          VariantValue<ScriptableInterface*>()(value.v());
        if (ptr) {
          ptr->EnumerateElements ( NewSlot (this, &Impl::CallbackConnections) );
        }
      }
    }
  }

  void GetWirelessAPs (ScriptableInterface *result) {
    if (result) {
      ResultVariant value = result->GetProperty("Services");
      if (value.v().type() == Variant::TYPE_SCRIPTABLE) {
        ScriptableInterface *ptr =
          VariantValue<ScriptableInterface*>()(value.v());
        if (ptr) {
          ptr->EnumerateElements ( NewSlot (this, &Impl::CallbackAPs) );
        }
      }
    }
  }

  void GetServicesInfo (ScriptableInterface *result) {
    if (result) {
      ResultVariant State = result->GetProperty("State");
      ResultVariant Name  = result->GetProperty("Name");
      ResultVariant Type  = result->GetProperty("Type");

      std::string state, name, type;
      if (State.v().type() != Variant::TYPE_STRING ||
           Name.v().type() != Variant::TYPE_STRING ||
           Type.v().type() != Variant::TYPE_STRING )
        return;

      state = VariantValue<std::string>()(State.v());
      name  = VariantValue<std::string>()(Name.v());
      type  = VariantValue<std::string>()(Type.v());
      if (state == "ready" && type == "wifi") {
        connected_wifi_name_ = name;
      }
    }
  }

  void UpdateServicesInfo() {
    StringMap::iterator it;
    for (it = ap_paths_.begin(); it != ap_paths_.end(); it++) {
      DBusProxy* service = DBusProxy::NewSystemProxy(CONNMAN_SERVICE, it->second,
                                                     CONNMAN_SERVICE_INTERFACE);
      DBusScriptableReceiver result;
      if (service->CallMethod("GetProperties",
                              true, kDefaultDBusTimeout, result.NewSlot(),
                              MESSAGE_TYPE_INVALID)) {
        GetServicesInfo (result.GetValue());
      }
      delete service;
    }
  }

  void UpdateWirelessInfo() {
    DBusScriptableReceiver result;

    available_ = false;
    connected_ = false;
    ap_paths_.clear();
    ap_index_.clear();

    if (connman_->CallMethod("GetProperties",
                             true, kDefaultDBusTimeout, result.NewSlot(),
                             MESSAGE_TYPE_INVALID)) {
      ScriptableInterface* ret = result.GetValue();
      GetWirelessAvail (ret);
      GetWirelessAPs (ret);
      GetWirelessStatus (ret);
      GetWirelessConnections (ret);
      //UpdateServicesInfo ();
    }
  }

  void UpdateAPList (ScriptableInterface* obj) {
    ap_paths_.clear();
    ap_index_.clear();

    obj->EnumerateElements ( NewSlot (this, &Impl::CallbackAPs) );
  }

  void OnSignal(const std::string &name, int argc, const Variant *argv) {
    DLOG("Got signal from connman: %s", name.c_str());
    if (name == "PropertyChanged" && argc == 2) {
      std::string name;
      argv[0].ConvertToString (&name);
      if (name == "Strength" && argv[1].type() == Variant::TYPE_INT64) {
        connected_wifi_strength_ = (int)VariantValue<char>()(argv[1]);
      }
      else if (name == "StateChanged" && argv[1].type() == Variant::TYPE_STRING) {
        std::string state = VariantValue<std::string>()(argv[1]);
        connected_ = state == "online";
      }
      else if (argv[1].type() == Variant::TYPE_SCRIPTABLE)  {
        ScriptableInterface *obj = VariantValue<ScriptableInterface*>()(argv[1]);
        if (name == "ConnectedTechnologies")
          GetWirelessStatus (obj);
        else if (name == "AvailableTechnologies")
          GetWirelessAvail (obj);
        else if (name == "Services") {
          // update ap list
          UpdateAPList(obj);
        }
      }
    }
  }

 private:
  bool available_;
  bool connected_;
  StringMap ap_paths_;
  StringVector ap_index_;
  std::string connected_wifi_name_;
  std::string connected_wifi_interface_;
  int connected_wifi_strength_;

  DBusProxy *connman_;
  DBusProxy *connected_connection_;
  Connection *on_signal_connection_;
  Connection *on_signal_manager_;
};

WirelessConnman::WirelessConnman()
  : impl_(new Impl()) {
}
WirelessConnman::~WirelessConnman() {
  delete impl_;
  impl_ = NULL;
}
bool WirelessConnman::IsAvailable() const {
  return impl_->IsAvailable();
}
bool WirelessConnman::IsConnected() const {
  return impl_->IsConnected();
}
bool WirelessConnman::EnumerationSupported() const {
  return impl_->EnumerationSupported();
}
int WirelessConnman::GetAPCount() const {
  return impl_->GetAPCount();
}
WirelessAccessPointInterface *WirelessConnman::GetWirelessAccessPoint(int index) {
  return impl_->GetWirelessAccessPoint(index);
}
std::string WirelessConnman::GetName() const {
  return impl_->GetName();
}
std::string WirelessConnman::GetNetworkName() const {
  return impl_->GetNetworkName();
}
int WirelessConnman::GetSignalStrength() const {
  return impl_->GetSignalStrength();
}
void WirelessConnman::ConnectAP(const char *ap_name, Slot1<void, bool> *callback) {
  impl_->ConnectAP(ap_name, callback);
}
void WirelessConnman::DisconnectAP(const char *ap_name, Slot1<void, bool> *callback) {
  impl_->DisconnectAP(ap_name, callback);
}

} // namespace linux_system
} // namespace framework
} // namespace ggadget
