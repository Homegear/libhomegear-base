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

#ifndef ICENTRAL_H_
#define ICENTRAL_H_

#include "../Variable.h"
#include "../IEvents.h"
#include "../DeviceDescription/HomegearDevice.h"
#include "IPhysicalInterface.h"
#include "Peer.h"

#include <set>

using namespace BaseLib::DeviceDescription;

namespace BaseLib
{

namespace Systems
{

class ICentral : public Peer::IPeerEventSink, public IPhysicalInterface::IPhysicalInterfaceEventSink, public IEvents
{
public:
	//Event handling
	class ICentralEventSink : public IEventSinkBase
	{
	public:
		//Hooks
		virtual void onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers) = 0;
		virtual void onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers) = 0;

		virtual void onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
		virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint) = 0;
		virtual void onRPCNewDevices(std::shared_ptr<Variable> deviceDescriptions) = 0;
		virtual void onRPCDeleteDevices(std::shared_ptr<Variable> deviceAddresses, std::shared_ptr<Variable> deviceInfo) = 0;
		virtual void onEvent(uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<BaseLib::Variable>>> values) = 0;
		virtual void onRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval) = 0;
		virtual int32_t onIsAddonClient(int32_t clientId) = 0;
	};
	//End event handling

	ICentral(int32_t deviceFamily, BaseLib::Obj* baseLib, ICentralEventSink* eventHandler);
	ICentral(int32_t deviceFamily, BaseLib::Obj* baseLib, uint32_t deviceId, std::string serialNumber, int32_t address, ICentralEventSink* eventHandler);
	virtual ~ICentral();
	virtual void dispose(bool wait = true);

	virtual void load();
	virtual void save(bool saveDevice);

	/*
     * Executed when Homegear is fully started.
     */
    virtual void homegearStarted();

	/*
     * Executed before Homegear starts shutting down.
     */
    virtual void homegearShuttingDown();

	virtual int32_t deviceFamily();

	virtual int32_t getAddress() { return _address; }
	virtual uint64_t getId() { return _deviceId; }
    virtual std::string getSerialNumber() { return _serialNumber; }
	virtual std::string handleCliCommand(std::string command) { return ""; }
	void getPeers(std::vector<std::shared_ptr<Peer>>& peers, std::shared_ptr<std::set<uint64_t>> knownDevices = std::shared_ptr<std::set<uint64_t>>());
	std::shared_ptr<Peer> getPeer(int32_t address);
    std::shared_ptr<Peer> getPeer(uint64_t id);
    std::shared_ptr<Peer> getPeer(std::string serialNumber);
	virtual bool peerSelected();

	virtual bool peerExists(int32_t address);
	virtual bool peerExists(std::string serialNumber);
	virtual bool peerExists(uint64_t id);
	virtual uint64_t getPeerIdFromSerial(std::string serialNumber) { return 0; }

	virtual std::shared_ptr<Variable> activateLinkParamset(int32_t clientId, std::string serialNumber, int32_t channel, std::string remoteSerialNumber, int32_t remoteChannel, bool longPress) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> activateLinkParamset(int32_t clientId, uint64_t peerId, int32_t channel, uint64_t remoteId, int32_t remoteChannel, bool longPress) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> addDevice(int32_t clientId, std::string serialNumber) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> addLink(int32_t clientId, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel, std::string name, std::string description) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> addLink(int32_t clientId, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel, std::string name, std::string description) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> createDevice(int32_t clientId, int32_t deviceType, std::string serialNumber, int32_t address, int32_t firmwareVersion) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> deleteDevice(int32_t clientId, std::string serialNumber, int32_t flags) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> deleteDevice(int32_t clientId, uint64_t peerId, int32_t flags) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> getAllValues(int32_t clientId, uint64_t peerId, bool returnWriteOnly);
	virtual std::shared_ptr<Variable> getDeviceDescription(int32_t clientId, std::string serialNumber, int32_t channel);
	virtual std::shared_ptr<Variable> getDeviceDescription(int32_t clientId, uint64_t id, int32_t channel);
	virtual std::shared_ptr<Variable> getDeviceInfo(int32_t clientId, uint64_t id, std::map<std::string, bool> fields) = 0;
	virtual std::shared_ptr<Variable> getPeerId(int32_t clientId, int32_t filterType, std::string filterValue);
	virtual std::shared_ptr<Variable> getPeerId(int32_t clientId, int32_t address);
	virtual std::shared_ptr<Variable> getPeerId(int32_t clientId, std::string serialNumber);
	virtual std::shared_ptr<Variable> getInstallMode(int32_t clientId) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> getLinkInfo(int32_t clientId, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel);
	virtual std::shared_ptr<Variable> getLinkInfo(int32_t clientId, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel);
	virtual std::shared_ptr<Variable> getLinkPeers(int32_t clientId, std::string serialNumber, int32_t channel);
	virtual std::shared_ptr<Variable> getLinkPeers(int32_t clientId, uint64_t peerId, int32_t channel);
	virtual std::shared_ptr<Variable> getLinks(int32_t clientId, std::string serialNumber, int32_t channel, int32_t flags);
	virtual std::shared_ptr<Variable> getLinks(int32_t clientId, uint64_t peerId, int32_t channel, int32_t flags);
	virtual std::shared_ptr<Variable> getName(int32_t clientId, uint64_t id);
	virtual std::shared_ptr<Variable> getParamsetDescription(int32_t clientId, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel);
	virtual std::shared_ptr<Variable> getParamsetDescription(int32_t clientId, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel);
	virtual std::shared_ptr<Variable> getParamsetId(int32_t clientId, std::string serialNumber, uint32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel);
	virtual std::shared_ptr<Variable> getParamsetId(int32_t clientId, uint64_t peerId, uint32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel);
	virtual std::shared_ptr<Variable> getParamset(int32_t clientId, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel);
	virtual std::shared_ptr<Variable> getParamset(int32_t clientId, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel);
	virtual std::shared_ptr<Variable> getServiceMessages(int32_t clientId, bool returnId);
	virtual std::shared_ptr<Variable> getValue(int32_t clientId, std::string serialNumber, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous);
	virtual std::shared_ptr<Variable> getValue(int32_t clientId, uint64_t id, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous);
	virtual std::shared_ptr<Variable> listDevices(int32_t clientId, bool channels, std::map<std::string, bool> fields);
	virtual std::shared_ptr<Variable> listKnownDeviceTypes(int32_t clientId, bool channels, std::map<std::string, bool> fields);
	virtual std::shared_ptr<Variable> listDevices(int32_t clientId, bool channels, std::map<std::string, bool> fields, std::shared_ptr<std::set<uint64_t>> knownDevices);
	virtual std::shared_ptr<Variable> listTeams(int32_t clientId) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> putParamset(int32_t clientId, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel, std::shared_ptr<Variable> paramset) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> putParamset(int32_t clientId, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel, std::shared_ptr<Variable> paramset) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> reportValueUsage(int32_t clientId, std::string serialNumber);
	virtual std::shared_ptr<Variable> removeLink(int32_t clientId, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> removeLink(int32_t clientId, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> rssiInfo(int32_t clientId);
	virtual std::shared_ptr<Variable> searchDevices(int32_t clientId) { return Variable::createError(-32601, "Method not implemented for this central."); }

	/**
     * RPC function to change the Id of the peer.
     *
     * @param oldPeerId The current ID of the peer.
     * @param newPeerId The new ID of the peer.
     * @return Returns "RPC void" on success or RPC error "-100" when the new peer ID is invalid and error "-101" when the new peer ID is already in use.
     */
    virtual std::shared_ptr<BaseLib::Variable> setId(int32_t clientId, uint64_t oldPeerId, uint64_t newPeerId);
	virtual std::shared_ptr<Variable> setInstallMode(int32_t clientId, bool on, uint32_t duration = 60, bool debugOutput = true) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> setInterface(int32_t clientId, uint64_t peerId, std::string interfaceId) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> setLinkInfo(int32_t clientId, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel, std::string name, std::string description);
	virtual std::shared_ptr<Variable> setLinkInfo(int32_t clientId, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel, std::string name, std::string description);
	virtual std::shared_ptr<Variable> setName(int32_t clientId, uint64_t id, std::string name);
	virtual std::shared_ptr<Variable> setTeam(int32_t clientId, std::string serialNumber, int32_t channel, std::string teamSerialNumber, int32_t teamChannel, bool force = false, bool burst = true) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> setTeam(int32_t clientId, uint64_t peerId, int32_t channel, uint64_t teamId, int32_t teamChannel, bool force = false, bool burst = true) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual std::shared_ptr<Variable> setValue(int32_t clientId, std::string serialNumber, uint32_t channel, std::string valueKey, std::shared_ptr<Variable> value);
	virtual std::shared_ptr<Variable> setValue(int32_t clientId, uint64_t id, uint32_t channel, std::string valueKey, std::shared_ptr<Variable> value);
	virtual std::shared_ptr<Variable> updateFirmware(int32_t clientId, std::vector<uint64_t> ids, bool manual) { return Variable::createError(-32601, "Method not implemented for this central."); }
protected:
	BaseLib::Obj* _bl = nullptr;
	int32_t _deviceFamily = -1;
	uint64_t _deviceId = 0;
	int32_t _address = 0;
    std::string _serialNumber;
    std::map<uint32_t, uint32_t> _variableDatabaseIds;
    bool _initialized = false;
    bool _disposing = false;

	std::shared_ptr<Peer> _currentPeer;
    std::unordered_map<int32_t, std::shared_ptr<Peer>> _peers;
    std::unordered_map<std::string, std::shared_ptr<Peer>> _peersBySerial;
    std::map<uint64_t, std::shared_ptr<Peer>> _peersById;
    std::mutex _peersMutex;

	//Event handling
    PEventHandler _physicalInterfaceEventhandler;

	ICentral(const ICentral&);
	ICentral& operator=(const ICentral&);

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

	virtual void setPeerId(uint64_t oldPeerId, uint64_t newPeerId);
	virtual void deletePeersFromDatabase();
	virtual void loadVariables() = 0;
	virtual void loadPeers() {}
	virtual void savePeers() {}
	virtual void saveVariables() = 0;
	virtual void saveVariable(uint32_t index, int64_t intValue);
	virtual void saveVariable(uint32_t index, std::string& stringValue);
	virtual void saveVariable(uint32_t index, std::vector<uint8_t>& binaryValue);
	virtual void savePeers(bool full) = 0;
};

}
}
#endif /* CENTRAL_H_ */
