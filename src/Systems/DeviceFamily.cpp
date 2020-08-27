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

#include "DeviceFamily.h"
#include "../BaseLib.h"

namespace BaseLib {
namespace Systems {
std::shared_ptr<DeviceDescription::Devices> DeviceFamily::getRpcDevices() { return _rpcDevices; }
std::shared_ptr<ICentral> DeviceFamily::getCentral() { return _central; }
bool DeviceFamily::hasPhysicalInterface() { return true; }
std::shared_ptr<PhysicalInterfaces> DeviceFamily::physicalInterfaces() { return _physicalInterfaces; }

DeviceFamily::DeviceFamily(BaseLib::SharedObjects *bl, IFamilyEventSink *eventHandler, int32_t id, std::string name) : IDeviceFamily(bl, eventHandler, id, name, FamilyType::sharedObject) {
  _physicalInterfaces.reset(new PhysicalInterfaces(bl, id, std::map<std::string, PPhysicalInterfaceSettings>()));
  _rpcDevices.reset(new DeviceDescription::Devices(bl, this, id));
}

bool DeviceFamily::init() {
  _bl->out.printInfo("Loading XML RPC devices...");
  _rpcDevices->load();
  if (_rpcDevices->empty()) return false;
  return true;
}

bool DeviceFamily::lifetick() {
  if (_physicalInterfaces) return _physicalInterfaces->lifetick();
  return true;
}

void DeviceFamily::load() {
  try {
    std::shared_ptr<BaseLib::Database::DataTable> rows = _bl->db->getDevices((uint32_t)getFamily());
    for (BaseLib::Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row) {
      uint32_t deviceId = row->second.at(0)->intValue;
      _bl->out.printMessage("Loading device " + std::to_string(deviceId));
      int32_t address = row->second.at(1)->intValue;
      std::string serialNumber = row->second.at(2)->textValue;
      uint32_t deviceType = row->second.at(3)->intValue;

      if (deviceType == 0xFFFFFFFD) {
        _central = initializeCentral(deviceId, address, serialNumber);
        _central->load();
      }
    }
    if (!_central) {
      createCentral();
      _central->save(true);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void DeviceFamily::save(bool full) {
  try {
    _bl->out.printMessage("(Shutdown) => Saving devices");
    if (_central) {
      _bl->out.printMessage("(Shutdown) => Saving " + getName() + " central...");
      _central->save(full);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void DeviceFamily::dispose() {
  try {
    if (_disposed) return;
    _disposed = true;

    _physicalInterfaces->dispose();

    _bl->out.printDebug("Debug: Disposing central...");
    if (_central) _central->dispose(false);

    _physicalInterfaces.reset();
    _settings->dispose();
    _settings.reset();

    _central.reset();
    _rpcDevices.reset();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void DeviceFamily::homegearStarted() {
  try {
    if (_central) _central->homegearStarted();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void DeviceFamily::homegearShuttingDown() {
  try {
    if (_central) _central->homegearShuttingDown();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::string DeviceFamily::handleCliCommand(std::string &command) {
  try {
    std::ostringstream stringStream;
    if (!_central) return "Error: No central exists.\n";
    return _central->handleCliCommand(command);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return "Error executing command. See log file for more details.\n";
}

// {{{ RPC
std::shared_ptr<Variable> DeviceFamily::getParamsetDescription(PRpcClientInfo clientInfo, int32_t deviceId, int32_t firmwareVersion, int32_t channel, ParameterGroup::Type::Enum type) {
  try {
    if (_rpcDevices) return _rpcDevices->getParamsetDescription(clientInfo, deviceId, firmwareVersion, channel, type);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable DeviceFamily::listKnownDeviceTypes(PRpcClientInfo clientInfo, bool channels, std::set<std::string> &fields) {
  try {
    if (_rpcDevices) return _rpcDevices->listKnownDeviceTypes(clientInfo, channels, fields);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}
// }}}

}
}
