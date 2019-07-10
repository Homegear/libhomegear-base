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

#ifndef PEER_H_
#define PEER_H_

#include "../DeviceDescription/HomegearDevice.h"
#include "../Database/DatabaseTypes.h"
#include "../Sockets/RpcClientInfo.h"
#include "../ScriptEngine/ScriptInfo.h"
#include "ServiceMessages.h"
#include "../DeviceDescription/DeviceTranslations.h"

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

class RpcConfigurationParameter
{
public:
	RpcConfigurationParameter() {}
	RpcConfigurationParameter(RpcConfigurationParameter const& rhs);
	virtual ~RpcConfigurationParameter() {}
	RpcConfigurationParameter& operator=(const RpcConfigurationParameter& rhs);

	/**
	 * Locks the internal binary data vector so one can work with a reference to it externally.
	 */
	void lock() noexcept;

	/**
	 * Unlocks the internal binary data vector.
	 */
	void unlock() noexcept;

	/**
	 * Returns the size of the data vector. This method is thread safe. Make sure the vector is unlocked ("unlock()" was called after calling "lock()") before executing this method.
	 * @return Returns the size of the internal binary data vector.
	 */
	std::vector<uint8_t>::size_type getBinaryDataSize() noexcept;

	/**
	 * Returns a copy of the data vector. This method is thread safe. Make sure the vector is unlocked ("unlock()" was called after calling "lock()") before executing this method.
	 * @return Returns a copy of the internal binary data vector.
	 */
	std::vector<uint8_t> getBinaryData() noexcept;

	/**
	 * Returns a reference to the data vector. Call "lock()" before executing this method and "unlock()" when you're done working with the reference.
	 * @return Returns a reference to the internal binary data vector.
	 */
	std::vector<uint8_t>& getBinaryDataReference() noexcept;

	/**
	 * Sets the internal binary data vector to "value". This method is thread safe. Make sure the vector is unlocked ("unlock()" was called after calling "lock()") before executing this method.
	 * @param value The new data vector.
	 */
	void setBinaryData(std::vector<uint8_t>& value) noexcept;

	/**
	 * Returns a copy of the data vector. This method is thread safe. Make sure the vector is unlocked ("unlock()" was called after calling "lock()") before executing this method.
	 * @return Returns a copy of the internal binary data vector.
	 */
	std::vector<uint8_t> getPartialBinaryData() noexcept;

	/**
	 * Returns a reference to the data vector. Call "lock()" before executing this method and "unlock()" when you're done working with the reference.
	 * @return Returns a reference to the internal binary data vector.
	 */
	std::vector<uint8_t>& getPartialBinaryDataReference() noexcept;

	/**
	 * Sets the internal binary data vector to "value". This method is thread safe. Make sure the vector is unlocked ("unlock()" was called after calling "lock()") before executing this method.
	 * @param value The new data vector.
	 */
	void setPartialBinaryData(std::vector<uint8_t>& value) noexcept;

	/**
	 * Returns the logical data object. This method is thread safe.
	 * @return Returns the logical data object.
	 */
	BaseLib::PVariable getLogicalData() noexcept;

	/**
	 * Sets the logical data object. This method is thread safe.
	 * @param value The new logical data object.
	 */
	void setLogicalData(PVariable value) noexcept;

	/**
	 * Compares the passed vector with the internal one. This method is thread safe.
	 * @return Returns "true" if both vectors are equal. "false" otherwise.
	 */
	bool equals(std::vector<uint8_t>& value) noexcept;

	bool hasCategory(uint64_t id) { std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex); return _categories.find(id) != _categories.end(); }
	void addCategory(uint64_t id) { std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex); _categories.emplace(id); }
	void removeCategory(uint64_t id) { std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex); _categories.erase(id); }
    std::set<uint64_t> getCategories() { std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex); return _categories; }
	std::string getCategoryString();
	bool hasCategories() { std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex); return !_categories.empty(); }

	bool hasRole(uint64_t id) { std::lock_guard<std::mutex> rolesGuard(_rolesMutex); return _roles.find(id) != _roles.end(); }
	void addRole(uint64_t id) { std::lock_guard<std::mutex> rolesGuard(_rolesMutex); _roles.emplace(id); }
	void removeRole(uint64_t id) { std::lock_guard<std::mutex> rolesGuard(_rolesMutex); _roles.erase(id); }
	std::set<uint64_t> getRoles() { std::lock_guard<std::mutex> rolesGuard(_rolesMutex); return _roles; }
	std::string getRoleString();
	bool hasRoles() { std::lock_guard<std::mutex> rolesGuard(_rolesMutex); return !_roles.empty(); }

    uint64_t getRoom() { std::lock_guard<std::mutex> roomGuard(_roomMutex); return _room; }
    void setRoom(uint64_t id) { std::lock_guard<std::mutex> roomGuard(_roomMutex); _room = id; }

	/**
	 * The id of this parameter in the database.
	 */
	uint64_t databaseId = 0;

	/**
	 * The special type of the parameter (0 = none, 1 = roles).
	 */
	int32_t specialType = 0;

	/**
	 * The RPC parameter as defined in the XML file.
	 */
	DeviceDescription::PParameter rpcParameter;
