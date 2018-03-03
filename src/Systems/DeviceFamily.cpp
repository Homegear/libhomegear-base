/* Copyright 2013-2017 Sathya Laufer
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

namespace BaseLib
{
namespace Systems
{
int32_t DeviceFamily::getFamily(){ return _family; }
std::shared_ptr<DeviceDescription::Devices> DeviceFamily::getRpcDevices() { return _rpcDevices; }
std::shared_ptr<ICentral> DeviceFamily::getCentral() { return _central; }
std::string DeviceFamily::getName() { return _name; }
bool DeviceFamily::peerSelected() { if(!_central) return false; return _central->peerSelected(); }
bool DeviceFamily::hasPhysicalInterface() { return true; }
std::shared_ptr<PhysicalInterfaces> DeviceFamily::physicalInterfaces() { return _physicalInterfaces; }

DeviceFamily::DeviceFamily(BaseLib::SharedObjects* bl, IFamilyEventSink* eventHandler, int32_t id, std::string name)
{
	_locked = false;

	_bl = bl;
	_eventHandler = eventHandler;
	_family = id;
	_name = name;
	_physicalInterfaces.reset(new PhysicalInterfaces(bl, id, std::map<std::string, PPhysicalInterfaceSettings>()));
	if(_eventHandler) setEventHandler(_eventHandler);
	std::string filename = getName();
	HelperFunctions::toLower(filename);
	filename = _bl->settings.familyConfigPath() + HelperFunctions::stripNonAlphaNumeric(filename) + ".conf";
	_settings.reset(new FamilySettings(bl, id));
	_bl->out.printInfo(filename);
	_settings->load(filename);
	_rpcDevices.reset(new DeviceDescription::Devices(bl, this, id));
}

DeviceFamily::~DeviceFamily()
{
	dispose();
}

bool DeviceFamily::init()
{
	_bl->out.printInfo("Loading XML RPC devices...");
	_rpcDevices->load();
	if(_rpcDevices->empty()) return false;
	return true;
}

bool DeviceFamily::enabled()
{
	std::string settingName = "moduleenabled";
	FamilySettings::PFamilySetting setting = _settings->get(settingName);
	if(!setting) return true;
	return setting->integerValue != 0;
}

bool DeviceFamily::lifetick()
{
	if(_physicalInterfaces) return _physicalInterfaces->lifetick();
	return true;
}

FamilySettings::PFamilySetting DeviceFamily::getFamilySetting(std::string& name)
{
	return _settings->get(name);
}

void DeviceFamily::setFamilySetting(std::string& name, std::string& value)
{
	_settings->set(name, value);
}

void DeviceFamily::setFamilySetting(std::string& name, int32_t value)
{
	_settings->set(name, value);
}

void DeviceFamily::setFamilySetting(std::string& name, std::vector<char>& value)
{
	_settings->set(name, value);
}

void DeviceFamily::deleteFamilySettingFromDatabase(std::string& name)
{
	_settings->deleteFromDatabase(name);
}

//Event handling
void DeviceFamily::raiseAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onAddWebserverEventHandler(eventHandler, eventHandlers);
}

void DeviceFamily::raiseRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRemoveWebserverEventHandler(eventHandlers);
}

void DeviceFamily::raiseRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCEvent(id, channel, deviceAddress, valueKeys, values);
}

void DeviceFamily::raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCUpdateDevice(id, channel, address, hint);
}

void DeviceFamily::raiseRPCNewDevices(std::vector<uint64_t>& ids, PVariable deviceDescriptions)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCNewDevices(ids, deviceDescriptions);
}

void DeviceFamily::raiseRPCDeleteDevices(std::vector<uint64_t>& ids, PVariable deviceAddresses, PVariable deviceInfo)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCDeleteDevices(ids, deviceAddresses, deviceInfo);
}

void DeviceFamily::raiseEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onEvent(peerID, channel, variables, values);
}

void DeviceFamily::raiseRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRunScript(scriptInfo, wait);
}

BaseLib::PVariable DeviceFamily::raiseInvokeRpc(std::string& methodName, BaseLib::PArray& parameters)
{
	if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onInvokeRpc(methodName, parameters);
	else return std::make_shared<BaseLib::Variable>();
}

int32_t DeviceFamily::raiseCheckLicense(int32_t moduleId, int32_t familyId, int32_t deviceId, const std::string& licenseKey)
{
	if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onCheckLicense(moduleId, familyId, deviceId, licenseKey);
	return -1;
}

uint64_t DeviceFamily::raiseGetRoomIdByName(std::string& name)
{
	if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onGetRoomIdByName(name);
	return 0;
}

void DeviceFamily::raiseDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output)
{
	if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onDecryptDeviceDescription(moduleId, input, output);
}
//End event handling

//Device event handling
void DeviceFamily::onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers)
{
	raiseAddWebserverEventHandler(eventHandler, eventHandlers);
}

void DeviceFamily::onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers)
{
	raiseRemoveWebserverEventHandler(eventHandlers);
}

void DeviceFamily::onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values)
{
	raiseRPCEvent(id, channel, deviceAddress, valueKeys, values);
}

void DeviceFamily::onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
{
	raiseRPCUpdateDevice(id, channel, address, hint);
}

void DeviceFamily::onRPCNewDevices(std::vector<uint64_t>& ids, PVariable deviceDescriptions)
{
	raiseRPCNewDevices(ids, deviceDescriptions);
}

void DeviceFamily::onRPCDeleteDevices(std::vector<uint64_t>& ids, PVariable deviceAddresses, PVariable deviceInfo)
{
	raiseRPCDeleteDevices(ids, deviceAddresses, deviceInfo);
}

void DeviceFamily::onEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
{
	raiseEvent(peerID, channel, variables, values);
}

void DeviceFamily::onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait)
{
	raiseRunScript(scriptInfo, wait);
}

BaseLib::PVariable DeviceFamily::onInvokeRpc(std::string& methodName, BaseLib::PArray& parameters)
{
    return raiseInvokeRpc(methodName, parameters);
}

void DeviceFamily::onDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output)
{
	raiseDecryptDeviceDescription(moduleId, input, output);
}

uint64_t DeviceFamily::onGetRoomIdByName(std::string& name)
{
	return raiseGetRoomIdByName(name);
}
//End Device event handling

void DeviceFamily::load()
{
	try
	{
		std::shared_ptr<BaseLib::Database::DataTable> rows = _bl->db->getDevices((uint32_t)getFamily());
		for(BaseLib::Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row)
		{
			uint32_t deviceId = row->second.at(0)->intValue;
			_bl->out.printMessage("Loading device " + std::to_string(deviceId));
			int32_t address = row->second.at(1)->intValue;
			std::string serialNumber = row->second.at(2)->textValue;
			uint32_t deviceType = row->second.at(3)->intValue;

			if(deviceType == 0xFFFFFFFD)
			{
				_central = initializeCentral(deviceId, address, serialNumber);
				_central->load();
			}
		}
		if(!_central)
		{
			createCentral();
			_central->save(true);
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void DeviceFamily::save(bool full)
{
	try
	{
		_bl->out.printMessage("(Shutdown) => Saving devices");
		if(_central)
		{
			_bl->out.printMessage("(Shutdown) => Saving " + getName() + " central...");
			_central->save(full);
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void DeviceFamily::dispose()
{
	try
	{
		if(_disposed) return;
		_disposed = true;

		_physicalInterfaces->dispose();

		_bl->out.printDebug("Debug: Disposing central...");
		if(_central) _central->dispose(false);

		_physicalInterfaces.reset();
		_settings->dispose();
		_settings.reset();

		_central.reset();
		_rpcDevices.reset();
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void DeviceFamily::lock()
{
	_locked = true;
}

void DeviceFamily::unlock()
{
	_locked = false;
}

bool DeviceFamily::locked()
{
	return _locked;
}

void DeviceFamily::homegearStarted()
{
	try
	{
		if(_central) _central->homegearStarted();
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void DeviceFamily::homegearShuttingDown()
{
	try
	{
		if(_central) _central->homegearShuttingDown();
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

std::string DeviceFamily::handleCliCommand(std::string& command)
{
	try
	{
		std::ostringstream stringStream;
		if(!_central) return "Error: No central exists.\n";
		return _central->handleCliCommand(command);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return "Error executing command. See log file for more details.\n";
}

// {{{ RPC
std::shared_ptr<Variable> DeviceFamily::getParamsetDescription(PRpcClientInfo clientInfo, int32_t deviceId, int32_t firmwareVersion, int32_t channel, ParameterGroup::Type::Enum type)
{
	try
	{
		if(_rpcDevices) return _rpcDevices->getParamsetDescription(clientInfo, deviceId, firmwareVersion, channel, type);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable DeviceFamily::listKnownDeviceTypes(PRpcClientInfo clientInfo, bool channels, std::set<std::string>& fields)
{
	try
	{
		if(_rpcDevices) return _rpcDevices->listKnownDeviceTypes(clientInfo, channels, fields);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return Variable::createError(-32500, "Unknown application error.");
}
// }}}

}
}
