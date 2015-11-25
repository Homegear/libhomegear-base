/* Copyright 2013-2015 Sathya Laufer
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
DeviceFamily::DeviceFamily(BaseLib::Obj* bl, IFamilyEventSink* eventHandler, int32_t id, std::string name)
{
	_bl = bl;
	_eventHandler = eventHandler;
	_family = id;
	_name = name;
	_physicalInterfaces.reset(new PhysicalInterfaces(bl, id, std::vector<std::shared_ptr<PhysicalInterfaceSettings>>()));
	if(_eventHandler) setEventHandler(_eventHandler);
	std::string filename = getName();
	HelperFunctions::toLower(filename);
	filename = _bl->settings.familyConfigPath() + HelperFunctions::stripNonAlphaNumeric(filename) + ".conf";
	_settings.reset(new FamilySettings(bl));
	_bl->out.printInfo(filename);
	_settings->load(filename);
}

DeviceFamily::~DeviceFamily()
{
	dispose();
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

void DeviceFamily::raiseRPCNewDevices(PVariable deviceDescriptions)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCNewDevices(deviceDescriptions);
}

void DeviceFamily::raiseRPCDeleteDevices(PVariable deviceAddresses, PVariable deviceInfo)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRPCDeleteDevices(deviceAddresses, deviceInfo);
}

void DeviceFamily::raiseEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onEvent(peerID, channel, variables, values);
}

void DeviceFamily::raiseRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval)
{
	if(_eventHandler) ((IFamilyEventSink*)_eventHandler)->onRunScript(script, peerId, args, keepAlive, interval);
}

int32_t DeviceFamily::raiseIsAddonClient(int32_t clientID)
{
	if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onIsAddonClient(clientID);
	return -1;
}

int32_t DeviceFamily::raiseCheckLicense(int32_t moduleId, int32_t familyId, int32_t deviceId, const std::string& licenseKey)
{
	if(_eventHandler) return ((IFamilyEventSink*)_eventHandler)->onCheckLicense(moduleId, familyId, deviceId, licenseKey);
	return -1;
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

void DeviceFamily::onRPCNewDevices(PVariable deviceDescriptions)
{
	raiseRPCNewDevices(deviceDescriptions);
}

void DeviceFamily::onRPCDeleteDevices(PVariable deviceAddresses, PVariable deviceInfo)
{
	raiseRPCDeleteDevices(deviceAddresses, deviceInfo);
}

void DeviceFamily::onEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
{
	raiseEvent(peerID, channel, variables, values);
}

void DeviceFamily::onRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval)
{
	raiseRunScript(script, peerId, args, keepAlive, interval);
}

int32_t DeviceFamily::onIsAddonClient(int32_t clientID)
{
	return raiseIsAddonClient(clientID);
}

void DeviceFamily::onDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output)
{
	raiseDecryptDeviceDescription(moduleId, input, output);
}
//End Device event handling

void DeviceFamily::save(bool full)
{
	try
	{
		_bl->out.printMessage("(Shutdown) => Saving devices");
		if(_central)
		{
			_bl->out.printMessage("(Shutdown) => Saving " + getName() + " central...");
			_central->save(full);
			_central->savePeers(full);
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

}
}
