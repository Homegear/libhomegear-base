/* Copyright 2013-2019 Homegear GmbH
 *
 * libhomegear-base is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libhomegear-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libhomegear-base.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

#ifndef IPHYSICALINTERFACE_H_
#define IPHYSICALINTERFACE_H_

#include "../IEvents.h"
#include "PhysicalInterfaceSettings.h"
#include "../Managers/FileDescriptorManager.h"
#include "../IQueue.h"

#include <list>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include <atomic>

#include <dirent.h>

namespace BaseLib {

class SharedObjects;

namespace Systems {

class Packet;

class IPhysicalInterface : public IEventsEx, public IQueue {
 public:
  struct GPIODirection {
    enum Enum {
      IN,
      OUT
    };
  };

  struct GPIOEdge {
    enum Enum {
      RISING,
      FALLING,
      BOTH
    };
  };

  //Event handling
  class IPhysicalInterfaceEventSink : public IEventSinkBase {
   public:
    virtual bool onPacketReceived(std::string &senderID, std::shared_ptr<Packet> packet) = 0;
  };
  //End event handling

  IPhysicalInterface(BaseLib::SharedObjects *baseLib, int32_t familyId);
  IPhysicalInterface(BaseLib::SharedObjects *baseLib, int32_t familyId, std::shared_ptr<PhysicalInterfaceSettings> settings);

  virtual ~IPhysicalInterface();

  virtual void startListening();
  virtual void stopListening();
  virtual void enableUpdateMode();
  virtual void disableUpdateMode();
  virtual void sendPacket(std::shared_ptr<Packet> packet) = 0;
  virtual bool lifetick();
  virtual bool isOpen() { return _fileDescriptor && _fileDescriptor->descriptor != -1; }
  virtual uint32_t responseDelay() { return _settings->responseDelay; }
  virtual int64_t lastPacketSent() { return _lastPacketSent; }
  virtual int64_t lastPacketReceived() { return _lastPacketReceived; }
  virtual void setup(int32_t userID, int32_t groupID, bool setPermissions) {}
  virtual std::string getType() { return _settings->type; }
  virtual std::string getID() { return _settings->id; }
  virtual std::string getSerialNumber() { return ""; }
  virtual std::string getFirmwareVersion() { return ""; }
  virtual bool isDefault() { return _settings->isDefault; }
  virtual bool isNetworkDevice() { return _settings->device.empty() && !_settings->host.empty() && !_settings->port.empty(); }
  virtual int32_t getAddress() { return _myAddress; }
  virtual std::string getIpAddress() { return _ipAddress; }
  virtual std::string getHostname() { return _hostname; }

  void setRawPacketEvent(std::function<void(int32_t familyId, const std::string &interfaceId, const BaseLib::PVariable &packet)> value) { _rawPacketEvent.swap(value); }
 protected:
  class QueueEntry : public BaseLib::IQueueEntry {
   public:
    QueueEntry(const std::shared_ptr<Packet> &packet) { this->packet = packet; };

    std::shared_ptr<Packet> packet;
  };

  BaseLib::SharedObjects *_bl = nullptr;
  int32_t _familyId = -1;
  std::shared_ptr<PhysicalInterfaceSettings> _settings;
  std::thread _listenThread; //Used by derived classes
  std::thread _callbackThread; //Used by derived classes
  std::atomic_bool _stopCallbackThread {false}; //Used by derived classes
  std::string _lockfile;
  std::mutex _sendMutex;
  std::atomic_bool _stopped; //Used by derived classes
  std::shared_ptr<FileDescriptor> _fileDescriptor;
  std::map<uint32_t, std::shared_ptr<FileDescriptor>> _gpioDescriptors;
  std::atomic<int64_t> _lastPacketSent{-1};
  std::atomic<int64_t> _lastPacketReceived{-1};
  int64_t _maxPacketProcessingTime = 1000;
  std::atomic_bool _updateMode{false};
  std::atomic<int64_t> _lifetickTime{0};
  std::atomic_bool _lifetickState{true};

  int32_t _myAddress = 0;
  std::string _hostname;
  std::string _ipAddress;

  std::function<void(int32_t familyId, const std::string &interfaceId, const BaseLib::PVariable &packet)> _rawPacketEvent;

  //Event handling
  virtual void raisePacketReceived(std::shared_ptr<Packet> packet);
  //End event handling
  void processQueueEntry(int32_t index, std::shared_ptr<BaseLib::IQueueEntry> &entry) override;
  virtual void setDevicePermission(int32_t userID, int32_t groupID);
  virtual void openGPIO(uint32_t index, bool readOnly);
  virtual void getGPIOPath(uint32_t index);
  virtual void closeGPIO(uint32_t index);
  virtual bool getGPIO(uint32_t index);
  virtual void setGPIO(uint32_t index, bool value);
  virtual void setGPIOPermission(uint32_t index, int32_t userID, int32_t groupID, bool readOnly);
  virtual void exportGPIO(uint32_t index);
  virtual bool setGPIODirection(uint32_t index, GPIODirection::Enum direction);
  virtual bool setGPIOEdge(uint32_t index, GPIOEdge::Enum edge);
  virtual bool gpioDefined(uint32_t);
  virtual bool gpioOpen(uint32_t);

  virtual void saveSettingToDatabase(std::string setting, std::string &value);
  virtual void saveSettingToDatabase(std::string setting, int32_t value);
  virtual void saveSettingToDatabase(std::string setting, std::vector<char> &value);
};

}
}
#endif
