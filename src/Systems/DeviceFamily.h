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

#ifndef DEVICEFAMILY_H_
#define DEVICEFAMILY_H_

#include "FamilySettings.h"
#include "../Database/DatabaseTypes.h"
#include "ICentral.h"
#include "PhysicalInterfaceSettings.h"
#include "IPhysicalInterface.h"
#include "../Variable.h"
#include "../DeviceDescription/Devices.h"

#include <iostream>
#include <string>
#include <memory>

#include "PhysicalInterfaces.h"

namespace BaseLib
{

class Obj;

namespace Systems
{
class DeviceFamily : public ICentral::ICentralEventSink, public DeviceDescription::Devices::IDevicesEventSink, public IEvents
{
public:
	//Event handling
	class IFamilyEventSink : public IEventSinkBase
	{
	public:
		//Hooks
		virtual void onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers) = 0;
		virtual void onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers) = 0;

		virtual void onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
		virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint) = 0;
		virtual void onRPCNewDevices(std::shared_ptr<Variable> deviceDescriptions) = 0;
		virtual void onRPCDeleteDevices(std::shared_ptr<Variable> deviceAddresses, std::shared_ptr<Variable> deviceInfo) = 0;
		virtual void onEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
		virtual void onRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval) = 0;
		virtual int32_t onIsAddonClient(int32_t clientID) = 0;
		virtual int32_t onCheckLicense(int32_t moduleId, int32_t familyId, int32_t deviceId, const std::string& licenseKey) = 0;

		//Device description
		virtual void onDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output) = 0;
	};
	//End event handling

	DeviceFamily(BaseLib::Obj* bl, IFamilyEventSink* eventHandler, int32_t id, std::string name);
	virtual ~DeviceFamily();

	virtual bool init();
	virtual void dispose();

	virtual bool lifetick();
	virtual void lock();
	virtual void unlock();
	virtual bool locked();

	virtual int32_t getFamily();
	virtual std::shared_ptr<DeviceDescription::Devices> getRpcDevices();
	virtual void load();
	virtual void save(bool full);
	virtual std::shared_ptr<ICentral> getCentral();
	virtual std::string getName();
	virtual std::string handleCliCommand(std::string& command);
	virtual bool peerSelected();
	virtual bool hasPhysicalInterface();
	virtual std::shared_ptr<PhysicalInterfaces> physicalInterfaces();

	/*
     * Executed when Homegear is fully started.
     */
    virtual void homegearStarted();

	/*
     * Executed before Homegear starts shutting down.
     */
    virtual void homegearShuttingDown();

    // {{{ RPC
    virtual std::shared_ptr<Variable> getPairingMethods() = 0;
    PVariable listKnownDeviceTypes(int32_t clientId, bool channels, std::map<std::string, bool>& fields);
    // }}}
protected:
	BaseLib::Obj* _bl = nullptr;
	std::shared_ptr<ICentral> _central;
	std::shared_ptr<FamilySettings> _settings;
	std::shared_ptr<PhysicalInterfaces> _physicalInterfaces;
	bool _locked = false;
	bool _disposed = false;

	// {{{ Event handling
		//Hooks
		virtual void raiseAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers);
		virtual void raiseRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers);

		virtual void raiseRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
		virtual void raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
		virtual void raiseRPCNewDevices(std::shared_ptr<Variable> deviceDescriptions);
		virtual void raiseRPCDeleteDevices(std::shared_ptr<Variable> deviceAddresses, std::shared_ptr<Variable> deviceInfo);
		virtual void raiseEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
		virtual void raiseRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval);
		virtual int32_t raiseIsAddonClient(int32_t clientID);
		virtual int32_t raiseCheckLicense(int32_t moduleId, int32_t familyId, int32_t deviceId, const std::string& licenseKey);

		// {{{ Device description event handling
			virtual void raiseDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output);
		// }}}
	// }}}

	// {{{ Device event handling
		//Hooks
		virtual void onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers);
		virtual void onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers);

		virtual void onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
		virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
		virtual void onRPCNewDevices(std::shared_ptr<Variable> deviceDescriptions);
		virtual void onRPCDeleteDevices(std::shared_ptr<Variable> deviceAddresses, std::shared_ptr<Variable> deviceInfo);
		virtual void onEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
		virtual void onRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval);
		virtual int32_t onIsAddonClient(int32_t clientID);
	// }}}

	// {{{ Device description event handling
		virtual void onDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output);
	// }}}
protected:
	virtual std::shared_ptr<ICentral> initializeCentral(uint32_t deviceId, int32_t address, std::string serialNumber) = 0;
	virtual void createCentral() = 0;
private:
	DeviceFamily(const DeviceFamily&);
	DeviceFamily& operator=(const DeviceFamily&);

	IFamilyEventSink* _eventHandler;
	int32_t _family = -1;
	std::string _name;
	std::shared_ptr<DeviceDescription::Devices> _rpcDevices;
};

}
}
#endif
