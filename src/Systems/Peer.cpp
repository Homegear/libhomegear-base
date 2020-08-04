#include <memory>
#include <iostream>

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

#include "../../config.h"
#include "Peer.h"
#include "ServiceMessages.h"
#include "../BaseLib.h"

namespace BaseLib {
namespace Systems {

RpcConfigurationParameter::RpcConfigurationParameter(RpcConfigurationParameter const &rhs) {
  rpcParameter = rhs.rpcParameter;
  databaseId = rhs.databaseId;
  specialType = rhs.specialType;
  _binaryData = rhs._binaryData;
  _partialBinaryData = rhs._partialBinaryData;
  _logicalData = rhs._logicalData;
  _room = rhs._room;
  _categories = rhs._categories;
  _roles = rhs._roles;
  _invert = rhs._invert;
  _scale = rhs._scale;
  _mainRole = rhs._mainRole;
}

RpcConfigurationParameter &RpcConfigurationParameter::operator=(const RpcConfigurationParameter &rhs) {
  if (&rhs == this) return *this;
  rpcParameter = rhs.rpcParameter;
  databaseId = rhs.databaseId;
  specialType = rhs.specialType;
  _binaryData = rhs._binaryData;
  _partialBinaryData = rhs._partialBinaryData;
  _logicalData = rhs._logicalData;
  _room = rhs._room;
  _categories = rhs._categories;
  _roles = rhs._roles;
  _invert = rhs._invert;
  _scale = rhs._scale;
  _mainRole = rhs._mainRole;
  return *this;
}

std::string RpcConfigurationParameter::getCategoryString() {
  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  std::ostringstream categories;
  for (auto category : _categories) {
    categories << std::to_string(category) << ",";
  }
  return categories.str();
}

std::string RpcConfigurationParameter::getRoleString() {
  std::lock_guard<std::mutex> rolesGuard(_rolesMutex);
  std::ostringstream roles;
  for (auto role : _roles) {
    roles << std::to_string(role.first) << "-" << std::to_string((int32_t)role.second.direction) << "-" << std::to_string((int32_t)role.second.invert) << "-" << std::to_string((int32_t)role.second.scaleInfo.valueSet) << "-"
          << std::to_string((int32_t)role.second.scaleInfo.valueMin) << "-" << std::to_string((int32_t)role.second.scaleInfo.valueMax) << "-" << std::to_string((int32_t)role.second.scaleInfo.scaleMin) << "-"
          << std::to_string((int32_t)role.second.scaleInfo.scaleMax) << ",";
  }
  return roles.str();
}

void RpcConfigurationParameter::lock() noexcept {
  _binaryDataMutex.lock();
}

void RpcConfigurationParameter::unlock() noexcept {
  _binaryDataMutex.unlock();
}

std::vector<uint8_t>::size_type RpcConfigurationParameter::getBinaryDataSize() noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  return _binaryData.size();
}

std::vector<uint8_t> RpcConfigurationParameter::getBinaryData() noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  return _binaryData;
}

std::vector<uint8_t> &RpcConfigurationParameter::getBinaryDataReference() noexcept {
  return _binaryData;
}

void RpcConfigurationParameter::setBinaryData(std::vector<uint8_t> &value) noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  _binaryData = value;
}

std::vector<uint8_t> RpcConfigurationParameter::getPartialBinaryData() noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  return _partialBinaryData;
}

std::vector<uint8_t> &RpcConfigurationParameter::getPartialBinaryDataReference() noexcept {
  return _partialBinaryData;
}

void RpcConfigurationParameter::setPartialBinaryData(std::vector<uint8_t> &value) noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  _partialBinaryData = value;
}

PVariable RpcConfigurationParameter::getLogicalData() noexcept {
  return _logicalData;
}

void RpcConfigurationParameter::setLogicalData(PVariable value) noexcept {
  _logicalData = value;
}

bool RpcConfigurationParameter::equals(std::vector<uint8_t> &value) noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  return value == _binaryData;
}

void RpcConfigurationParameter::addRole(const Role &role) {
  std::lock_guard<std::mutex> rolesGuard(_rolesMutex);
  _roles.emplace(role.id, role);
  if (role.invert) _invert = true;
  if (role.scale) _scale = true;
  if (role.level == RoleLevel::role && !_mainRole.scale && !_mainRole.invert) _mainRole = role;
}

void RpcConfigurationParameter::addRole(uint64_t id, RoleDirection direction, bool invert, bool scale, RoleScaleInfo scaleInfo) {
  std::lock_guard<std::mutex> rolesGuard(_rolesMutex);
  auto role = Role(id, direction, invert, scale, scaleInfo);
  _roles.emplace(id, role);
  if (role.level == RoleLevel::role && !_mainRole.scale && !_mainRole.invert) {
    _invert = true;
    _scale = true;
    _mainRole = role;
  }
}

void RpcConfigurationParameter::removeRole(uint64_t id) {
  std::lock_guard<std::mutex> rolesGuard(_rolesMutex);
  _roles.erase(id);
  if (id == _mainRole.id) {
    _mainRole = Role();
    _invert = false;
    _scale = false;
  }
}

bool RpcConfigurationParameter::invert() {
  std::lock_guard<std::mutex> rolesGuard(_rolesMutex);
  return _invert;
}

bool RpcConfigurationParameter::scale() {
  std::lock_guard<std::mutex> rolesGuard(_rolesMutex);
  return _scale;
}

Role RpcConfigurationParameter::mainRole() {
  std::lock_guard<std::mutex> rolesGuard(_rolesMutex);
  return _mainRole;
}

ConfigDataBlock::ConfigDataBlock(ConfigDataBlock const &rhs) {
  databaseId = rhs.databaseId;
  _binaryData = rhs._binaryData;
}

ConfigDataBlock &ConfigDataBlock::operator=(const ConfigDataBlock &rhs) {
  if (&rhs == this) return *this;
  databaseId = rhs.databaseId;
  _binaryData = rhs._binaryData;
  return *this;
}

void ConfigDataBlock::lock() noexcept {
  _binaryDataMutex.lock();
}

void ConfigDataBlock::unlock() noexcept {
  _binaryDataMutex.unlock();
}

std::vector<uint8_t>::size_type ConfigDataBlock::getBinaryDataSize() noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  return _binaryData.size();
}

std::vector<uint8_t> ConfigDataBlock::getBinaryData() noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  return _binaryData;
}

std::vector<uint8_t> &ConfigDataBlock::getBinaryDataReference() noexcept {
  return _binaryData;
}

void ConfigDataBlock::setBinaryData(std::vector<uint8_t> &value) noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  _binaryData = value;
}

bool ConfigDataBlock::equals(std::vector<uint8_t> &value) noexcept {
  std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
  return value == _binaryData;
}

