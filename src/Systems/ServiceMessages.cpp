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

#include "ServiceMessages.h"
#include "../BaseLib.h"

namespace BaseLib::Systems {

ServiceMessages::ServiceMessages(BaseLib::SharedObjects *baseLib, uint64_t peerId, std::string peerSerial, IServiceEventSink *eventHandler) {
  _bl = baseLib;
  _peerId = peerId;
  _peerSerial = peerSerial;
  setEventHandler(eventHandler);
  _configPendingSetTime = _bl->hf.getTime();
}

ServiceMessages::~ServiceMessages() {

}

//Event handling
void ServiceMessages::raiseConfigPending(bool configPending) {
  if (_eventHandler) ((IServiceEventSink *)_eventHandler)->onConfigPending(configPending);
}

void ServiceMessages::raiseEvent(std::string &source, uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> &variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> &values) {
  if (_eventHandler) ((IServiceEventSink *)_eventHandler)->onEvent(source, peerId, channel, variables, values);
}

void ServiceMessages::raiseRPCEvent(std::string &source, uint64_t peerId, int32_t channel, std::string &deviceAddress, std::shared_ptr<std::vector<std::string>> &valueKeys, std::shared_ptr<std::vector<PVariable>> &values) {
  if (_eventHandler) ((IServiceEventSink *)_eventHandler)->onRPCEvent(source, peerId, channel, deviceAddress, valueKeys, values);
}

void ServiceMessages::raiseServiceMessageEvent(const PServiceMessage &serviceMessage) {
  if (_eventHandler) ((IServiceEventSink *)_eventHandler)->onServiceMessageEvent(serviceMessage);
}

void ServiceMessages::raiseSaveParameter(std::string name, uint32_t channel, std::vector<uint8_t> &data) {
  if (_eventHandler) ((IServiceEventSink *)_eventHandler)->onSaveParameter(name, channel, data);
}

std::shared_ptr<Database::DataTable> ServiceMessages::raiseGetServiceMessages() {
  if (!_eventHandler) return std::shared_ptr<Database::DataTable>();
  return ((IServiceEventSink *)_eventHandler)->onGetServiceMessages();
}

void ServiceMessages::raiseSaveServiceMessage(Database::DataRow &data) {
  if (!_eventHandler) return;
  ((IServiceEventSink *)_eventHandler)->onSaveServiceMessage(data);
}

void ServiceMessages::raiseDeleteServiceMessage(uint64_t databaseID) {
  ((IServiceEventSink *)_eventHandler)->onDeleteServiceMessage(databaseID);
}

void ServiceMessages::raiseEnqueuePendingQueues() {
  if (_eventHandler) ((IServiceEventSink *)_eventHandler)->onEnqueuePendingQueues();
}
//End event handling

void ServiceMessages::load() {
  try {
    std::shared_ptr<BaseLib::Database::DataTable> rows = raiseGetServiceMessages();
    for (BaseLib::Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row) {
      _variableDatabaseIDs[row->second.at(3)->intValue] = row->second.at(0)->intValue;
      if (row->second.at(3)->intValue < 1000) {
        switch (row->second.at(3)->intValue) {
          /*case 0:
              _unreach = (bool)row->second.at(5)->intValue;
              break;*/
          case 1: _stickyUnreach = (bool)row->second.at(6)->intValue;
            _stickyUnreachTime = row->second.at(5)->intValue;
            break;
          case 2: _configPending = (bool)row->second.at(6)->intValue;
            _configPendingTime = row->second.at(5)->intValue;
            break;
          case 3: _lowbat = (bool)row->second.at(6)->intValue;
            _lowbatTime = row->second.at(5)->intValue;
            break;
        }
      } else {
        int32_t channel = row->second.at(6)->intValue;
        std::string id = row->second.at(7)->textValue;
        std::shared_ptr<std::vector<char>> value = row->second.at(9)->binaryValue;
        if (channel < 0 || id.empty() || value->empty()) continue;
        ErrorInfo errorInfo;
        errorInfo.value = (uint8_t)value->at(0);
        errorInfo.timestamp = row->second.at(5)->intValue;
        std::lock_guard<std::mutex> errorsGuard(_errorMutex);
        _errors[channel][id] = errorInfo;
      }
    }
    _unreach = false; //Always set _unreach to false on start up.

    //Synchronize service message data with peer parameters:
    std::vector<uint8_t> data = {(uint8_t)_unreach};
    raiseSaveParameter("UNREACH", 0, data);
    data = {(uint8_t)_stickyUnreach};
    raiseSaveParameter("STICKY_UNREACH", 0, data);
    data = {(uint8_t)_configPending};
    raiseSaveParameter("CONFIG_PENDING", 0, data);
    data = {(uint8_t)_lowbat};
    raiseSaveParameter("LOWBAT", 0, data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void ServiceMessages::save(ServiceMessagePriority priority, int64_t timestamp, uint32_t index, bool value) {
  try {
    bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
    Database::DataRow data;
    if (value || !idIsKnown) {
      if (idIsKnown) {
        data.push_back(std::make_shared<Database::DataColumn>(timestamp));
        data.push_back(std::make_shared<Database::DataColumn>((int32_t)value));
        data.push_back(std::make_shared<Database::DataColumn>(_variableDatabaseIDs[index]));
        raiseSaveServiceMessage(data);
      } else {
        if (_peerId == 0) return;
        data.push_back(std::make_shared<Database::DataColumn>(-1)); //familyID
        data.push_back(std::make_shared<Database::DataColumn>(_peerId)); //peerID
        data.push_back(std::make_shared<Database::DataColumn>(index)); //messageID
        data.push_back(std::make_shared<Database::DataColumn>(std::string())); //messageSubID
        data.push_back(std::make_shared<Database::DataColumn>(timestamp)); //timestamp
        data.push_back(std::make_shared<Database::DataColumn>((int32_t)value)); //integerValue
        data.push_back(std::make_shared<Database::DataColumn>()); //message
        data.push_back(std::make_shared<Database::DataColumn>()); //variables
        data.push_back(std::make_shared<Database::DataColumn>()); //binaryData
        data.push_back(std::make_shared<Database::DataColumn>((int32_t)priority)); //priority
        raiseSaveServiceMessage(data);
      }
    } else if (idIsKnown) {
      raiseDeleteServiceMessage(_variableDatabaseIDs[index]);
      _variableDatabaseIDs.erase(index);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void ServiceMessages::save(ServiceMessagePriority priority, int64_t timestamp, int32_t channel, std::string id, uint8_t value) {
  try {
    uint32_t index = 1000;
    for (std::string::iterator i = id.begin(); i != id.end(); ++i) {
      index += (int32_t)*i;
    }
    bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
    if (value > 0 || !idIsKnown) {
      std::vector<char> binaryValue{(char)value};
      Database::DataRow data;
      if (idIsKnown) {
        data.push_back(std::make_shared<Database::DataColumn>(timestamp));
        data.push_back(std::make_shared<Database::DataColumn>(channel));
        data.push_back(std::make_shared<Database::DataColumn>(id));
        data.push_back(std::make_shared<Database::DataColumn>(binaryValue));
        data.push_back(std::make_shared<Database::DataColumn>(_variableDatabaseIDs[index]));
        raiseSaveServiceMessage(data);
      } else {
        if (_peerId == 0) return;
        data.push_back(std::make_shared<Database::DataColumn>(-1)); //familyID
        data.push_back(std::make_shared<Database::DataColumn>(_peerId)); //peerID
        data.push_back(std::make_shared<Database::DataColumn>(index)); //messageID
        data.push_back(std::make_shared<Database::DataColumn>(std::string())); //messageSubID
        data.push_back(std::make_shared<Database::DataColumn>(timestamp)); //timestamp
        data.push_back(std::make_shared<Database::DataColumn>(channel)); //integerValue
        data.push_back(std::make_shared<Database::DataColumn>(id)); //message
        data.push_back(std::make_shared<Database::DataColumn>()); //variables
        data.push_back(std::make_shared<Database::DataColumn>(binaryValue)); //binaryData
        data.push_back(std::make_shared<Database::DataColumn>((int32_t)priority)); //priority
        raiseSaveServiceMessage(data);
      }
    } else if (idIsKnown) {
      raiseDeleteServiceMessage(_variableDatabaseIDs[index]);
      _variableDatabaseIDs.erase(index);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

bool ServiceMessages::set(std::string id, bool value) {
  try {
    if (_disposing) return false;

    auto serviceMessage = std::make_shared<ServiceMessage>();
    serviceMessage->type = ServiceMessageType::kDevice;
    serviceMessage->timestamp = HelperFunctions::getTimeSeconds();
    serviceMessage->peerId = _peerId;
    serviceMessage->channel = 0;
    serviceMessage->variable = id;
    serviceMessage->value = value;
    serviceMessage->priority = ServiceMessagePriority::kWarning;

    if (id == "LOWBAT_REPORTING") id = "LOWBAT"; //HM-TC-IT-WM-W-EU
    if (id == "UNREACH") {
      if (value != _unreach) {
        if (value && (_bl->booting || _bl->shuttingDown)) return true;
        _unreach = value;
        _unreachTime = HelperFunctions::getTimeSeconds();
        save(serviceMessage->priority, _unreachTime, 0, value);
      }
      serviceMessage->message = "l10n.common.serviceMessage.unreach";
    } else if (id == "STICKY_UNREACH") {
      if (value != _stickyUnreach) {
        if (value && (_bl->booting || _bl->shuttingDown)) return true;
        _stickyUnreach = value;
        _stickyUnreachTime = HelperFunctions::getTimeSeconds();
        save(serviceMessage->priority, _stickyUnreachTime, 1, value);
      }
      serviceMessage->message = "l10n.common.serviceMessage.stickyUnreach";
    } else if (id == "CONFIG_PENDING") {
      serviceMessage->priority = ServiceMessagePriority::kInfo;
      if (value != _configPending) {
        _configPending = value;
        _configPendingTime = HelperFunctions::getTimeSeconds();
        save(serviceMessage->priority, _configPendingTime, 2, value);
        if (_configPending) _configPendingSetTime = HelperFunctions::getTime();
      }
      serviceMessage->message = "l10n.common.serviceMessage.configPending";
    } else if (id == "LOWBAT") {
      if (value != _lowbat) {
        _lowbat = value;
        _lowbatTime = HelperFunctions::getTimeSeconds();
        save(serviceMessage->priority, _lowbatTime, 3, value);
      }
      serviceMessage->message = "l10n.common.serviceMessage.lowbat";
    } else //false == 0, a little dirty, but it works
    {
      if (!value) {
        std::lock_guard<std::mutex> errorGuard(_errorMutex);
        auto channelIterator = _errors.find(0);
        if (channelIterator != _errors.end()) {
          auto errorIterator = channelIterator->second.find(id);
          if (errorIterator != channelIterator->second.end()) {
            channelIterator->second.erase(errorIterator);
            if (channelIterator->second.empty()) _errors.erase(0);
          }
        }
      } else if (value > 0) {
        ErrorInfo errorInfo;
        errorInfo.value = value;
        errorInfo.timestamp = HelperFunctions::getTimeSeconds();
        std::lock_guard<std::mutex> errorsGuard(_errorMutex);
        _errors[0][id] = errorInfo;
      }

      std::vector<uint8_t> data = {(uint8_t)value};
      save(serviceMessage->priority, HelperFunctions::getTimeSeconds(), 0, id, value);
      raiseSaveParameter(id, 0, data);

      auto variableName = id;
      HelperFunctions::toLower(variableName);
      serviceMessage->message = "l10n.common.serviceMessage." + variableName;
      raiseServiceMessageEvent(serviceMessage);

      std::shared_ptr<std::vector<std::string>> valueKeys(new std::vector<std::string>({id}));
      std::shared_ptr<std::vector<PVariable>> rpcValues(new std::vector<PVariable>());
      rpcValues->push_back(std::make_shared<Variable>((int32_t)0));
      std::string eventSource = "device-" + std::to_string(_peerId);
      std::string address = _peerSerial + ":" + std::to_string(0);
      raiseEvent(eventSource, _peerId, 0, valueKeys, rpcValues);
      raiseRPCEvent(eventSource, _peerId, 0, address, valueKeys, rpcValues);

      if (!value) //Set for all other channels
      {
        std::lock_guard<std::mutex> errorsGuard(_errorMutex);
        for (auto &error : _errors) {
          if (error.first == 0) continue;
          auto errorIterator = error.second.find(id);
          if (errorIterator != error.second.end()) {
            errorIterator->second.value = 0;
            errorIterator->second.timestamp = HelperFunctions::getTimeSeconds();
            std::vector<uint8_t> data2 = {(uint8_t)value};
            save(serviceMessage->priority, errorIterator->second.timestamp, error.first, id, value);
            raiseSaveParameter(id, error.first, data2);

            auto serviceMessage2 = std::make_shared<ServiceMessage>();
            *serviceMessage2 = *serviceMessage;
            serviceMessage2->channel = error.first;
            raiseServiceMessageEvent(serviceMessage2);

            std::shared_ptr<std::vector<std::string>> valueKeys2(new std::vector<std::string>({id}));
            std::shared_ptr<std::vector<PVariable>> rpcValues2(new std::vector<PVariable>());
            rpcValues2->push_back(std::make_shared<Variable>((int32_t)0));
            std::string eventSource2 = "device-" + std::to_string(_peerId);
            std::string address2 = _peerSerial + ":" + std::to_string(error.first);
            raiseEvent(eventSource2, _peerId, 0, valueKeys2, rpcValues2);
            raiseRPCEvent(eventSource2, _peerId, 0, address2, valueKeys2, rpcValues2);
          }
        }
      }
      return true;
    }

    std::vector<uint8_t> data = {(uint8_t)value};
    raiseSaveParameter(id, 0, data);

    raiseServiceMessageEvent(serviceMessage);

    std::shared_ptr<std::vector<std::string>> valueKeys(new std::vector<std::string>({id}));
    std::shared_ptr<std::vector<PVariable>> rpcValues(new std::vector<PVariable>());
    rpcValues->push_back(std::make_shared<Variable>(value));

    std::string eventSource = "device-" + std::to_string(_peerId);
    std::string address = _peerSerial + ":" + std::to_string(0);
    raiseEvent(eventSource, _peerId, 0, valueKeys, rpcValues);
    raiseRPCEvent(eventSource, _peerId, 0, address, valueKeys, rpcValues);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return true;
}

void ServiceMessages::set(std::string id, uint8_t value, uint32_t channel) {
  try {
    if (_disposing) return;
    {
      std::lock_guard<std::mutex> errorGuard(_errorMutex);
      if (value == 0) {
        auto channelIterator = _errors.find(channel);
        if (channelIterator != _errors.end()) {
          auto errorIterator = channelIterator->second.find(id);
          if (errorIterator != channelIterator->second.end()) {
            channelIterator->second.erase(errorIterator);
            if (channelIterator->second.empty()) _errors.erase(channel);
          }
        }
      } else if (value > 0) {
        ErrorInfo errorInfo;
        errorInfo.value = value;
        errorInfo.timestamp = HelperFunctions::getTimeSeconds();
        _errors[channel][id] = errorInfo;
      }
    }
    save(ServiceMessagePriority::kWarning, HelperFunctions::getTimeSeconds(), channel, id, value);
    //RPC Broadcast is done in peer's packetReceived
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

PVariable ServiceMessages::get(const PRpcClientInfo &clientInfo, bool returnId, const std::string &language) {
  try {
    PVariable serviceMessages(new Variable(VariableType::tArray));
    if (returnId && _peerId == 0) return serviceMessages;
    if (_unreach) {
      if (returnId) {
        auto element = std::make_shared<Variable>(VariableType::tStruct);
        element->structValue->emplace("TYPE", std::make_shared<Variable>(2));
        element->structValue->emplace("PEER_ID", std::make_shared<Variable>(_peerId));
        element->structValue->emplace("CHANNEL", std::make_shared<Variable>(0));
        element->structValue->emplace("VARIABLE", std::make_shared<Variable>("UNREACH"));
        element->structValue->emplace("PRIORITY", std::make_shared<Variable>((int32_t)ServiceMessagePriority::kWarning));
        element->structValue->emplace("TIMESTAMP", std::make_shared<Variable>(_unreachTime));
        if (!language.empty()) element->structValue->emplace("MESSAGE", std::make_shared<Variable>(TranslationManager::getTranslation("l10n.common.serviceMessage.unreach", language)));
        else element->structValue->emplace("MESSAGE", TranslationManager::getTranslations("l10n.common.serviceMessage.unreach"));
        element->structValue->emplace("VALUE", std::make_shared<Variable>(true));
        serviceMessages->arrayValue->push_back(element);
      } else {
        auto element = std::make_shared<Variable>(VariableType::tArray);
        element->arrayValue->reserve(3);
        element->arrayValue->push_back(std::make_shared<Variable>(_peerSerial + ":0"));
        element->arrayValue->push_back(std::make_shared<Variable>(std::string("UNREACH")));
        element->arrayValue->push_back(std::make_shared<Variable>(true));
        serviceMessages->arrayValue->push_back(element);
      }
    }
    if (_stickyUnreach) {
      if (returnId) {
        auto element = std::make_shared<Variable>(VariableType::tStruct);
        element->structValue->emplace("TYPE", std::make_shared<Variable>(2));
        element->structValue->emplace("PEER_ID", std::make_shared<Variable>(_peerId));
        element->structValue->emplace("CHANNEL", std::make_shared<Variable>(0));
        element->structValue->emplace("VARIABLE", std::make_shared<Variable>("STICKY_UNREACH"));
        element->structValue->emplace("PRIORITY", std::make_shared<Variable>((int32_t)ServiceMessagePriority::kWarning));
        element->structValue->emplace("TIMESTAMP", std::make_shared<Variable>(_stickyUnreachTime));
        if (!language.empty()) element->structValue->emplace("MESSAGE", std::make_shared<Variable>(TranslationManager::getTranslation("l10n.common.serviceMessage.stickyUnreach", language)));
        else element->structValue->emplace("MESSAGE", TranslationManager::getTranslations("l10n.common.serviceMessage.stickyUnreach"));
        element->structValue->emplace("VALUE", std::make_shared<Variable>(true));
        serviceMessages->arrayValue->push_back(element);
      } else {
        auto element = std::make_shared<Variable>(VariableType::tArray);
        element->arrayValue->reserve(3);
        element->arrayValue->push_back(std::make_shared<Variable>(_peerSerial + ":0"));
        element->arrayValue->push_back(std::make_shared<Variable>(std::string("STICKY_UNREACH")));
        element->arrayValue->push_back(std::make_shared<Variable>(true));
        serviceMessages->arrayValue->push_back(element);
      }
    }
    if (_configPending) {
      if (returnId) {
        auto element = std::make_shared<Variable>(VariableType::tStruct);
        element->structValue->emplace("TYPE", std::make_shared<Variable>(2));
        element->structValue->emplace("PEER_ID", std::make_shared<Variable>(_peerId));
        element->structValue->emplace("CHANNEL", std::make_shared<Variable>(0));
        element->structValue->emplace("VARIABLE", std::make_shared<Variable>("CONFIG_PENDING"));
        element->structValue->emplace("PRIORITY", std::make_shared<Variable>((int32_t)ServiceMessagePriority::kInfo));
        element->structValue->emplace("TIMESTAMP", std::make_shared<Variable>(_configPendingTime));
        if (!language.empty()) element->structValue->emplace("MESSAGE", std::make_shared<Variable>(TranslationManager::getTranslation("l10n.common.serviceMessage.configPending", language)));
        else element->structValue->emplace("MESSAGE", TranslationManager::getTranslations("l10n.common.serviceMessage.configPending"));
        element->structValue->emplace("VALUE", std::make_shared<Variable>(true));
        serviceMessages->arrayValue->push_back(element);
      } else {
        auto element = std::make_shared<Variable>(VariableType::tArray);
        element->arrayValue->reserve(3);
        element->arrayValue->push_back(std::make_shared<Variable>(_peerSerial + ":0"));
        element->arrayValue->push_back(std::make_shared<Variable>(std::string("CONFIG_PENDING")));
        element->arrayValue->push_back(std::make_shared<Variable>(true));
        serviceMessages->arrayValue->push_back(element);
      }
    }
    if (_lowbat) {
      if (returnId) {
        auto element = std::make_shared<Variable>(VariableType::tStruct);
        element->structValue->emplace("TYPE", std::make_shared<Variable>(2));
        element->structValue->emplace("PEER_ID", std::make_shared<Variable>(_peerId));
        element->structValue->emplace("CHANNEL", std::make_shared<Variable>(0));
        element->structValue->emplace("VARIABLE", std::make_shared<Variable>("LOWBAT"));
        element->structValue->emplace("PRIORITY", std::make_shared<Variable>((int32_t)ServiceMessagePriority::kWarning));
        element->structValue->emplace("TIMESTAMP", std::make_shared<Variable>(_lowbatTime));
        if (!language.empty()) element->structValue->emplace("MESSAGE", std::make_shared<Variable>(TranslationManager::getTranslation("l10n.common.serviceMessage.lowbat", language)));
        else element->structValue->emplace("MESSAGE", TranslationManager::getTranslations("l10n.common.serviceMessage.lowbat"));
        element->structValue->emplace("VALUE", std::make_shared<Variable>(true));
        serviceMessages->arrayValue->push_back(element);
      } else {
        auto element = std::make_shared<Variable>(VariableType::tArray);
        element->arrayValue->reserve(3);
        element->arrayValue->push_back(std::make_shared<Variable>(_peerSerial + ":0"));
        element->arrayValue->push_back(std::make_shared<Variable>(std::string("LOWBAT")));
        element->arrayValue->push_back(std::make_shared<Variable>(true));
        serviceMessages->arrayValue->push_back(element);
      }
    }
    std::lock_guard<std::mutex> errorGuard(_errorMutex);
    for (auto &error : _errors) {
      for (auto &inner : error.second) {
        if (inner.second.value == 0) continue;

        if (returnId) {
          auto element = std::make_shared<Variable>(VariableType::tStruct);
          element->structValue->emplace("TYPE", std::make_shared<Variable>(2));
          element->structValue->emplace("PEER_ID", std::make_shared<Variable>(_peerId));
          element->structValue->emplace("CHANNEL", std::make_shared<Variable>(error.first));
          element->structValue->emplace("VARIABLE", std::make_shared<Variable>(inner.first));
          element->structValue->emplace("PRIORITY", std::make_shared<Variable>((int32_t)ServiceMessagePriority::kWarning));
          element->structValue->emplace("TIMESTAMP", std::make_shared<Variable>(inner.second.timestamp));
          auto variableName = inner.first;
          if (!language.empty()) element->structValue->emplace("MESSAGE", std::make_shared<Variable>(TranslationManager::getTranslation("l10n.common.serviceMessage." + HelperFunctions::toLower(variableName), language)));
          else element->structValue->emplace("MESSAGE", TranslationManager::getTranslations("l10n.common.serviceMessage." + HelperFunctions::toLower(variableName)));
          element->structValue->emplace("VALUE", std::make_shared<Variable>((uint32_t)inner.second.value));
          serviceMessages->arrayValue->push_back(element);
        } else {
          auto element = std::make_shared<Variable>(VariableType::tArray);
          element->arrayValue->reserve(3);
          element->arrayValue->push_back(std::make_shared<Variable>(_peerSerial + ":" + std::to_string(error.first)));
          element->arrayValue->push_back(std::make_shared<Variable>(inner.first));
          element->arrayValue->push_back(std::make_shared<Variable>((uint32_t)inner.second.value));
          serviceMessages->arrayValue->push_back(element);
        }
      }
    }
    return serviceMessages;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

void ServiceMessages::checkUnreach(int32_t cyclicTimeout, int64_t lastPacketReceived) {
  try {
    if (_bl->booting || _bl->shuttingDown) return;
    int64_t time = HelperFunctions::getTimeSeconds();
    if (cyclicTimeout > 0 && (time - lastPacketReceived) > cyclicTimeout) {
      if (!_unreach) {
        _unreach = true;
        _stickyUnreach = true;

        _bl->out.printInfo("Info: Peer " + std::to_string(_peerId) + " is set to unreachable, because no packet was received within " + std::to_string(cyclicTimeout) + " seconds. The Last packet was received at "
                               + BaseLib::HelperFunctions::getTimeString(lastPacketReceived * 1000));

        std::vector<uint8_t> data = {1};
        raiseSaveParameter("UNREACH", 0, data);
        raiseSaveParameter("STICKY_UNREACH", 0, data);

        std::shared_ptr<std::vector<std::string>> valueKeys(new std::vector<std::string>({std::string("UNREACH"), std::string("STICKY_UNREACH")}));
        std::shared_ptr<std::vector<PVariable>> rpcValues(new std::vector<PVariable>());
        rpcValues->push_back(std::make_shared<Variable>(true));
        rpcValues->push_back(std::make_shared<Variable>(true));

        std::string eventSource = "device-" + std::to_string(_peerId);
        std::string address = _peerSerial + ":" + std::to_string(0);
        raiseEvent(eventSource, _peerId, 0, valueKeys, rpcValues);
        raiseRPCEvent(eventSource, _peerId, 0, address, valueKeys, rpcValues);
      }
    } else if (_unreach) endUnreach();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void ServiceMessages::endUnreach() {
  try {
    if (_unreach) {
      _unreach = false;
      _unreachResendCounter = 0;

      _bl->out.printInfo("Info: Peer " + std::to_string(_peerId) + " is reachable again.");

      std::vector<uint8_t> data = {0};
      raiseSaveParameter("UNREACH", 0, data);

      std::shared_ptr<std::vector<std::string>> valueKeys(new std::vector<std::string>({std::string("UNREACH")}));
      std::shared_ptr<std::vector<PVariable>> rpcValues(new std::vector<PVariable>());
      rpcValues->push_back(std::make_shared<Variable>(false));

      std::string eventSource = "device-" + std::to_string(_peerId);
      std::string address = _peerSerial + ":" + std::to_string(0);
      raiseEvent(eventSource, _peerId, 0, valueKeys, rpcValues);
      raiseRPCEvent(eventSource, _peerId, 0, address, valueKeys, rpcValues);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void ServiceMessages::resetConfigPendingSetTime() {
  try {
    _configPendingSetTime = HelperFunctions::getTime();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void ServiceMessages::setConfigPending(bool value) {
  try {
    if (value != _configPending) {
      _configPending = value;
      _configPendingTime = BaseLib::HelperFunctions::getTimeSeconds();
      save(ServiceMessagePriority::kInfo, _configPendingTime, 2, value);
      if (_configPending) _configPendingSetTime = BaseLib::HelperFunctions::getTime();
      std::vector<uint8_t> data = {(uint8_t)value};
      raiseSaveParameter("CONFIG_PENDING", 0, data);

      std::shared_ptr<std::vector<std::string>> valueKeys(new std::vector<std::string>({std::string("CONFIG_PENDING")}));
      std::shared_ptr<std::vector<PVariable>> rpcValues(new std::vector<PVariable>());
      rpcValues->push_back(std::make_shared<Variable>(value));

      std::string eventSource = "device-" + std::to_string(_peerId);
      std::string address = _peerSerial + ":" + std::to_string(0);
      raiseEvent(eventSource, _peerId, 0, valueKeys, rpcValues);
      raiseRPCEvent(eventSource, _peerId, 0, address, valueKeys, rpcValues);
      raiseConfigPending(value);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void ServiceMessages::setUnreach(bool value, bool requeue) {
  try {
    if (_disposing || (value && (_bl->booting || _bl->shuttingDown))) return;
    if (value != _unreach) {
      if (value && requeue && _unreachResendCounter < 3) {
        raiseEnqueuePendingQueues();
        _unreachResendCounter++;
        return;
      }
      _unreachResendCounter = 0;
      _unreach = value;
      _unreachTime = HelperFunctions::getTimeSeconds();
      save(ServiceMessagePriority::kWarning, _unreachTime, 0, value);

      if (value) _bl->out.printInfo("Info: Peer " + std::to_string(_peerId) + " is unreachable.");
      std::vector<uint8_t> data = {(uint8_t)value};
      raiseSaveParameter("UNREACH", 0, data);

      std::shared_ptr<std::vector<std::string>> valueKeys(new std::vector<std::string>({std::string("UNREACH")}));
      std::shared_ptr<std::vector<PVariable>> rpcValues(new std::vector<PVariable>{std::make_shared<Variable>(value)});

      if (value) {
        _stickyUnreach = value;
        _stickyUnreachTime = HelperFunctions::getTimeSeconds();
        save(ServiceMessagePriority::kWarning, _stickyUnreachTime, 1, value);
        raiseSaveParameter("STICKY_UNREACH", 0, data);

        valueKeys->push_back("STICKY_UNREACH");
        rpcValues->push_back(std::make_shared<Variable>(true));
      }

      std::string eventSource = "device-" + std::to_string(_peerId);
      std::string address = _peerSerial + ":" + std::to_string(0);
      raiseEvent(eventSource, _peerId, 0, valueKeys, rpcValues);
      raiseRPCEvent(eventSource, _peerId, 0, address, valueKeys, rpcValues);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}