private:
	std::mutex _logicalDataMutex;
	BaseLib::PVariable _logicalData;
	std::mutex _binaryDataMutex;
	std::vector<uint8_t> _binaryData;
	std::vector<uint8_t> _partialBinaryData;
    std::mutex _categoriesMutex;
	std::set<uint64_t> _categories;
	std::mutex _rolesMutex;
	std::set<uint64_t> _roles;
    std::mutex _roomMutex;
    uint64_t _room = 0;
};

class ConfigDataBlock
{
public:
	ConfigDataBlock() {}
	ConfigDataBlock(ConfigDataBlock const& rhs);
	virtual ~ConfigDataBlock() {}
	ConfigDataBlock& operator=(const ConfigDataBlock& rhs);

	/**
	 * Locks the internal binary data vector so one can work with a reference to it externally.
	 */
	void lock() noexcept;

	/**
	 * Unlocks the internal binary data vector.
	 */
	void unlock() noexcept;

	/**
	 * Returns the size of the data vector. This method is thread safe. Make sure the vector is unlocked ("unlock()" was called after calling "lock()") before executing this method.
	 * @return Returns the size of the internal binary data vector.
	 */
	std::vector<uint8_t>::size_type getBinaryDataSize() noexcept;

	/**
	 * Returns a copy of the data vector. This method is thread safe. Make sure the vector is unlocked ("unlock()" was called after calling "lock()") before executing this method.
	 * @return Returns a copy of the internal binary data vector.
	 */
	std::vector<uint8_t> getBinaryData() noexcept;

	/**
	 * Returns a reference to the data vector. Call "lock()" before executing this method and "unlock()" when you're done working with the reference.
	 * @return Returns a reference to the internal binary data vector.
	 */
	std::vector<uint8_t>& getBinaryDataReference() noexcept;

	/**
	 * Sets the internal binary data vector to "value". This method is thread safe. Make sure the vector is unlocked ("unlock()" was called after calling "lock()") before executing this method.
	 * @param value The new data vector.
	 */
	void setBinaryData(std::vector<uint8_t>& value) noexcept;

	/**
	 * Compares the passed vector with the internal one. This method is thread safe.
	 * @return Returns "true" if both vectors are equal. "false" otherwise.
	 */
	bool equals(std::vector<uint8_t>& value) noexcept;

	/**
	 * The id of this parameter in the database.
	 */
	uint64_t databaseId = 0;
private:
	std::mutex _binaryDataMutex;
	std::vector<uint8_t> _binaryData;
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