Peer::Peer(SharedObjects *baseLib, uint32_t parentId, IPeerEventSink *eventHandler) {
  try {
    deleting = false;

    _bl = baseLib;
    _parentID = parentId;
    serviceMessages.reset(new ServiceMessages(baseLib, 0, "", this));
    _lastPacketReceived = HelperFunctions::getTimeSeconds();
    _rpcDevice.reset();
    setEventHandler(eventHandler);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

Peer::Peer(SharedObjects *baseLib, uint64_t id, int32_t address, std::string serialNumber, uint32_t parentId, IPeerEventSink *eventHandler) : Peer(baseLib, parentId, eventHandler) {
  try {
    _peerID = id;
    _address = address;
    _serialNumber = serialNumber;
    if (serviceMessages) {
      serviceMessages->setPeerId(id);
      serviceMessages->setPeerSerial(serialNumber);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

Peer::~Peer() {
  dispose();
}

void Peer::dispose() {
  if (_disposing) return;
  _disposing = true;
  _central.reset();
  _peersMutex.lock();
  _peers.clear();
  _peersMutex.unlock();
  _variableDatabaseIDs.clear();
  if (serviceMessages) serviceMessages->resetEventHandler();
  serviceMessages.reset();
}

void Peer::homegearStarted() {
  std::string source = "homegear";
  auto variables = std::make_shared<std::vector<std::string>>();
  variables->emplace_back("INITIALIZED");
  auto values = std::make_shared<Array>();
  values->emplace_back(std::make_shared<Variable>(true));
  raiseEvent(source, _peerID, -1, variables, values);
}

void Peer::homegearShuttingDown() {
  std::string source = "homegear";
  auto variables = std::make_shared<std::vector<std::string>>();
  variables->emplace_back("DISPOSING");
  auto values = std::make_shared<Array>();
  values->emplace_back(std::make_shared<Variable>(true));
  raiseEvent(source, _peerID, -1, variables, values);
}

//Event handling
void Peer::raiseAddWebserverEventHandler(Rpc::IWebserverEventSink *eventHandler) {
  if (_eventHandler) ((IPeerEventSink *)_eventHandler)->onAddWebserverEventHandler(eventHandler, _webserverEventHandlers);
}

void Peer::raiseRemoveWebserverEventHandler() {
  if (_eventHandler) ((IPeerEventSink *)_eventHandler)->onRemoveWebserverEventHandler(_webserverEventHandlers);
}

void Peer::raiseRPCEvent(std::string &source, uint64_t peerId, int32_t channel, std::string &deviceAddress, std::shared_ptr<std::vector<std::string>> &valueKeys, std::shared_ptr<std::vector<PVariable>> &values) {
  if (_peerID == 0) return;
  if (_eventHandler) ((IPeerEventSink *)_eventHandler)->onRPCEvent(source, peerId, channel, deviceAddress, valueKeys, values);
}

void Peer::raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint) {
  if (_eventHandler) ((IPeerEventSink *)_eventHandler)->onRPCUpdateDevice(id, channel, address, hint);
}

void Peer::raiseEvent(std::string &source, uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> &variables, std::shared_ptr<std::vector<PVariable>> &values) {
  if (_peerID == 0) return;
  if (_eventHandler) ((IPeerEventSink *)_eventHandler)->onEvent(source, peerId, channel, variables, values);
}

void Peer::raiseRunScript(ScriptEngine::PScriptInfo &scriptInfo, bool wait) {
  if (_eventHandler) ((IPeerEventSink *)_eventHandler)->onRunScript(scriptInfo, wait);
}

PVariable Peer::raiseInvokeRpc(std::string &methodName, PArray &parameters) {
  if (_eventHandler) return ((IPeerEventSink *)_eventHandler)->onInvokeRpc(methodName, parameters);
  else return std::make_shared<Variable>();
}
//End event handling

//ServiceMessages event handling
void Peer::onConfigPending(bool configPending) {

}

void Peer::onEvent(std::string &source, uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> &variables, std::shared_ptr<std::vector<PVariable>> &values) {
  raiseEvent(source, peerId, channel, variables, values);
}

void Peer::onRPCEvent(std::string &source, uint64_t id, int32_t channel, std::string &deviceAddress, std::shared_ptr<std::vector<std::string>> &valueKeys, std::shared_ptr<std::vector<PVariable>> &values) {
  raiseRPCEvent(source, id, channel, deviceAddress, valueKeys, values);
}

void Peer::onSaveParameter(std::string name, uint32_t channel, std::vector<uint8_t> &data) {
  try {
    if (_peerID == 0) return; //Peer not saved yet
    if (valuesCentral.find(channel) == valuesCentral.end()) {
      //Service message variables sometimes just don't exist. So only output a debug message.
      if (channel != 0) _bl->out.printWarning("Warning: Could not set parameter " + name + " on channel " + std::to_string(channel) + " for peer " + std::to_string(_peerID) + ". Channel does not exist.");
      else _bl->out.printDebug("Debug: Could not set parameter " + name + " on channel " + std::to_string(channel) + " for peer " + std::to_string(_peerID) + ". Channel does not exist.");
      return;
    }
    if (valuesCentral.at(channel).find(name) == valuesCentral.at(channel).end()) {
      if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Could not set parameter " + name + " on channel " + std::to_string(channel) + " for peer " + std::to_string(_peerID) + ". Parameter does not exist.");
      return;
    }
    RpcConfigurationParameter &parameter = valuesCentral.at(channel).at(name);
    if (parameter.equals(data)) return;
    parameter.setBinaryData(data);
    saveParameter(parameter.databaseId, ParameterGroup::Type::Enum::variables, channel, name, data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::shared_ptr<Database::DataTable> Peer::onGetServiceMessages() {
  return _bl->db->getServiceMessages(_peerID);
}

void Peer::onSaveServiceMessage(Database::DataRow &data) {
  _bl->db->saveServiceMessageAsynchronous(_peerID, data);
}

void Peer::onDeleteServiceMessage(uint64_t databaseID) {
  _bl->db->deleteServiceMessage(databaseID);
}

void Peer::onEnqueuePendingQueues() {
  try {
    if (pendingQueuesEmpty()) return;
    if (!(getRXModes() & HomegearDevice::ReceiveModes::Enum::always) && !(getRXModes() & HomegearDevice::ReceiveModes::Enum::wakeOnRadio)) return;
    enqueuePendingQueues();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}
//End ServiceMessages event handling

void Peer::setID(uint64_t id) {
  if (_peerID == 0) {
    _peerID = id;
    if (serviceMessages) serviceMessages->setPeerId(id);
  } else _bl->out.printError("Cannot reset peer ID");
}

void Peer::setSerialNumber(std::string serialNumber) {
  if (serialNumber.length() > 20) return;
  _serialNumber = serialNumber;
  if (serviceMessages) serviceMessages->setPeerSerial(serialNumber);
  if (_peerID > 0) save(true, false, false);
}

std::string Peer::getName(int32_t channel) {
  std::lock_guard<std::mutex> namesGuard(_namesMutex);
  auto namesIterator = _names.find(channel);
  if (namesIterator == _names.end()) return "";
  return namesIterator->second;
}

void Peer::setName(int32_t channel, std::string value) {
  if (channel != -1) {
    auto channelIterator = _rpcDevice->functions.find(channel);
    if (channelIterator == _rpcDevice->functions.end()) return;
  }

  std::lock_guard<std::mutex> namesGuard(_namesMutex);
  _names[channel] = value;

  std::ostringstream names;
  for (auto namePair : _names) {
    names << std::to_string(namePair.first) << "," << namePair.second << ";";
  }
  std::string namesString = names.str();

  saveVariable(1000, namesString);
}

std::set<int32_t> Peer::getChannelsInRoom(uint64_t roomId) {
  std::set<int32_t> result;
  std::lock_guard<std::mutex> roomGuard(_roomMutex);
  for (auto &roomIterator : _rooms) {
    if (roomIterator.second == roomId) result.emplace(roomIterator.first);
  }
  return result;
}

uint64_t Peer::getRoom(int32_t channel) {
  std::lock_guard<std::mutex> roomGuard(_roomMutex);
  auto roomIterator = _rooms.find(channel);
  if (roomIterator != _rooms.end()) return roomIterator->second;
  return 0;
}

bool Peer::hasRoomInChannels(uint64_t roomId) {
  std::lock_guard<std::mutex> roomGuard(_roomMutex);
  for (auto &roomIterator : _rooms) {
    if (roomIterator.second == roomId) return true;
  }
  return false;
}

bool Peer::roomsSet() {
  std::lock_guard<std::mutex> roomGuard(_roomMutex);
  for (auto &roomIterator : _rooms) {
    if (roomIterator.second != 0) return true;
  }
  return false;
}

bool Peer::setRoom(int32_t channel, uint64_t roomId) {
  if (channel != -1) {
    auto channelIterator = _rpcDevice->functions.find(channel);
    if (channelIterator == _rpcDevice->functions.end()) return false;
  }

  std::lock_guard<std::mutex> roomGuard(_roomMutex);
  _rooms[channel] = roomId;

  std::ostringstream rooms;
  for (auto roomPair : _rooms) {
    rooms << std::to_string(roomPair.first) << "," << std::to_string(roomPair.second) << ";";
  }
  std::string roomString = rooms.str();

  saveVariable(1007, roomString);
  return true;
}

std::unordered_map<int32_t, std::set<uint64_t>> Peer::getCategories() {
  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  return _categories;
}

std::set<uint64_t> Peer::getCategories(int32_t channel) {
  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  auto categoryIterator = _categories.find(channel);
  if (categoryIterator != _categories.end()) return categoryIterator->second;

  return std::set<uint64_t>();
}

std::set<int32_t> Peer::getChannelsInCategory(uint64_t categoryId) {
  std::set<int32_t> result;
  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  for (auto &categoryIterator : _categories) {
    if (categoryIterator.second.find(categoryId) != categoryIterator.second.end()) result.emplace(categoryIterator.first);
  }
  return result;
}

bool Peer::hasCategories() {
  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  return !_categories.empty();
}

bool Peer::hasCategories(int32_t channel) {
  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  auto categoryIterator = _categories.find(channel);
  return categoryIterator != _categories.end();
}

bool Peer::hasCategoryInChannels(uint64_t categoryId) {
  if (categoryId == 0) return false;

  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  for (auto &categoryIterator : _categories) {
    if (categoryIterator.second.find(categoryId) != categoryIterator.second.end()) return true;
  }
  return false;
}

bool Peer::hasCategory(int32_t channel, uint64_t categoryId) {
  if (categoryId == 0) return false;
  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  auto categoryIterator = _categories.find(channel);
  if (categoryIterator == _categories.end()) return false;
  return categoryIterator->second.find(categoryId) != categoryIterator->second.end();
}

bool Peer::addCategory(int32_t channel, uint64_t categoryId) {
  if (categoryId == 0) return false;

  if (channel != -1) {
    auto channelIterator = _rpcDevice->functions.find(channel);
    if (channelIterator == _rpcDevice->functions.end()) return false;
  }

  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  _categories[channel].emplace(categoryId);

  std::ostringstream categories;
  for (auto categoryPair : _categories) {
    categories << categoryPair.first << "~";
    for (auto category : categoryPair.second) {
      categories << std::to_string(category) << ",";
    }
    categories << ";";
  }
  std::string categoryString = categories.str();

  saveVariable(1008, categoryString);
  return true;
}

bool Peer::removeCategory(int32_t channel, uint64_t categoryId) {
  if (categoryId == 0) return false;

  std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
  auto channelIterator = _categories.find(channel);
  if (channelIterator == _categories.end()) return false;

  channelIterator->second.erase(categoryId);
  if (channelIterator->second.empty()) _categories.erase(channel);

  std::ostringstream categories;
  for (auto categoryPair : _categories) {
    categories << categoryPair.first << "~";
    for (auto category : categoryPair.second) {
      categories << std::to_string(category) << ",";
    }
    categories << ";";
  }
  std::string categoryString = categories.str();

  saveVariable(1008, categoryString);
  return true;
}

HomegearDevice::ReceiveModes::Enum Peer::getRXModes() {
  try {
    if (_rpcDevice) {
      _rxModes = _rpcDevice->receiveModes;
      std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator configIterator = configCentral.find(0);
      if (configIterator != configCentral.end()) {
        std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = configIterator->second.find("WAKE_ON_RADIO");
        if (parameterIterator == configIterator->second.end()) parameterIterator = configIterator->second.find("BURST_RX");
        if (parameterIterator == configIterator->second.end()) parameterIterator = configIterator->second.find("LIVE_MODE_RX");
        if (parameterIterator != configIterator->second.end()) {
          if (!parameterIterator->second.rpcParameter) return _rxModes;
          std::vector<uint8_t> data = parameterIterator->second.getBinaryData();
          if (parameterIterator->second.rpcParameter->convertFromPacket(data, Role(), false)->booleanValue) {
            _rxModes = (HomegearDevice::ReceiveModes::Enum)(_rxModes | HomegearDevice::ReceiveModes::Enum::wakeOnRadio);
          } else {
            _rxModes = (HomegearDevice::ReceiveModes::Enum)(_rxModes & (~HomegearDevice::ReceiveModes::Enum::wakeOnRadio));
          }
        }
      }
    }
    return _rxModes;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return _rxModes;
}

void Peer::setLastPacketReceived() {
  uint32_t now = HelperFunctions::getTimeSeconds();
  if (_lastPacketReceived == now) return;
  _lastPacketReceived = now;
  std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator valuesIterator = valuesCentral.find(0);
  if (valuesIterator != valuesCentral.end()) {
    std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = valuesIterator->second.find("LAST_PACKET_RECEIVED");
    if (parameterIterator != valuesIterator->second.end() && parameterIterator->second.rpcParameter) {
      std::vector<uint8_t> parameterData;
      parameterIterator->second.rpcParameter->convertToPacket(std::make_shared<Variable>(_lastPacketReceived), parameterIterator->second.mainRole(), parameterData);
      parameterIterator->second.setBinaryData(parameterData);
      if (parameterIterator->second.databaseId > 0) saveParameter(parameterIterator->second.databaseId, parameterData);
      else saveParameter(0, ParameterGroup::Type::Enum::variables, 0, "LAST_PACKET_RECEIVED", parameterData);

      // Don't raise event as this is not necessary and some programs like OpenHAB have problems with it
    }
  }
}

std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>> Peer::getPeers() {
  _peersMutex.lock();
  try {
    std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>> peers = _peers;
    _peersMutex.unlock();
    return peers;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  _peersMutex.unlock();
  return std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>>();
}

std::shared_ptr<BasicPeer> Peer::getPeer(int32_t channel, std::string serialNumber, int32_t remoteChannel) {
  _peersMutex.lock();
  try {
    if (_peers.find(channel) == _peers.end()) {
      _peersMutex.unlock();
      return std::shared_ptr<BasicPeer>();
    }

    for (std::vector<std::shared_ptr<BasicPeer>>::iterator i = _peers[channel].begin(); i != _peers[channel].end(); ++i) {
      if ((*i)->serialNumber.empty()) {
        std::shared_ptr<ICentral> central(getCentral());
        if (central) {
          std::shared_ptr<Peer> peer(central->getPeer((*i)->id));
          if (peer) (*i)->serialNumber = peer->getSerialNumber();
        }
      }
      if ((*i)->serialNumber == serialNumber && (remoteChannel < 0 || remoteChannel == (*i)->channel)) {
        std::shared_ptr<BasicPeer> peer = *i;
        _peersMutex.unlock();
        return peer;
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  _peersMutex.unlock();
  return std::shared_ptr<BasicPeer>();
}

std::shared_ptr<BasicPeer> Peer::getPeer(int32_t channel, int32_t address, int32_t remoteChannel) {
  _peersMutex.lock();
  try {
    if (_peers.find(channel) == _peers.end()) {
      _peersMutex.unlock();
      return std::shared_ptr<BasicPeer>();
    }

    for (std::vector<std::shared_ptr<BasicPeer>>::iterator i = _peers[channel].begin(); i != _peers[channel].end(); ++i) {
      if ((*i)->address == address && (remoteChannel < 0 || remoteChannel == (*i)->channel)) {
        std::shared_ptr<BasicPeer> peer = *i;
        _peersMutex.unlock();
        return peer;
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  _peersMutex.unlock();
  return std::shared_ptr<BasicPeer>();
}

std::shared_ptr<BasicPeer> Peer::getPeer(int32_t channel, uint64_t id, int32_t remoteChannel) {
  _peersMutex.lock();
  try {
    if (_peers.find(channel) == _peers.end()) {
      _peersMutex.unlock();
      return std::shared_ptr<BasicPeer>();
    }

    bool modified = false;
    for (std::vector<std::shared_ptr<BasicPeer>>::iterator i = _peers[channel].begin(); i != _peers[channel].end(); ++i) {
      if ((*i)->id == 0) //ID is not always available after pairing. So try to get ID if not there.
      {
        std::shared_ptr<Peer> peer1 = getCentral()->getPeer((*i)->serialNumber);
        std::shared_ptr<Peer> peer2 = getCentral()->getPeer((*i)->address);
        if (peer1) {
          (*i)->id = peer1->getID();
          modified = true;
        } else if (peer2) {
          (*i)->id = peer2->getID();
          modified = true;
        } else if ((*i)->isVirtual && (*i)->address == getCentral()->getAddress()) {
          (*i)->id = 0xFFFFFFFFFFFFFFFF;
          modified = true;
        }
      }
      if ((*i)->id == id && (remoteChannel < 0 || remoteChannel == (*i)->channel)) {
        std::shared_ptr<BasicPeer> peer = *i;
        _peersMutex.unlock();
        if (modified) savePeers();
        return peer;
      }
    }
    _peersMutex.unlock();
    if (modified) savePeers();
    return std::shared_ptr<BasicPeer>();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  _peersMutex.unlock();
  return std::shared_ptr<BasicPeer>();
}

void Peer::updatePeer(uint64_t oldId, uint64_t newId) {
  try {
    bool changed = false;
    {
      std::lock_guard<std::mutex> peersGuard(_peersMutex);
      for (std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>>::iterator i = _peers.begin(); i != _peers.end(); ++i) {
        for (std::vector<std::shared_ptr<BasicPeer>>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
          if ((*j)->id == oldId) {
            (*j)->id = newId;
            changed = true;
          }
        }
      }
    }
    if (changed) savePeers();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::deleteFromDatabase() {
  try {
    deleting = true;
    std::string dataId = "";
    _bl->db->deleteMetadata(_peerID, _serialNumber, dataId);
    _bl->db->deletePeer(_peerID);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::initializeCentralConfig() {
  std::string savepointName("PeerConfig" + std::to_string(_peerID));
  try {
    if (!_rpcDevice) {
      _bl->out.printWarning("Warning: Tried to initialize peer's central config without rpcDevice being set.");
      return;
    }
    _bl->db->createSavepointAsynchronous(savepointName);
    for (Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i) {
      initializeMasterSet(i->first, i->second->configParameters);
      initializeValueSet(i->first, i->second->variables);
      for (std::vector<PFunction>::iterator j = i->second->alternativeFunctions.begin(); j != i->second->alternativeFunctions.end(); ++j) {
        initializeMasterSet(i->first, (*j)->configParameters);
        initializeValueSet(i->first, (*j)->variables);
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  _bl->db->releaseSavepointAsynchronous(savepointName);
}

void Peer::initializeMasterSet(int32_t channel, PConfigParameters masterSet) {
  try {
    if (!masterSet || masterSet->parameters.empty()) return;
    auto channelIterator = configCentral.find(channel);
    if (channelIterator == configCentral.end()) channelIterator = configCentral.emplace(channel, std::unordered_map<std::string, RpcConfigurationParameter>()).first;
    for (auto &parameter : masterSet->parameters) {
      if (!parameter.second) continue;
      if (!parameter.second->id.empty() && channelIterator->second.find(parameter.second->id) == channelIterator->second.end()) {
        RpcConfigurationParameter parameterStruct;
        parameterStruct.rpcParameter = parameter.second;
        setDefaultValue(parameterStruct);

        std::vector<uint8_t> data = parameterStruct.getBinaryData();
        parameterStruct.databaseId = createParameter(ParameterGroup::Type::config, channel, parameter.second->id, data);
        channelIterator->second.emplace(parameter.second->id, std::move(parameterStruct));
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::initializeValueSet(int32_t channel, PVariables valueSet) {
  try {
    if (!valueSet || valueSet->parameters.empty()) return;
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) {
      channelIterator = valuesCentral.emplace(channel, std::unordered_map<std::string, RpcConfigurationParameter>()).first;
    }
    for (auto &parameter : valueSet->parameters) {
      if (!parameter.second) continue;
      if (!parameter.second->id.empty() && channelIterator->second.find(parameter.second->id) == channelIterator->second.end()) {
        RpcConfigurationParameter parameterStruct;
        parameterStruct.rpcParameter = parameter.second;
        setDefaultValue(parameterStruct);

        std::vector<uint8_t> data = parameterStruct.getBinaryData();
        parameterStruct.databaseId = createParameter(ParameterGroup::Type::variables, channel, parameter.second->id, data);
        channelIterator->second.emplace(parameter.second->id, std::move(parameterStruct));

        //Add roles
        for (auto &role : parameter.second->roles) {
          addRoleToVariable(channel, parameter.second->id, role.second.id, role.second.direction, role.second.invert, role.second.scale, role.second.scaleInfo);
        }
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::setDefaultValue(RpcConfigurationParameter &parameter) {
  try {
    std::vector<uint8_t> parameterData;
    auto defaultValue = parameter.rpcParameter->logical->getDefaultValue();
    if (!convertToPacketHook(parameter, defaultValue, parameterData)) parameter.rpcParameter->convertToPacket(parameter.rpcParameter->logical->getDefaultValue(), Role(), parameterData);
    parameter.setBinaryData(parameterData);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::save(bool savePeer, bool variables, bool centralConfig) {
  std::string savepointName("peer_54" + std::to_string(_parentID) + std::to_string(_peerID));
  try {
    if (deleting || (isTeam() && !_saveTeam)) return;

    if (savePeer) {
      uint64_t result = _bl->db->savePeer(_peerID, _parentID, _address, _serialNumber, _deviceType);
      if (_peerID == 0 && result > 0) setID(result);
    }
    if (variables || centralConfig) _bl->db->createSavepointAsynchronous(savepointName);
    if (variables) saveVariables();
    if (centralConfig) saveConfig();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  if (variables || centralConfig) _bl->db->releaseSavepointAsynchronous(savepointName);
}

void Peer::saveParameter(uint32_t parameterID, std::vector<uint8_t> &value) {
  try {
    if (parameterID == 0) {
      if (!isTeam() || _saveTeam) _bl->out.printError("Error: Peer " + std::to_string(_peerID) + ": Tried to save parameter without parameterID");
      return;
    }
    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(value));
    data.push_back(std::make_shared<Database::DataColumn>(parameterID));
    _bl->db->savePeerParameterAsynchronous(data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::saveParameter(uint32_t parameterID, uint32_t address, std::vector<uint8_t> &value) {
  try {
    if (parameterID > 0) {
      saveParameter(parameterID, value);
      return;
    }
    if (_peerID == 0 || (isTeam() && !_saveTeam)) return;
    //Creates a new entry for parameter in database
    Database::DataRow data;
    data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
    data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(0)));
    data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(address)));
    data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(0)));
    data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(0)));
    data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(std::string(""))));
    data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(value)));
    _bl->db->savePeerParameterAsynchronous(data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

uint64_t Peer::createParameter(ParameterGroup::Type::Enum parameterSetType, uint32_t channel, const std::string &parameterName, std::vector<uint8_t> &value, int32_t remoteAddress, uint32_t remoteChannel) {
  try {
    if (_peerID == 0 || (isTeam() && !_saveTeam)) return 0;
    //Creates a new entry for parameter in database
    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(_peerID));
    data.push_back(std::make_shared<Database::DataColumn>((uint32_t)parameterSetType));
    data.push_back(std::make_shared<Database::DataColumn>(channel));
    data.push_back(std::make_shared<Database::DataColumn>(remoteAddress));
    data.push_back(std::make_shared<Database::DataColumn>(remoteChannel));
    data.push_back(std::make_shared<Database::DataColumn>(parameterName));
    data.push_back(std::make_shared<Database::DataColumn>(value));
    return _bl->db->savePeerParameterSynchronous(data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return 0;
}

void Peer::saveParameter(uint32_t parameterID, ParameterGroup::Type::Enum parameterSetType, uint32_t channel, const std::string &parameterName, std::vector<uint8_t> &value, int32_t remoteAddress, uint32_t remoteChannel) {
  try {
    if (parameterID > 0) {
      saveParameter(parameterID, value);
      return;
    }
    if (_peerID == 0 || (isTeam() && !_saveTeam)) return;
    //Creates a new entry for parameter in database
    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(_peerID));
    data.push_back(std::make_shared<Database::DataColumn>((uint32_t)parameterSetType));
    data.push_back(std::make_shared<Database::DataColumn>(channel));
    data.push_back(std::make_shared<Database::DataColumn>(remoteAddress));
    data.push_back(std::make_shared<Database::DataColumn>(remoteChannel));
    data.push_back(std::make_shared<Database::DataColumn>(parameterName));
    data.push_back(std::make_shared<Database::DataColumn>(value));
    _bl->db->savePeerParameterAsynchronous(data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::saveSpecialTypeParameter(uint32_t parameterID,
                                    ParameterGroup::Type::Enum parameterSetType,
                                    uint32_t channel,
                                    const std::string &parameterName,
                                    std::vector<uint8_t> &value,
                                    int32_t specialType,
                                    const PVariable &metadata,
                                    const std::string &roles) {
  try {
    if (parameterID > 0) {
      saveParameter(parameterID, value);
      return;
    }
    if (_peerID == 0 || (isTeam() && !_saveTeam)) return;

    Rpc::RpcEncoder encoder(_bl, true, true);
    std::vector<uint8_t> encodedMetadata;
    encoder.encodeResponse(metadata, encodedMetadata);

    //Creates a new entry for parameter in database
    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(_peerID));
    data.push_back(std::make_shared<Database::DataColumn>((uint32_t)parameterSetType));
    data.push_back(std::make_shared<Database::DataColumn>(channel));
    data.push_back(std::make_shared<Database::DataColumn>(parameterName));
    data.push_back(std::make_shared<Database::DataColumn>(value));
    data.push_back(std::make_shared<Database::DataColumn>(specialType));
    data.push_back(std::make_shared<Database::DataColumn>(encodedMetadata));
    data.push_back(std::make_shared<Database::DataColumn>(roles));
    _bl->db->saveSpecialPeerParameterAsynchronous(data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::loadVariables(ICentral *central, std::shared_ptr<Database::DataTable> &rows) {
  try {
    if (!rows) return;
    for (Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row) {
      _variableDatabaseIDs[row->second.at(2)->intValue] = row->second.at(0)->intValue;
      switch (row->second.at(2)->intValue) {
        case 1000: {
          std::vector<std::string> nameStrings = HelperFunctions::splitAll(row->second.at(4)->textValue, ';');
          for (auto nameString : nameStrings) {
            if (nameString.empty()) continue;
            auto namePair = HelperFunctions::splitFirst(nameString, ',');
            if (namePair.second.empty() && !Math::isNumber(namePair.first)) _names[-1] = namePair.first;
            else {
              int32_t channel = Math::getNumber(namePair.first);
              _names[channel] = namePair.second;
            }
          }
          break;
        }
        case 1001:_firmwareVersion = row->second.at(3)->intValue;
          break;
        case 1002:_deviceType = (uint64_t)row->second.at(3)->intValue;
          if ((_deviceType & 0xFFFFFFFF00000000ull) == 0xFFFFFFFF00000000ull) {
            _bl->out.printWarning("Warning: Converting 32 bit device type to 64 bit.");
            _deviceType = _deviceType & 0x00000000FFFFFFFFull;
            saveVariable(1002, (int64_t)_deviceType);
          }
          break;
        case 1003:_firmwareVersionString = row->second.at(4)->textValue;
          break;
        case 1004:_ip = row->second.at(4)->textValue;
          break;
        case 1005:_idString = row->second.at(4)->textValue;
          break;
        case 1006:_typeString = row->second.at(4)->textValue;
          break;
        case 1007: {
          std::vector<std::string> roomStrings = HelperFunctions::splitAll(row->second.at(4)->textValue, ';');
          for (auto roomString : roomStrings) {
            if (roomString.empty()) continue;
            auto roomPair = HelperFunctions::splitFirst(roomString, ',');
            int32_t channel = Math::getNumber(roomPair.first);
            uint64_t room = Math::getNumber64(roomPair.second);
            if (room != 0) _rooms[channel] = room;
          }
          break;
        }
        case 1008: {
          std::vector<std::string> categoryStrings = HelperFunctions::splitAll(row->second.at(4)->textValue, ';');
          for (auto categoryString : categoryStrings) {
            if (categoryString.empty()) continue;
            auto categoryPair = HelperFunctions::splitFirst(categoryString, '~');
            if (categoryPair.first.empty() || categoryPair.second.empty()) continue;
            int32_t channel = Math::getNumber(categoryPair.first);
            auto categoryStrings2 = HelperFunctions::splitAll(categoryPair.second, ',');
            for (auto categoryString2 : categoryStrings2) {
              uint64_t category = Math::getNumber64(categoryString2);
              if (category != 0) _categories[channel].emplace(category);
            }
          }
          break;
        }
      }
    }
    return;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::saveVariables() {
  try {
    if (_peerID == 0 || (isTeam() && !_saveTeam)) return;
    saveVariable(1001, _firmwareVersion);
    saveVariable(1002, (int64_t)_deviceType);
    saveVariable(1003, _firmwareVersionString);
    saveVariable(1004, _ip);
    saveVariable(1005, _idString);
    saveVariable(1006, _typeString);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::saveVariable(uint32_t index, int32_t intValue) {
  try {
    if (isTeam() && !_saveTeam) return;
    bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
    Database::DataRow data;
    if (idIsKnown) {
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
      _bl->db->savePeerVariableAsynchronous(data);
    } else {
      if (_peerID == 0) return;
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      _bl->db->savePeerVariableAsynchronous(data);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::saveVariable(uint32_t index, int64_t intValue) {
  try {
    if (isTeam() && !_saveTeam) return;
    bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
    Database::DataRow data;
    if (idIsKnown) {
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
      _bl->db->savePeerVariableAsynchronous(data);
    } else {
      if (_peerID == 0) return;
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      _bl->db->savePeerVariableAsynchronous(data);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::saveVariable(uint32_t index, std::string &stringValue) {
  try {
    if (isTeam() && !_saveTeam) return;
    bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
    Database::DataRow data;
    if (idIsKnown) {
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(stringValue)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
      _bl->db->savePeerVariableAsynchronous(data);
    } else {
      if (_peerID == 0) return;
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(stringValue)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      _bl->db->savePeerVariableAsynchronous(data);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::saveVariable(uint32_t index, std::vector<char> &binaryValue) {
  try {
    if (isTeam() && !_saveTeam) return;
    bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
    Database::DataRow data;
    if (idIsKnown) {
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
      _bl->db->savePeerVariableAsynchronous(data);
    } else {
      if (_peerID == 0) return;
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
      _bl->db->savePeerVariableAsynchronous(data);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::saveVariable(uint32_t index, std::vector<uint8_t> &binaryValue) {
  try {
    if (isTeam() && !_saveTeam) return;
    bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
    Database::DataRow data;
    if (idIsKnown) {
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
      _bl->db->savePeerVariableAsynchronous(data);
    } else {
      if (_peerID == 0) return;
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
      data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
      _bl->db->savePeerVariableAsynchronous(data);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

DeviceDescription::PParameter Peer::createRoleRpcParameter(PVariable &variableInfo, const std::string &baseVariableName, const PParameterGroup &parameterGroup) {
  try {
    auto idIterator = variableInfo->structValue->find("id");
    auto typeIterator = variableInfo->structValue->find("type");
    if (idIterator == variableInfo->structValue->end() || idIterator->second->stringValue.empty() || typeIterator == variableInfo->structValue->end() || baseVariableName.empty()) {
      return PParameter();
    }

    auto defaultValueIterator = variableInfo->structValue->find("default");
    PVariable defaultValue;
    if (defaultValueIterator != variableInfo->structValue->end()) defaultValue = defaultValueIterator->second;
    auto minValueIterator = variableInfo->structValue->find("min");
    PVariable minValue;
    if (minValueIterator != variableInfo->structValue->end()) minValue = minValueIterator->second;
    auto maxValueIterator = variableInfo->structValue->find("max");
    PVariable maxValue;
    if (maxValueIterator != variableInfo->structValue->end()) maxValue = maxValueIterator->second;

    RpcConfigurationParameter parameterStruct;

    auto parameter = std::make_shared<Parameter>(_bl, parameterGroup);
    parameter->id = baseVariableName + ".RV." + idIterator->second->stringValue;
    parameter->readable = true;
    parameter->writeable = true;
    parameter->casts.emplace_back(std::make_shared<ParameterCast::RpcBinary>(_bl));
    parameter->physical = std::make_shared<PhysicalNone>(_bl);
    parameter->physical->operationType = IPhysical::OperationType::Enum::store;

    if (typeIterator->second->stringValue == "ACTION") {
      parameter->logical = std::make_shared<LogicalAction>(_bl);
    } else if (typeIterator->second->stringValue == "BOOL") {
      auto logicalBoolean = std::make_shared<LogicalBoolean>(_bl);
      parameter->logical = logicalBoolean;
      if (defaultValue) {
        logicalBoolean->defaultValueExists = true;
        logicalBoolean->defaultValue = defaultValue->booleanValue;
      }
    } else if (typeIterator->second->stringValue == "INTEGER") {
      auto logicalInteger = std::make_shared<LogicalInteger>(_bl);
      parameter->logical = logicalInteger;
      if (defaultValue) {
        logicalInteger->defaultValueExists = true;
        logicalInteger->defaultValue = defaultValue->integerValue;
      }
      if (minValue) logicalInteger->minimumValue = minValue->integerValue;
      if (maxValue) logicalInteger->maximumValue = maxValue->integerValue;
    } else if (typeIterator->second->stringValue == "INTEGER64") {
      auto logicalInteger = std::make_shared<LogicalInteger64>(_bl);
      parameter->logical = logicalInteger;
      if (defaultValue) {
        logicalInteger->defaultValueExists = true;
        logicalInteger->defaultValue = defaultValue->integerValue64;
      }
      if (minValue) logicalInteger->minimumValue = minValue->integerValue64;
      if (maxValue) logicalInteger->maximumValue = maxValue->integerValue64;
    } else if (typeIterator->second->stringValue == "ENUM") {
      auto enumerationIterator = variableInfo->structValue->find("enumeration");

      auto logicalEnumeration = std::make_shared<LogicalEnumeration>(_bl);
      parameter->logical = logicalEnumeration;
      if (enumerationIterator != variableInfo->structValue->end()) {
        logicalEnumeration->values.reserve(enumerationIterator->second->arrayValue->size());
        int32_t index = 0;
        for (auto &entry : *enumerationIterator->second->arrayValue) {
          logicalEnumeration->values.emplace_back(std::move(EnumerationValue(entry->stringValue, index++)));
        }
      }
      if (defaultValue) {
        logicalEnumeration->defaultValueExists = true;
        logicalEnumeration->defaultValue = defaultValue->integerValue64;
      }
      logicalEnumeration->minimumValue = 0;
      logicalEnumeration->maximumValue = logicalEnumeration->values.size() - 1;
    } else if (typeIterator->second->stringValue == "FLOAT") {
      auto logicalDecimal = std::make_shared<LogicalDecimal>(_bl);
      parameter->logical = logicalDecimal;
      if (defaultValue) {
        logicalDecimal->defaultValueExists = true;
        logicalDecimal->defaultValue = defaultValue->floatValue;
      }
      if (minValue) logicalDecimal->minimumValue = minValue->floatValue;
      if (maxValue) logicalDecimal->maximumValue = maxValue->floatValue;
    } else if (typeIterator->second->stringValue == "STRING") {
      auto logicalString = std::make_shared<LogicalString>(_bl);
      parameter->logical = logicalString;
      if (defaultValue) {
        logicalString->defaultValueExists = true;
        logicalString->defaultValue = defaultValue->stringValue;
      }
    } else {
      _bl->out.printWarning("Warning: Unsupported variable type in addRoleToVariable: " + typeIterator->second->stringValue);
      return PParameter();
    }

    return parameter;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return PParameter();
}

void Peer::saveConfig() {
  try {
    if (_peerID == 0 || (isTeam() && !_saveTeam)) return;
    for (std::unordered_map<uint32_t, ConfigDataBlock>::iterator i = binaryConfig.begin(); i != binaryConfig.end(); ++i) {
      std::string emptyString;
      std::vector<uint8_t> configData = i->second.getBinaryData();
      if (i->second.databaseId > 0) saveParameter(i->second.databaseId, configData);
      else saveParameter(0, i->first, configData);
    }
    for (std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator i = configCentral.begin(); i != configCentral.end(); ++i) {
      for (std::unordered_map<std::string, RpcConfigurationParameter>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
        if (j->first.empty()) {
          _bl->out.printError("Error: Parameter has no id.");
          continue;
        }
        std::vector<uint8_t> data = j->second.getBinaryData();
        if (j->second.databaseId > 0) saveParameter(j->second.databaseId, data);
        else saveParameter(0, ParameterGroup::Type::Enum::config, i->first, j->first, data);
      }
    }
    for (std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator i = valuesCentral.begin(); i != valuesCentral.end(); ++i) {
      for (std::unordered_map<std::string, RpcConfigurationParameter>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
        if (j->first.empty()) {
          _bl->out.printError("Error: Parameter has no id.");
          continue;
        }
        std::vector<uint8_t> data = j->second.getBinaryData();
        if (j->second.databaseId > 0) saveParameter(j->second.databaseId, data);
        else saveParameter(0, ParameterGroup::Type::Enum::variables, i->first, j->first, data);
      }
    }
    for (std::unordered_map<uint32_t, std::unordered_map<int32_t, std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>>>::iterator i = linksCentral.begin(); i != linksCentral.end(); ++i) {
      for (std::unordered_map<int32_t, std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
        for (std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator k = j->second.begin(); k != j->second.end(); ++k) {
          for (std::unordered_map<std::string, RpcConfigurationParameter>::iterator l = k->second.begin(); l != k->second.end(); ++l) {
            if (l->first.empty()) {
              _bl->out.printError("Error: Parameter has no id.");
              continue;
            }
            std::vector<uint8_t> data = l->second.getBinaryData();
            if (l->second.databaseId > 0) saveParameter(l->second.databaseId, data);
            else saveParameter(0, ParameterGroup::Type::Enum::link, i->first, l->first, data, j->first, k->first);
          }
        }
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::loadConfig() {
  try {
    struct ParameterInfo {
      ParameterGroup::Type::Enum parameterGroupType;
      uint32_t channel;
      int32_t remoteAddress;
      int32_t remoteChannel;
      std::string parameterName;
      RpcConfigurationParameter parameter;
      PFunction function;
      int32_t specialType;
      PVariable metadata;
    };

    Rpc::RpcDecoder rpcDecoder(_bl, false, false);
    Database::DataRow data;
    std::shared_ptr<Database::DataTable> rows = _bl->db->getPeerParameters(_peerID);
    std::shared_ptr<ParameterInfo> parameterGroupSelector;
    std::vector<std::shared_ptr<ParameterInfo>> parameters;
    parameters.reserve(rows->size());
    for (Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row) {
      uint64_t databaseId = row->second.at(0)->intValue;
      ParameterGroup::Type::Enum parameterGroupType = (ParameterGroup::Type::Enum)row->second.at(2)->intValue;

      if (parameterGroupType == ParameterGroup::Type::Enum::none) {
        uint32_t index = row->second.at(3)->intValue;
        ConfigDataBlock &config = binaryConfig[index];
        config.databaseId = databaseId;
        std::vector<uint8_t> configData;
        configData.insert(configData.begin(), row->second.at(7)->binaryValue->begin(), row->second.at(7)->binaryValue->end());
        config.setBinaryData(configData);
      } else {
        auto parameterInfo = std::make_shared<ParameterInfo>();
        parameterInfo->parameterGroupType = parameterGroupType;
        parameterInfo->channel = row->second.at(3)->intValue;
        parameterInfo->remoteAddress = row->second.at(4)->intValue;
        parameterInfo->remoteChannel = row->second.at(5)->intValue;
        parameterInfo->parameterName = row->second.at(6)->textValue;
        if (parameterInfo->parameterName.empty()) {
          _bl->out.printCritical("Critical: Added central config parameter without id. Device: " + std::to_string(_peerID) + " Channel: " + std::to_string(parameterInfo->channel));
          data.clear();
          data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
          data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(std::string(""))));
          _bl->db->deletePeerParameter(_peerID, data);
          continue;
        }

        parameterInfo->parameter.databaseId = databaseId;
        std::vector<uint8_t> parameterData;
        parameterData.insert(parameterData.begin(), row->second.at(7)->binaryValue->begin(), row->second.at(7)->binaryValue->end());
        parameterInfo->parameter.setBinaryData(parameterData);
        if (!_rpcDevice) {
          _bl->out.printCritical("Critical: No xml-rpc device found for peer " + std::to_string(_peerID) + ".");
          continue;
        }

        { // Rooms / categories / roles
          parameterInfo->parameter.setRoom((uint64_t)row->second.at(8)->intValue);

          std::vector<std::string> categoryStrings = HelperFunctions::splitAll(row->second.at(9)->textValue, ',');
          for (auto categoryString : categoryStrings) {
            if (categoryString.empty()) continue;
            uint64_t category = (uint64_t)Math::getNumber64(categoryString);
            if (category != 0) parameterInfo->parameter.addCategory(category);
          }

          std::vector<std::string> roleStrings = HelperFunctions::splitAll(row->second.at(10)->textValue, ',');
          for (auto roleString : roleStrings) {
            if (roleString.empty()) continue;
            auto parts = HelperFunctions::splitAll(roleString, '-');
            uint64_t roleId = Math::getUnsignedNumber64(parts.at(0));
            RoleDirection direction = parts.size() >= 2 ? (RoleDirection)Math::getNumber(parts.at(1)) : RoleDirection::both;
            bool invert = parts.size() >= 3 ? (bool)Math::getNumber(parts.at(2)) : false;
            bool scale = parts.size() >= 8 ? (bool)Math::getNumber(parts.at(3)) : false;
            RoleScaleInfo scaleInfo;
            if (scale) {
              scaleInfo.valueSet = (bool)Math::getNumber(parts.at(4));
              if (scaleInfo.valueSet) {
                scaleInfo.valueMin = Math::getDouble(parts.at(5));
                scaleInfo.valueMax = Math::getDouble(parts.at(6));
              }
              scaleInfo.scaleMin = Math::getDouble(parts.at(7));
              scaleInfo.scaleMax = Math::getDouble(parts.at(8));
            }
            if (roleId != 0) parameterInfo->parameter.addRole(roleId, direction, invert, scale, scaleInfo);
          }
        }

        parameterInfo->specialType = row->second.at(11)->intValue;
        if (row->second.at(12)->binaryValue->empty()) parameterInfo->metadata = std::make_shared<Variable>();
        else parameterInfo->metadata = rpcDecoder.decodeResponse(*row->second.at(12)->binaryValue);

        Functions::iterator functionIterator = _rpcDevice->functions.find(parameterInfo->channel);
        if (functionIterator == _rpcDevice->functions.end()) {
          _bl->out.printError("Error: Added peer parameter with unknown channel. Device: " + std::to_string(_peerID) + " Channel: " + std::to_string(parameterInfo->channel));
          data.clear();
          data.push_back(std::make_shared<Database::DataColumn>(_peerID));
          data.push_back(std::make_shared<Database::DataColumn>((int64_t)parameterInfo->parameterGroupType));
          data.push_back(std::make_shared<Database::DataColumn>(parameterInfo->channel));
          data.push_back(std::make_shared<Database::DataColumn>(parameterInfo->parameterName));
          data.push_back(std::make_shared<Database::DataColumn>(parameterInfo->remoteAddress));
          data.push_back(std::make_shared<Database::DataColumn>(parameterInfo->remoteChannel));
          _bl->db->deletePeerParameter(_peerID, data);
          continue;
        }
        parameterInfo->function = functionIterator->second;
        if (parameterGroupType == ParameterGroup::Type::Enum::config && parameterInfo->function->parameterGroupSelector && !parameterInfo->function->alternativeFunctions.empty()
            && parameterInfo->parameterName == parameterInfo->function->parameterGroupSelector->id) {
          std::vector<uint8_t> parameterData = parameterInfo->parameter.getBinaryData();
          int32_t index = parameterInfo->function->parameterGroupSelector->convertFromPacket(parameterData, Role(), false)->integerValue;
          if (parameterInfo->function->parameterGroupSelector->logical->type != ILogical::Type::Enum::tBoolean && index > (signed)parameterInfo->function->alternativeFunctions.size()) {
            _bl->out.printError("Error: Parameter group selector \"" + parameterInfo->parameterName + "\" has invalid value (" + std::to_string(parameterData.back()) + "). Peer: " + std::to_string(_peerID) + ".");
            continue;
          }
          parameterInfo->parameter.rpcParameter = parameterInfo->function->parameterGroupSelector;
          parameterGroupSelector = parameterInfo;
        }
        parameters.push_back(parameterInfo);
      }
    }

    std::unordered_set<std::shared_ptr<ParameterInfo>> roleParameters;
    for (auto &parameter : parameters) {
      PParameterGroup currentParameterGroup;
      if (parameterGroupSelector && parameter->function->parameterGroupSelector && !parameter->function->alternativeFunctions.empty()) {
        std::vector<uint8_t> parameterData = parameterGroupSelector->parameter.getBinaryData();
        int32_t index = parameterGroupSelector->parameter.rpcParameter->logical->type == ILogical::Type::Enum::tBoolean ? (int32_t)parameterGroupSelector->parameter.rpcParameter->convertFromPacket(parameterData,
                                                                                                                                                                                                     Role(),
                                                                                                                                                                                                     false)->booleanValue : parameterGroupSelector->parameter.rpcParameter->convertFromPacket(
            parameterData,
            Role(),
            false)->integerValue;
        if (index == 0) {
          if (parameter->parameterGroupType == ParameterGroup::Type::Enum::config) {
            currentParameterGroup = parameter->function->configParameters;
            auto parameterIterator = parameter->function->configParameters->parameters.find(parameter->parameterName);
            if (parameterIterator != parameter->function->configParameters->parameters.end()) parameter->parameter.rpcParameter = parameterIterator->second;
          } else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::variables) {
            currentParameterGroup = parameter->function->variables;
            auto parameterIterator = parameter->function->variables->parameters.find(parameter->parameterName);
            if (parameterIterator != parameter->function->variables->parameters.end()) parameter->parameter.rpcParameter = parameterIterator->second;
          } else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::link) {
            currentParameterGroup = parameter->function->linkParameters;
            auto parameterIterator = parameter->function->linkParameters->parameters.find(parameter->parameterName);
            if (parameterIterator != parameter->function->linkParameters->parameters.end()) parameter->parameter.rpcParameter = parameterIterator->second;
          }
        } else {
          index--;
          PFunction alternativeFunction = parameter->function->alternativeFunctions.at(index);

          if (parameter->parameterGroupType == ParameterGroup::Type::Enum::config) {
            currentParameterGroup = alternativeFunction->configParameters;
            auto parameterIterator = alternativeFunction->configParameters->parameters.find(parameter->parameterName);
            if (parameterIterator != alternativeFunction->configParameters->parameters.end()) parameter->parameter.rpcParameter = parameterIterator->second;
          } else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::variables) {
            currentParameterGroup = alternativeFunction->variables;
            auto parameterIterator = alternativeFunction->variables->parameters.find(parameter->parameterName);
            if (parameterIterator != alternativeFunction->variables->parameters.end()) parameter->parameter.rpcParameter = parameterIterator->second;
          } else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::link) {
            currentParameterGroup = alternativeFunction->linkParameters;
            auto parameterIterator = alternativeFunction->linkParameters->parameters.find(parameter->parameterName);
            if (parameterIterator != alternativeFunction->linkParameters->parameters.end()) parameter->parameter.rpcParameter = parameterIterator->second;
          }
        }
      } else {
        if (parameter->parameterGroupType == ParameterGroup::Type::Enum::config) {
          currentParameterGroup = parameter->function->configParameters;
          auto parameterIterator = parameter->function->configParameters->parameters.find(parameter->parameterName);
          if (parameterIterator != parameter->function->configParameters->parameters.end()) parameter->parameter.rpcParameter = parameterIterator->second;
        } else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::variables) {
          currentParameterGroup = parameter->function->variables;
          auto parameterIterator = parameter->function->variables->parameters.find(parameter->parameterName);
          if (parameterIterator != parameter->function->variables->parameters.end()) parameter->parameter.rpcParameter = parameterIterator->second;
        } else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::link) {
          currentParameterGroup = parameter->function->linkParameters;
          auto parameterIterator = parameter->function->linkParameters->parameters.find(parameter->parameterName);
          if (parameterIterator != parameter->function->linkParameters->parameters.end()) parameter->parameter.rpcParameter = parameterIterator->second;
        }
      }

      if (parameter->specialType == 1 && parameter->metadata && currentParameterGroup) //Role parameter
      {
        auto roleInfoIterator = parameter->metadata->structValue->find("roleInfo");
        if (roleInfoIterator != parameter->metadata->structValue->end()) {
          parameter->parameter.specialType = parameter->specialType;
          auto variableInfoIterator = roleInfoIterator->second->structValue->find("variableInfo");
          auto variableBaseNameIterator = roleInfoIterator->second->structValue->find("variableBaseName");

          if (variableInfoIterator != roleInfoIterator->second->structValue->end() && variableBaseNameIterator != roleInfoIterator->second->structValue->end()) {
            roleParameters.emplace(parameter);
            parameter->parameter.rpcParameter = createRoleRpcParameter(variableInfoIterator->second, variableBaseNameIterator->second->stringValue, currentParameterGroup);
          }
        }
      }

      if (!parameter->parameter.rpcParameter) {
        if (!parameterGroupSelector || !parameter->function->parameterGroupSelector || parameter->function->alternativeFunctions.empty())
          _bl->out.printWarning(
              "Warning: Deleting parameter " + parameter->parameterName + ", because no corresponding RPC parameter was found. Peer: " + std::to_string(_peerID) + " Channel: " + std::to_string(parameter->channel) + " Parameter set type: "
                  + std::to_string((uint32_t)(parameter->parameterGroupType)));
        Database::DataRow data;
        data.push_back(std::make_shared<Database::DataColumn>(_peerID));
        data.push_back(std::make_shared<Database::DataColumn>((int32_t)(parameter->parameterGroupType)));
        data.push_back(std::make_shared<Database::DataColumn>(parameter->channel));
        data.push_back(std::make_shared<Database::DataColumn>(parameter->parameterName));
        if (parameter->parameterGroupType == ParameterGroup::Type::Enum::config) {
          configCentral[parameter->channel].erase(parameter->parameterName);
        } else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::variables) {
          valuesCentral[parameter->channel].erase(parameter->parameterName);
        } else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::link) {
          linksCentral[parameter->channel][parameter->remoteAddress][parameter->remoteChannel].erase(parameter->parameterName);
          data.push_back(std::make_shared<Database::DataColumn>(parameter->remoteAddress));
          data.push_back(std::make_shared<Database::DataColumn>(parameter->remoteChannel));
        }
        _bl->db->deletePeerParameter(_peerID, data);
      } else {
        if (parameter->parameterGroupType == ParameterGroup::Type::Enum::config) configCentral[parameter->channel].emplace(parameter->parameterName, parameter->parameter);
        else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::variables) {
          if (parameter->parameter.rpcParameter->resetAfterRestart) {
            std::vector<uint8_t> parameterData;
            parameter->parameter.rpcParameter->convertToPacket(parameter->parameter.rpcParameter->logical->getDefaultValue(), Role(), parameterData);
            parameter->parameter.setBinaryData(parameterData);
          }
          valuesCentral[parameter->channel][parameter->parameterName] = parameter->parameter;
        } else if (parameter->parameterGroupType == ParameterGroup::Type::Enum::link) linksCentral[parameter->channel][parameter->remoteAddress][parameter->remoteChannel].emplace(parameter->parameterName, parameter->parameter);

        //Add roles from XML - only if parameter has no roles
        if (!parameter->parameter.hasRoles() && !parameter->parameter.rpcParameter->roles.empty()) {
          for (auto &role : parameter->parameter.rpcParameter->roles) {
            uint64_t middleGroupRoleId = 0;
            uint64_t mainGroupRoleId = 0;

            //{{{ Get parent roles
            {
              uint64_t hexRoleId = BaseLib::Math::getNumber64(std::to_string(role.second.id), true);
              middleGroupRoleId = BaseLib::Math::getNumber64(BaseLib::HelperFunctions::getHexString(hexRoleId & 0x00FFFF00, 6));
              mainGroupRoleId = BaseLib::Math::getNumber64(BaseLib::HelperFunctions::getHexString(hexRoleId & 0x00FF0000, 6));
              if (middleGroupRoleId == mainGroupRoleId || middleGroupRoleId == role.second.id || !_bl->db->roleExists(middleGroupRoleId)) middleGroupRoleId = 0;
              if (mainGroupRoleId == role.second.id || !_bl->db->roleExists(mainGroupRoleId)) mainGroupRoleId = 0;
            }
            //}}}

            addRoleToVariable(parameter->channel, parameter->parameterName, role.second.id, role.second.direction, role.second.invert, role.second.scale, role.second.scaleInfo);
            if (middleGroupRoleId != 0) addRoleToVariable(parameter->channel, parameter->parameterName, middleGroupRoleId, role.second.direction, role.second.invert, false, RoleScaleInfo());
            if (mainGroupRoleId != 0) addRoleToVariable(parameter->channel, parameter->parameterName, mainGroupRoleId, role.second.direction, role.second.invert, false, RoleScaleInfo());
          }
        }
      }
    }

    //{{{ Delete role parameters when variable does not exist anymore
    for (auto &roleParameter : roleParameters) {
      bool deleteParameter = false;
      auto channelIterator = valuesCentral.find(roleParameter->channel);
      if (channelIterator != valuesCentral.end()) {
        auto roleInfoIterator = roleParameter->metadata->structValue->find("roleInfo");
        if (roleInfoIterator != roleParameter->metadata->structValue->end()) {
          auto variableBaseNameIterator = roleInfoIterator->second->structValue->find("variableBaseName");
          if (variableBaseNameIterator != roleInfoIterator->second->structValue->end()) {
            auto variableIterator = channelIterator->second.find(variableBaseNameIterator->second->stringValue);
            if (variableIterator != channelIterator->second.end()) {
              auto roleIdIterator = roleInfoIterator->second->structValue->find("roleId");
              if (roleIdIterator != roleInfoIterator->second->structValue->end()) {
                if (!variableIterator->second.hasRole(roleIdIterator->second->integerValue64)) {
                  deleteParameter = true;
                }
              }
              //Don't delete parameter when key roleId doesn't exist, because the parameter is missing in
              //older Homegear versions (up to 0.8.0-3009 or 0.7.45).
            } else deleteParameter = true;
          } else deleteParameter = true;
        } else deleteParameter = true;
      } else deleteParameter = true;

      if (deleteParameter) {
        valuesCentral[roleParameter->channel].erase(roleParameter->parameterName);
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Peer::initializeTypeString() {
  try {
    if (!_rpcDevice) return;
    if (!_typeString.empty()) {
      _rpcTypeString = _typeString;
      return;
    }
    PSupportedDevice rpcDeviceType = _rpcDevice->getType(_deviceType, _firmwareVersion);
    if (rpcDeviceType) _rpcTypeString = rpcDeviceType->id;
    else if (_deviceType == 0) _rpcTypeString = "HM-RCV-50"; //Central
    else if (!_rpcDevice->supportedDevices.empty()) _rpcTypeString = _rpcDevice->supportedDevices.at(0)->id;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

bool Peer::setVariableRoom(int32_t channel, std::string &variableName, uint64_t roomId) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) {
      _bl->out.printWarning("Warning: Could not add variable to room: Channel not found.");
      return false;
    }
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end()) {
      _bl->out.printWarning("Warning: Could not add variable " + std::to_string(_peerID) + "." + std::to_string(channel) + "." + variableName + " to room " + std::to_string(roomId) + ": Variable not found.");
      return false;
    }
    if (!variableIterator->second.rpcParameter) {
      _bl->out.printWarning("Warning: Could not add variable to room " + std::to_string(_peerID) + "." + std::to_string(channel) + "." + variableName + ": Variable has no associated RPC parameter. Try to restart Homegear.");
      return false;
    }
    if (variableIterator->second.databaseId == 0) {
      _bl->out.printWarning("Warning: Could not add variable to room " + std::to_string(_peerID) + "." + std::to_string(channel) + "." + variableName + ": Variable has no associated database ID. Try to restart Homegear.");
      return false;
    }
    variableIterator->second.setRoom(roomId);

    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(roomId));
    data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.databaseId));
    _bl->db->savePeerParameterRoomAsynchronous(data);

    return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void Peer::removeRoomFromVariables(uint64_t roomId) {
  try {
    for (auto &channelIterator : valuesCentral) {
      for (auto &variableIterator : channelIterator.second) {
        if (!variableIterator.second.rpcParameter || variableIterator.second.databaseId == 0) continue;
        if (variableIterator.second.getRoom() == roomId) {
          variableIterator.second.setRoom(0);

          Database::DataRow data;
          data.push_back(std::make_shared<Database::DataColumn>(roomId));
          data.push_back(std::make_shared<Database::DataColumn>(variableIterator.second.databaseId));
          _bl->db->savePeerParameterRoomAsynchronous(data);
        }
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

uint64_t Peer::getVariableRoom(int32_t channel, const std::string &variableName) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return 0;
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return 0;

    return variableIterator->second.getRoom();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return 0;
}

bool Peer::addCategoryToVariable(int32_t channel, std::string &variableName, uint64_t categoryId) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return false;
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

    variableIterator->second.addCategory(categoryId);

    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.getCategoryString()));
    data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.databaseId));
    _bl->db->savePeerParameterCategoriesAsynchronous(data);

    return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Peer::removeCategoryFromVariable(int32_t channel, std::string &variableName, uint64_t categoryId) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return false;
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

    variableIterator->second.removeCategory(categoryId);

    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.getCategoryString()));
    data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.databaseId));
    _bl->db->savePeerParameterCategoriesAsynchronous(data);

    return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void Peer::removeCategoryFromVariables(uint64_t categoryId) {
  try {
    for (auto &channelIterator : valuesCentral) {
      for (auto &variableIterator : channelIterator.second) {
        if (!variableIterator.second.rpcParameter || variableIterator.second.databaseId == 0) continue;
        variableIterator.second.removeCategory(categoryId);

        Database::DataRow data;
        data.push_back(std::make_shared<Database::DataColumn>(variableIterator.second.getCategoryString()));
        data.push_back(std::make_shared<Database::DataColumn>(variableIterator.second.databaseId));
        _bl->db->savePeerParameterCategoriesAsynchronous(data);
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::set<uint64_t> Peer::getVariableCategories(int32_t channel, std::string &variableName) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return std::set<uint64_t>();
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return std::set<uint64_t>();

    return variableIterator->second.getCategories();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::set<uint64_t>();
}

bool Peer::variableHasCategory(int32_t channel, const std::string &variableName, uint64_t categoryId) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return false;
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

    return variableIterator->second.hasCategory(categoryId);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Peer::variableHasCategories(int32_t channel, const std::string &variableName) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return false;
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

    return variableIterator->second.hasCategories();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Peer::addRoleToVariable(int32_t channel, std::string &variableName, uint64_t roleId, RoleDirection direction, bool invert, bool scale, RoleScaleInfo scaleInfo) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return false;
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;
    if (variableIterator->second.hasRole(roleId)) return false;

    if (direction == RoleDirection::undefined) {
      if (variableIterator->second.rpcParameter->readable && variableIterator->second.rpcParameter->writeable) {
        direction = RoleDirection::both;
      } else if (variableIterator->second.rpcParameter->readable) {
        direction = RoleDirection::input;
      } else if (variableIterator->second.rpcParameter->writeable) {
        direction = RoleDirection::output;
      } else return false;
    }

    variableIterator->second.addRole(roleId, direction, invert, scale, scaleInfo);

    {
      Database::DataRow data;
      data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.getRoleString()));
      data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.databaseId));
      _bl->db->savePeerParameterRolesAsynchronous(data);
    }

    //{{{ Add variables from metadata
    auto roleMetadata = _bl->db->getRoleMetadata(roleId);
    auto addVariablesIterator = roleMetadata->structValue->find("addVariables");
    if (addVariablesIterator != roleMetadata->structValue->end()) {
      for (auto &variableInfo : *addVariablesIterator->second->arrayValue) {
        auto parameter = createRoleRpcParameter(variableInfo, variableIterator->first, variableIterator->second.rpcParameter->parent());
        if (!parameter) continue;

        RpcConfigurationParameter parameterStruct;
        parameterStruct.rpcParameter = parameter;
        setDefaultValue(parameterStruct);

        auto metadata = std::make_shared<Variable>(VariableType::tStruct);
        auto roleInfo = std::make_shared<Variable>(VariableType::tStruct);
        metadata->structValue->emplace("roleInfo", roleInfo);
        roleInfo->structValue->emplace("variableInfo", variableInfo);
        roleInfo->structValue->emplace("roleId", std::make_shared<Variable>(roleId));
        roleInfo->structValue->emplace("variableBaseName", std::make_shared<Variable>(variableIterator->first));

        auto rolesIterator = variableInfo->structValue->find("roles");
        if (rolesIterator != variableInfo->structValue->end()) {
          for (auto &role : *rolesIterator->second->arrayValue) {
            if (role->integerValue64 != 0) parameterStruct.addRole(role->integerValue64, RoleDirection::both, false, false, RoleScaleInfo());
          }
        }

        std::vector<uint8_t> data = parameterStruct.getBinaryData();
        saveSpecialTypeParameter(0, ParameterGroup::Type::variables, channel, parameter->id, data, 1, metadata, parameterStruct.getRoleString());
      }
    }
    //}}}

    return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Peer::removeRoleFromVariable(int32_t channel, std::string &variableName, uint64_t roleId) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return false;
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

    //{{{ Remove variables from metadata
    auto roleMetadata = _bl->db->getRoleMetadata(roleId);
    auto addVariablesIterator = roleMetadata->structValue->find("addVariables");
    if (addVariablesIterator != roleMetadata->structValue->end()) {
      for (auto &variableInfo : *addVariablesIterator->second->arrayValue) {
        auto idIterator = variableInfo->structValue->find("id");
        if (idIterator != variableInfo->structValue->end() && !idIterator->second->stringValue.empty()) {
          Database::DataRow data;
          data.push_back(std::make_shared<Database::DataColumn>(_peerID));
          data.push_back(std::make_shared<Database::DataColumn>((int32_t)(variableIterator->second.rpcParameter->parent()->type())));
          data.push_back(std::make_shared<Database::DataColumn>(channel));
          data.push_back(std::make_shared<Database::DataColumn>(variableIterator->first + ".RV." + idIterator->second->stringValue));
          _bl->db->deletePeerParameter(_peerID, data);
        }
      }
    }
    //}}}

    variableIterator->second.removeRole(roleId);

    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.getRoleString()));
    data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.databaseId));
    _bl->db->savePeerParameterRolesAsynchronous(data);

    return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void Peer::removeRoleFromVariables(uint64_t roleId) {
  try {
    for (auto &channelIterator : valuesCentral) {
      for (auto &variableIterator : channelIterator.second) {
        if (!variableIterator.second.rpcParameter || variableIterator.second.databaseId == 0) continue;
        variableIterator.second.removeRole(roleId);

        Database::DataRow data;
        data.push_back(std::make_shared<Database::DataColumn>(variableIterator.second.getRoleString()));
        data.push_back(std::make_shared<Database::DataColumn>(variableIterator.second.databaseId));
        _bl->db->savePeerParameterRolesAsynchronous(data);
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::unordered_map<uint64_t, Role> Peer::getVariableRoles(int32_t channel, std::string &variableName) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return std::unordered_map<uint64_t, Role>();
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return std::unordered_map<uint64_t, Role>();

    return variableIterator->second.getRoles();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::unordered_map<uint64_t, Role>();
}

bool Peer::variableHasRole(int32_t channel, const std::string &variableName, uint64_t roleId) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return false;
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

    return variableIterator->second.hasRole(roleId);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Peer::variableHasRoles(int32_t channel, const std::string &variableName) {
  try {
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return false;
    auto variableIterator = channelIterator->second.find(variableName);
    if (variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

    return variableIterator->second.hasRoles();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

//RPC methods
PVariable Peer::getAllConfig(PRpcClientInfo clientInfo) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!clientInfo) clientInfo.reset(new RpcClientInfo());
    PVariable config(new Variable(VariableType::tStruct));

    config->structValue->insert(StructElement("FAMILY", PVariable(new Variable((uint32_t)getCentral()->deviceFamily()))));
    config->structValue->insert(StructElement("ID", PVariable(new Variable((uint32_t)_peerID))));
    config->structValue->insert(StructElement("ADDRESS", PVariable(new Variable(_serialNumber))));
    config->structValue->insert(StructElement("TYPE", PVariable(new Variable(_rpcTypeString))));
    if (_deviceType <= 0xFFFFFFFF) config->structValue->emplace("TYPE_ID", std::make_shared<BaseLib::Variable>((int32_t)_deviceType));
    else config->structValue->emplace("TYPE_ID", std::make_shared<BaseLib::Variable>(_deviceType));
    config->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(-1)))));
    PVariable channels(new Variable(VariableType::tArray));
    for (auto i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i) {
      if (!i->second) continue;
      if (!i->second->countFromVariable.empty() && configCentral[0].find(i->second->countFromVariable) != configCentral[0].end()) {
        std::vector<uint8_t> parameterData = configCentral[0][i->second->countFromVariable].getBinaryData();
        if (parameterData.size() > 0 && i->first >= i->second->channel + parameterData.at(parameterData.size() - 1)) continue;
      }
      PVariable channel(new Variable(VariableType::tStruct));
      channel->structValue->insert(StructElement("INDEX", PVariable(new Variable(i->first))));
      channel->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(i->first)))));
      channel->structValue->insert(StructElement("TYPE", PVariable(new Variable(i->second->type))));

      PVariable parameters(new Variable(VariableType::tStruct));
      channel->structValue->insert(StructElement("PARAMSET", parameters));
      channels->arrayValue->push_back(channel);

      auto configIterator = configCentral.find(i->first);
      if (configIterator == configCentral.end()) continue;

      PParameterGroup parameterGroup = getParameterSet(i->first, ParameterGroup::Type::config);
      if (!parameterGroup) continue;

      for (auto &parameterIterator : configIterator->second) {
        RpcConfigurationParameter &parameter = parameterIterator.second;
        if (!parameter.rpcParameter || parameter.rpcParameter->id.empty() || !parameter.rpcParameter->visible) continue;
        if (parameter.specialType == 0) {
          //Parameter also needs to be in ParamsetDescription, this is not necessarily the case (e. g. for switchable parameter sets)
          auto parameter2 = parameterGroup->getParameter(parameter.rpcParameter->id);
          if (!parameter2) continue;
        }
        if (!parameter.rpcParameter->visible && !parameter.rpcParameter->service && !parameter.rpcParameter->internal && !parameter.rpcParameter->transform) {
          _bl->out.printDebug("Debug: Omitting parameter " + parameter.rpcParameter->id + " because of it's ui flag.");
          continue;
        }
#ifdef CCU2
        if(parameter.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif

        if (getAllConfigHook2(clientInfo, parameter.rpcParameter, i->first, parameters)) continue;

        PVariable element(new Variable(VariableType::tStruct));
        PVariable value;
        if (parameter.rpcParameter->readable) {
          std::vector<uint8_t> parameterData = parameter.getBinaryData();
          if (!convertFromPacketHook(parameter, parameterData, value)) value = parameter.rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);
          if (parameter.rpcParameter->password && (!clientInfo || !clientInfo->scriptEngineServer)) value.reset(new Variable(value->type));
          if (!value) continue;
          element->structValue->insert(StructElement("VALUE", value));
        }

        if (parameter.rpcParameter->logical->type == ILogical::Type::tBoolean) {
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("BOOL")))));
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tString) {
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRING")))));
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tInteger) {
          LogicalInteger *logicalInteger = (LogicalInteger *)parameter.rpcParameter->logical.get();
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER")))));
          element->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalInteger->minimumValue))));
          element->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalInteger->maximumValue))));

          if (!logicalInteger->specialValuesStringMap.empty()) {
            PVariable specialValues(new Variable(VariableType::tArray));
            for (auto j = logicalInteger->specialValuesStringMap.begin(); j != logicalInteger->specialValuesStringMap.end(); ++j) {
              PVariable specialElement(new Variable(VariableType::tStruct));
              specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
              specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
              specialValues->arrayValue->push_back(specialElement);
            }
            element->structValue->insert(StructElement("SPECIAL", specialValues));
          }
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tInteger64) {
          LogicalInteger64 *logicalInteger = (LogicalInteger64 *)parameter.rpcParameter->logical.get();
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER64")))));
          element->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalInteger->minimumValue))));
          element->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalInteger->maximumValue))));

          if (!logicalInteger->specialValuesStringMap.empty()) {
            PVariable specialValues(new Variable(VariableType::tArray));
            for (auto j = logicalInteger->specialValuesStringMap.begin(); j != logicalInteger->specialValuesStringMap.end(); ++j) {
              PVariable specialElement(new Variable(VariableType::tStruct));
              specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
              specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
              specialValues->arrayValue->push_back(specialElement);
            }
            element->structValue->insert(StructElement("SPECIAL", specialValues));
          }
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tEnum) {
          LogicalEnumeration *logicalEnumeration = (LogicalEnumeration *)parameter.rpcParameter->logical.get();
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ENUM")))));
          element->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalEnumeration->minimumValue))));
          element->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalEnumeration->maximumValue))));

          PVariable valueList(new Variable(VariableType::tArray));
          for (std::vector<EnumerationValue>::iterator j = logicalEnumeration->values.begin(); j != logicalEnumeration->values.end(); ++j) {
            valueList->arrayValue->push_back(PVariable(new Variable(j->id)));
          }
          element->structValue->insert(StructElement("VALUE_LIST", valueList));
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tFloat) {
          LogicalDecimal *logicalDecimal = (LogicalDecimal *)parameter.rpcParameter->logical.get();
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("FLOAT")))));
          element->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalDecimal->minimumValue))));
          element->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalDecimal->maximumValue))));

          if (!logicalDecimal->specialValuesStringMap.empty()) {
            PVariable specialValues(new Variable(VariableType::tArray));
            for (std::unordered_map<std::string, double>::iterator j = logicalDecimal->specialValuesStringMap.begin(); j != logicalDecimal->specialValuesStringMap.end(); ++j) {
              PVariable specialElement(new Variable(VariableType::tStruct));
              specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
              specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
              specialValues->arrayValue->push_back(specialElement);
            }
            element->structValue->insert(StructElement("SPECIAL", specialValues));
          }
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tArray) {
          if (!clientInfo->initNewFormat) continue;
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ARRAY")))));
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tStruct) {
          if (!clientInfo->initNewFormat) continue;
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRUCT")))));
        }
        parameters->structValue->insert(StructElement(parameter.rpcParameter->id, element));
      }
    }
    config->structValue->insert(StructElement("CHANNELS", channels));

    return config;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getAllValues(PRpcClientInfo clientInfo, bool returnWriteOnly, bool checkAcls) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!clientInfo) clientInfo.reset(new RpcClientInfo());
    PVariable values(new Variable(VariableType::tStruct));

    auto central = getCentral();
    if (!central) return Variable::createError(-32500, "Could not get central.");

    values->structValue->insert(StructElement("FAMILY", std::make_shared<Variable>((uint32_t)getCentral()->deviceFamily())));
    values->structValue->insert(StructElement("ID", std::make_shared<Variable>((uint32_t)_peerID)));
    values->structValue->insert(StructElement("ADDRESS", std::make_shared<Variable>(_serialNumber)));
    values->structValue->insert(StructElement("TYPE", std::make_shared<Variable>(_rpcTypeString)));
    if (_deviceType <= 0xFFFFFFFF) values->structValue->emplace("TYPE_ID", std::make_shared<BaseLib::Variable>((int32_t)_deviceType));
    else values->structValue->emplace("TYPE_ID", std::make_shared<BaseLib::Variable>(_deviceType));
    values->structValue->insert(StructElement("NAME", std::make_shared<Variable>(getName(-1))));
    auto room = getRoom(-1);
    if (room != 0) values->structValue->insert(StructElement("ROOM", std::make_shared<Variable>(room)));
    auto categoryIds = getCategories(-1);
    if (!categoryIds.empty()) {
      auto categories = std::make_shared<Variable>(VariableType::tArray);
      categories->arrayValue->reserve(categoryIds.size());
      for (auto categoryId : categoryIds) {
        categories->arrayValue->push_back(std::make_shared<Variable>(categoryId));
      }
      values->structValue->insert(StructElement("CATEGORIES", categories));
    }
    PVariable channels(new Variable(VariableType::tArray));
    for (auto i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i) {
      if (!i->second) continue;
      if (!i->second->countFromVariable.empty() && configCentral[0].find(i->second->countFromVariable) != configCentral[0].end()) {
        std::vector<uint8_t> parameterData = configCentral[0][i->second->countFromVariable].getBinaryData();
        if (!parameterData.empty() && i->first >= i->second->channel + parameterData.at(parameterData.size() - 1)) continue;
      }
      PVariable channel(new Variable(VariableType::tStruct));
      channel->structValue->insert(StructElement("INDEX", std::make_shared<Variable>(i->first)));
      channel->structValue->insert(StructElement("NAME", std::make_shared<Variable>(getName(i->first))));
      channel->structValue->insert(StructElement("TYPE", std::make_shared<Variable>(i->second->type)));
      auto room = getRoom(i->first);
      if (room != 0) channel->structValue->insert(StructElement("ROOM", std::make_shared<Variable>(room)));
      auto categoryIds = getCategories(i->first);
      if (!categoryIds.empty()) {
        auto categories = std::make_shared<Variable>(VariableType::tArray);
        categories->arrayValue->reserve(categoryIds.size());
        for (auto categoryId : categoryIds) {
          categories->arrayValue->push_back(std::make_shared<Variable>(categoryId));
        }
        channel->structValue->insert(StructElement("CATEGORIES", categories));
      }

      PVariable parameters(new Variable(VariableType::tStruct));
      channel->structValue->insert(StructElement("PARAMSET", parameters));
      channels->arrayValue->push_back(channel);

      PParameterGroup parameterGroup = getParameterSet(i->first, ParameterGroup::Type::variables);
      if (!parameterGroup) continue;

      auto valuesIterator = valuesCentral.find(i->first);
      if (valuesIterator == valuesCentral.end()) continue;

      for (auto &parameterIterator : valuesIterator->second) {
        RpcConfigurationParameter &parameter = parameterIterator.second;
        if (checkAcls && !clientInfo->acls->checkVariableReadAccess(central->getPeer(_peerID), i->first, parameter.rpcParameter->id)) continue;

        if (!parameter.rpcParameter || parameter.rpcParameter->id.empty() || !parameter.rpcParameter->visible) continue;
        if (parameter.specialType == 0) {
          //Parameter also needs to be in ParamsetDescription, this is not necessarily the case (e. g. for switchable parameter sets)
          auto parameter2 = parameterGroup->getParameter(parameter.rpcParameter->id);
          if (!parameter2) continue;
        }
        if (!parameter.rpcParameter->visible && !parameter.rpcParameter->service && !parameter.rpcParameter->internal && !parameter.rpcParameter->transform) {
          _bl->out.printDebug("Debug: Omitting parameter " + parameter.rpcParameter->id + " because of it's ui flag.");
          continue;
        }
        if (!parameter.rpcParameter->readable && !parameter.rpcParameter->transmitted && !returnWriteOnly) continue;
#ifdef CCU2
        if(parameter.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif
        if (clientInfo->clientType == RpcClientType::ccu2 && !parameter.rpcParameter->ccu2Visible) continue;

        if (getAllValuesHook2(clientInfo, parameter.rpcParameter, i->first, parameters)) continue;

        PVariable element(new Variable(VariableType::tStruct));
        PVariable value;
        if (parameter.rpcParameter->readable || parameter.rpcParameter->transmitted) {
          std::vector<uint8_t> parameterData = parameter.getBinaryData();
          if ((parameter.rpcParameter->password && (!clientInfo || !clientInfo->scriptEngineServer)) || parameterData.empty()) value.reset(new Variable(parameter.rpcParameter->logical->type));
          else {
            if (!convertFromPacketHook(parameter, parameterData, value)) value = parameter.rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);
          }
          if (!value) continue;
          element->structValue->insert(StructElement("VALUE", value));
        }

        element->structValue->insert(StructElement("READABLE", PVariable(new Variable(parameter.rpcParameter->readable))));
        element->structValue->insert(StructElement("WRITEABLE", PVariable(new Variable(parameter.rpcParameter->writeable))));
        element->structValue->insert(StructElement("TRANSMITTED", PVariable(new Variable(parameter.rpcParameter->transmitted))));
        element->structValue->insert(StructElement("UNIT", PVariable(new Variable(parameter.rpcParameter->unit))));
        auto room = parameter.getRoom();
        if (room != 0) element->structValue->insert(StructElement("ROOM", std::make_shared<Variable>(room)));
        auto categoryIds = parameter.getCategories();
        if (!categoryIds.empty()) {
          auto categories = std::make_shared<Variable>(VariableType::tArray);
          categories->arrayValue->reserve(categoryIds.size());
          for (auto categoryId : categoryIds) {
            categories->arrayValue->push_back(std::make_shared<Variable>(categoryId));
          }
          element->structValue->insert(StructElement("CATEGORIES", categories));
        }
        auto roles = parameter.getRoles();
        if (!roles.empty()) {
          auto rolesArray = std::make_shared<Variable>(VariableType::tArray);
          rolesArray->arrayValue->reserve(roles.size());
          for (auto role : roles) {
            auto roleStruct = std::make_shared<Variable>(VariableType::tStruct);
            roleStruct->structValue->emplace("id", std::make_shared<BaseLib::Variable>(role.second.id));
            roleStruct->structValue->emplace("direction", std::make_shared<BaseLib::Variable>((int32_t)role.second.direction));
            if (role.second.invert) roleStruct->structValue->emplace("invert", std::make_shared<BaseLib::Variable>(role.second.invert));
            roleStruct->structValue->emplace("level", std::make_shared<BaseLib::Variable>((role.second.id / 10000) * 10000 == role.second.id ? 0 : ((role.second.id / 100) * 100 == role.second.id ? 1 : 2)));
            rolesArray->arrayValue->emplace_back(std::move(roleStruct));
          }
          element->structValue->insert(StructElement("ROLES", rolesArray));
        }
        if (parameter.rpcParameter->logical->type == ILogical::Type::tBoolean) {
          if (value) value->type = VariableType::tBoolean; //For some families/variables "convertFromPacket" returns wrong type
          element->structValue->insert(StructElement("TYPE", std::make_shared<Variable>(std::string("BOOL"))));
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tString) {
          if (value) value->type = VariableType::tString; //For some families/variables "convertFromPacket" returns wrong type
          element->structValue->insert(StructElement("TYPE", std::make_shared<Variable>(std::string("STRING"))));
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tAction) {
          if (value) value->type = VariableType::tBoolean; //For some families/variables "convertFromPacket" returns wrong type
          element->structValue->insert(StructElement("TYPE", std::make_shared<Variable>(std::string("ACTION"))));
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tInteger) {
          if (value) value->type = VariableType::tInteger; //For some families/variables "convertFromPacket" returns wrong type
          LogicalInteger *logicalInteger = (LogicalInteger *)parameter.rpcParameter->logical.get();
          element->structValue->insert(StructElement("TYPE", std::make_shared<Variable>(std::string("INTEGER"))));
          element->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalInteger->minimumValue))));
          element->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalInteger->maximumValue))));

          if (!logicalInteger->specialValuesStringMap.empty()) {
            PVariable specialValues(new Variable(VariableType::tArray));
            for (std::unordered_map<std::string, int32_t>::iterator j = logicalInteger->specialValuesStringMap.begin(); j != logicalInteger->specialValuesStringMap.end(); ++j) {
              PVariable specialElement(new Variable(VariableType::tStruct));
              specialElement->structValue->insert(StructElement("ID", std::make_shared<Variable>(j->first)));
              specialElement->structValue->insert(StructElement("VALUE", std::make_shared<Variable>(j->second)));
              specialValues->arrayValue->push_back(specialElement);
            }
            element->structValue->insert(StructElement("SPECIAL", specialValues));
          }
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tInteger64) {
          if (value) value->type = VariableType::tInteger64; //For some families/variables "convertFromPacket" returns wrong type
          LogicalInteger64 *logicalInteger64 = (LogicalInteger64 *)parameter.rpcParameter->logical.get();
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER64")))));
          element->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalInteger64->minimumValue))));
          element->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalInteger64->maximumValue))));

          if (!logicalInteger64->specialValuesStringMap.empty()) {
            PVariable specialValues(new Variable(VariableType::tArray));
            for (std::unordered_map<std::string, int64_t>::iterator j = logicalInteger64->specialValuesStringMap.begin(); j != logicalInteger64->specialValuesStringMap.end(); ++j) {
              PVariable specialElement(new Variable(VariableType::tStruct));
              specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
              specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
              specialValues->arrayValue->push_back(specialElement);
            }
            element->structValue->insert(StructElement("SPECIAL", specialValues));
          }
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tEnum) {
          if (value) value->type = VariableType::tInteger; //For some families/variables "convertFromPacket" returns wrong type
          LogicalEnumeration *logicalEnumeration = (LogicalEnumeration *)parameter.rpcParameter->logical.get();
          element->structValue->insert(StructElement("TYPE", std::make_shared<Variable>(std::string("ENUM"))));
          element->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalEnumeration->minimumValue))));
          element->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalEnumeration->maximumValue))));

          PVariable valueList(new Variable(VariableType::tArray));
          for (std::vector<EnumerationValue>::iterator j = logicalEnumeration->values.begin(); j != logicalEnumeration->values.end(); ++j) {
            valueList->arrayValue->push_back(std::make_shared<Variable>(j->id));
          }
          element->structValue->insert(StructElement("VALUE_LIST", valueList));
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tFloat) {
          if (value) value->type = VariableType::tFloat; //For some families/variables "convertFromPacket" returns wrong type
          LogicalDecimal *logicalDecimal = (LogicalDecimal *)parameter.rpcParameter->logical.get();
          element->structValue->insert(StructElement("TYPE", std::make_shared<Variable>(std::string("FLOAT"))));
          element->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalDecimal->minimumValue))));
          element->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalDecimal->maximumValue))));

          if (!logicalDecimal->specialValuesStringMap.empty()) {
            PVariable specialValues(new Variable(VariableType::tArray));
            for (std::unordered_map<std::string, double>::iterator j = logicalDecimal->specialValuesStringMap.begin(); j != logicalDecimal->specialValuesStringMap.end(); ++j) {
              PVariable specialElement(new Variable(VariableType::tStruct));
              specialElement->structValue->insert(StructElement("ID", std::make_shared<Variable>(j->first)));
              specialElement->structValue->insert(StructElement("VALUE", std::make_shared<Variable>(j->second)));
              specialValues->arrayValue->push_back(specialElement);
            }
            element->structValue->insert(StructElement("SPECIAL", specialValues));
          }
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tArray) {
          if (!clientInfo->initNewFormat) continue;
          if (value) value->type = VariableType::tArray; //For some families/variables "convertFromPacket" returns wrong type
          element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ARRAY")))));
        } else if (parameter.rpcParameter->logical->type == ILogical::Type::tStruct) {
          if (!clientInfo->initNewFormat) continue;
          if (value) value->type = VariableType::tStruct; //For some families/variables "convertFromPacket" returns wrong type
          element->structValue->insert(StructElement("TYPE", std::make_shared<Variable>(std::string("STRUCT"))));
        }
        parameters->structValue->insert(StructElement(parameter.rpcParameter->id, element));
      }
    }
    values->structValue->insert(StructElement("CHANNELS", channels));

    return values;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getConfigParameter(PRpcClientInfo clientInfo, uint32_t channel, std::string name) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");
    std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator channelIterator = configCentral.find(channel);
    if (channelIterator == configCentral.end()) return Variable::createError(-2, "Unknown channel.");
    std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = channelIterator->second.find(name);
    if (parameterIterator == channelIterator->second.end() || !parameterIterator->second.rpcParameter) return Variable::createError(-5, "Unknown parameter.");

    //Check if channel still exists in device description
    Functions::iterator functionIterator = _rpcDevice->functions.find(channel);
    if (functionIterator == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel (2).");

    PParameterGroup parameterGroup = getParameterSet(channel, ParameterGroup::Type::Enum::config);
    if (!parameterIterator->second.rpcParameter->readable) return Variable::createError(-6, "Parameter is not readable.");
    std::vector<uint8_t> parameterData = parameterIterator->second.getBinaryData();
    PVariable variable;
    if (!convertFromPacketHook(parameterIterator->second, parameterData, variable))
      variable = parameterIterator->second.rpcParameter->convertFromPacket(parameterData,
                                                                           clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameterIterator->second.mainRole(),
                                                                           false);
    if (parameterIterator->second.rpcParameter->password && (!clientInfo || !clientInfo->scriptEngineServer)) variable.reset(new Variable(variable->type));
    return variable;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getDeviceDescription(PRpcClientInfo clientInfo, int32_t channel, std::map<std::string, bool> fields) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    PVariable description(new Variable(VariableType::tStruct));

    if (channel == -1) //Base device
    {
      DeviceDescription::PSupportedDevice supportedDevice = _rpcDevice->getType(_deviceType, _firmwareVersion);

      std::shared_ptr<ICentral> central = getCentral();
      if (!central) return description;
      std::string language = clientInfo ? clientInfo->language : "en-US";
      std::string filename = _rpcDevice->getFilename();

      if (fields.empty() || fields.find("FAMILY") != fields.end()) description->structValue->insert(StructElement("FAMILY", PVariable(new Variable((uint32_t)getCentral()->deviceFamily()))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable((uint32_t)_peerID))));
      if (fields.empty() || fields.find("ADDRESS") != fields.end()) description->structValue->insert(StructElement("ADDRESS", PVariable(new Variable(_serialNumber))));
      if (fields.empty() || fields.find("NAME") != fields.end()) description->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(-1)))));
      if (supportedDevice && !supportedDevice->serialPrefix.empty() && (fields.empty() || fields.find("SERIAL_PREFIX") != fields.end()))
        description->structValue->insert(StructElement("SERIAL_PREFIX",
                                                       PVariable(new Variable(supportedDevice->serialPrefix))));

      if (supportedDevice) {
        std::string descriptionText = central->getTranslations()->getTypeDescription(filename, language, supportedDevice->id);
        if (!descriptionText.empty() && fields.find("DESCRIPTION") != fields.end()) description->structValue->insert(StructElement("DESCRIPTION", PVariable(new Variable(descriptionText))));
        std::string longDescriptionText = central->getTranslations()->getTypeLongDescription(filename, language, supportedDevice->id);
        if (!longDescriptionText.empty() && fields.find("LONG_DESCRIPTION") != fields.end()) description->structValue->insert(StructElement("LONG_DESCRIPTION", PVariable(new Variable(longDescriptionText))));
      }

      if (fields.empty() || fields.find("PAIRING_METHOD") != fields.end()) description->structValue->insert(StructElement("PAIRING_METHOD", std::make_shared<Variable>(_rpcDevice->pairingMethod)));

      PVariable variable = PVariable(new Variable(VariableType::tArray));
      PVariable variable2 = PVariable(new Variable(VariableType::tArray));
      if (fields.empty() || fields.find("CHILDREN") != fields.end()) description->structValue->insert(StructElement("CHILDREN", variable));
      if (fields.empty() || fields.find("CHANNELS") != fields.end()) description->structValue->insert(StructElement("CHANNELS", variable2));

      if (fields.empty() || fields.find("CHILDREN") != fields.end() || fields.find("CHANNELS") != fields.end()) {
        for (Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i) {
          if (!i->second->visible) continue;
          if (!i->second->countFromVariable.empty() && configCentral[0].find(i->second->countFromVariable) != configCentral[0].end()) {
            std::vector<uint8_t> parameterData = configCentral[0][i->second->countFromVariable].getBinaryData();
            if (parameterData.size() > 0 && i->first >= i->second->channel + parameterData.at(parameterData.size() - 1)) continue;
          }
          if (fields.empty() || fields.find("CHILDREN") != fields.end()) variable->arrayValue->push_back(PVariable(new Variable(_serialNumber + ":" + std::to_string(i->first))));
          if (fields.empty() || fields.find("CHANNELS") != fields.end()) variable2->arrayValue->push_back(PVariable(new Variable(i->first)));
        }
      }

      if (fields.empty() || fields.find("FIRMWARE") != fields.end()) {
        if (_firmwareVersion != -1) description->structValue->insert(StructElement("FIRMWARE", PVariable(new Variable(getFirmwareVersionString(_firmwareVersion)))));
        else if (!_firmwareVersionString.empty()) description->structValue->insert(StructElement("FIRMWARE", PVariable(new Variable(_firmwareVersionString))));
        else description->structValue->insert(StructElement("FIRMWARE", PVariable(new Variable(std::string("?")))));
      }

      if ((fields.empty() || fields.find("AVAILABLE_FIRMWARE") != fields.end()) && (_firmwareVersion != -1 || !_firmwareVersionString.empty())) {
        int32_t newFirmwareVersion = getNewFirmwareVersion();
        if (newFirmwareVersion > _firmwareVersion) description->structValue->insert(StructElement("AVAILABLE_FIRMWARE", PVariable(new Variable(getFirmwareVersionString(newFirmwareVersion)))));
      }

      if (fields.empty() || fields.find("FLAGS") != fields.end()) {
        int32_t uiFlags = 0;
        if (_rpcDevice->visible) uiFlags += 1;
        if (_rpcDevice->internal) uiFlags += 2;
        if (!_rpcDevice->deletable || isTeam()) uiFlags += 8;
        description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      }

      if (fields.empty() || fields.find("INTERFACE") != fields.end()) description->structValue->insert(StructElement("INTERFACE", PVariable(new Variable(getCentral()->getSerialNumber()))));

      if (fields.empty() || fields.find("PARAMSETS") != fields.end()) {
        variable = PVariable(new Variable(VariableType::tArray));
        description->structValue->insert(StructElement("PARAMSETS", variable));
        variable->arrayValue->push_back(PVariable(new Variable(std::string("MASTER")))); //Always MASTER
      }

      if (fields.empty() || fields.find("PARENT") != fields.end()) description->structValue->insert(StructElement("PARENT", PVariable(new Variable(std::string("")))));

      if (!_ip.empty() && (fields.empty() || fields.find("IP_ADDRESS") != fields.end())) description->structValue->insert(StructElement("IP_ADDRESS", PVariable(new Variable(_ip))));

      if (fields.empty() || fields.find("PHYSICAL_ADDRESS") != fields.end()) description->structValue->insert(StructElement("PHYSICAL_ADDRESS", PVariable(new Variable(_address))));

      //Compatibility
      if (fields.empty() || fields.find("RF_ADDRESS") != fields.end()) description->structValue->insert(StructElement("RF_ADDRESS", PVariable(new Variable(_address))));
      //Compatibility
      if (fields.empty() || fields.find("ROAMING") != fields.end()) description->structValue->insert(StructElement("ROAMING", PVariable(new Variable((int32_t)0))));

      if (fields.empty() || fields.find("RX_MODE") != fields.end()) description->structValue->insert(StructElement("RX_MODE", PVariable(new Variable((int32_t)_rpcDevice->receiveModes))));

      if (!_rpcTypeString.empty() && (fields.empty() || fields.find("TYPE") != fields.end())) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(_rpcTypeString))));

      if (fields.empty() || fields.find("TYPE_ID") != fields.end()) {
        if (_deviceType <= 0xFFFFFFFF) description->structValue->emplace("TYPE_ID", std::make_shared<Variable>((int32_t)_deviceType));
        else description->structValue->emplace("TYPE_ID", std::make_shared<Variable>(_deviceType));
      }

      if (fields.empty() || fields.find("VERSION") != fields.end()) description->structValue->insert(StructElement("VERSION", PVariable(new Variable(_rpcDevice->version))));

      if (fields.find("WIRELESS") != fields.end()) description->structValue->insert(StructElement("WIRELESS", PVariable(new Variable(wireless()))));

      auto room = getRoom(-1);
      if ((fields.empty() || fields.find("ROOM") != fields.end()) && room != 0) description->structValue->emplace("ROOM", std::make_shared<Variable>(room));

      if (fields.find("ROOMNAME") != fields.end() && room != 0) {
        auto name = _bl->db->getRoomName(clientInfo, room);
        if (!name.empty()) description->structValue->emplace("ROOMNAME", std::make_shared<Variable>(name));
      }

      auto categories = getCategories(-1);
      if (fields.find("CATEGORIES") != fields.end() && !categories.empty()) {
        PVariable categoriesResult = std::make_shared<Variable>(VariableType::tArray);
        categoriesResult->arrayValue->reserve(categories.size());
        for (auto category : categories) {
          categoriesResult->arrayValue->push_back(std::make_shared<Variable>(category));
        }
        description->structValue->emplace("CATEGORIES", categoriesResult);
      }
    } else {
      if (_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
      PFunction rpcFunction = _rpcDevice->functions.at(channel);
      if (!rpcFunction->countFromVariable.empty() && configCentral[0].find(rpcFunction->countFromVariable) != configCentral[0].end()) {
        std::vector<uint8_t> parameterData = configCentral[0][rpcFunction->countFromVariable].getBinaryData();
        if (parameterData.size() > 0 && channel >= (int32_t)rpcFunction->channel + parameterData.at(parameterData.size() - 1)) return Variable::createError(-2, "Channel index larger than defined.");
      }
      if (!rpcFunction->visible) return description;

      if (fields.empty() || fields.find("FAMILYID") != fields.end()) description->structValue->insert(StructElement("FAMILY", PVariable(new Variable((uint32_t)getCentral()->deviceFamily()))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable((uint32_t)_peerID))));
      if (fields.empty() || fields.find("CHANNEL") != fields.end()) description->structValue->insert(StructElement("CHANNEL", PVariable(new Variable(channel))));
      if (fields.empty() || fields.find("NAME") != fields.end()) description->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(channel)))));
      if (fields.empty() || fields.find("ADDRESS") != fields.end()) description->structValue->insert(StructElement("ADDRESS", PVariable(new Variable(_serialNumber + ":" + std::to_string(channel)))));

      if (fields.empty() || fields.find("AES_ACTIVE") != fields.end()) {
        int32_t aesActive = 0;
        if (configCentral.find(channel) != configCentral.end() && configCentral.at(channel).find("AES_ACTIVE") != configCentral.at(channel).end()) {
          std::vector<uint8_t> parameterData = configCentral.at(channel).at("AES_ACTIVE").getBinaryData();
          if (!parameterData.empty() && parameterData.at(0) != 0) {
            aesActive = 1;
          }
        }
        //Integer for compatability
        description->structValue->insert(StructElement("AES_ACTIVE", PVariable(new Variable(aesActive))));
      }

      if (fields.empty() || fields.find("DIRECTION") != fields.end() || fields.find("LINK_SOURCE_ROLES") != fields.end() || fields.find("LINK_TARGET_ROLES") != fields.end()) {
        int32_t direction = 0;
        std::ostringstream linkSourceRoles;
        std::ostringstream linkTargetRoles;
        for (LinkFunctionTypes::iterator k = rpcFunction->linkSenderFunctionTypes.begin(); k != rpcFunction->linkSenderFunctionTypes.end(); ++k) {
          //Probably only one direction is supported, but just in case I use the "or"
          if (!k->empty()) {
            if (direction & 1) linkSourceRoles << " ";
            linkSourceRoles << *k;
            direction |= 1;
          }
        }
        for (LinkFunctionTypes::iterator k = rpcFunction->linkReceiverFunctionTypes.begin(); k != rpcFunction->linkReceiverFunctionTypes.end(); ++k) {
          //Probably only one direction is supported, but just in case I use the "or"
          if (!k->empty()) {
            if (direction & 2) linkTargetRoles << " ";
            linkTargetRoles << *k;
            direction |= 2;
          }
        }

        //Overwrite direction when manually set
        if (rpcFunction->direction != Function::Direction::Enum::none) direction = (int32_t)rpcFunction->direction;
        if (fields.empty() || fields.find("DIRECTION") != fields.end()) description->structValue->insert(StructElement("DIRECTION", PVariable(new Variable(direction))));
        if (fields.empty() || fields.find("LINK_SOURCE_ROLES") != fields.end()) description->structValue->insert(StructElement("LINK_SOURCE_ROLES", PVariable(new Variable(linkSourceRoles.str()))));
        if (fields.empty() || fields.find("LINK_TARGET_ROLES") != fields.end()) description->structValue->insert(StructElement("LINK_TARGET_ROLES", PVariable(new Variable(linkTargetRoles.str()))));
      }

      if (fields.empty() || fields.find("FLAGS") != fields.end()) {
        int32_t uiFlags = 0;
        if (rpcFunction->visible) uiFlags += 1;
        if (rpcFunction->internal) uiFlags += 2;
        if (rpcFunction->deletable || isTeam()) uiFlags += 8;
        description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      }

      if (fields.empty() || fields.find("GROUP") != fields.end()) {
        int32_t groupedWith = getChannelGroupedWith(channel);
        if (groupedWith > -1) {
          description->structValue->insert(StructElement("GROUP", PVariable(new Variable(_serialNumber + ":" + std::to_string(groupedWith)))));
        }
      }

      if (fields.empty() || fields.find("INDEX") != fields.end()) description->structValue->insert(StructElement("INDEX", PVariable(new Variable(channel))));

      if (fields.empty() || fields.find("PARAMSETS") != fields.end()) {
        PVariable variable = PVariable(new Variable(VariableType::tArray));
        description->structValue->insert(StructElement("PARAMSETS", variable));
        if (!rpcFunction->configParameters->parameters.empty() || !rpcFunction->configParameters->id.empty()) variable->arrayValue->push_back(PVariable(new Variable(std::string("MASTER"))));
        if (!rpcFunction->variables->parameters.empty() || !rpcFunction->variables->id.empty()) variable->arrayValue->push_back(PVariable(new Variable(std::string("VALUES"))));
        if (!rpcFunction->linkParameters->parameters.empty() || !rpcFunction->linkParameters->id.empty()) variable->arrayValue->push_back(PVariable(new Variable(std::string("LINK"))));
      }
      //if(rpcChannel->parameterSets.find(Rpc::ParameterSet::Type::Enum::link) != rpcChannel->parameterSets.end()) variable->arrayValue->push_back(PVariable(new Variable(rpcChannel->parameterSets.at(Rpc::ParameterSet::Type::Enum::link)->typeString())));
      //if(rpcChannel->parameterSets.find(Rpc::ParameterSet::Type::Enum::master) != rpcChannel->parameterSets.end()) variable->arrayValue->push_back(PVariable(new Variable(rpcChannel->parameterSets.at(Rpc::ParameterSet::Type::Enum::master)->typeString())));
      //if(rpcChannel->parameterSets.find(Rpc::ParameterSet::Type::Enum::values) != rpcChannel->parameterSets.end()) variable->arrayValue->push_back(PVariable(new Variable(rpcChannel->parameterSets.at(Rpc::ParameterSet::Type::Enum::values)->typeString())));

      if (fields.empty() || fields.find("PARENT") != fields.end()) description->structValue->insert(StructElement("PARENT", PVariable(new Variable(_serialNumber))));

      if (!_rpcTypeString.empty() && (fields.empty() || fields.find("PARENT_TYPE") != fields.end())) description->structValue->insert(StructElement("PARENT_TYPE", PVariable(new Variable(_rpcTypeString))));

      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(rpcFunction->type))));

      if (fields.empty() || fields.find("VERSION") != fields.end()) description->structValue->insert(StructElement("VERSION", PVariable(new Variable(_rpcDevice->version))));

      auto room = getRoom(channel);
      if ((fields.empty() || fields.find("ROOM") != fields.end()) && room != 0) description->structValue->emplace("ROOM", std::make_shared<Variable>(room));

      if (fields.find("ROOMNAME") != fields.end() && room != 0) {
        auto name = _bl->db->getRoomName(clientInfo, room);
        if (!name.empty()) description->structValue->emplace("ROOMNAME", std::make_shared<Variable>(name));
      }

      auto categories = getCategories(channel);
      if (fields.find("CATEGORIES") != fields.end() && !categories.empty()) {
        PVariable categoriesResult = std::make_shared<Variable>(VariableType::tArray);
        categoriesResult->arrayValue->reserve(categories.size());
        for (auto category : categories) {
          categoriesResult->arrayValue->push_back(std::make_shared<Variable>(category));
        }
        description->structValue->emplace("CATEGORIES", categoriesResult);
      }
    }
    return description;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

