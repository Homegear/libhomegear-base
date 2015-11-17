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

#ifndef LOGICALDEVICE_H_
#define LOGICALDEVICE_H_

#include "../HelperFunctions/HelperFunctions.h"
#include "IPhysicalInterface.h"
#include "Packet.h"
#include "../Variable.h"
#include "../IEvents.h"
#include "Peer.h"

#include <memory>
#include <set>

using namespace BaseLib::DeviceDescription;

namespace BaseLib
{

class Obj;

namespace Systems
{

class Central;

class LogicalDevice : public Peer::IPeerEventSink, public IPhysicalInterface::IPhysicalInterfaceEventSink, public IEvents
{
public:
	//Event handling
	class IDeviceEventSink : public IEventSinkBase
	{
	public:
		//Hooks
		virtual void onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers) = 0;
		virtual void onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers) = 0;

		virtual void onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
		virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint) = 0;
		virtual void onRPCNewDevices(std::shared_ptr<Variable> deviceDescriptions) = 0;
		virtual void onRPCDeleteDevices(std::shared_ptr<Variable> deviceAddresses, std::shared_ptr<Variable> deviceInfo) = 0;
		virtual void onEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<BaseLib::Variable>>> values) = 0;
		virtual void onRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval) = 0;
		virtual int32_t onIsAddonClient(int32_t clientID) = 0;
	};
	//End event handling

	LogicalDevice(int32_t deviceFamily, BaseLib::Obj* baseLib, IDeviceEventSink* eventHandler);
	LogicalDevice(int32_t deviceFamily, BaseLib::Obj* baseLib, uint32_t deviceID, std::string serialNumber, int32_t address, IDeviceEventSink* eventHandler);
	virtual ~LogicalDevice();
	virtual void dispose(bool wait = true);

	virtual int32_t deviceFamily();

	virtual int32_t getAddress() { return _address; }
	virtual uint64_t getID() { return _deviceID; }
    virtual std::string getSerialNumber() { return _serialNumber; }
    virtual uint32_t getDeviceType() { return _deviceType; }
	virtual std::string handleCLICommand(std::string command) { return ""; }
	virtual std::shared_ptr<Central> getCentral() = 0;
	void getPeers(std::vector<std::shared_ptr<Peer>>& peers, std::shared_ptr<std::set<uint64_t>> knownDevices = std::shared_ptr<std::set<uint64_t>>());
	std::shared_ptr<Peer> getPeer(int32_t address);
    std::shared_ptr<Peer> getPeer(uint64_t id);
    std::shared_ptr<Peer> getPeer(std::string serialNumber);
	virtual bool peerSelected() { return (bool)_currentPeer; }
	virtual void setPeerID(uint64_t oldPeerID, uint64_t newPeerID);
	virtual void deletePeersFromDatabase();
	virtual void load();
	virtual void loadVariables() = 0;
	virtual void loadPeers() {}
	virtual void save(bool saveDevice);
	virtual void saveVariables() = 0;
	virtual void saveVariable(uint32_t index, int64_t intValue);
	virtual void saveVariable(uint32_t index, std::string& stringValue);
	virtual void saveVariable(uint32_t index, std::vector<uint8_t>& binaryValue);
	virtual void savePeers(bool full) = 0;

	virtual bool peerExists(int32_t address);
	virtual bool peerExists(std::string serialNumber);
	virtual bool peerExists(uint64_t id);

	/*
     * Executed when Homegear is fully started.
     */
    virtual void homegearStarted();

	/*
     * Executed before Homegear starts shutting down.
     */
    virtual void homegearShuttingDown();
protected:
	BaseLib::Obj* _bl = nullptr;
	int32_t _deviceFamily = -1;
	uint64_t _deviceID = 0;
	int32_t _address = 0;
    std::string _serialNumber;
    uint32_t _deviceType = 0;
    std::map<uint32_t, uint32_t> _variableDatabaseIDs;
    bool _initialized = false;
    bool _disposing = false;
	bool _disposed = false;

	std::shared_ptr<Central> _central;
	std::shared_ptr<Peer> _currentPeer;
    std::unordered_map<int32_t, std::shared_ptr<Peer>> _peers;
    std::unordered_map<std::string, std::shared_ptr<Peer>> _peersBySerial;
    std::map<uint64_t, std::shared_ptr<Peer>> _peersByID;
    std::mutex _peersMutex;

	//Event handling
    PEventHandler _physicalInterfaceEventhandler;

	//Hooks
	virtual void raiseAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers);
	virtual void raiseRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers);

	virtual void raiseRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
	virtual void raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
	virtual void raiseRPCNewDevices(std::shared_ptr<Variable> deviceDescriptions);
	virtual void raiseRPCDeleteDevices(std::shared_ptr<Variable> deviceAddresses, std::shared_ptr<Variable> deviceInfo);
	virtual void raiseEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<BaseLib::Variable>>> values);
	virtual void raiseRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval);
	virtual int32_t raiseIsAddonClient(int32_t clientID);
	//End event handling

	//Physical device event handling
	virtual bool onPacketReceived(std::string& senderID, std::shared_ptr<Packet> packet) = 0;
	//End physical device event handling

	// {{{ Peer event handling
		//Hooks
		virtual void onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers);
		virtual void onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers);

		virtual void onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
		virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
		virtual void onEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<BaseLib::Variable>>> values);
		virtual void onRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval);
		virtual int32_t onIsAddonClient(int32_t clientID);
	// }}}
};

}
}
#endif