		virtual void onRPCEvent(std::string& source, uint64_t id, int32_t channel, std::string& deviceAddress, std::shared_ptr<std::vector<std::string>>& valueKeys, std::shared_ptr<std::vector<PVariable>>& values) = 0;
		virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint) = 0;
		virtual void onEvent(std::string& source, uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>>& variables, std::shared_ptr<std::vector<PVariable>>& values) = 0;
		virtual void onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait) = 0;
		virtual BaseLib::PVariable onInvokeRpc(std::string& methodName, BaseLib::PArray& parameters) = 0;
	};
	//End event handling

	std::atomic_bool deleting; //Needed, so the peer gets not saved in central's worker thread while being deleted

	void setRpcDevice(std::shared_ptr<HomegearDevice> value) { _rpcDevice = value; initializeTypeString(); }
	std::shared_ptr<HomegearDevice> getRpcDevice() { return _rpcDevice; }

	std::unordered_map<uint32_t, ConfigDataBlock> binaryConfig;
	std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>> configCentral;
	std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>> valuesCentral;
	std::unordered_map<uint32_t, std::unordered_map<int32_t, std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>>> linksCentral;
	std::shared_ptr<ServiceMessages> serviceMessages;

	Peer(BaseLib::SharedObjects* baseLib, uint32_t parentId, IPeerEventSink* eventHandler);
	Peer(BaseLib::SharedObjects* baseLib, uint64_t id, int32_t address, std::string serialNumber, uint32_t parentId, IPeerEventSink* eventHandler);
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
    virtual std::string getName() { return getName(-1); }
	virtual std::string getName(int32_t channel);
	virtual void setName(std::string value) { setName(-1, value); }
	virtual void setName(int32_t channel, std::string value);
	virtual std::string getIp() { return _ip; }
	virtual void setIp(std::string value) { _ip = value; saveVariable(1004, value); }
	virtual std::string getIdString() { return _idString; }
	virtual void setIdString(std::string value) { _idString = value; saveVariable(1005, value); }
	virtual std::string getTypeString() { return _typeString; }
	virtual void setTypeString(std::string value) { _typeString = value; saveVariable(1006, value); }
    virtual std::set<int32_t> getChannelsInRoom(uint64_t roomId);
	virtual uint64_t getRoom(int32_t channel);
    virtual bool hasRoomInChannels(uint64_t roomId);
	bool roomsSet();
	virtual bool setRoom(int32_t channel, uint64_t value);
    virtual std::unordered_map<int32_t, std::set<uint64_t>> getCategories();
	virtual std::set<uint64_t> getCategories(int32_t channel);
    virtual std::set<int32_t> getChannelsInCategory(uint64_t categoryId);
	virtual bool hasCategories();
	virtual bool hasCategories(int32_t channel);
	virtual bool hasCategory(int32_t channel, uint64_t id);
    virtual bool hasCategoryInChannels(uint64_t categoryId);
	virtual bool addCategory(int32_t channel, uint64_t id);
	virtual bool removeCategory(int32_t channel, uint64_t id);
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

    virtual bool setVariableRoom(int32_t channel, std::string& variableName, uint64_t roomId);
	virtual void removeRoomFromVariables(uint64_t roomId);
    virtual uint64_t getVariableRoom(int32_t channel, const std::string& variableName);
    virtual bool addCategoryToVariable(int32_t channel, std::string& variableName, uint64_t categoryId);
    virtual bool removeCategoryFromVariable(int32_t channel, std::string& variableName, uint64_t categoryId);
	virtual void removeCategoryFromVariables(uint64_t categoryId);
    virtual std::set<uint64_t> getVariableCategories(int32_t channel, std::string& variableName);
    virtual bool variableHasCategory(int32_t channel, const std::string& variableName, uint64_t categoryId);
	virtual bool variableHasCategories(int32_t channel, const std::string& variableName);
	virtual bool addRoleToVariable(int32_t channel, std::string& variableName, uint64_t roleId);
	virtual bool removeRoleFromVariable(int32_t channel, std::string& variableName, uint64_t roleId);
	virtual void removeRoleFromVariables(uint64_t roleId);
	virtual std::set<uint64_t> getVariableRoles(int32_t channel, std::string& variableName);
	virtual bool variableHasRole(int32_t channel, const std::string& variableName, uint64_t roleId);
	virtual bool variableHasRoles(int32_t channel, const std::string& variableName);

	virtual bool load(ICentral* central) { return false; }
	virtual void save(bool savePeer, bool saveVariables, bool saveCentralConfig);
	virtual void loadConfig();
    virtual void saveConfig();
	virtual void saveParameter(uint32_t parameterID, ParameterGroup::Type::Enum parameterSetType, uint32_t channel, const std::string& parameterName, std::vector<uint8_t>& value, int32_t remoteAddress = 0, uint32_t remoteChannel = 0);
    virtual void saveSpecialTypeParameter(uint32_t parameterID, ParameterGroup::Type::Enum parameterSetType, uint32_t channel, const std::string& parameterName, std::vector<uint8_t>& value, int32_t specialType, const BaseLib::PVariable& metadata, const std::string& roles);
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
	virtual PVariable activateLinkParamset(PRpcClientInfo clientInfo, int32_t channel, uint64_t remoteID, int32_t remoteChannel, bool longPress) { return Variable::createError(-32601, "Method not implemented by this device family."); }
    virtual PVariable forceConfigUpdate(PRpcClientInfo clientInfo) { return Variable::createError(-32601, "Method not implemented for this peer."); }
	virtual PVariable getAllConfig(PRpcClientInfo clientInfo);
	virtual PVariable getAllValues(PRpcClientInfo clientInfo, bool returnWriteOnly, bool checkAcls);
	virtual PVariable getConfigParameter(PRpcClientInfo clientInfo, uint32_t channel, std::string name);
	virtual std::shared_ptr<std::vector<PVariable>> getDeviceDescriptions(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields);
    virtual PVariable getDeviceDescription(PRpcClientInfo clientInfo, int32_t channel, std::map<std::string, bool> fields);
    virtual PVariable getDeviceInfo(PRpcClientInfo clientInfo, std::map<std::string, bool> fields);
    virtual PVariable getLink(PRpcClientInfo clientInfo, int32_t channel, int32_t flags, bool avoidDuplicates);
    virtual PVariable getLinkInfo(PRpcClientInfo clientInfo, int32_t senderChannel, uint64_t receiverID, int32_t receiverChannel);
	virtual PVariable setLinkInfo(PRpcClientInfo clientInfo, int32_t senderChannel, uint64_t receiverID, int32_t receiverChannel, std::string name, std::string description);
	virtual PVariable getLinkPeers(PRpcClientInfo clientInfo, int32_t channel, bool returnID);
    virtual PVariable getParamset(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, bool checkAcls);
    virtual PVariable getParamsetDescription(PRpcClientInfo clientInfo, int32_t channel, PParameterGroup parameterSet, bool checkAcls);
    virtual PVariable getParamsetDescription(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, bool checkAcls);
    virtual PVariable getParamsetId(PRpcClientInfo clientInfo, uint32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel);
    virtual PVariable getServiceMessages(PRpcClientInfo clientInfo, bool returnID);
    virtual PVariable getValue(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous);
    virtual PVariable getVariableDescription(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, const std::unordered_set<std::string>& fields);
    virtual PVariable getVariablesInCategory(PRpcClientInfo clientInfo, uint64_t categoryId, bool checkAcls);
	virtual PVariable getVariablesInRole(PRpcClientInfo clientInfo, uint64_t roleId, bool checkAcls);
    virtual PVariable getVariablesInRoom(PRpcClientInfo clientInfo, uint64_t roomId, bool checkAcls);
    virtual PVariable putParamset(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, PVariable variables, bool checkAcls, bool onlyPushing = false) = 0;
    virtual PVariable reportValueUsage(PRpcClientInfo clientInfo);
    virtual PVariable rssiInfo(PRpcClientInfo clientInfo);

    /**
     * RPC function to change the ID of the peer.
     *
     * @param clientInfo Information about the RPC client.
     * @param newPeerID The new ID of the peer.
     * @return Returns "RPC void" on success or RPC error "-100" when the new peer ID is invalid and error "-101" when the new peer ID is already in use.
     */
    virtual PVariable setId(PRpcClientInfo clientInfo, uint64_t newPeerID);
    virtual PVariable setInterface(PRpcClientInfo clientInfo, std::string interfaceID) { return Variable::createError(-32601, "Method not implemented for this Peer."); }
	virtual PVariable setValue(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, PVariable value, bool wait);
    //End RPC methods