std::shared_ptr<std::vector<PVariable>> Peer::getDeviceDescriptions(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields) {
  try {
    std::shared_ptr<std::vector<PVariable>> descriptions(new std::vector<PVariable>());
    PVariable description = getDeviceDescription(clientInfo, -1, fields);
    if (!description->errorStruct && !description->structValue->empty()) descriptions->push_back(description);

    if (channels) {
      for (Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i) {
        if (!i->second->countFromVariable.empty() && configCentral[0].find(i->second->countFromVariable) != configCentral[0].end()) {
          std::vector<uint8_t> parameterData = configCentral[0][i->second->countFromVariable].getBinaryData();
          if (parameterData.size() > 0 && i->first >= i->second->channel + parameterData.at(parameterData.size() - 1)) continue;
        }
        description = getDeviceDescription(clientInfo, (int32_t)i->first, fields);
        if (!description->errorStruct && !description->structValue->empty()) descriptions->push_back(description);
      }
    }

    return descriptions;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::shared_ptr<std::vector<PVariable>>();
}

PVariable Peer::getDeviceInfo(PRpcClientInfo clientInfo, std::map<std::string, bool> fields) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    PVariable info(new Variable(VariableType::tStruct));

    info->structValue->insert(StructElement("ID", PVariable(new Variable((int32_t)_peerID))));

    if (wireless()) {
      if (fields.empty() || fields.find("RSSI") != fields.end()) {
        if (valuesCentral.find(0) != valuesCentral.end() && valuesCentral.at(0).find("RSSI_DEVICE") != valuesCentral.at(0).end() && valuesCentral.at(0).at("RSSI_DEVICE").rpcParameter) {
          auto &parameter = valuesCentral.at(0).at("RSSI_DEVICE");
          std::vector<uint8_t> parameterData = parameter.getBinaryData();
          info->structValue->insert(StructElement("RSSI", parameter.rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false)));
        }
      }
    }

    return info;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return PVariable();
}

