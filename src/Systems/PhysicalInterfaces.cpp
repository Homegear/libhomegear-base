/* Copyright 2013-2019 Homegear GmbH
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Homegear.  If not, see
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

#include "PhysicalInterfaces.h"
#include "IPhysicalInterface.h"
#include "../BaseLib.h"

namespace BaseLib {
namespace Systems {

PhysicalInterfaces::PhysicalInterfaces(BaseLib::SharedObjects *bl, int32_t familyId, std::map<std::string, PPhysicalInterfaceSettings> physicalInterfaceSettings) {
  try {
    _bl = bl;
    _familyId = familyId;
    _physicalInterfaceSettings = physicalInterfaceSettings;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

PhysicalInterfaces::~PhysicalInterfaces() {
}

void PhysicalInterfaces::dispose() {
  _physicalInterfacesMutex.lock();
  _physicalInterfaceSettings.clear();
  _physicalInterfaces.clear();
  _physicalInterfacesMutex.unlock();

  //Function pointers need to be cleaned up before unloading the module
  _rawPacketEvent = std::function<void(int32_t, const std::string &, const BaseLib::PVariable &)>();
}

bool PhysicalInterfaces::lifetick() {
  try {
    std::lock_guard<std::mutex> interfacesGuard(_physicalInterfacesMutex);
    for (std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j) {
      if (!j->second->lifetick()) return false;
    }
    return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

uint32_t PhysicalInterfaces::count() {
  try {
    std::lock_guard<std::mutex> interfacesGuard(_physicalInterfacesMutex);
    return _physicalInterfaces.size();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return 0;
}

bool PhysicalInterfaces::isOpen() {
  try {
    if (_physicalInterfaces.empty()) return true;
    std::lock_guard<std::mutex> interfacesGuard(_physicalInterfacesMutex);
    for (std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j) {
      if (!j->second->isNetworkDevice() && !j->second->isOpen()) {
        return false;
      }
    }
    return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void PhysicalInterfaces::startListening() {
  try {
    std::lock_guard<std::mutex> interfacesGuard(_physicalInterfacesMutex);
    for (std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j) {
      j->second->setRawPacketEvent(std::function<void(int32_t, const std::string &, const BaseLib::PVariable &)>(std::bind(&PhysicalInterfaces::rawPacketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
      j->second->startListening();
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void PhysicalInterfaces::stopListening() {
  try {
    std::lock_guard<std::mutex> interfacesGuard(_physicalInterfacesMutex);
    for (std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j) {
      j->second->stopListening();
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void PhysicalInterfaces::setup(int32_t userID, int32_t groupID, bool setPermissions) {
  try {
    std::lock_guard<std::mutex> interfacesGuard(_physicalInterfacesMutex);
    for (std::map<std::string, std::shared_ptr<IPhysicalInterface>>::iterator j = _physicalInterfaces.begin(); j != _physicalInterfaces.end(); ++j) {
      if (!j->second) {
        _bl->out.printCritical("Critical: Could not setup device: Device pointer was empty.");
        continue;
      }
      _bl->out.printDebug("Debug: Setting up physical device.");
      j->second->setup(userID, groupID, setPermissions);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

BaseLib::PVariable PhysicalInterfaces::listInterfaces() {
  try {
    BaseLib::PVariable array(new BaseLib::Variable(BaseLib::VariableType::tArray));

    std::lock_guard<std::mutex> interfacesGuard(_physicalInterfacesMutex);
    for (auto interface : _physicalInterfaces) {
      BaseLib::PVariable interfaceStruct(new BaseLib::Variable(BaseLib::VariableType::tStruct));

      interfaceStruct->structValue->insert(BaseLib::StructElement("FAMILYID", BaseLib::PVariable(new BaseLib::Variable(_familyId))));
      interfaceStruct->structValue->insert(BaseLib::StructElement("ID", BaseLib::PVariable(new BaseLib::Variable(interface.second->getID()))));
      interfaceStruct->structValue->insert(BaseLib::StructElement("PHYSICALADDRESS", BaseLib::PVariable(new BaseLib::Variable(interface.second->getAddress()))));
      interfaceStruct->structValue->insert(BaseLib::StructElement("TYPE", BaseLib::PVariable(new BaseLib::Variable(interface.second->getType()))));
      interfaceStruct->structValue->insert(BaseLib::StructElement("CONNECTED", BaseLib::PVariable(new BaseLib::Variable(interface.second->isOpen()))));
      interfaceStruct->structValue->insert(BaseLib::StructElement("DEFAULT", BaseLib::PVariable(new BaseLib::Variable(interface.second->isDefault()))));
      interfaceStruct->structValue->insert(BaseLib::StructElement("IP_ADDRESS", BaseLib::PVariable(new BaseLib::Variable(interface.second->getIpAddress()))));
      interfaceStruct->structValue->insert(BaseLib::StructElement("HOSTNAME", BaseLib::PVariable(new BaseLib::Variable(interface.second->getHostname()))));
      interfaceStruct->structValue->insert(BaseLib::StructElement("LASTPACKETSENT", BaseLib::PVariable(new BaseLib::Variable((uint32_t)(interface.second->lastPacketSent() / 1000)))));
      interfaceStruct->structValue->insert(BaseLib::StructElement("LASTPACKETRECEIVED", BaseLib::PVariable(new BaseLib::Variable((uint32_t)(interface.second->lastPacketReceived() / 1000)))));
      array->arrayValue->push_back(interfaceStruct);
    }
    return array;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return BaseLib::Variable::createError(-32500, "Unknown application error.");
}

void PhysicalInterfaces::rawPacketEvent(int32_t familyId, const std::string &interfaceId, const PVariable &packet) {
  if (_rawPacketEvent) _rawPacketEvent(familyId, interfaceId, packet);
}

}
}