protected:
    BaseLib::SharedObjects* _bl = nullptr;
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
	std::mutex _namesMutex;
	std::unordered_map<int32_t, std::string> _names;
	std::string _ip;
	std::string _idString;

	/*
	 * Stores a manually set type string. Overrides _rpcTypeString.
	 * @see _rpcTypeString
	 */
	std::string _typeString;

    std::mutex _roomMutex;
	std::unordered_map<int32_t, uint64_t> _rooms;
    std::mutex _categoriesMutex;
	std::unordered_map<int32_t, std::set<uint64_t>> _categories;
	//End

	/*
	 * Stores the type string defined in the device's XML file. Can be overridden by _typeString.
	 * @see _typeString
	 */
	std::string _rpcTypeString;
	HomegearDevice::ReceiveModes::Enum _rxModes = HomegearDevice::ReceiveModes::Enum::none;

	bool _disposing = false;
	bool _saveTeam = false;
	uint32_t _lastPacketReceived = 0;

	// {{{ Event handling
		//Hooks
		std::map<int32_t, PEventHandler> _webserverEventHandlers;
		virtual void raiseAddWebserverEventHandler(Rpc::IWebserverEventSink* eventHandler);
		virtual void raiseRemoveWebserverEventHandler();

		virtual void raiseRPCEvent(std::string& source, uint64_t id, int32_t channel, std::string& deviceAddress, std::shared_ptr<std::vector<std::string>>& valueKeys, std::shared_ptr<std::vector<PVariable>>& values);
		virtual void raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
		virtual void raiseEvent(std::string& source, uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>>& variables, std::shared_ptr<std::vector<PVariable>>& values);
		virtual void raiseRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait);
		virtual BaseLib::PVariable raiseInvokeRpc(std::string& methodName, BaseLib::PArray& parameters);
	// }}}

	//ServiceMessages event handling
	virtual void onConfigPending(bool configPending);

	virtual void onEvent(std::string& source, uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>>& variables, std::shared_ptr<std::vector<PVariable>>& values);
	virtual void onRPCEvent(std::string& source, uint64_t peerId, int32_t channel, std::string& deviceAddress, std::shared_ptr<std::vector<std::string>>& valueKeys, std::shared_ptr<std::vector<PVariable>>& values);
	virtual void onSaveParameter(std::string name, uint32_t channel, std::vector<uint8_t>& data);
	virtual std::shared_ptr<Database::DataTable> onGetServiceMessages();
	virtual void onSaveServiceMessage(Database::DataRow& data);
	virtual void onDeleteServiceMessage(uint64_t databaseID);
	virtual void onEnqueuePendingQueues();
	//End ServiceMessages event handling

	/**
	 * Creates an RPC parameter based on settings provided in variableInfo. Available settings are: "id" (String), "type" (String), "default", "min", "max".
	 *
	 * @param variableInfo A Struct containing the information to create a RPC parameter.
	 * @param parameterGroup The parameter group the parameter should belong to.
	 * @return Returns a RPC parameter based on variableInfo.
	 */
	BaseLib::DeviceDescription::PParameter createRoleRpcParameter(BaseLib::PVariable& variableInfo, const std::string& baseVariableName, const PParameterGroup& parameterGroup);

	/**
	 * Gets a variable value directly from the device. This method is used as an inheritable hook method within getValue().
	 *
	 * @see getValue
	 * @param parameter The parameter to get the value for.
	 * @param channel The channel to get the value for.
	 * @param asynchronous If set to true, the method returns immediately, otherwise it waits for the response packet.
	 * @return Returns the requested value when asynchronous is false. If asynchronous is true, rpcVoid is returned.
	 */
	virtual PVariable getValueFromDevice(std::shared_ptr<Parameter>& parameter, int32_t channel, bool asynchronous) { return Variable::createError(-32601, "Method not implemented for this device family."); }

	/**
	 * This method returns the correct parameter set for a specified channel and parameter set type. It is necessary, because sometimes multiple parameter sets are available for one channel depending on family specific conditions. This method is a mandatory hook method.
	 *
	 * @param channel The channel index.
	 * @param type The parameter set type.
	 * @return Returns the parameter set for the specified channel and type.
	 */
	virtual PParameterGroup getParameterSet(int32_t channel, ParameterGroup::Type::Enum type) = 0;

	virtual PVariable getVariableDescription(PRpcClientInfo clientInfo, const PParameter& parameter, int32_t channel, ParameterGroup::Type::Enum type, int32_t index, const std::unordered_set<std::string>& fields);

	/**
	 * Overridable hook in initializeCentralConfig to set a custom default value. See BidCoSPeer for an implementation example. There it is used to conditionally set "AES_ACTIVE", depending on whether the physical interface supports it.
	 *
	 * @param parameter The parameter to set the default value for.
	 * @see initializeCentralConfig
	 */
	virtual void setDefaultValue(RpcConfigurationParameter& parameter);

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
		 * This hook is executed for each parameter in getAllConfig after the initial checks if the parameter exists etc. It can be used for example to set the value of the parameter.
		 *
		 * @param parameter The current parameter.
		 * @param channel The channel of the parameter.
		 * @param parameters The parameters struct which will be returned by getAllConfig.
		 * @return "true" means the function handled the parameter completely and there is nothing to do anymore in getAllConfig.
		 */
		virtual bool getAllConfigHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters) { return false; }

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