PVariable Peer::getLink(PRpcClientInfo clientInfo, int32_t channel, int32_t flags, bool avoidDuplicates) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    PVariable array(new Variable(VariableType::tArray));
    std::shared_ptr<ICentral> central = getCentral();
    if (!central) return array; //central actually should always be set at this point
    PVariable element;
    bool groupFlag = false;
    if (flags & 0x01) groupFlag = true;
    if (channel > -1 && !groupFlag) //Get link of single channel
    {
      if (_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
      bool isSender = false;
      //Return if there are no link roles defined
      PFunction rpcFunction = _rpcDevice->functions.at(channel);
      if (!rpcFunction->linkSenderFunctionTypes.empty()) isSender = true;
      else if (rpcFunction->linkReceiverFunctionTypes.empty()) return array;
      //Return if no peers are paired to the channel
      std::vector<std::shared_ptr<BasicPeer>> peers;
      {
        std::lock_guard<std::mutex> peersGuard(_peersMutex);
        std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>>::iterator peersIterator = _peers.find(channel);
        if (peersIterator == _peers.end() || peersIterator->second.empty()) {
          return array;
        }
        peers = peersIterator->second;
      }
      for (std::vector<std::shared_ptr<BasicPeer>>::iterator i = peers.begin(); i != peers.end(); ++i) {
        if ((*i)->isVirtual) continue;
        if ((*i)->hasSender) isSender = (*i)->isSender;
        else (*i)->isSender = isSender; //Todo: Remove in future versions. Sets isSender of the peer if previously unset.
        std::shared_ptr<Peer> remotePeer;
        if ((*i)->id != 0) remotePeer = central->getPeer((*i)->id);
        else if ((*i)->address != 0) remotePeer = central->getPeer((*i)->address);
        else if (!(*i)->serialNumber.empty()) remotePeer = central->getPeer((*i)->serialNumber);
        if (!remotePeer) {
          _bl->out.printDebug("Debug: Can't return link description for peer with id " + std::to_string((*i)->id) + ". The peer is not paired to Homegear.");
          continue;
        }
        uint64_t peerID = remotePeer->getID();
        bool peerKnowsMe = false;
        if (remotePeer->getPeer((*i)->channel, _peerID, channel)) peerKnowsMe = true;

        //Don't continue if peer is sender and exists in central's peer array to avoid generation of duplicate results when requesting all links (only generate results when we are sender)
        //Previously there was the following check: peerID != _peerID. It needed to be removed, because otherwise duplicate links were returned
        //when a device was linked to itself.
        if (!isSender && peerKnowsMe && avoidDuplicates) return array;
        //If we are receiver this point is only reached, when the sender is not paired to this central

        std::string peerSerialNumber = remotePeer->getSerialNumber();
        int32_t brokenFlags = 0;
        /*if(peerID == 0 || peerSerialNumber.empty())
                {
                    if(peerKnowsMe || (*i)->id == _peerID) //Link to myself with non-existing (virtual) channel (e. g. switches use this)
                    {
                        (*i)->id = remotePeer->getID();
                        (*i)->serialNumber = remotePeer->getSerialNumber();
                        peerID = (*i)->id;
                        peerSerialNumber = remotePeer->getSerialNumber();
                    }
                    else
                    {
                        //Peer not paired to central
                        std::ostringstream stringstream;
                        stringstream << '@' << std::dec << (*i)->id;
                        peerSerialNumber = stringstream.str();
                        if(isSender) brokenFlags = 2; //LINK_FLAG_RECEIVER_BROKEN
                        else brokenFlags = 1; //LINK_FLAG_SENDER_BROKEN
                    }
                }*/
        //Relevent for switches
        if (peerID == _peerID && _rpcDevice->functions.find((*i)->channel) == _rpcDevice->functions.end()) {
          if (isSender) brokenFlags = 2 | 4; //LINK_FLAG_RECEIVER_BROKEN | PEER_IS_ME
          else brokenFlags = 1 | 4; //LINK_FLAG_SENDER_BROKEN | PEER_IS_ME
        }
        if (brokenFlags == 0 && remotePeer && remotePeer->serviceMessages->getUnreach()) brokenFlags = 2;
        if (serviceMessages->getUnreach()) brokenFlags |= 1;
        element.reset(new Variable(VariableType::tStruct));
        element->structValue->insert(StructElement("DESCRIPTION", PVariable(new Variable((*i)->linkDescription))));
        element->structValue->insert(StructElement("FLAGS", PVariable(new Variable(brokenFlags))));
        element->structValue->insert(StructElement("NAME", PVariable(new Variable((*i)->linkName))));
        if (isSender) {
          element->structValue->insert(StructElement("RECEIVER", PVariable(new Variable(peerSerialNumber + ":" + std::to_string((*i)->channel)))));
          element->structValue->insert(StructElement("RECEIVER_ID", PVariable(new Variable((int32_t)remotePeer->getID()))));
          element->structValue->insert(StructElement("RECEIVER_CHANNEL", PVariable(new Variable((*i)->channel))));
          if (flags & 4) {
            PVariable paramset;
            if (!(brokenFlags & 2) && remotePeer) paramset = remotePeer->getParamset(clientInfo, (*i)->channel, ParameterGroup::Type::Enum::link, _peerID, channel, false);
            else paramset.reset(new Variable(VariableType::tStruct));
            if (paramset->errorStruct) paramset.reset(new Variable(VariableType::tStruct));
            element->structValue->insert(StructElement("RECEIVER_PARAMSET", paramset));
          }
          if (flags & 16) {
            PVariable description;
            if (!(brokenFlags & 2) && remotePeer) description = remotePeer->getDeviceDescription(clientInfo, (*i)->channel, std::map<std::string, bool>());
            else description.reset(new Variable(VariableType::tStruct));
            if (description->errorStruct) description.reset(new Variable(VariableType::tStruct));
            element->structValue->insert(StructElement("RECEIVER_DESCRIPTION", description));
          }
          element->structValue->insert(StructElement("SENDER", PVariable(new Variable(_serialNumber + ":" + std::to_string(channel)))));
          element->structValue->insert(StructElement("SENDER_ID", PVariable(new Variable((int32_t)_peerID))));
          element->structValue->insert(StructElement("SENDER_CHANNEL", PVariable(new Variable(channel))));
          if (flags & 2) {
            PVariable paramset;
            if (!(brokenFlags & 1)) paramset = getParamset(clientInfo, channel, ParameterGroup::Type::Enum::link, peerID, (*i)->channel, false);
            else paramset.reset(new Variable(VariableType::tStruct));
            if (paramset->errorStruct) paramset.reset(new Variable(VariableType::tStruct));
            element->structValue->insert(StructElement("SENDER_PARAMSET", paramset));
          }
          if (flags & 8) {
            PVariable description;
            if (!(brokenFlags & 1)) description = getDeviceDescription(clientInfo, channel, std::map<std::string, bool>());
            else description.reset(new Variable(VariableType::tStruct));
            if (description->errorStruct) description.reset(new Variable(VariableType::tStruct));
            element->structValue->insert(StructElement("SENDER_DESCRIPTION", description));
          }
        } else //When sender is broken
        {
          element->structValue->insert(StructElement("RECEIVER", PVariable(new Variable(_serialNumber + ":" + std::to_string(channel)))));
          element->structValue->insert(StructElement("RECEIVER_ID", PVariable(new Variable((int32_t)_peerID))));
          element->structValue->insert(StructElement("RECEIVER_CHANNEL", PVariable(new Variable(channel))));
          if (flags & 4) {
            PVariable paramset;
            if (!(brokenFlags & 2) && remotePeer) paramset = getParamset(clientInfo, channel, ParameterGroup::Type::Enum::link, peerID, (*i)->channel, false);
            else paramset.reset(new Variable(VariableType::tStruct));
            if (paramset->errorStruct) paramset.reset(new Variable(VariableType::tStruct));
            element->structValue->insert(StructElement("RECEIVER_PARAMSET", paramset));
          }
          if (flags & 16) {
            PVariable description;
            if (!(brokenFlags & 2)) description = getDeviceDescription(clientInfo, channel, std::map<std::string, bool>());
            else description.reset(new Variable(VariableType::tStruct));
            if (description->errorStruct) description.reset(new Variable(VariableType::tStruct));
            element->structValue->insert(StructElement("RECEIVER_DESCRIPTION", description));
          }
          element->structValue->insert(StructElement("SENDER", PVariable(new Variable(peerSerialNumber + ":" + std::to_string((*i)->channel)))));
          element->structValue->insert(StructElement("SENDER_ID", PVariable(new Variable((int32_t)remotePeer->getID()))));
          element->structValue->insert(StructElement("SENDER_CHANNEL", PVariable(new Variable((*i)->channel))));
          if (flags & 2) {
            PVariable paramset;
            if (!(brokenFlags & 1) && remotePeer) paramset = remotePeer->getParamset(clientInfo, (*i)->channel, ParameterGroup::Type::Enum::link, _peerID, channel, false);
            else paramset.reset(new Variable(VariableType::tStruct));
            if (paramset->errorStruct) paramset.reset(new Variable(VariableType::tStruct));
            element->structValue->insert(StructElement("SENDER_PARAMSET", paramset));
          }
          if (flags & 8) {
            PVariable description;
            if (!(brokenFlags & 1) && remotePeer) description = remotePeer->getDeviceDescription(clientInfo, (*i)->channel, std::map<std::string, bool>());
            else description.reset(new Variable(VariableType::tStruct));
            if (description->errorStruct) description.reset(new Variable(VariableType::tStruct));
            element->structValue->insert(StructElement("SENDER_DESCRIPTION", description));
          }
        }
        array->arrayValue->push_back(element);
      }
    } else {
      if (channel > -1 && groupFlag) //Get links for each grouped channel
      {
        if (_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
        PFunction rpcFunction = _rpcDevice->functions.at(channel);
        if (rpcFunction->grouped) {
          element = getLink(clientInfo, channel, flags & 0xFFFFFFFE, avoidDuplicates);
          array->arrayValue->insert(array->arrayValue->end(), element->arrayValue->begin(), element->arrayValue->end());

          int32_t groupedWith = getChannelGroupedWith(channel);
          if (groupedWith > -1) {
            element = getLink(clientInfo, groupedWith, flags & 0xFFFFFFFE, avoidDuplicates);
            array->arrayValue->insert(array->arrayValue->end(), element->arrayValue->begin(), element->arrayValue->end());
          }
        } else {
          element = getLink(clientInfo, channel, flags & 0xFFFFFFFE, avoidDuplicates);
          array->arrayValue->insert(array->arrayValue->end(), element->arrayValue->begin(), element->arrayValue->end());
        }
      } else //Get links for all channels
      {
        for (Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i) {
          element.reset(new Variable(VariableType::tArray));
          element = getLink(clientInfo, i->first, flags, avoidDuplicates);
          array->arrayValue->insert(array->arrayValue->end(), element->arrayValue->begin(), element->arrayValue->end());
        }
      }
    }
    return array;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getLinkInfo(PRpcClientInfo clientInfo, int32_t senderChannel, uint64_t receiverID, int32_t receiverChannel) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    std::shared_ptr<BasicPeer> remotePeer = getPeer(senderChannel, receiverID, receiverChannel);
    if (!remotePeer) return Variable::createError(-2, "No peer found for sender channel.");
    PVariable response(new Variable(VariableType::tStruct));
    response->structValue->insert(StructElement("DESCRIPTION", PVariable(new Variable(remotePeer->linkDescription))));
    response->structValue->insert(StructElement("NAME", PVariable(new Variable(remotePeer->linkName))));
    return response;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getLinkPeers(PRpcClientInfo clientInfo, int32_t channel, bool returnID) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    PVariable array(new Variable(VariableType::tArray));
    if (channel > -1) {
      if (_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
      PFunction rpcFunction = _rpcDevice->functions.at(channel);
      //Return if there are no link roles defined
      if (rpcFunction->linkSenderFunctionTypes.empty() && rpcFunction->linkReceiverFunctionTypes.empty()) return array;
      std::shared_ptr<ICentral> central(getCentral());
      if (!central) return array; //central actually should always be set at this point
      _peersMutex.lock();
      std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>>::iterator peersIterator = _peers.find(channel);
      if (peersIterator == _peers.end() || peersIterator->second.empty()) {
        //Return if no peers are paired to the channel
        _peersMutex.unlock();
        return array;
      }
      std::vector<std::shared_ptr<BasicPeer>> peers = peersIterator->second;
      _peersMutex.unlock();
      for (std::vector<std::shared_ptr<Systems::BasicPeer>>::iterator i = peers.begin(); i != peers.end(); ++i) {
        if ((*i)->isVirtual) continue;
        std::shared_ptr<Peer> peer(central->getPeer((*i)->id));
        if (returnID && !peer) continue;
        bool peerKnowsMe = false;
        if (peer && peer->getPeer(channel, _peerID)) peerKnowsMe = true;

        std::string peerSerial = (*i)->serialNumber;
        if ((*i)->serialNumber.empty() || (*i)->id == 0) {
          if (peerKnowsMe || (*i)->id == _peerID) {
            (*i)->serialNumber = peer->getSerialNumber();
            (*i)->id = peer->getID();
            peerSerial = (*i)->serialNumber;
          } else {
            //Peer not paired to central
            std::ostringstream stringstream;
            stringstream << '@' << std::dec << (*i)->id;
            peerSerial = stringstream.str();
          }
        }
        if (returnID) {
          PVariable address(new Variable(VariableType::tArray));
          array->arrayValue->push_back(address);
          address->arrayValue->push_back(PVariable(new Variable(peer->getID())));
          address->arrayValue->push_back(PVariable(new Variable((*i)->channel)));
        } else array->arrayValue->push_back(PVariable(new Variable(peerSerial + ":" + std::to_string((*i)->channel))));
      }
    } else {
      for (Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i) {
        PVariable linkPeers = getLinkPeers(clientInfo, i->first, returnID);
        array->arrayValue->insert(array->arrayValue->end(), linkPeers->arrayValue->begin(), linkPeers->arrayValue->end());
      }
    }
    return array;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getRolesInDevice(PRpcClientInfo clientInfo, bool checkAcls) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");

    auto central = getCentral();
    if (!central) return Variable::createError(-32500, "Could not get central.");
    auto me = central->getPeer(_peerID);
    if (!me) return Variable::createError(-32500, "Could not get peer object.");

    auto channels = std::make_shared<Variable>(VariableType::tStruct);

    for (auto &channelIterator : valuesCentral) {
      auto variables = std::make_shared<Variable>(VariableType::tStruct);
      for (auto &variableIterator : channelIterator.second) {
        if (checkAcls && !clientInfo->acls->checkVariableReadAccess(me, channelIterator.first, variableIterator.first)) continue;

        auto roles = variableIterator.second.getRoles();
        if (!roles.empty()) {
          auto rolesArray = std::make_shared<Variable>(VariableType::tArray);
          rolesArray->arrayValue->reserve(roles.size());
          for (auto role : roles) {
            auto roleStruct = std::make_shared<Variable>(VariableType::tStruct);
            roleStruct->structValue->emplace("id", std::make_shared<BaseLib::Variable>(role.second.id));
            roleStruct->structValue->emplace("direction", std::make_shared<BaseLib::Variable>((int32_t)role.second.direction));
            if (role.second.invert) roleStruct->structValue->emplace("invert", std::make_shared<BaseLib::Variable>(role.second.invert));
            roleStruct->structValue->emplace("level", std::make_shared<BaseLib::Variable>((role.second.id / 10000) * 10000 == role.second.id ? 0 : ((role.second.id / 100) * 100 == role.second.id ? 1 : 2)));
            rolesArray->arrayValue->emplace_back(std::move(roleStruct));
          }
          variables->structValue->emplace(variableIterator.first, rolesArray);
        }
      }
      if (!variables->structValue->empty()) channels->structValue->emplace(std::to_string(channelIterator.first), variables);
    }

    return channels;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getRolesInRoom(PRpcClientInfo clientInfo, uint64_t roomId, bool checkAcls) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");

    auto central = getCentral();
    if (!central) return Variable::createError(-32500, "Could not get central.");
    auto me = central->getPeer(_peerID);
    if (!me) return Variable::createError(-32500, "Could not get peer object.");

    auto channels = std::make_shared<Variable>(VariableType::tStruct);

    for (auto &channelIterator : valuesCentral) {
      auto variables = std::make_shared<Variable>(VariableType::tStruct);
      for (auto &variableIterator : channelIterator.second) {
        if (checkAcls && !clientInfo->acls->checkVariableReadAccess(me, channelIterator.first, variableIterator.first)) continue;

        auto peerRoomId = variableIterator.second.getRoom();
        if (peerRoomId == 0) peerRoomId = getRoom(channelIterator.first);
        if (peerRoomId == 0) peerRoomId = getRoom(-1);
        if (peerRoomId != 0 && peerRoomId == roomId) {
          auto roles = variableIterator.second.getRoles();
          if (!roles.empty()) {
            auto rolesArray = std::make_shared<Variable>(VariableType::tArray);
            rolesArray->arrayValue->reserve(roles.size());
            for (auto role : roles) {
              auto roleStruct = std::make_shared<Variable>(VariableType::tStruct);
              roleStruct->structValue->emplace("id", std::make_shared<BaseLib::Variable>(role.second.id));
              roleStruct->structValue->emplace("direction", std::make_shared<BaseLib::Variable>((int32_t)role.second.direction));
              if (role.second.invert) roleStruct->structValue->emplace("invert", std::make_shared<BaseLib::Variable>(role.second.invert));
              roleStruct->structValue->emplace("level", std::make_shared<BaseLib::Variable>((role.second.id / 10000) * 10000 == role.second.id ? 0 : ((role.second.id / 100) * 100 == role.second.id ? 1 : 2)));
              rolesArray->arrayValue->emplace_back(std::move(roleStruct));
            }
            variables->structValue->emplace(variableIterator.first, rolesArray);
          }
        }
      }
      if (!variables->structValue->empty()) channels->structValue->emplace(std::to_string(channelIterator.first), variables);
    }

    return channels;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getParamset(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, bool checkAcls) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (channel < 0) channel = 0;
    if (remoteChannel < 0) remoteChannel = 0;
    Functions::iterator functionIterator = _rpcDevice->functions.find(channel);
    if (functionIterator == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
    if (type == ParameterGroup::Type::none) type = ParameterGroup::Type::link;
    PParameterGroup parameterGroup = getParameterSet(channel, type);
    if (!parameterGroup) return Variable::createError(-3, "Unknown parameter set.");
    PVariable variables = std::make_shared<Variable>(VariableType::tStruct);

    auto central = getCentral();
    if (!central) return Variable::createError(-32500, "Could not get central.");

    if (type == ParameterGroup::Type::Enum::variables) {
      auto valuesIterator = valuesCentral.find(channel);
      if (valuesIterator == valuesCentral.end()) return variables;
      for (auto &parameterIterator : valuesIterator->second) {
        RpcConfigurationParameter &parameter = parameterIterator.second;
        if (parameter.rpcParameter->id.empty() || !parameter.rpcParameter->visible) continue;
        if (checkAcls && !clientInfo->acls->checkVariableReadAccess(central->getPeer(_peerID), channel, parameter.rpcParameter->id)) continue;
        if (parameter.specialType == 0) {
          //Parameter also needs to be in ParamsetDescription, this is not necessarily the case (e. g. for switchable parameter sets)
          auto parameter2 = parameterGroup->getParameter(parameter.rpcParameter->id);
          if (!parameter2) continue;
        }
        if (!parameter.rpcParameter->visible && !parameter.rpcParameter->service && !parameter.rpcParameter->internal && !parameter.rpcParameter->transform) {
          _bl->out.printDebug("Debug: Omitting parameter " + parameter.rpcParameter->id + " because of it's ui flag.");
          continue;
        }
        if (clientInfo->clientType == RpcClientType::ccu2 && !parameter.rpcParameter->ccu2Visible) continue;
#ifdef CCU2
        if(parameter.rpcParameter && parameter.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif
        if (getParamsetHook2(clientInfo, parameter.rpcParameter, channel, variables)) continue;
        std::vector<uint8_t> parameterData = parameter.getBinaryData();
        PVariable element;
        if (!convertFromPacketHook(parameter, parameterData, element)) element = parameter.rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);
        if (!element) continue;
        if (element->type == VariableType::tVoid) continue;
        if (parameter.rpcParameter->password && (!clientInfo || !clientInfo->scriptEngineServer)) element.reset(new Variable(element->type));
        variables->structValue->insert(StructElement(parameter.rpcParameter->id, element));
      }
    } else if (type == ParameterGroup::Type::Enum::config) {
      auto configIterator = configCentral.find(channel);
      if (configIterator == configCentral.end()) return variables;
      for (auto &parameterIterator : configIterator->second) {
        RpcConfigurationParameter &parameter = parameterIterator.second;
        if (parameter.rpcParameter->id.empty() || !parameter.rpcParameter->visible) continue;
        if (parameter.specialType == 0) {
          //Parameter also needs to be in ParamsetDescription, this is not necessarily the case (e. g. for switchable parameter sets)
          auto parameter2 = parameterGroup->getParameter(parameter.rpcParameter->id);
          if (!parameter2) continue;
        }
        if (!parameter.rpcParameter->visible && !parameter.rpcParameter->service && !parameter.rpcParameter->internal && !parameter.rpcParameter->transform) {
          _bl->out.printDebug("Debug: Omitting parameter " + parameter.rpcParameter->id + " because of it's ui flag.");
          continue;
        }
        if (clientInfo->clientType == RpcClientType::ccu2 && !parameter.rpcParameter->ccu2Visible) continue;
#ifdef CCU2
        if(parameter.rpcParameter && parameter.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif
        std::vector<uint8_t> parameterData = parameter.getBinaryData();
        PVariable element;
        if (!convertFromPacketHook(parameter, parameterData, element)) element = parameter.rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);
        if (!element) continue;
        if (element->type == VariableType::tVoid) continue;
        if (parameter.rpcParameter->password && (!clientInfo || !clientInfo->scriptEngineServer)) element.reset(new Variable(element->type));
        variables->structValue->insert(StructElement(parameter.rpcParameter->id, element));
      }
    } else if (type == ParameterGroup::Type::Enum::link) {
      std::shared_ptr<BasicPeer> remotePeer;
      if (remoteID == 0) remoteID = 0xFFFFFFFFFFFFFFFF; //Remote peer is central
      remotePeer = getPeer(channel, remoteID, remoteChannel);
      if (!remotePeer || remotePeer->channel != remoteChannel) return Variable::createError(-3, "Not paired to this peer.");

      auto linksIterator = linksCentral.find(channel);
      if (linksIterator == linksCentral.end()) return Variable::createError(-2, "Unknown channel.");
      auto remotePeerIterator = linksIterator->second.find(remotePeer->address);
      if (remotePeerIterator == linksIterator->second.end()) return Variable::createError(-3, "Unknown remote peer.");
      auto remoteChannelIterator = remotePeerIterator->second.find(remoteChannel);
      if (remoteChannelIterator == remotePeerIterator->second.end()) return Variable::createError(-3, "Unknown remote channel.");
      for (auto &parameterIterator : remoteChannelIterator->second) {
        RpcConfigurationParameter &parameter = parameterIterator.second;
        if (parameter.rpcParameter->id.empty() || !parameter.rpcParameter->visible) continue;
        if (parameter.specialType == 0) {
          //Parameter also needs to be in ParamsetDescription, this is not necessarily the case (e. g. for switchable parameter sets)
          auto parameter2 = parameterGroup->getParameter(parameter.rpcParameter->id);
          if (!parameter2) continue;
        }
        if (!parameter.rpcParameter->visible && !parameter.rpcParameter->service && !parameter.rpcParameter->internal && !parameter.rpcParameter->transform) {
          _bl->out.printDebug("Debug: Omitting parameter " + parameter.rpcParameter->id + " because of it's ui flag.");
          continue;
        }
        if (clientInfo->clientType == RpcClientType::ccu2 && !parameter.rpcParameter->ccu2Visible) continue;
#ifdef CCU2
        if(parameter.rpcParameter && parameter.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif
        std::vector<uint8_t> parameterData = parameter.getBinaryData();
        PVariable element;
        if (!convertFromPacketHook(parameter, parameterData, element)) element = parameter.rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);
        if (!element) continue;
        if (element->type == VariableType::tVoid) continue;
        if (parameter.rpcParameter->password && (!clientInfo || !clientInfo->scriptEngineServer)) element.reset(new Variable(element->type));
        variables->structValue->insert(StructElement(parameter.rpcParameter->id, element));
      }
    }

    return variables;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getParamsetDescription(PRpcClientInfo clientInfo, int32_t channel, PParameterGroup parameterGroup, bool checkAcls) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!clientInfo) clientInfo.reset(new RpcClientInfo());

    auto central = getCentral();
    if (!central) return Variable::createError(-32500, "Could not get central.");

    PVariable descriptions(new Variable(VariableType::tStruct));
    uint32_t index = 0;

    if (parameterGroup->type() == ParameterGroup::Type::Enum::variables) {
      auto valuesIterator = valuesCentral.find(channel);
      if (valuesIterator == valuesCentral.end()) return descriptions; //Parameter set exists but is empty
      for (auto &parameterIterator : valuesIterator->second) {
        RpcConfigurationParameter &parameter = parameterIterator.second;
        if (parameter.rpcParameter->id.empty() || !parameter.rpcParameter->visible) continue;
        if (checkAcls && !clientInfo->acls->checkVariableReadAccess(central->getPeer(_peerID), channel, parameter.rpcParameter->id)) continue;
        if (parameter.specialType == 0) {
          //Parameter also needs to be in ParamsetDescription, this is not necessarily the case (e. g. for switchable parameter sets)
          auto parameter2 = parameterGroup->getParameter(parameter.rpcParameter->id);
          if (!parameter2) continue;
        }
        if (!parameter.rpcParameter->visible && !parameter.rpcParameter->service && !parameter.rpcParameter->internal && !parameter.rpcParameter->transform) {
          _bl->out.printDebug("Debug: Omitting parameter " + parameter.rpcParameter->id + " because of it's ui flag.");
          continue;
        }
        if (clientInfo->clientType == RpcClientType::ccu2 && !parameter.rpcParameter->ccu2Visible) continue;
#ifdef CCU2
        if(parameter.rpcParameter && parameter.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif

        PVariable description = getVariableDescription(clientInfo, parameter.rpcParameter, channel, parameterGroup->type(), index, std::unordered_set<std::string>());
        if (!description || description->errorStruct) continue;

        index++;
        descriptions->structValue->emplace(StructElement(parameter.rpcParameter->id, std::move(description)));
      }
    } else if (parameterGroup->type() == ParameterGroup::Type::Enum::config) {
      auto configIterator = configCentral.find(channel);
      if (configIterator == configCentral.end()) return descriptions; //Parameter set exists but is empty
      for (auto &parameterIterator : configIterator->second) {
        RpcConfigurationParameter &parameter = parameterIterator.second;
        if (parameter.rpcParameter->id.empty()) continue;
        if (parameter.specialType == 0) {
          //Parameter also needs to be in ParamsetDescription, this is not necessarily the case (e. g. for switchable parameter sets)
          auto parameter2 = parameterGroup->getParameter(parameter.rpcParameter->id);
          if (!parameter2) continue;
        }
        if (!parameter.rpcParameter->visible && !parameter.rpcParameter->service && !parameter.rpcParameter->internal && !parameter.rpcParameter->transform) {
          _bl->out.printDebug("Debug: Omitting parameter " + parameter.rpcParameter->id + " because of it's ui flag.");
          continue;
        }
        if (clientInfo->clientType == RpcClientType::ccu2 && !parameter.rpcParameter->ccu2Visible) continue;
#ifdef CCU2
        if(parameter.rpcParameter && parameter.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif

        PVariable description = getVariableDescription(clientInfo, parameter.rpcParameter, channel, parameterGroup->type(), index, std::unordered_set<std::string>());
        if (!description || description->errorStruct) continue;

        index++;
        descriptions->structValue->emplace(StructElement(parameter.rpcParameter->id, std::move(description)));
      }
    } else if (parameterGroup->type() == ParameterGroup::Type::Enum::link) {
      for (auto &parameter : parameterGroup->parameters) {
        if (!parameter.second || parameter.second->id.empty() || !parameter.second->visible) continue;
        if (!parameter.second->visible && !parameter.second->service && !parameter.second->internal && !parameter.second->transform) {
          _bl->out.printDebug("Debug: Omitting parameter " + parameter.second->id + " because of it's ui flag.");
          continue;
        }

        PVariable description = getVariableDescription(clientInfo, parameter.second, channel, parameterGroup->type(), index, std::unordered_set<std::string>());
        if (!description || description->errorStruct) continue;

        index++;
        descriptions->structValue->insert(StructElement(parameter.second->id, description));
      }
    }

    return descriptions;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getParamsetDescription(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, bool checkAcls) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (channel < 0) channel = 0;
    if (type == ParameterGroup::Type::none) type = ParameterGroup::Type::link;
    PParameterGroup parameterGroup = getParameterSet(channel, type);
    if (!parameterGroup) return Variable::createError(-3, "Unknown parameter set.");
    if (type == ParameterGroup::Type::link && remoteID > 0) {
      std::shared_ptr<Systems::BasicPeer> remotePeer = getPeer(channel, remoteID, remoteChannel);
      if (!remotePeer) return Variable::createError(-2, "Unknown remote peer.");
    }

    return Peer::getParamsetDescription(clientInfo, channel, parameterGroup, checkAcls);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getParamsetId(PRpcClientInfo clientInfo, uint32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
    PFunction rpcFunction = _rpcDevice->functions.at(channel);
    std::shared_ptr<BasicPeer> remotePeer;
    if (type == ParameterGroup::Type::link && remoteID > 0) {
      remotePeer = getPeer(channel, remoteID, remoteChannel);
      if (!remotePeer) return Variable::createError(-2, "Unknown remote peer.");
    }

    std::string id;
    if (type == ParameterGroup::Type::Enum::config) id = rpcFunction->configParameters->id;
    else if (type == ParameterGroup::Type::Enum::variables) id = rpcFunction->variables->id;
    else if (type == ParameterGroup::Type::Enum::link) id = rpcFunction->linkParameters->id;
    int32_t pos = id.find_last_of("--");
    if (pos > 0) id = id.substr(0, pos - 1);
    return PVariable(new Variable(id));
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getServiceMessages(PRpcClientInfo clientInfo, bool returnID) {
  if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
  if (!serviceMessages) return Variable::createError(-32500, "Service messages are not initialized.");
  return serviceMessages->get(clientInfo, returnID);
}

PVariable Peer::getValue(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");
    if (valueKey == "IP_ADDRESS") return PVariable(new Variable(_ip));
    else if (valueKey == "PEER_ID") return PVariable(new Variable((int32_t)_peerID));
    std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return Variable::createError(-2, "Unknown channel.");
    std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = channelIterator->second.find(valueKey);
    if (parameterIterator == channelIterator->second.end()) return Variable::createError(-5, "Unknown parameter.");
    RpcConfigurationParameter &parameter = parameterIterator->second;

    //Check if channel still exists in device description
    Functions::iterator functionIterator = _rpcDevice->functions.find(channel);
    if (functionIterator == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel (2).");

    if (!parameter.rpcParameter->readable && !parameter.rpcParameter->transmitted) return Variable::createError(-6, "Parameter is not readable.");
    PVariable variable;
    if (requestFromDevice) {
      variable = getValueFromDevice(parameter.rpcParameter, channel, asynchronous);
      if (parameter.rpcParameter->password && (!clientInfo || !clientInfo->scriptEngineServer)) variable.reset(new Variable(variable->type));
      if ((!asynchronous && variable->type != VariableType::tVoid) || variable->errorStruct) return variable;
    }
    std::vector<uint8_t> parameterData = parameterIterator->second.getBinaryData();
    if (!convertFromPacketHook(parameter, parameterData, variable)) variable = parameter.rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);
    if (parameter.rpcParameter->password && (!clientInfo || !clientInfo->scriptEngineServer)) variable.reset(new Variable(variable->type));
    return variable;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getVariableDescription(PRpcClientInfo clientInfo, const PParameter &parameter, int32_t channel, ParameterGroup::Type::Enum type, int32_t index, const std::unordered_set<std::string> &fields) {
  try {
    if (!parameter || parameter->id.empty() || !parameter->visible) return Variable::createError(-5, "Unknown parameter.");
    if (!parameter->visible && !parameter->service && !parameter->internal && !parameter->transform) {
      _bl->out.printDebug("Debug: Omitting parameter " + parameter->id + " because of it's ui flag.");
      return Variable::createError(-5, "Unknown parameter (1).");
    }
#ifdef CCU2
    if(parameter->logical->type == ILogical::Type::tInteger64) continue;
#endif
    if (clientInfo->clientType == RpcClientType::ccu2 && !parameter->ccu2Visible) return Variable::createError(-5, "Parameter is invisible on the CCU2.");

    PVariable description = std::make_shared<Variable>(VariableType::tStruct);

    int32_t operations = 0;
    if (parameter->readable) operations += 4;
    if (parameter->writeable) operations += 2;
    if (parameter->transmitted) operations += 1;
    int32_t uiFlags = 0;
    if (parameter->visible) uiFlags += 1;
    if (parameter->internal) uiFlags += 2;
    if (parameter->transform) uiFlags += 4;
    if (parameter->service) uiFlags += 8;
    if (parameter->sticky) uiFlags += 0x10;
    if (parameter->logical->type == ILogical::Type::tBoolean) {
      LogicalBoolean *logicalBoolean = (LogicalBoolean *)parameter->logical.get();

      if ((fields.empty() || fields.find("CONTROL") != fields.end()) && !parameter->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameter->control))));
      if ((fields.empty() || fields.find("DEFAULT") != fields.end()) && logicalBoolean->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(logicalBoolean->defaultValue))));
      if (fields.empty() || fields.find("FLAGS") != fields.end()) description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable(parameter->id))));
      if (fields.empty() || fields.find("MAX") != fields.end()) description->structValue->insert(StructElement("MAX", PVariable(new Variable(true))));
      if (fields.empty() || fields.find("MIN") != fields.end()) description->structValue->insert(StructElement("MIN", PVariable(new Variable(false))));
      if (fields.empty() || fields.find("OPERATIONS") != fields.end()) description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
      if ((fields.empty() || fields.find("TAB_ORDER") != fields.end()) && index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("BOOL")))));
    } else if (parameter->logical->type == ILogical::Type::tString) {
      LogicalString *logicalString = (LogicalString *)parameter->logical.get();

      if ((fields.empty() || fields.find("CONTROL") != fields.end()) && !parameter->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameter->control))));
      if ((fields.empty() || fields.find("DEFAULT") != fields.end()) && logicalString->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(logicalString->defaultValue))));
      if (fields.empty() || fields.find("FLAGS") != fields.end()) description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable(parameter->id))));
      if (fields.empty() || fields.find("MAX") != fields.end()) description->structValue->insert(StructElement("MAX", PVariable(new Variable(std::string("")))));
      if (fields.empty() || fields.find("MIN") != fields.end()) description->structValue->insert(StructElement("MIN", PVariable(new Variable(std::string("")))));
      if (fields.empty() || fields.find("OPERATIONS") != fields.end()) description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
      if ((fields.empty() || fields.find("TAB_ORDER") != fields.end()) && index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRING")))));
    } else if (parameter->logical->type == ILogical::Type::tAction) {
      LogicalAction *logicalAction = (LogicalAction *)parameter->logical.get();

      if ((fields.empty() || fields.find("CONTROL") != fields.end()) && !parameter->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameter->control))));
      if ((fields.empty() || fields.find("DEFAULT") != fields.end()) && logicalAction->defaultValueExists)
        description->structValue->insert(StructElement("DEFAULT",
                                                       PVariable(new Variable(logicalAction->defaultValue)))); //CCU needs this, otherwise updates are not processed in programs
      if (fields.empty() || fields.find("FLAGS") != fields.end()) description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable(parameter->id))));
      if (fields.empty() || fields.find("MAX") != fields.end()) description->structValue->insert(StructElement("MAX", PVariable(new Variable(true))));
      if (fields.empty() || fields.find("MIN") != fields.end()) description->structValue->insert(StructElement("MIN", PVariable(new Variable(false))));
      if (fields.empty() || fields.find("OPERATIONS") != fields.end()) description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations & 0xFE)))); //Remove read
      if ((fields.empty() || fields.find("TAB_ORDER") != fields.end()) && index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ACTION")))));
    } else if (parameter->logical->type == ILogical::Type::tInteger) {
      LogicalInteger *logicalInteger = (LogicalInteger *)parameter->logical.get();

      if ((fields.empty() || fields.find("CONTROL") != fields.end()) && !parameter->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameter->control))));
      if ((fields.empty() || fields.find("DEFAULT") != fields.end()) && logicalInteger->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(logicalInteger->defaultValue))));
      if (fields.empty() || fields.find("FLAGS") != fields.end()) description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable(parameter->id))));
      if (fields.empty() || fields.find("MAX") != fields.end()) description->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalInteger->maximumValue))));
      if (fields.empty() || fields.find("MIN") != fields.end()) description->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalInteger->minimumValue))));
      if (fields.empty() || fields.find("OPERATIONS") != fields.end()) description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));

      if ((fields.empty() || fields.find("SPECIAL") != fields.end()) && !logicalInteger->specialValuesStringMap.empty()) {
        PVariable specialValues(new Variable(VariableType::tArray));
        for (std::unordered_map<std::string, int32_t>::iterator j = logicalInteger->specialValuesStringMap.begin(); j != logicalInteger->specialValuesStringMap.end(); ++j) {
          PVariable specialElement(new Variable(VariableType::tStruct));
          specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
          specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
          specialValues->arrayValue->push_back(specialElement);
        }
        description->structValue->insert(StructElement("SPECIAL", specialValues));
      }

      if ((fields.empty() || fields.find("TAB_ORDER") != fields.end()) && index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER")))));
    } else if (parameter->logical->type == ILogical::Type::tInteger64) {
      LogicalInteger64 *logicalInteger64 = (LogicalInteger64 *)parameter->logical.get();

      if ((fields.empty() || fields.find("CONTROL") != fields.end()) && !parameter->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameter->control))));
      if ((fields.empty() || fields.find("DEFAULT") != fields.end()) && logicalInteger64->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(logicalInteger64->defaultValue))));
      if (fields.empty() || fields.find("FLAGS") != fields.end()) description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable(parameter->id))));
      if (fields.empty() || fields.find("MAX") != fields.end()) description->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalInteger64->maximumValue))));
      if (fields.empty() || fields.find("MIN") != fields.end()) description->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalInteger64->minimumValue))));
      if (fields.empty() || fields.find("OPERATIONS") != fields.end()) description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));

      if ((fields.empty() || fields.find("SPECIAL") != fields.end()) && !logicalInteger64->specialValuesStringMap.empty()) {
        PVariable specialValues(new Variable(VariableType::tArray));
        for (std::unordered_map<std::string, int64_t>::iterator j = logicalInteger64->specialValuesStringMap.begin(); j != logicalInteger64->specialValuesStringMap.end(); ++j) {
          PVariable specialElement(new Variable(VariableType::tStruct));
          specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
          specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
          specialValues->arrayValue->push_back(specialElement);
        }
        description->structValue->insert(StructElement("SPECIAL", specialValues));
      }

      if ((fields.empty() || fields.find("TAB_ORDER") != fields.end()) && index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER64")))));
    } else if (parameter->logical->type == ILogical::Type::tEnum) {
      LogicalEnumeration *logicalEnumeration = (LogicalEnumeration *)parameter->logical.get();

      if ((fields.empty() || fields.find("CONTROL") != fields.end()) && !parameter->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameter->control))));
      if (fields.empty() || fields.find("DEFAULT") != fields.end()) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(logicalEnumeration->defaultValueExists ? logicalEnumeration->defaultValue : 0))));
      if (fields.empty() || fields.find("FLAGS") != fields.end()) description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable(parameter->id))));
      if (fields.empty() || fields.find("MAX") != fields.end()) description->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalEnumeration->maximumValue))));
      if (fields.empty() || fields.find("MIN") != fields.end()) description->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalEnumeration->minimumValue))));
      if (fields.empty() || fields.find("OPERATIONS") != fields.end()) description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
      if ((fields.empty() || fields.find("TAB_ORDER") != fields.end()) && index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ENUM")))));

      if (fields.empty() || fields.find("VALUE_LIST") != fields.end()) {
        PVariable valueList(new Variable(VariableType::tArray));
        for (std::vector<EnumerationValue>::iterator j = logicalEnumeration->values.begin(); j != logicalEnumeration->values.end(); ++j) {
          valueList->arrayValue->push_back(PVariable(new Variable(j->id)));
        }
        description->structValue->insert(StructElement("VALUE_LIST", valueList));
      }
    } else if (parameter->logical->type == ILogical::Type::tFloat) {
      LogicalDecimal *logicalDecimal = (LogicalDecimal *)parameter->logical.get();

      if ((fields.empty() || fields.find("CONTROL") != fields.end()) && !parameter->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameter->control))));
      if ((fields.empty() || fields.find("DEFAULT") != fields.end()) && logicalDecimal->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(logicalDecimal->defaultValue))));
      if (fields.empty() || fields.find("FLAGS") != fields.end()) description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable(parameter->id))));
      if (fields.empty() || fields.find("MAX") != fields.end()) description->structValue->insert(StructElement("MAX", PVariable(new Variable(logicalDecimal->maximumValue))));
      if (fields.empty() || fields.find("MIN") != fields.end()) description->structValue->insert(StructElement("MIN", PVariable(new Variable(logicalDecimal->minimumValue))));
      if (fields.empty() || fields.find("OPERATIONS") != fields.end()) description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));

      if ((fields.empty() || fields.find("SPECIAL") != fields.end()) && !logicalDecimal->specialValuesStringMap.empty()) {
        PVariable specialValues(new Variable(VariableType::tArray));
        for (std::unordered_map<std::string, double>::iterator j = logicalDecimal->specialValuesStringMap.begin(); j != logicalDecimal->specialValuesStringMap.end(); ++j) {
          PVariable specialElement(new Variable(VariableType::tStruct));
          specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
          specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
          specialValues->arrayValue->push_back(specialElement);
        }
        description->structValue->insert(StructElement("SPECIAL", specialValues));
      }

      if ((fields.empty() || fields.find("TAB_ORDER") != fields.end()) && index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("FLOAT")))));
    } else if (parameter->logical->type == ILogical::Type::tArray) {
      if (!clientInfo->initNewFormat) return Variable::createError(-5, "Parameter is unsupported by this client.");
      if ((fields.empty() || fields.find("CONTROL") != fields.end()) && !parameter->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameter->control))));
      if (fields.empty() || fields.find("FLAGS") != fields.end()) description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable(parameter->id))));
      if (fields.empty() || fields.find("OPERATIONS") != fields.end()) description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
      if ((fields.empty() || fields.find("TAB_ORDER") != fields.end()) && index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ARRAY")))));
    } else if (parameter->logical->type == ILogical::Type::tStruct) {
      if (!clientInfo->initNewFormat) return Variable::createError(-5, "Parameter is unsupported by this client.");
      if ((fields.empty() || fields.find("CONTROL") != fields.end()) && !parameter->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameter->control))));
      if (fields.empty() || fields.find("FLAGS") != fields.end()) description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
      if (fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable(parameter->id))));
      if (fields.empty() || fields.find("OPERATIONS") != fields.end()) description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
      if ((fields.empty() || fields.find("TAB_ORDER") != fields.end()) && index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
      if (fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRUCT")))));
    }

    if (fields.empty() || fields.find("UNIT") != fields.end()) description->structValue->insert(StructElement("UNIT", PVariable(new Variable(parameter->unit))));
    if ((fields.empty() || fields.find("MANDATORY") != fields.end()) && parameter->mandatory) description->structValue->emplace("MANDATORY", std::make_shared<Variable>(parameter->mandatory));
    if ((fields.empty() || fields.find("FORM_FIELD_TYPE") != fields.end()) && !parameter->formFieldType.empty()) description->structValue->insert(StructElement("FORM_FIELD_TYPE", PVariable(new Variable(parameter->formFieldType))));
    if ((fields.empty() || fields.find("FORM_POSITION") != fields.end()) && parameter->formPosition != -1) description->structValue->insert(StructElement("FORM_POSITION", PVariable(new Variable(parameter->formPosition))));

    if (type == ParameterGroup::Type::Enum::variables) {
      auto valuesCentralIterator = valuesCentral.find(channel);
      if (valuesCentralIterator == valuesCentral.end()) return Variable::createError(-5, "Unknown parameter (2).");
      auto valueParameterIterator = valuesCentralIterator->second.find(parameter->id);
      if (valueParameterIterator == valuesCentralIterator->second.end()) return Variable::createError(-5, "Unknown parameter (3).");

      if (fields.empty() || fields.find("ROOM") != fields.end()) {
        auto room = valueParameterIterator->second.getRoom();
        if (room != 0) description->structValue->emplace("ROOM", std::make_shared<Variable>(room));
      }

      if (fields.empty() || fields.find("CATEGORIES") != fields.end()) {
        auto categories = valueParameterIterator->second.getCategories();
        if (!categories.empty()) {
          PVariable categoriesResult = std::make_shared<Variable>(VariableType::tArray);
          categoriesResult->arrayValue->reserve(categories.size());
          for (auto category : categories) {
            categoriesResult->arrayValue->push_back(std::make_shared<Variable>(category));
          }
          description->structValue->emplace("CATEGORIES", categoriesResult);
        }
      }

      if (fields.empty() || fields.find("ROLES") != fields.end()) {
        auto roles = valueParameterIterator->second.getRoles();
        if (!roles.empty()) {
          auto rolesArray = std::make_shared<Variable>(VariableType::tArray);
          rolesArray->arrayValue->reserve(roles.size());
          for (auto role : roles) {
            auto roleStruct = std::make_shared<Variable>(VariableType::tStruct);
            roleStruct->structValue->emplace("id", std::make_shared<BaseLib::Variable>(role.second.id));
            roleStruct->structValue->emplace("direction", std::make_shared<BaseLib::Variable>((int32_t)role.second.direction));
            if (role.second.invert) roleStruct->structValue->emplace("invert", std::make_shared<BaseLib::Variable>(role.second.invert));
            roleStruct->structValue->emplace("level", std::make_shared<BaseLib::Variable>((role.second.id / 10000) * 10000 == role.second.id ? 0 : ((role.second.id / 100) * 100 == role.second.id ? 1 : 2)));
            rolesArray->arrayValue->emplace_back(std::move(roleStruct));
          }
          description->structValue->emplace("ROLES", rolesArray);
        }
      }
    }

    if (fields.empty() || fields.find("LABEL") != fields.end() || fields.find("DESCRIPTION") != fields.end()) {
      std::shared_ptr<ICentral> central = getCentral();
      if (!central) return description;
      std::string language = clientInfo ? clientInfo->language : "en-US";
      std::string filename = _rpcDevice->getFilename();
      if (parameter->parent()) {
        auto parameterTranslations = central->getTranslations()->getParameterTranslations(filename, language, parameter->parent()->type(), parameter->parent()->id, parameter->id);
        if (!parameterTranslations.first.empty()) description->structValue->insert(StructElement("LABEL", std::shared_ptr<Variable>(new Variable(parameterTranslations.first))));
        if (!parameterTranslations.second.empty()) description->structValue->insert(StructElement("DESCRIPTION", std::shared_ptr<Variable>(new Variable(parameterTranslations.second))));
      }
    }

    return description;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getVariableDescription(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, const std::unordered_set<std::string> &fields) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");

    PParameterGroup parameterGroup = getParameterSet(channel, ParameterGroup::Type::Enum::variables);
    if (!parameterGroup) return Variable::createError(-2, "Unknown channel.");

    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return Variable::createError(-2, "Unknown channel.");

    auto parameterIterator = channelIterator->second.find(valueKey);
    if (parameterIterator == channelIterator->second.end()) return Variable::createError(-5, "Unknown parameter.");

    if (parameterIterator->second.specialType == 0) {
      //Parameter also needs to be in ParamsetDescription, this is not necessarily the case (e. g. for switchable parameter sets)
      auto parameterIterator2 = channelIterator->second.find(valueKey);
      if (parameterIterator2 == channelIterator->second.end()) return Variable::createError(-5, "Unknown parameter.");
    }

    return getVariableDescription(clientInfo, parameterIterator->second.rpcParameter, channel, ParameterGroup::Type::Enum::variables, -1, fields);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getVariablesInCategory(PRpcClientInfo clientInfo, uint64_t categoryId, bool checkAcls) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");

    auto central = getCentral();
    if (!central) return Variable::createError(-32500, "Could not get central.");
    auto me = central->getPeer(_peerID);
    if (!me) return Variable::createError(-32500, "Could not get peer object.");

    auto channels = std::make_shared<Variable>(VariableType::tStruct);

    for (auto &channelIterator : valuesCentral) {
      auto variables = std::make_shared<Variable>(VariableType::tArray);
      variables->arrayValue->reserve(channelIterator.second.size());
      for (auto &variableIterator : channelIterator.second) {
        if (checkAcls && !clientInfo->acls->checkVariableReadAccess(me, channelIterator.first, variableIterator.first)) continue;
        if (variableIterator.second.hasCategory(categoryId)) variables->arrayValue->push_back(std::make_shared<Variable>(variableIterator.first));
      }
      if (!variables->arrayValue->empty()) channels->structValue->emplace(std::to_string(channelIterator.first), variables);
    }

    return channels;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getVariablesInRole(PRpcClientInfo clientInfo, uint64_t roleId, bool checkAcls) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");

    auto central = getCentral();
    if (!central) return Variable::createError(-32500, "Could not get central.");
    auto me = central->getPeer(_peerID);
    if (!me) return Variable::createError(-32500, "Could not get peer object.");

    auto channels = std::make_shared<Variable>(VariableType::tStruct);

    for (auto &channelIterator : valuesCentral) {
      auto variables = std::make_shared<Variable>(VariableType::tStruct);
      for (auto &variableIterator : channelIterator.second) {
        if (checkAcls && !clientInfo->acls->checkVariableReadAccess(me, channelIterator.first, variableIterator.first)) continue;
        if (variableIterator.second.hasRole(roleId)) {
          auto entry = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
          auto role = variableIterator.second.getRole(roleId);
          entry->structValue->emplace("direction", std::make_shared<BaseLib::Variable>((int32_t)role.direction));
          if (role.invert) entry->structValue->emplace("invert", std::make_shared<BaseLib::Variable>(role.invert));
          entry->structValue->emplace("level", std::make_shared<BaseLib::Variable>((role.id / 10000) * 10000 == role.id ? 0 : ((role.id / 100) * 100 == role.id ? 1 : 2)));
          variables->structValue->emplace(variableIterator.first, entry);
        }
      }
      if (!variables->structValue->empty()) channels->structValue->emplace(std::to_string(channelIterator.first), variables);
    }

    return channels;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::getVariablesInRoom(PRpcClientInfo clientInfo, uint64_t roomId, bool returnDeviceAssigned, bool checkAcls) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");

    auto central = getCentral();
    if (!central) return Variable::createError(-32500, "Could not get central.");
    auto me = central->getPeer(_peerID);
    if (!me) return Variable::createError(-32500, "Could not get peer object.");

    auto channels = std::make_shared<Variable>(VariableType::tStruct);

    for (auto &channelIterator : valuesCentral) {
      auto variables = std::make_shared<Variable>(VariableType::tArray);
      variables->arrayValue->reserve(channelIterator.second.size());
      for (auto &variableIterator : channelIterator.second) {
        if (checkAcls && !clientInfo->acls->checkVariableReadAccess(me, channelIterator.first, variableIterator.first)) continue;
        if (variableIterator.second.getRoom() == 0) {
          if (returnDeviceAssigned) {
            auto channelRoomId = getRoom(channelIterator.first);
            if (channelRoomId == 0) channelRoomId = getRoom(-1);
            if (roomId == channelRoomId) variables->arrayValue->push_back(std::make_shared<Variable>(variableIterator.first));
          }
        } else if (variableIterator.second.getRoom() == roomId) variables->arrayValue->push_back(std::make_shared<Variable>(variableIterator.first));
      }
      if (!variables->arrayValue->empty()) channels->structValue->emplace(std::to_string(channelIterator.first), variables);
    }

    return channels;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::reportValueUsage(PRpcClientInfo clientInfo) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    return PVariable(new Variable(!serviceMessages->getConfigPending()));
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::rssiInfo(PRpcClientInfo clientInfo) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (!wireless()) return Variable::createError(-100, "Peer is not a wireless peer.");
    if (valuesCentral.find(0) == valuesCentral.end() || valuesCentral.at(0).find("RSSI_DEVICE") == valuesCentral.at(0).end() || !valuesCentral.at(0).at("RSSI_DEVICE").rpcParameter) {
      return Variable::createError(-101, "Peer has no rssi information.");
    }
    PVariable response(new Variable(VariableType::tStruct));
    PVariable rpcArray(new Variable(VariableType::tArray));

    std::vector<uint8_t> parameterData;
    PVariable element;
    if (valuesCentral.at(0).find("RSSI_PEER") != valuesCentral.at(0).end() && valuesCentral.at(0).at("RSSI_PEER").rpcParameter) {
      auto &parameter = valuesCentral.at(0).at("RSSI_PEER");
      parameterData = parameter.getBinaryData();
      element = parameter.rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);
      if (element->integerValue == 0) element->integerValue = 65536;
      rpcArray->arrayValue->push_back(element);
    } else {
      element = PVariable(new Variable(65536));
      rpcArray->arrayValue->push_back(element);
    }

    {
      auto &parameter = valuesCentral.at(0).at("RSSI_DEVICE");
      parameterData = parameter.getBinaryData();
      element = parameter.rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);
      if (element->integerValue == 0) element->integerValue = 65536;
      rpcArray->arrayValue->push_back(element);
    }

    std::shared_ptr<ICentral> central = getCentral();
    if (!central) return Variable::createError(-32500, "Central is nullptr.");
    response->structValue->insert(StructElement(central->getSerialNumber(), rpcArray));
    return response;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::setLinkInfo(PRpcClientInfo clientInfo, int32_t senderChannel, uint64_t receiverID, int32_t receiverChannel, std::string name, std::string description) {
  try {
    std::shared_ptr<BasicPeer> remotePeer = getPeer(senderChannel, receiverID, receiverChannel);
    if (!remotePeer) return Variable::createError(-2, "No peer found for sender channel..");
    remotePeer->linkDescription = description;
    remotePeer->linkName = name;
    savePeers();
    return PVariable(new Variable(VariableType::tVoid));
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable Peer::setId(PRpcClientInfo clientInfo, uint64_t newPeerId) {
  try {
    if (newPeerId == 0 || newPeerId >= 0x40000000) return Variable::createError(-100, "New peer ID is invalid.");
    if (newPeerId == _peerID) return Variable::createError(-100, "New peer ID is the same as the old one.");

    std::shared_ptr<ICentral> central(getCentral());
    if (central) {
      std::shared_ptr<Peer> peer = central->getPeer(newPeerId);
      if (peer) return Variable::createError(-101, "New peer ID is already in use.");
      if (!_bl->db->setPeerID(_peerID, newPeerId)) return Variable::createError(-32500, "Error setting id. See log for more details.");
      _peerID = newPeerId;
      if (serviceMessages) serviceMessages->setPeerId(newPeerId);
      return PVariable(new Variable(VariableType::tVoid));
    } else return Variable::createError(-32500, "Application error. Central could not be found.");
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error. See error log for more details.");
}

PVariable Peer::setValue(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, PVariable value, bool wait) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (valueKey.empty()) return Variable::createError(-5, "Value key is empty.");
    auto channelIterator = valuesCentral.find(channel);
    if (channelIterator == valuesCentral.end()) return Variable::createError(-2, "Unknown channel.");
    auto parameterIterator = channelIterator->second.find(valueKey);
    if (parameterIterator == channelIterator->second.end()) return Variable::createError(-5, "Unknown parameter.");
    RpcConfigurationParameter &parameter = parameterIterator->second;
    PParameter rpcParameter = parameter.rpcParameter;
    if (!rpcParameter) return Variable::createError(-5, "Unknown parameter.");

    // {{{ Boundary check and variable conversion
    if (rpcParameter->logical->type == ILogical::Type::tBoolean) {
      if (value->type == VariableType::tInteger) value->booleanValue = (bool)value->integerValue;
      else if (value->type == VariableType::tInteger64) value->booleanValue = (bool)value->integerValue64;
      else if (value->type == VariableType::tFloat) value->booleanValue = (bool)value->floatValue;
    } else if (rpcParameter->logical->type == ILogical::Type::tInteger) {
      if (value->type == VariableType::tInteger64) value->integerValue = value->integerValue64;
      else if (value->type == VariableType::tFloat) value->integerValue = (int32_t)value->floatValue;
      else if (value->type == VariableType::tBoolean) value->integerValue = value->booleanValue;

      PLogicalInteger logical = std::dynamic_pointer_cast<LogicalInteger>(rpcParameter->logical);
      if (logical) {
        if (logical->specialValuesIntegerMap.find(value->integerValue) == logical->specialValuesIntegerMap.end()) {
          if (value->integerValue > logical->maximumValue) value->integerValue = logical->maximumValue;
          else if (value->integerValue < logical->minimumValue) value->integerValue = logical->minimumValue;
        }
      }
    } else if (rpcParameter->logical->type == ILogical::Type::tInteger64) {
      if (value->type == VariableType::tInteger && value->integerValue64 == 0) value->integerValue64 = value->integerValue;
      else if (value->type == VariableType::tFloat) value->integerValue64 = (int64_t)value->floatValue;
      else if (value->type == VariableType::tBoolean) value->integerValue64 = value->booleanValue;

      PLogicalInteger64 logical = std::dynamic_pointer_cast<LogicalInteger64>(rpcParameter->logical);
      if (logical) {
        if (logical->specialValuesIntegerMap.find(value->integerValue64) == logical->specialValuesIntegerMap.end()) {
          if (value->integerValue64 > logical->maximumValue) value->integerValue64 = logical->maximumValue;
          else if (value->integerValue64 < logical->minimumValue) value->integerValue64 = logical->minimumValue;
        }
      }
    } else if (rpcParameter->logical->type == ILogical::Type::tFloat) {
      if (value->type == VariableType::tInteger) value->floatValue = value->integerValue;
      else if (value->type == VariableType::tInteger64) value->floatValue = value->integerValue64;
      else if (value->type == VariableType::tBoolean) value->floatValue = value->booleanValue;

      PLogicalDecimal logical = std::dynamic_pointer_cast<LogicalDecimal>(rpcParameter->logical);
      if (logical) {
        if (logical->specialValuesFloatMap.find(value->floatValue) == logical->specialValuesFloatMap.end()) {
          if (value->floatValue > logical->maximumValue) value->floatValue = logical->maximumValue;
          else if (value->floatValue < logical->minimumValue) value->floatValue = logical->minimumValue;
        }
      }
    } else if (rpcParameter->logical->type == ILogical::Type::tEnum) {
      int32_t enumValue = 0;
      if (value->type == VariableType::tInteger) enumValue = value->integerValue;
      else if (value->type == VariableType::tInteger64) enumValue = value->integerValue64;
      else if (value->type == VariableType::tFloat) enumValue = (int32_t)value->floatValue;
      else if (value->type == VariableType::tBoolean) enumValue = value->booleanValue;

      PLogicalEnumeration logical = std::dynamic_pointer_cast<LogicalEnumeration>(rpcParameter->logical);
      if (logical) {
        int32_t index = std::abs(logical->minimumValue) + enumValue;
        if (index < 0 || index >= (signed)logical->values.size() || !logical->values.at(index).indexDefined) return Variable::createError(-11, "Unknown enumeration index.");
      }
    }
    // }}}

    value->setType(rpcParameter->logical->type);

    //Perform operation on value
    if (value->stringValue.size() > 2 && value->stringValue.at(1) == '=' && (value->stringValue.at(0) == '+' || value->stringValue.at(0) == '-' || value->stringValue.at(0) == '*' || value->stringValue.at(0) == '/')) {
      std::vector<uint8_t> parameterData = parameter.getBinaryData();
      PVariable currentValue;
      if (!convertFromPacketHook(parameter, parameterData, currentValue)) currentValue = rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);

      std::string numberPart = value->stringValue.substr(2);
      double factor = Math::getDouble(numberPart);
      if (factor == 0) return Variable::createError(-1, "Factor is \"0\" or no valid number.");

      if (rpcParameter->logical->type == ILogical::Type::Enum::tFloat) {
        if (value->stringValue.at(0) == '+') value->floatValue = currentValue->floatValue + factor;
        else if (value->stringValue.at(0) == '-') value->floatValue = currentValue->floatValue - factor;
        else if (value->stringValue.at(0) == '*') value->floatValue = currentValue->floatValue * factor;
        else if (value->stringValue.at(0) == '/') value->floatValue = currentValue->floatValue / factor;
        value->type = VariableType::tFloat;
      } else if (rpcParameter->logical->type == ILogical::Type::Enum::tInteger) {
        if (value->stringValue.at(0) == '+') value->integerValue = currentValue->integerValue + factor;
        else if (value->stringValue.at(0) == '-') value->integerValue = currentValue->integerValue - factor;
        else if (value->stringValue.at(0) == '*') value->integerValue = currentValue->integerValue * factor;
        else if (value->stringValue.at(0) == '/') value->integerValue = currentValue->integerValue / factor;
        value->integerValue64 = value->integerValue;
        value->type = VariableType::tInteger;
      } else if (rpcParameter->logical->type == ILogical::Type::Enum::tInteger64) {
        if (value->stringValue.at(0) == '+') value->integerValue64 = currentValue->integerValue64 + factor;
        else if (value->stringValue.at(0) == '-') value->integerValue64 = currentValue->integerValue64 - factor;
        else if (value->stringValue.at(0) == '*') value->integerValue64 = currentValue->integerValue64 * factor;
        else if (value->stringValue.at(0) == '/') value->integerValue64 = currentValue->integerValue64 / factor;
        value->integerValue = value->integerValue64;
        value->type = VariableType::tInteger64;
      } else if (rpcParameter->logical->type == ILogical::Type::Enum::tBoolean) {
        if (value->stringValue.at(0) == '+') value->booleanValue = true;
        else if (value->stringValue.at(0) == '-') value->booleanValue = false;
        else if (value->stringValue.at(0) == '*') value->booleanValue = true;
        else if (value->stringValue.at(0) == '/') value->booleanValue = false;
        value->type = VariableType::tBoolean;
      }
      value->stringValue.clear();
    } else if (value->stringValue == "!") // Toggle boolean
    {
      std::vector<uint8_t> parameterData = parameter.getBinaryData();
      PVariable currentValue;
      if (!convertFromPacketHook(parameter, parameterData, currentValue)) currentValue = rpcParameter->convertFromPacket(parameterData, clientInfo->addon && clientInfo->peerId == _peerID ? Role() : parameter.mainRole(), false);
      if (rpcParameter->logical->type == ILogical::Type::Enum::tBoolean) {
        value->booleanValue = !currentValue->booleanValue;
        value->type = VariableType::tBoolean;
        value->stringValue.clear();
      } else if (rpcParameter->logical->type == ILogical::Type::Enum::tFloat) {
        value->floatValue = !currentValue->floatValue;
        value->type = VariableType::tFloat;
        value->stringValue.clear();
      } else if (rpcParameter->logical->type == ILogical::Type::Enum::tInteger) {
        value->integerValue = !currentValue->integerValue;
        value->type = VariableType::tInteger;
        value->stringValue.clear();
      } else if (rpcParameter->logical->type == ILogical::Type::Enum::tInteger64) {
        value->integerValue64 = !currentValue->integerValue64;
        value->type = VariableType::tInteger64;
        value->stringValue.clear();
      }
    }
    return std::make_shared<Variable>();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error. See error log for more details.");
}

//End RPC methods
}
}
