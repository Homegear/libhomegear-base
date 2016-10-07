/* Copyright 2013-2016 Sathya Laufer
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

#ifndef PEER_H_
#define PEER_H_

#include "../DeviceDescription/HomegearDevice.h"
#include "../Database/DatabaseTypes.h"
#include "../Sockets/RpcClientInfo.h"
#include "../ScriptEngine/ScriptInfo.h"
#include "ServiceMessages.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

using namespace BaseLib::DeviceDescription;

namespace BaseLib
{

namespace Rpc
{
	class IWebserverEventSink;
}

namespace Systems
{
class ICentral;
class Peer;

class RPCConfigurationParameter
{
public:
	RPCConfigurationParameter() {}
	virtual ~RPCConfigurationParameter() {}

	uint64_t databaseID = 0;
	DeviceDescription::PParameter rpcParameter;
	std::vector<uint8_t> data;
	std::vector<uint8_t> partialData;
};

class ConfigDataBlock
{
public:
	ConfigDataBlock() {}
	virtual ~ConfigDataBlock() {}

	uint32_t databaseID = 0;
	std::vector<uint8_t> data;
};

class BasicPeer
{
public:
	BasicPeer() {}
	BasicPeer(int32_t address, std::string serial, bool isVirtual) { this->address = address; serialNumber = serial; this->isVirtual = isVirtual; }
	virtual ~BasicPeer() {}

	std::shared_ptr<Peer> peer;
	bool hasSender = false; //Todo: Can be removed in future versions
	bool isSender = false;
	uint64_t id = 0;
	int32_t address = 0;
	std::string serialNumber;
	int32_t channel = 0;
	int32_t physicalIndexOffset = 0;
	bool isVirtual = false;
	std::string linkName;
	std::string linkDescription;
	std::vector<uint8_t> data;
	int32_t configEEPROMAddress = -1;
};

class Peer : public ServiceMessages::IServiceEventSink, public IEvents
{
public:
	//Event handling
	class IPeerEventSink : public IEventSinkBase
	{
	public:
		//Hooks
		virtual void onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers) = 0;
		virtual void onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers) = 0;

		virtual void onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
		virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint) = 0;
		virtual void onEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
		virtual void onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait) = 0;
		virtual int32_t onIsAddonClient(int32_t clientID) = 0;
	};
	//End event handling

	bool deleting = false; //Needed, so the peer gets not saved in central's worker thread while being deleted

	void setRpcDevice(std::shared_ptr<HomegearDevice> value) { _rpcDevice = value; initializeTypeString(); }
	std::shared_ptr<HomegearDevice> getRpcDevice() { return _rpcDevice; }

	std::unordered_map<uint32_t, ConfigDataBlock> binaryConfig;
	std::unordered_map<uint32_t, std::unordered_map<std::string, RPCConfigurationParameter>> configCentral;
	std::unordered_map<uint32_t, std::unordered_map<std::string, RPCConfigurationParameter>> valuesCentral;
	std::unordered_map<uint32_t, std::unordered_map<int32_t, std::unordered_map<uint32_t, std::unordered_map<std::string, RPCConfigurationParameter>>>> linksCentral;
	std::shared_ptr<ServiceMessages> serviceMessages;

	Peer(BaseLib::Obj* baseLib, uint32_t parentID, IPeerEventSink* eventHandler);
	Peer(BaseLib::Obj* baseLib, int32_t id, int32_t address, std::string serialNumber, uint32_t parentID, IPeerEventSink* eventHandler);
	virtual ~Peer();
	virtual void dispose();

	//Features
	virtual bool wireless() = 0;
	//End features

	//In table peers:
	virtual uint32_t getParentID() { return _parentID; }
	virtual int32_t getAddress() { return _address; }
	virtual uint64_t getID() { return _peerID; }
	virtual void setID(uint64_t id);
	virtual void setAddress(int32_t value) { _address = value; if(_peerID > 0) save(true, false, false); }
	virtual std::string getSerialNumber() { return _serialNumber; }
	virtual void setSerialNumber(std::string serialNumber);
	//End

	//In table variables:
	virtual int32_t getFirmwareVersion() { return _firmwareVersion; }
	virtual void setFirmwareVersion(int32_t value) { _firmwareVersion = value; saveVariable(1001, value); }
	virtual std::string getFirmwareVersionString() { return _firmwareVersionString; }
	virtual void setFirmwareVersionString(std::string value) { _firmwareVersionString = value; saveVariable(1003, value); }
	virtual uint32_t getDeviceType() { return _deviceType; }
	virtual void setDeviceType(uint32_t value) { _deviceType = value; saveVariable(1002, (int32_t)_deviceType); initializeTypeString(); }
    virtual std::string getName() { return _name; }
	virtual void setName(std::string value) { _name = value; saveVariable(1000, _name); }
	virtual std::string getIp() { return _ip; }
	virtual void setIp(std::string value) { _ip = value; saveVariable(1004, value); }
	virtual std::string getIdString() { return _idString; }
	virtual void setIdString(std::string value) { _idString = value; saveVariable(1005, value); }
	virtual std::string getTypeString() { return _typeString; }
	virtual void setTypeString(std::string value) { _typeString = value; saveVariable(1006, value); }
    //End

	virtual std::string getRpcTypeString() { return _rpcTypeString; }

	virtual std::string handleCliCommand(std::string command) = 0;
	virtual HomegearDevice::ReceiveModes::Enum getRXModes();
	virtual bool isTeam() { return false; }
	virtual void setLastPacketReceived();
	virtual uint32_t getLastPacketReceived() { return _lastPacketReceived; }
	virtual bool pendingQueuesEmpty() { return true; }
	virtual void enqueuePendingQueues() {}
	virtual int32_t getChannelGroupedWith(int32_t channel) = 0;
	virtual int32_t getNewFirmwareVersion() = 0;
	virtual std::string getFirmwareVersionString(int32_t firmwareVersion) = 0;
    virtual bool firmwareUpdateAvailable() = 0;

	virtual bool load(ICentral* central) { return false; }
	virtual void save(bool savePeer, bool saveVariables, bool saveCentralConfig);
	virtual void loadConfig();
    virtual void saveConfig();
	virtual void saveParameter(uint32_t parameterID, ParameterGroup::Type::Enum parameterSetType, uint32_t channel, std::string parameterName, std::vector<uint8_t>& value, int32_t remoteAddress = 0, uint32_t remoteChannel = 0);
	virtual void saveParameter(uint32_t parameterID, uint32_t address, std::vector<uint8_t>& value);
	virtual void saveParameter(uint32_t parameterID, std::vector<uint8_t>& value);
	virtual void loadVariables(ICentral* central, std::shared_ptr<BaseLib::Database::DataTable>& rows);
	virtual void saveVariables();
	virtual void saveVariable(uint32_t index, int32_t intValue);
    virtual void saveVariable(uint32_t index, int64_t intValue);
    virtual void saveVariable(uint32_t index, std::string& stringValue);
    virtual void saveVariable(uint32_t index, std::vector<char>& binaryValue);
    virtual void saveVariable(uint32_t index, std::vector<uint8_t>& binaryValue);
    virtual void deleteFromDatabase();
    virtual void savePeers() = 0;

    /*
     * Executed when Homegear is fully started.
     */
    virtual void homegearStarted();

    /*
     * Executed before Homegear starts shutting down.
     */
    virtual void homegearShuttingDown();

    /*
     * Initializes the MASTER and VALUES config parameter sets by inserting all missing RPC parameters into configCentral and valuesCentral and creating the database entries. If a parameter does not exist yet, it is initialized with the default value defined in the device's XML file.
     *
     * @see configCentral
     * @see valuesCentral
     */
    virtual void initializeCentralConfig();

    virtual std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>> getPeers();
    virtual std::shared_ptr<BasicPeer> getPeer(int32_t channel, int32_t address, int32_t remoteChannel = -1);
	virtual std::shared_ptr<BasicPeer> getPeer(int32_t channel, uint64_t id, int32_t remoteChannel = -1);
	virtual std::shared_ptr<BasicPeer> getPeer(int32_t channel, std::string serialNumber, int32_t remoteChannel = -1);
	virtual void updatePeer(uint64_t oldId, uint64_t newId);

    virtual std::shared_ptr<ICentral> getCentral() = 0;

    //RPC methods
	virtual std::shared_ptr<Variable> activateLinkParamset(PRpcClientInfo clientInfo, int32_t channel, uint64_t remoteID, int32_t remoteChannel, bool longPress) { return Variable::createError(-32601, "Method not implemented by this device family."); }
	virtual std::shared_ptr<Variable> getAllConfig(PRpcClientInfo clientInfo);
	virtual std::shared_ptr<Variable> getAllValues(PRpcClientInfo clientInfo, bool returnWriteOnly);
	virtual std::shared_ptr<Variable> getConfigParameter(PRpcClientInfo clientInfo, uint32_t channel, std::string name);
	virtual std::shared_ptr<std::vector<std::shared_ptr<Variable>>> getDeviceDescriptions(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields);
    virtual std::shared_ptr<Variable> getDeviceDescription(PRpcClientInfo clientInfo, int32_t channel, std::map<std::string, bool> fields);
    virtual std::shared_ptr<Variable> getDeviceInfo(PRpcClientInfo clientInfo, std::map<std::string, bool> fields);
    virtual std::shared_ptr<Variable> getLink(PRpcClientInfo clientInfo, int32_t channel, int32_t flags, bool avoidDuplicates);
    virtual std::shared_ptr<Variable> getLinkInfo(PRpcClientInfo clientInfo, int32_t senderChannel, uint64_t receiverID, int32_t receiverChannel);
	virtual std::shared_ptr<Variable> setLinkInfo(PRpcClientInfo clientInfo, int32_t senderChannel, uint64_t receiverID, int32_t receiverChannel, std::string name, std::string description);
	virtual std::shared_ptr<Variable> getLinkPeers(PRpcClientInfo clientInfo, int32_t channel, bool returnID);
    virtual std::shared_ptr<Variable> getParamset(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel);
    virtual std::shared_ptr<Variable> getParamsetDescription(PRpcClientInfo clientInfo, std::shared_ptr<ParameterGroup> parameterSet);
    virtual std::shared_ptr<Variable> getParamsetDescription(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel);
    virtual std::shared_ptr<Variable> getParamsetId(PRpcClientInfo clientInfo, uint32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel);
    virtual std::shared_ptr<Variable> getServiceMessages(PRpcClientInfo clientInfo, bool returnID);
    virtual std::shared_ptr<Variable> getValue(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous);
    virtual std::shared_ptr<Variable> putParamset(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, std::shared_ptr<BaseLib::Variable> variables, bool onlyPushing = false) = 0;
    virtual std::shared_ptr<Variable> reportValueUsage(PRpcClientInfo clientInfo);
    virtual std::shared_ptr<Variable> rssiInfo(PRpcClientInfo clientInfo);

    /**
     * RPC function to change the ID of the peer.
     *
     * @param newPeerID The new ID of the peer.
     * @return Returns "RPC void" on success or RPC error "-100" when the new peer ID is invalid and error "-101" when the new peer ID is already in use.
     */
    virtual std::shared_ptr<BaseLib::Variable> setId(PRpcClientInfo clientInfo, uint64_t newPeerID);
    virtual std::shared_ptr<Variable> setInterface(PRpcClientInfo clientInfo, std::string interfaceID) { return Variable::createError(-32601, "Method not implemented for this Peer."); }
	virtual std::shared_ptr<BaseLib::Variable> setValue(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, std::shared_ptr<BaseLib::Variable> value, bool wait);
    //End RPC methods
