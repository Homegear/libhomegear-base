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

#include "IDeviceFamily.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Systems
{
FamilyType IDeviceFamily::type() { return _type; }
int32_t IDeviceFamily::getFamily(){ return _family; }
std::string IDeviceFamily::getName() { return _name; }

IDeviceFamily::IDeviceFamily(BaseLib::SharedObjects* bl, IFamilyEventSink* eventHandler, int32_t id, std::string name, FamilyType type)
{
    _bl = bl;
    _eventHandler = eventHandler;
    _family = id;
    _name = name;
    _type = type;
    if(_eventHandler) setEventHandler(_eventHandler);
    std::string filename = getName();
    HelperFunctions::toLower(filename);
    filename = _bl->settings.familyConfigPath() + HelperFunctions::stripNonAlphaNumeric(filename) + ".conf";
    _settings.reset(new FamilySettings(bl, id));
    _bl->out.printInfo("Info: Loading settings from " + filename);
    _settings->load(filename);
}

IDeviceFamily::~IDeviceFamily()
{
    dispose();
}

bool IDeviceFamily::enabled()
{
    std::string settingName = "moduleenabled";
    FamilySettings::PFamilySetting setting = _settings->get(settingName);
    if(!setting) return true;
    return setting->integerValue != 0;
}

FamilySettings::PFamilySetting IDeviceFamily::getFamilySetting(std::string& name)
{
    return _settings->get(name);
}

void IDeviceFamily::setFamilySetting(std::string& name, std::string& value)
{
    _settings->set(name, value);
}

void IDeviceFamily::setFamilySetting(std::string& name, int32_t value)
{
    _settings->set(name, value);
}

void IDeviceFamily::setFamilySetting(std::string& name, std::vector<char>& value)
{
    _settings->set(name, value);
}

void IDeviceFamily::deleteFamilySettingFromDatabase(std::string& name)
{
    _settings->deleteFromDatabase(name);
}

//Event handling
void IDeviceFamily::raiseAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers)
{
    if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onAddWebserverEventHandler(eventHandler, eventHandlers);
}

void IDeviceFamily::raiseRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers)
{
    if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRemoveWebserverEventHandler(eventHandlers);
}

void IDeviceFamily::raiseRPCEvent(std::string& source, uint64_t id, int32_t channel, std::string& deviceAddress, std::shared_ptr<std::vector<std::string>>& valueKeys, std::shared_ptr<std::vector<PVariable>>& values)
{
    if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCEvent(source, id, channel, deviceAddress, valueKeys, values);
}

void IDeviceFamily::raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
{
    if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCUpdateDevice(id, channel, address, hint);
}

void IDeviceFamily::raiseRPCNewDevices(std::vector<uint64_t>& ids, PVariable deviceDescriptions)
{
    if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCNewDevices(ids, deviceDescriptions);
}

void IDeviceFamily::raiseRPCDeleteDevices(std::vector<uint64_t>& ids, PVariable deviceAddresses, PVariable deviceInfo)
{
    if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCDeleteDevices(ids, deviceAddresses, deviceInfo);
}

void IDeviceFamily::raiseEvent(std::string& source, uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>>& variables, std::shared_ptr<std::vector<PVariable>>& values)
{
    if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onEvent(source, peerID, channel, variables, values);
}

void IDeviceFamily::raiseRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait)
{
    if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRunScript(scriptInfo, wait);
}

BaseLib::PVariable IDeviceFamily::raiseInvokeRpc(std::string& methodName, BaseLib::PArray& parameters)
{
    if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onInvokeRpc(methodName, parameters);
    else return std::make_shared<BaseLib::Variable>();
}

int32_t IDeviceFamily::raiseCheckLicense(int32_t moduleId, int32_t familyId, int32_t deviceId, const std::string& licenseKey)
{
    if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onCheckLicense(moduleId, familyId, deviceId, licenseKey);
    return -1;
}

uint64_t IDeviceFamily::raiseGetRoomIdByName(std::string& name)
{
    if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onGetRoomIdByName(name);
    return 0;
}

void IDeviceFamily::raiseDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output)
{
    if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onDecryptDeviceDescription(moduleId, input, output);
}
//End event handling

//Device event handling
void IDeviceFamily::onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers)
{
    raiseAddWebserverEventHandler(eventHandler, eventHandlers);
}

void IDeviceFamily::onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers)
{
    raiseRemoveWebserverEventHandler(eventHandlers);
}

void IDeviceFamily::onRPCEvent(std::string& source, uint64_t id, int32_t channel, std::string& deviceAddress, std::shared_ptr<std::vector<std::string>>& valueKeys, std::shared_ptr<std::vector<PVariable>>& values)
{
    raiseRPCEvent(source, id, channel, deviceAddress, valueKeys, values);
}

void IDeviceFamily::onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
{
    raiseRPCUpdateDevice(id, channel, address, hint);
}

void IDeviceFamily::onRPCNewDevices(std::vector<uint64_t>& ids, PVariable deviceDescriptions)
{
    raiseRPCNewDevices(ids, deviceDescriptions);
}

void IDeviceFamily::onRPCDeleteDevices(std::vector<uint64_t>& ids, PVariable deviceAddresses, PVariable deviceInfo)
{
    raiseRPCDeleteDevices(ids, deviceAddresses, deviceInfo);
}

void IDeviceFamily::onEvent(std::string& source, uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>>& variables, std::shared_ptr<std::vector<PVariable>>& values)
{
    raiseEvent(source, peerID, channel, variables, values);
}

void IDeviceFamily::onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait)
{
    raiseRunScript(scriptInfo, wait);
}

BaseLib::PVariable IDeviceFamily::onInvokeRpc(std::string& methodName, BaseLib::PArray& parameters)
{
    return raiseInvokeRpc(methodName, parameters);
}

void IDeviceFamily::onDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output)
{
    raiseDecryptDeviceDescription(moduleId, input, output);
}

uint64_t IDeviceFamily::onGetRoomIdByName(std::string& name)
{
    return raiseGetRoomIdByName(name);
}
//End Device event handling

void IDeviceFamily::lock()
{
    _locked = true;
}

void IDeviceFamily::unlock()
{
    _locked = false;
}

bool IDeviceFamily::locked()
{
    return _locked;
}

}
}
