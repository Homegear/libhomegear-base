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

#ifndef ICENTRAL_H_
#define ICENTRAL_H_

#include "../Variable.h"
#include "../IEvents.h"
#include "../DeviceDescription/HomegearDevice.h"
#include "../Sockets/RpcClientInfo.h"
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

		virtual void onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values) = 0;
		virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint) = 0;
		virtual void onRPCNewDevices(PVariable deviceDescriptions) = 0;
		virtual void onRPCDeleteDevices(PVariable deviceAddresses, PVariable deviceInfo) = 0;
		virtual void onEvent(uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<BaseLib::Variable>>> values) = 0;
		virtual void onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait) = 0;
		virtual uint64_t onGetRoomIdByName(std::string& name) = 0;
	};
	//End event handling

	ICentral(int32_t deviceFamily, BaseLib::SharedObjects* baseLib, ICentralEventSink* eventHandler);
	ICentral(int32_t deviceFamily, BaseLib::SharedObjects* baseLib, uint32_t deviceId, std::string serialNumber, int32_t address, ICentralEventSink* eventHandler);
	virtual ~ICentral();
	virtual void dispose(bool wait = true);

	virtual void load();
	virtual void save(bool full);

	/*
     * Executed when Homegear is fully started.
     */
    virtual void homegearStarted();

	/*
     * Executed before Homegear starts shutting down.
     */
    virtual void homegearShuttingDown();

	virtual int32_t deviceFamily();

    std::shared_ptr<DeviceTranslations> getTranslations() { return _translations; }
	virtual int32_t getAddress() { return _address; }
	virtual uint64_t getId() { return _deviceId; }
    virtual std::string getSerialNumber() { return _serialNumber; }
	virtual std::string handleCliCommand(std::string command) { return ""; }
	std::vector<std::shared_ptr<Peer>> getPeers();
	std::shared_ptr<Peer> getPeer(int32_t address);
    std::shared_ptr<Peer> getPeer(uint64_t id);
    std::shared_ptr<Peer> getPeer(std::string serialNumber);
	virtual bool peerSelected();

	virtual bool peerExists(int32_t address);
	virtual bool peerExists(std::string serialNumber);
	virtual bool peerExists(uint64_t id);
	virtual uint64_t getPeerIdFromSerial(std::string& serialNumber);

	virtual PVariable activateLinkParamset(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, std::string remoteSerialNumber, int32_t remoteChannel, bool longPress) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable activateLinkParamset(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, uint64_t remoteId, int32_t remoteChannel, bool longPress) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable addCategoryToDevice(PRpcClientInfo clientInfo, uint64_t peerId, uint64_t categoryId);
	virtual PVariable addDevice(PRpcClientInfo clientInfo, std::string serialNumber) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable addDeviceToRoom(PRpcClientInfo clientInfo, uint64_t peerId, uint64_t roomId);
	virtual PVariable addLink(PRpcClientInfo clientInfo, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel, std::string name, std::string description) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable addLink(PRpcClientInfo clientInfo, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel, std::string name, std::string description) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable createDevice(PRpcClientInfo clientInfo, int32_t deviceType, std::string serialNumber, int32_t address, int32_t firmwareVersion, std::string interfaceId) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable deleteDevice(PRpcClientInfo clientInfo, std::string serialNumber, int32_t flags) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable deleteDevice(PRpcClientInfo clientInfo, uint64_t peerId, int32_t flags) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable getAllConfig(PRpcClientInfo clientInfo, uint64_t peerId);
	virtual PVariable getAllValues(PRpcClientInfo clientInfo, uint64_t peerId, bool returnWriteOnly);
	virtual PVariable getConfigParameter(PRpcClientInfo clientInfo, std::string serialNumber, uint32_t channel, std::string name);
	virtual PVariable getConfigParameter(PRpcClientInfo clientInfo, uint64_t peerId, uint32_t channel, std::string name);
	virtual PVariable getDeviceDescription(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, std::map<std::string, bool> fields);
	virtual PVariable getDeviceDescription(PRpcClientInfo clientInfo, uint64_t id, int32_t channel, std::map<std::string, bool> fields);
	virtual PVariable getDeviceInfo(PRpcClientInfo clientInfo, uint64_t id, std::map<std::string, bool> fields) = 0;
	virtual PVariable getDevicesInCategory(PRpcClientInfo clientInfo, uint64_t categoryId);
	virtual PVariable getDevicesInRoom(PRpcClientInfo clientInfo, uint64_t roomId);
	virtual PVariable getPeerId(PRpcClientInfo clientInfo, int32_t filterType, std::string filterValue);
	virtual PVariable getPeerId(PRpcClientInfo clientInfo, int32_t address);
	virtual PVariable getPeerId(PRpcClientInfo clientInfo, std::string serialNumber);
	virtual PVariable getInstallMode(PRpcClientInfo clientInfo) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable getLinkInfo(PRpcClientInfo clientInfo, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel);
	virtual PVariable getLinkInfo(PRpcClientInfo clientInfo, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel);
	virtual PVariable getLinkPeers(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel);
	virtual PVariable getLinkPeers(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel);
	virtual PVariable getLinks(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, int32_t flags);
	virtual PVariable getLinks(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, int32_t flags);
	virtual PVariable getName(PRpcClientInfo clientInfo, uint64_t id);
	virtual PVariable getParamsetDescription(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel);
	virtual PVariable getParamsetDescription(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel);
	virtual PVariable getParamsetId(PRpcClientInfo clientInfo, std::string serialNumber, uint32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel);
	virtual PVariable getParamsetId(PRpcClientInfo clientInfo, uint64_t peerId, uint32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel);
	virtual PVariable getParamset(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel);
	virtual PVariable getParamset(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel);
	virtual PVariable getServiceMessages(PRpcClientInfo clientInfo, bool returnId);
	virtual PVariable getSniffedDevices(PRpcClientInfo clientInfo) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable getValue(PRpcClientInfo clientInfo, std::string serialNumber, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous);
	virtual PVariable getValue(PRpcClientInfo clientInfo, uint64_t id, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous);
	virtual PVariable getVariableDescription(PRpcClientInfo clientInfo, uint64_t id, uint32_t channel, std::string valueKey);
	virtual PVariable listDevices(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields);
	virtual PVariable listDevices(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields, std::shared_ptr<std::set<uint64_t>> knownDevices);
	virtual PVariable listTeams(BaseLib::PRpcClientInfo clientInfo);
	virtual PVariable putParamset(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel, PVariable paramset) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable putParamset(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel, PVariable paramset) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable reportValueUsage(PRpcClientInfo clientInfo, std::string serialNumber);
	virtual PVariable removeCategoryFromDevice(PRpcClientInfo clientInfo, uint64_t peerId, uint64_t categoryId);
	virtual PVariable removeDeviceFromRoom(PRpcClientInfo clientInfo, uint64_t peerId, uint64_t roomId);
	virtual PVariable removeLink(PRpcClientInfo clientInfo, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable removeLink(PRpcClientInfo clientInfo, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable rssiInfo(PRpcClientInfo clientInfo);
	virtual PVariable searchDevices(PRpcClientInfo clientInfo) { return Variable::createError(-32601, "Method not implemented for this central."); }

	/**
     * RPC function to change the Id of the peer.
     *
     * @param clientInfo Information about the RPC client.
     * @param oldPeerId The current ID of the peer.
     * @param newPeerId The new ID of the peer.
     * @return Returns "RPC void" on success or RPC error "-100" when the new peer ID is invalid and error "-101" when the new peer ID is already in use.
     */
    virtual std::shared_ptr<BaseLib::Variable> setId(PRpcClientInfo clientInfo, uint64_t oldPeerId, uint64_t newPeerId);
	virtual PVariable setInstallMode(PRpcClientInfo clientInfo, bool on, uint32_t duration = 60, bool debugOutput = true) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable setInterface(PRpcClientInfo clientInfo, uint64_t peerId, std::string interfaceId) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable setLinkInfo(PRpcClientInfo clientInfo, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel, std::string name, std::string description);
	virtual PVariable setLinkInfo(PRpcClientInfo clientInfo, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel, std::string name, std::string description);
	virtual PVariable setName(PRpcClientInfo clientInfo, uint64_t id, std::string name);
	virtual PVariable setTeam(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, std::string teamSerialNumber, int32_t teamChannel, bool force = false, bool burst = true) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable setTeam(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, uint64_t teamId, int32_t teamChannel, bool force = false, bool burst = true) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable setValue(PRpcClientInfo clientInfo, std::string serialNumber, uint32_t channel, std::string valueKey, PVariable value, bool wait);
	virtual PVariable setValue(PRpcClientInfo clientInfo, uint64_t id, uint32_t channel, std::string valueKey, PVariable value, bool wait);
	virtual PVariable startSniffing(PRpcClientInfo clientInfo) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable stopSniffing(PRpcClientInfo clientInfo) { return Variable::createError(-32601, "Method not implemented for this central."); }
	virtual PVariable updateFirmware(PRpcClientInfo clientInfo, std::vector<uint64_t> ids, bool manual) { return Variable::createError(-32601, "Method not implemented for this central."); }
protected:
	BaseLib::SharedObjects* _bl = nullptr;
	int32_t _deviceFamily = -1;
	uint64_t _deviceId = 0;
	int32_t _address = 0;
    std::string _serialNumber;
    std::map<uint32_t, uint32_t> _variableDatabaseIds;
    std::atomic_bool _initialized;
    std::atomic_bool _disposing;

	std::shared_ptr<DeviceDescription::DeviceTranslations> _translations;
	std::shared_ptr<Peer> _currentPeer;
    std::unordered_map<int32_t, std::shared_ptr<Peer>> _peers;
    std::unordered_map<std::string, std::shared_ptr<Peer>> _peersBySerial;
    std::map<uint64_t, std::shared_ptr<Peer>> _peersById;
    std::mutex _peersMutex;

	//Event handling
    std::map<std::string, PEventHandler> _physicalInterfaceEventhandlers;

	ICentral(const ICentral&);
	ICentral& operator=(const ICentral&);

	//Hooks
	virtual void raiseAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers);
	virtual void raiseRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers);

	virtual void raiseRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values);
	virtual void raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
	virtual void raiseRPCNewDevices(PVariable deviceDescriptions);
	virtual void raiseRPCDeleteDevices(PVariable deviceAddresses, PVariable deviceInfo);
	virtual void raiseEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<BaseLib::Variable>>> values);
	virtual void raiseRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait);
	virtual uint64_t raiseGetRoomIdByName(std::string& name);
	//End event handling

	//Physical device event handling
	virtual bool onPacketReceived(std::string& senderID, std::shared_ptr<Packet> packet) = 0;
	//End physical device event handling

	// {{{ Peer event handling
		//Hooks
		virtual void onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers);
		virtual void onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers);

		virtual void onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values);
		virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
		virtual void onEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<BaseLib::Variable>>> values);
		virtual void onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait);
	// }}}

	virtual void setPeerId(uint64_t oldPeerId, uint64_t newPeerId);
	virtual void deletePeersFromDatabase();
	virtual void loadVariables() = 0;
	virtual void loadPeers() {}
	virtual void savePeers(bool full) {}
	virtual void saveVariables() = 0;
	virtual void saveVariable(uint32_t index, int64_t intValue);
	virtual void saveVariable(uint32_t index, std::string& stringValue);
	virtual void saveVariable(uint32_t index, std::vector<uint8_t>& binaryValue);
};

}
}
#endif /* CENTRAL_H_ */