protected:
    BaseLib::Obj* _bl = nullptr;
    std::shared_ptr<HomegearDevice> _rpcDevice;
    std::map<uint32_t, uint32_t> _variableDatabaseIDs;
    std::shared_ptr<ICentral> _central;

	//In table peers:
	uint64_t _peerID = 0;
	uint32_t _parentID = 0;
	int32_t _address = 0;
	std::string _serialNumber;
	//End

	//In table variables:
	int32_t _firmwareVersion = 0;
	std::string _firmwareVersionString;
	uint32_t _deviceType = 0;
	std::mutex _peersMutex;
	std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>> _peers;
	std::string _name;
	std::string _ip;
	std::string _idString;

	/*
	 * Stores a manually set type string. Overrides _rpcTypeString.
	 * @see _rpcTypeString
	 */
	std::string _typeString;
	//End

	/*
	 * Stores the type string defined in the device's XML file. Can be overridden by _typeString.
	 * @see _typeString
	 */
	std::string _rpcTypeString;
	HomegearDevice::ReceiveModes::Enum _rxModes = HomegearDevice::ReceiveModes::Enum::none;

	bool _disposing = false;
	uint32_t _lastPacketReceived = 0;

	// {{{ Event handling
		//Hooks
		std::map<int32_t, PEventHandler> _webserverEventHandlers;
		virtual void raiseAddWebserverEventHandler(Rpc::IWebserverEventSink* eventHandler);
		virtual void raiseRemoveWebserverEventHandler();

		virtual void raiseRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
		virtual void raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
		virtual void raiseEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<BaseLib::Variable>>> values);
		virtual void raiseRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait);
		virtual int32_t raiseIsAddonClient(int32_t clientID);
	// }}}

	//ServiceMessages event handling
	virtual void onConfigPending(bool configPending);

	virtual void onEvent(uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
	virtual void onRPCEvent(uint64_t peerId, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
	virtual void onSaveParameter(std::string name, uint32_t channel, std::vector<uint8_t>& data);
	virtual std::shared_ptr<Database::DataTable> onGetServiceMessages();
	virtual void onSaveServiceMessage(Database::DataRow& data);
	virtual void onDeleteServiceMessage(uint64_t databaseID);
	virtual void onEnqueuePendingQueues();
	//End ServiceMessages event handling

	/**
	 * Gets a variable value directly from the device. This method is used as an inheritable hook method within getValue().
	 *
	 * @see getValue
	 * @param parameter The parameter to get the value for.
	 * @param channel The channel to get the value for.
	 * @param asynchronous If set to true, the method returns immediately, otherwise it waits for the response packet.
	 * @return Returns the requested value when asynchronous is false. If asynchronous is true, rpcVoid is returned.
	 */
	virtual std::shared_ptr<BaseLib::Variable> getValueFromDevice(std::shared_ptr<Parameter>& parameter, int32_t channel, bool asynchronous) { return Variable::createError(-32601, "Method not implemented for this device family."); }

	/**
	 * This method returns the correct parameter set for a specified channel and parameter set type. It is necessary, because sometimes multiple parameter sets are available for one channel depending on family specific conditions. This method is a mandatory hook method.
	 *
	 * @param channel The channel index.
	 * @param type The parameter set type.
	 * @return Returns the parameter set for the specified channel and type.
	 */
	virtual std::shared_ptr<ParameterGroup> getParameterSet(int32_t channel, ParameterGroup::Type::Enum type) = 0;

	/**
	 * Overridable hook in initializeCentralConfig to set a custom default value. See BidCoSPeer for an implementation example. There it is used to conditionally set "AES_ACTIVE", depending on whether the physical interface supports it.
	 *
	 * @param parameter The parameter to set the default value for.
	 * @see initializeCentralConfig
	 */
	virtual void setDefaultValue(RPCConfigurationParameter* parameter);

	/**
	 * Called by initializeCentralConfig().
	 *
	 * @see initializeCentralConfig()
	 */
	virtual void initializeMasterSet(int32_t channel, std::shared_ptr<ConfigParameters> masterSet);

	/**
	 * Called by initializeCentralConfig().
	 *
	 * @see initializeCentralConfig()
	 */
	virtual void initializeValueSet(int32_t channel, std::shared_ptr<Variables> valueSet);

	/**
	 * Initializes _typeString.
	 *
	 * @see _typeString
	 */
	virtual void initializeTypeString();

	// {{{ Hooks
		/*
		 * This hook is executed for each parameter in getAllValues after the initial checks if the parameter exists etc. It can be used for example to set the value of the parameter.
		 *
		 * @param parameter The current parameter.
		 * @param channel The channel of the parameter.
		 * @param parameters The parameters struct which will be returned by getAllValues.
		 * @return "true" means the function handled the parameter completely and there is nothing to do anymore in getAllValues.
		 */
		virtual bool getAllValuesHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters) { return false; }

		/*
		 * This hook is executed for each parameter in getParamset after the initial checks if the parameter exists etc. It can be used for example to set the value of the parameter.
		 *
		 * @param parameter The current parameter.
		 * @param channel The channel of the parameter.
		 * @param parameters The parameters struct which will be returned by getParamset.
		 * @return "true" means the function handled the parameter completely and there is nothing to do anymore in getParamset.
		 */
		virtual bool getParamsetHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters) { return false; }

		/*
		 * This hook is executed every time "convertFromPacket" is called in case custom conversions are used.
		 *
		 * @param parameter The current parameter.
		 * @param data The data to convert.
		 * @param result The conversion result.
		 * @return "true" means the function converted the parameter.
		 */
		virtual bool convertFromPacketHook(PParameter parameter, std::vector<uint8_t>& data, PVariable& result) { return false; }

		/*
		 * This hook is executed every time "convertToPacket" is called in case custom conversions are used.
		 *
		 * @param parameter The current parameter.
		 * @param data The data to convert.
		 * @param result The conversion result.
		 * @return "true" means the function converted the parameter.
		 */
		virtual bool convertToPacketHook(PParameter parameter, PVariable data, std::vector<uint8_t>& result) { return false; }
	// }}}
};

}
}
#endif
