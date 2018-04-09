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

#include "../../config.h"
#include "Peer.h"
#include "ServiceMessages.h"
#include "../BaseLib.h"
#include "../Security/Acls.h"

namespace BaseLib
{
namespace Systems
{

RpcConfigurationParameter::RpcConfigurationParameter(RpcConfigurationParameter const& rhs)
{
	rpcParameter = rhs.rpcParameter;
	databaseId = rhs.databaseId;
	_binaryData = rhs._binaryData;
	_partialBinaryData = rhs._partialBinaryData;
	_logicalData = rhs._logicalData;
    _room = rhs._room;
    _categories = rhs._categories;
}

RpcConfigurationParameter& RpcConfigurationParameter::operator=(const RpcConfigurationParameter& rhs)
{
	if(&rhs == this) return *this;
	rpcParameter = rhs.rpcParameter;
	databaseId = rhs.databaseId;
	_binaryData = rhs._binaryData;
	_partialBinaryData = rhs._partialBinaryData;
	_logicalData = rhs._logicalData;
    _room = rhs._room;
    _categories = rhs._categories;
	return *this;
}

void RpcConfigurationParameter::lock() noexcept
{
	_binaryDataMutex.lock();
}

void RpcConfigurationParameter::unlock() noexcept
{
	_binaryDataMutex.unlock();
}

std::vector<uint8_t>::size_type RpcConfigurationParameter::getBinaryDataSize() noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	return _binaryData.size();
}

std::vector<uint8_t> RpcConfigurationParameter::getBinaryData() noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	return _binaryData;
}

std::vector<uint8_t>& RpcConfigurationParameter::getBinaryDataReference() noexcept
{
	return _binaryData;
}

void RpcConfigurationParameter::setBinaryData(std::vector<uint8_t>& value) noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	_binaryData = value;
}

std::vector<uint8_t> RpcConfigurationParameter::getPartialBinaryData() noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	return _partialBinaryData;
}

std::vector<uint8_t>& RpcConfigurationParameter::getPartialBinaryDataReference() noexcept
{
	return _partialBinaryData;
}

void RpcConfigurationParameter::setPartialBinaryData(std::vector<uint8_t>& value) noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	_partialBinaryData = value;
}

BaseLib::PVariable RpcConfigurationParameter::getLogicalData() noexcept
{
	return _logicalData;
}

void RpcConfigurationParameter::setLogicalData(BaseLib::PVariable value) noexcept
{
	_logicalData = value;
}

bool RpcConfigurationParameter::equals(std::vector<uint8_t>& value) noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	return value == _binaryData;
}

ConfigDataBlock::ConfigDataBlock(ConfigDataBlock const& rhs)
{
	databaseId = rhs.databaseId;
	_binaryData = rhs._binaryData;
}

ConfigDataBlock& ConfigDataBlock::operator=(const ConfigDataBlock& rhs)
{
	if(&rhs == this) return *this;
	databaseId = rhs.databaseId;
	_binaryData = rhs._binaryData;
	return *this;
}

void ConfigDataBlock::lock() noexcept
{
	_binaryDataMutex.lock();
}

void ConfigDataBlock::unlock() noexcept
{
	_binaryDataMutex.unlock();
}

std::vector<uint8_t>::size_type ConfigDataBlock::getBinaryDataSize() noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	return _binaryData.size();
}

std::vector<uint8_t> ConfigDataBlock::getBinaryData() noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	return _binaryData;
}

std::vector<uint8_t>& ConfigDataBlock::getBinaryDataReference() noexcept
{
	return _binaryData;
}

void ConfigDataBlock::setBinaryData(std::vector<uint8_t>& value) noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	_binaryData = value;
}

bool ConfigDataBlock::equals(std::vector<uint8_t>& value) noexcept
{
	std::lock_guard<std::mutex> dataGuard(_binaryDataMutex);
	return value == _binaryData;
}

Peer::Peer(BaseLib::SharedObjects* baseLib, uint32_t parentId, IPeerEventSink* eventHandler)
{
	try
	{
        deleting = false;

		_bl = baseLib;
		_parentID = parentId;
		serviceMessages.reset(new ServiceMessages(baseLib, -1, 0, "", this));
		_lastPacketReceived = HelperFunctions::getTimeSeconds();
		_rpcDevice.reset();
		setEventHandler(eventHandler);
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

Peer::Peer(BaseLib::SharedObjects* baseLib, int32_t familyId, uint64_t id, int32_t address, std::string serialNumber, uint32_t parentId, IPeerEventSink* eventHandler) : Peer(baseLib, parentId, eventHandler)
{
	try
	{
		_peerID = id;
		_address = address;
		_serialNumber = serialNumber;
		if(serviceMessages)
		{
            serviceMessages->setFamilyId(familyId);
			serviceMessages->setPeerId(id);
			serviceMessages->setPeerSerial(serialNumber);
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

Peer::~Peer()
{
	serviceMessages->resetEventHandler();
}

void Peer::dispose()
{
	_disposing = true;
	_central.reset();
	_peersMutex.lock();
	_peers.clear();
	_peersMutex.unlock();
}

void Peer::homegearStarted()
{
	raiseEvent(_peerID, -1, std::shared_ptr<std::vector<std::string>>(new std::vector<std::string>{"INITIALIZED"}), PArray(new Array{PVariable(new Variable(true))}));
}

void Peer::homegearShuttingDown()
{
	raiseEvent(_peerID, -1, std::shared_ptr<std::vector<std::string>>(new std::vector<std::string>{"DISPOSING"}), PArray(new Array{PVariable(new Variable(true))}));
}

//Event handling
void Peer::raiseAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler)
{
	if(_eventHandler) ((IPeerEventSink*)_eventHandler)->onAddWebserverEventHandler(eventHandler, _webserverEventHandlers);
}

void Peer::raiseRemoveWebserverEventHandler()
{
	if(_eventHandler) ((IPeerEventSink*)_eventHandler)->onRemoveWebserverEventHandler(_webserverEventHandlers);
}

void Peer::raiseRPCEvent(uint64_t peerId, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values)
{
	if(_peerID == 0) return;
	if(_eventHandler) ((IPeerEventSink*)_eventHandler)->onRPCEvent(peerId, channel, deviceAddress, valueKeys, values);
}

void Peer::raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
{
	if(_eventHandler) ((IPeerEventSink*)_eventHandler)->onRPCUpdateDevice(id, channel, address, hint);
}

void Peer::raiseEvent(uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
{
	if(_peerID == 0) return;
	if(_eventHandler) ((IPeerEventSink*)_eventHandler)->onEvent(peerId, channel, variables, values);
}

void Peer::raiseRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait)
{
	if(_eventHandler) ((IPeerEventSink*)_eventHandler)->onRunScript(scriptInfo, wait);
}

BaseLib::PVariable Peer::raiseInvokeRpc(std::string& methodName, BaseLib::PArray& parameters)
{
    if(_eventHandler) return ((IPeerEventSink*)_eventHandler)->onInvokeRpc(methodName, parameters);
    else return std::make_shared<BaseLib::Variable>();
}
//End event handling

//ServiceMessages event handling
void Peer::onConfigPending(bool configPending)
{

}

void Peer::onEvent(uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
{
	raiseEvent(peerId, channel, variables, values);
}

void Peer::onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values)
{
	raiseRPCEvent(id, channel, deviceAddress, valueKeys, values);
}

void Peer::onSaveParameter(std::string name, uint32_t channel, std::vector<uint8_t>& data)
{
	try
	{
		if(_peerID == 0) return; //Peer not saved yet
		if(valuesCentral.find(channel) == valuesCentral.end())
		{
			//Service message variables sometimes just don't exist. So only output a debug message.
			if(channel != 0) _bl->out.printWarning("Warning: Could not set parameter " + name + " on channel " + std::to_string(channel) + " for peer " + std::to_string(_peerID) + ". Channel does not exist.");
			else _bl->out.printDebug("Debug: Could not set parameter " + name + " on channel " + std::to_string(channel) + " for peer " + std::to_string(_peerID) + ". Channel does not exist.");
			return;
		}
		if(valuesCentral.at(channel).find(name) == valuesCentral.at(channel).end())
		{
			if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Could not set parameter " + name + " on channel " + std::to_string(channel) + " for peer " + std::to_string(_peerID) + ". Parameter does not exist.");
			return;
		}
		RpcConfigurationParameter& parameter = valuesCentral.at(channel).at(name);
		if(parameter.equals(data)) return;
		parameter.setBinaryData(data);
		saveParameter(parameter.databaseId, ParameterGroup::Type::Enum::variables, channel, name, data);
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

std::shared_ptr<Database::DataTable> Peer::onGetServiceMessages()
{
	return _bl->db->getServiceMessages(_peerID);
}

void Peer::onSaveServiceMessage(Database::DataRow& data)
{
	_bl->db->saveServiceMessageAsynchronous(_peerID, data);
}

void Peer::onDeleteServiceMessage(uint64_t databaseID)
{
	_bl->db->deleteServiceMessage(databaseID);
}

void Peer::onEnqueuePendingQueues()
{
	try
	{
		if(pendingQueuesEmpty()) return;
		if(!(getRXModes() & HomegearDevice::ReceiveModes::Enum::always) && !(getRXModes() & HomegearDevice::ReceiveModes::Enum::wakeOnRadio)) return;
		enqueuePendingQueues();
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
//End ServiceMessages event handling

void Peer::setID(uint64_t id)
{
	if(_peerID == 0)
	{
		_peerID = id;
		if(serviceMessages) serviceMessages->setPeerId(id);
	}
	else _bl->out.printError("Cannot reset peer ID");
}

void Peer::setSerialNumber(std::string serialNumber)
{
	if(serialNumber.length() > 20) return;
	_serialNumber = serialNumber;
	if(serviceMessages) serviceMessages->setPeerSerial(serialNumber);
	if(_peerID > 0) save(true, false, false);
}

std::string Peer::getName(int32_t channel)
{
	std::lock_guard<std::mutex> namesGuard(_namesMutex);
	auto namesIterator = _names.find(channel);
	if(namesIterator == _names.end()) return "";
	return namesIterator->second;
}

void Peer::setName(int32_t channel, std::string value)
{
	if(channel != -1)
	{
		auto channelIterator = _rpcDevice->functions.find(channel);
		if(channelIterator == _rpcDevice->functions.end()) return;
	}

	std::lock_guard<std::mutex> namesGuard(_namesMutex);
	_names[channel] = value;

	std::ostringstream names;
	for(auto namePair : _names)
	{
		names << std::to_string(namePair.first) << "," << namePair.second << ";";
	}
	std::string namesString = names.str();

	saveVariable(1000, namesString);
}

std::set<int32_t> Peer::getChannelsInRoom(uint64_t roomId)
{
    std::set<int32_t> result;
    std::lock_guard<std::mutex> roomGuard(_roomMutex);
    for(auto& roomIterator : _rooms)
    {
        if(roomIterator.second == roomId) result.emplace(roomIterator.first);
    }
    return result;
}

uint64_t Peer::getRoom(int32_t channel)
{
    std::lock_guard<std::mutex> roomGuard(_roomMutex);
    auto roomIterator = _rooms.find(channel);
    if(roomIterator != _rooms.end()) return roomIterator->second;
    return 0;
}

bool Peer::hasRoomInChannels(uint64_t roomId)
{
    std::lock_guard<std::mutex> roomGuard(_roomMutex);
    for(auto& roomIterator : _rooms)
    {
        if(roomIterator.second == roomId) return true;
    }
    return false;
}

bool Peer::roomsSet()
{
    std::lock_guard<std::mutex> roomGuard(_roomMutex);
    for(auto& roomIterator : _rooms)
    {
        if(roomIterator.second != 0) return true;
    }
    return false;
}

bool Peer::setRoom(int32_t channel, uint64_t roomId)
{
	if(channel != -1)
	{
		auto channelIterator = _rpcDevice->functions.find(channel);
		if(channelIterator == _rpcDevice->functions.end()) return false;
	}

    std::lock_guard<std::mutex> roomGuard(_roomMutex);
    _rooms[channel] = roomId;

    std::ostringstream rooms;
    for(auto roomPair : _rooms)
    {
        rooms << std::to_string(roomPair.first) << "," << std::to_string(roomPair.second) << ";";
    }
    std::string roomString = rooms.str();

    saveVariable(1007, roomString);
    return true;
}

std::unordered_map<int32_t, std::set<uint64_t>> Peer::getCategories()
{
    std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
    return _categories;
}

std::set<uint64_t> Peer::getCategories(int32_t channel)
{
    std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
    auto categoryIterator = _categories.find(channel);
    if(categoryIterator != _categories.end()) return categoryIterator->second;

    return std::set<uint64_t>();
}

std::set<int32_t> Peer::getChannelsInCategory(uint64_t categoryId)
{
    std::set<int32_t> result;
    std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
    for(auto& categoryIterator : _categories)
    {
        if(categoryIterator.second.find(categoryId) != categoryIterator.second.end()) result.emplace(categoryIterator.first);
    }
    return result;
}

bool Peer::hasCategories()
{
    std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
    return !_categories.empty();
}

bool Peer::hasCategories(int32_t channel)
{
    std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
    auto categoryIterator = _categories.find(channel);
    return categoryIterator != _categories.end();
}

bool Peer::hasCategoryInChannels(uint64_t categoryId)
{
    if(categoryId == 0) return false;

    std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
    for(auto& categoryIterator : _categories)
    {
        if(categoryIterator.second.find(categoryId) != categoryIterator.second.end()) return true;
    }
    return false;
}

bool Peer::hasCategory(int32_t channel, uint64_t categoryId)
{
    if(categoryId == 0) return false;
    std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
    auto categoryIterator = _categories.find(channel);
    if(categoryIterator == _categories.end()) return false;
    return categoryIterator->second.find(categoryId) != categoryIterator->second.end();
}

bool Peer::addCategory(int32_t channel, uint64_t categoryId)
{
    if(categoryId == 0) return false;

    if(channel != -1)
    {
        auto channelIterator = _rpcDevice->functions.find(channel);
        if(channelIterator == _rpcDevice->functions.end()) return false;
    }

    std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
    _categories[channel].emplace(categoryId);

    std::ostringstream categories;
    for(auto categoryPair : _categories)
    {
        categories << categoryPair.first << "~";
        for(auto category : categoryPair.second)
        {
            categories << std::to_string(category) << ",";
        }
        categories << ";";
    }
    std::string categoryString = categories.str();

    saveVariable(1008, categoryString);
    return true;
}

bool Peer::removeCategory(int32_t channel, uint64_t categoryId)
{
    if(categoryId == 0) return false;

    std::lock_guard<std::mutex> categoriesGuard(_categoriesMutex);
    auto channelIterator = _categories.find(channel);
    if(channelIterator == _categories.end()) return false;

    channelIterator->second.erase(categoryId);
    if(channelIterator->second.empty()) _categories.erase(channel);

    std::ostringstream categories;
    for(auto categoryPair : _categories)
    {
        categories << categoryPair.first << "~";
        for(auto category : categoryPair.second)
        {
            categories << std::to_string(category) << ",";
        }
        categories << ";";
    }
    std::string categoryString = categories.str();

    saveVariable(1008, categoryString);
    return true;
}

HomegearDevice::ReceiveModes::Enum Peer::getRXModes()
{
	try
	{
		if(_rpcDevice)
		{
			_rxModes = _rpcDevice->receiveModes;
			std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator configIterator = configCentral.find(0);
			if(configIterator != configCentral.end())
			{
				std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = configIterator->second.find("WAKE_ON_RADIO");
				if(parameterIterator == configIterator->second.end()) parameterIterator = configIterator->second.find("BURST_RX");
				if(parameterIterator == configIterator->second.end()) parameterIterator = configIterator->second.find("LIVE_MODE_RX");
				if(parameterIterator != configIterator->second.end())
				{
					if(!parameterIterator->second.rpcParameter) return _rxModes;
					std::vector<uint8_t> data = parameterIterator->second.getBinaryData();
					if(parameterIterator->second.rpcParameter->convertFromPacket(data)->booleanValue)
					{
						_rxModes = (HomegearDevice::ReceiveModes::Enum)(_rxModes | HomegearDevice::ReceiveModes::Enum::wakeOnRadio);
					}
					else
					{
						_rxModes = (HomegearDevice::ReceiveModes::Enum)(_rxModes & (~HomegearDevice::ReceiveModes::Enum::wakeOnRadio));
					}
				}
			}
		}
		return _rxModes;
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
	return _rxModes;
}

void Peer::setLastPacketReceived()
{
	uint32_t now = HelperFunctions::getTimeSeconds();
	if(_lastPacketReceived == now) return;
	_lastPacketReceived = now;
	std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator valuesIterator = valuesCentral.find(0);
	if(valuesIterator != valuesCentral.end())
	{
		std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = valuesIterator->second.find("LAST_PACKET_RECEIVED");
		if(parameterIterator != valuesIterator->second.end() && parameterIterator->second.rpcParameter)
		{
			std::vector<uint8_t> parameterData;
			parameterIterator->second.rpcParameter->convertToPacket(std::make_shared<Variable>(_lastPacketReceived), parameterData);
			parameterIterator->second.setBinaryData(parameterData);
			if(parameterIterator->second.databaseId > 0) saveParameter(parameterIterator->second.databaseId, parameterData);
			else saveParameter(0, ParameterGroup::Type::Enum::variables, 0, "LAST_PACKET_RECEIVED", parameterData);

			// Don't raise event as this is not necessary and some programs like OpenHAB have problems with it
		}
	}
}

std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>> Peer::getPeers()
{
	_peersMutex.lock();
	try
	{
		std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>> peers = _peers;
		_peersMutex.unlock();
		return peers;
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
	_peersMutex.unlock();
	return std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>>();
}

std::shared_ptr<BasicPeer> Peer::getPeer(int32_t channel, std::string serialNumber, int32_t remoteChannel)
{
	_peersMutex.lock();
	try
	{
		if(_peers.find(channel) == _peers.end())
		{
			_peersMutex.unlock();
			return std::shared_ptr<BasicPeer>();
		}

		for(std::vector<std::shared_ptr<BasicPeer>>::iterator i = _peers[channel].begin(); i != _peers[channel].end(); ++i)
		{
			if((*i)->serialNumber.empty())
			{
				std::shared_ptr<ICentral> central(getCentral());
				if(central)
				{
					std::shared_ptr<Peer> peer(central->getPeer((*i)->id));
					if(peer) (*i)->serialNumber = peer->getSerialNumber();
				}
			}
			if((*i)->serialNumber == serialNumber && (remoteChannel < 0 || remoteChannel == (*i)->channel))
			{
				std::shared_ptr<BasicPeer> peer = *i;
				_peersMutex.unlock();
				return peer;
			}
		}
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
    _peersMutex.unlock();
	return std::shared_ptr<BasicPeer>();
}

std::shared_ptr<BasicPeer> Peer::getPeer(int32_t channel, int32_t address, int32_t remoteChannel)
{
	_peersMutex.lock();
	try
	{
		if(_peers.find(channel) == _peers.end())
		{
			_peersMutex.unlock();
			return std::shared_ptr<BasicPeer>();
		}

		for(std::vector<std::shared_ptr<BasicPeer>>::iterator i = _peers[channel].begin(); i != _peers[channel].end(); ++i)
		{
			if((*i)->address == address && (remoteChannel < 0 || remoteChannel == (*i)->channel))
			{
				std::shared_ptr<BasicPeer> peer = *i;
				_peersMutex.unlock();
				return peer;
			}
		}
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
    _peersMutex.unlock();
	return std::shared_ptr<BasicPeer>();
}

std::shared_ptr<BasicPeer> Peer::getPeer(int32_t channel, uint64_t id, int32_t remoteChannel)
{
	_peersMutex.lock();
	try
	{
		if(_peers.find(channel) == _peers.end())
		{
			_peersMutex.unlock();
			return std::shared_ptr<BasicPeer>();
		}

		bool modified = false;
		for(std::vector<std::shared_ptr<BasicPeer>>::iterator i = _peers[channel].begin(); i != _peers[channel].end(); ++i)
		{
			if((*i)->id == 0) //ID is not always available after pairing. So try to get ID if not there.
			{
				std::shared_ptr<Peer> peer1 = getCentral()->getPeer((*i)->serialNumber);
				std::shared_ptr<Peer> peer2 = getCentral()->getPeer((*i)->address);
				if(peer1)
				{
					(*i)->id = peer1->getID();
					modified = true;
				}
				else if(peer2)
				{
					(*i)->id = peer2->getID();
					modified = true;
				}
				else if((*i)->isVirtual && (*i)->address == getCentral()->getAddress())
				{
					(*i)->id = 0xFFFFFFFFFFFFFFFF;
					modified = true;
				}
			}
			if((*i)->id == id && (remoteChannel < 0 || remoteChannel == (*i)->channel))
			{
				std::shared_ptr<BasicPeer> peer = *i;
				_peersMutex.unlock();
				if(modified) savePeers();
				return peer;
			}
		}
		_peersMutex.unlock();
		if(modified) savePeers();
		return std::shared_ptr<BasicPeer>();
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
    _peersMutex.unlock();
	return std::shared_ptr<BasicPeer>();
}

void Peer::updatePeer(uint64_t oldId, uint64_t newId)
{
	try
	{
		bool changed = false;
		{
			std::lock_guard<std::mutex> peersGuard(_peersMutex);
			for(std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>>::iterator i = _peers.begin(); i != _peers.end(); ++i)
			{
				for(std::vector<std::shared_ptr<BasicPeer>>::iterator j = i->second.begin(); j != i->second.end(); ++j)
				{
					if((*j)->id == oldId)
					{
						(*j)->id = newId;
						changed = true;
					}
				}
			}
		}
		if(changed) savePeers();
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
}

void Peer::deleteFromDatabase()
{
	try
	{
		deleting = true;
		std::string dataId = "";
		_bl->db->deleteMetadata(_peerID, _serialNumber, dataId);
		_bl->db->deletePeer(_peerID);
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

void Peer::initializeCentralConfig()
{
	std::string savepointName("PeerConfig" + std::to_string(_peerID));
	try
	{
		if(!_rpcDevice)
		{
			_bl->out.printWarning("Warning: Tried to initialize peer's central config without rpcDevice being set.");
			return;
		}
		_bl->db->createSavepointAsynchronous(savepointName);
		for(Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i)
		{
			initializeMasterSet(i->first, i->second->configParameters);
			initializeValueSet(i->first, i->second->variables);
			for(std::vector<PFunction>::iterator j = i->second->alternativeFunctions.begin(); j != i->second->alternativeFunctions.end(); ++j)
			{
				initializeMasterSet(i->first, (*j)->configParameters);
				initializeValueSet(i->first, (*j)->variables);
			}
		}
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
    _bl->db->releaseSavepointAsynchronous(savepointName);
}

void Peer::initializeMasterSet(int32_t channel, PConfigParameters masterSet)
{
	try
	{
		if(!masterSet || masterSet->parameters.empty()) return;
		std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator channelIterator = configCentral.find(channel);
		if(channelIterator == configCentral.end()) channelIterator = configCentral.emplace(channel, std::unordered_map<std::string, RpcConfigurationParameter>()).first;
		for(Parameters::iterator j = masterSet->parameters.begin(); j != masterSet->parameters.end(); ++j)
		{
			if(!j->second) continue;
			if(!j->second->id.empty() && channelIterator->second.find(j->second->id) == channelIterator->second.end())
			{
				RpcConfigurationParameter parameter;
				parameter.rpcParameter = j->second;
				setDefaultValue(parameter);
				channelIterator->second.emplace(j->second->id, parameter);
				std::vector<uint8_t> data = parameter.getBinaryData();
				saveParameter(0, ParameterGroup::Type::config, channel, j->second->id, data);
			}
		}
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
}

void Peer::initializeValueSet(int32_t channel, PVariables valueSet)
{
	try
	{
		if(!valueSet || valueSet->parameters.empty()) return;
        auto channelIterator = valuesCentral.find(channel);
        if(channelIterator == valuesCentral.end())
        {
            channelIterator = valuesCentral.emplace(channel, std::unordered_map<std::string, RpcConfigurationParameter>()).first;
        }
		for(Parameters::iterator j = valueSet->parameters.begin(); j != valueSet->parameters.end(); ++j)
		{
			if(!j->second) continue;
			if(!j->second->id.empty() && channelIterator->second.find(j->second->id) == channelIterator->second.end())
			{
				RpcConfigurationParameter parameter;
				parameter.rpcParameter = j->second;
				setDefaultValue(parameter);
                channelIterator->second.emplace(j->second->id, parameter);
				std::vector<uint8_t> data = parameter.getBinaryData();
				saveParameter(0, ParameterGroup::Type::variables, channel, j->second->id, data);
			}
		}
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
}

void Peer::setDefaultValue(RpcConfigurationParameter& parameter)
{
	try
	{
		std::vector<uint8_t> parameterData;
		if(!convertToPacketHook(parameter.rpcParameter, parameter.rpcParameter->logical->getDefaultValue(), parameterData))	parameter.rpcParameter->convertToPacket(parameter.rpcParameter->logical->getDefaultValue(), parameterData);
		parameter.setBinaryData(parameterData);
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
}

void Peer::save(bool savePeer, bool variables, bool centralConfig)
{
	std::string savepointName("peer_54" + std::to_string(_parentID) + std::to_string(_peerID));
	try
	{
		if(deleting || (isTeam() && !_saveTeam)) return;

		if(savePeer)
		{
			uint64_t result = _bl->db->savePeer(_peerID, _parentID, _address, _serialNumber, _deviceType);
			if(_peerID == 0 && result > 0) setID(result);
		}
		if(variables || centralConfig) _bl->db->createSavepointAsynchronous(savepointName);
		if(variables) saveVariables();
		if(centralConfig) saveConfig();
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
    if(variables || centralConfig) _bl->db->releaseSavepointAsynchronous(savepointName);
}

void Peer::saveParameter(uint32_t parameterID, std::vector<uint8_t>& value)
{
	try
	{
		if(parameterID == 0)
		{
			if(!isTeam() || _saveTeam) _bl->out.printError("Error: Peer " + std::to_string(_peerID) + ": Tried to save parameter without parameterID");
			return;
		}
		Database::DataRow data;
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(value)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(parameterID)));
		_bl->db->savePeerParameterAsynchronous(data);
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

void Peer::saveParameter(uint32_t parameterID, uint32_t address, std::vector<uint8_t>& value)
{
	try
	{
		if(parameterID > 0)
		{
			saveParameter(parameterID, value);
			return;
		}
		if(_peerID == 0 || (isTeam() && !_saveTeam)) return;
		//Creates a new entry for parameter in database
		Database::DataRow data;
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(0)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(address)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(0)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(0)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(std::string(""))));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(value)));
		_bl->db->savePeerParameterAsynchronous(data);
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

void Peer::saveParameter(uint32_t parameterID, ParameterGroup::Type::Enum parameterSetType, uint32_t channel, std::string parameterName, std::vector<uint8_t>& value, int32_t remoteAddress, uint32_t remoteChannel)
{
	try
	{
		if(parameterID > 0)
		{
			saveParameter(parameterID, value);
			return;
		}
		if(_peerID == 0 || (isTeam() && !_saveTeam)) return;
		//Creates a new entry for parameter in database
		Database::DataRow data;
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn((uint32_t)parameterSetType)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(channel)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(remoteAddress)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(remoteChannel)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(parameterName)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(value)));
		_bl->db->savePeerParameterAsynchronous(data);
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

void Peer::loadVariables(ICentral* central, std::shared_ptr<BaseLib::Database::DataTable>& rows)
{
	try
	{
		if(!rows) return;
		for(BaseLib::Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row)
		{
			_variableDatabaseIDs[row->second.at(2)->intValue] = row->second.at(0)->intValue;
			switch(row->second.at(2)->intValue)
			{
			case 1000:
				{
					std::vector<std::string> nameStrings = BaseLib::HelperFunctions::splitAll(row->second.at(4)->textValue, ';');
					for(auto nameString : nameStrings)
					{
						if(nameString.empty()) continue;
						auto namePair = BaseLib::HelperFunctions::splitFirst(nameString, ',');
						if(namePair.second.empty()) _names[-1] = namePair.first;
						else
						{
							int32_t channel = BaseLib::Math::getNumber(namePair.first);
							_names[channel] = namePair.second;
						}
					}
					break;
				}
			case 1001:
				_firmwareVersion = row->second.at(3)->intValue;
				break;
			case 1002:
				_deviceType = row->second.at(3)->intValue;
				if(_deviceType == (uint32_t)0xFFFFFFFF)
				{
					_bl->out.printError("Error loading peer " + std::to_string(_peerID) + ": Device type unknown: 0x" + BaseLib::HelperFunctions::getHexString(row->second.at(3)->intValue) + " Firmware version: " + std::to_string(_firmwareVersion));
				}
				break;
			case 1003:
				_firmwareVersionString = row->second.at(4)->textValue;
				break;
			case 1004:
				_ip = row->second.at(4)->textValue;
				break;
			case 1005:
				_idString = row->second.at(4)->textValue;
				break;
			case 1006:
				_typeString = row->second.at(4)->textValue;
				break;
			case 1007:
				{
					std::vector<std::string> roomStrings = BaseLib::HelperFunctions::splitAll(row->second.at(4)->textValue, ';');
					for(auto roomString : roomStrings)
					{
						if(roomString.empty()) continue;
						auto roomPair = BaseLib::HelperFunctions::splitFirst(roomString, ',');
						int32_t channel = BaseLib::Math::getNumber(roomPair.first);
						uint64_t room = BaseLib::Math::getNumber64(roomPair.second);
						if(room != 0) _rooms[channel] = room;
					}
					break;
				}
			case 1008:
				{
					std::vector<std::string> categoryStrings = BaseLib::HelperFunctions::splitAll(row->second.at(4)->textValue, ';');
					for(auto categoryString : categoryStrings)
					{
						if(categoryString.empty()) continue;
						auto categoryPair = BaseLib::HelperFunctions::splitFirst(categoryString, '~');
						if(categoryPair.first.empty() || categoryPair.second.empty()) continue;
						int32_t channel = BaseLib::Math::getNumber(categoryPair.first);
						auto categoryStrings2 = BaseLib::HelperFunctions::splitAll(categoryPair.second, ',');
						for(auto categoryString2 : categoryStrings2)
						{
							uint64_t category = BaseLib::Math::getNumber64(categoryString2);
							if(category != 0) _categories[channel].emplace(category);
						}
					}
					break;
				}
			}
		}
		return;
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
}

void Peer::saveVariables()
{
	try
	{
		if(_peerID == 0 || (isTeam() && !_saveTeam)) return;
		saveVariable(1001, _firmwareVersion);
		saveVariable(1002, (int32_t)_deviceType);
		saveVariable(1003, _firmwareVersionString);
		saveVariable(1004, _ip);
		saveVariable(1005, _idString);
		saveVariable(1006, _typeString);
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
}

void Peer::saveVariable(uint32_t index, int32_t intValue)
{
	try
	{
		if(isTeam() && !_saveTeam) return;
		bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
			_bl->db->savePeerVariableAsynchronous(data);
		}
		else
		{
			if(_peerID == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			_bl->db->savePeerVariableAsynchronous(data);
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

void Peer::saveVariable(uint32_t index, int64_t intValue)
{
	try
	{
		if(isTeam() && !_saveTeam) return;
		bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
			_bl->db->savePeerVariableAsynchronous(data);
		}
		else
		{
			if(_peerID == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			_bl->db->savePeerVariableAsynchronous(data);
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

void Peer::saveVariable(uint32_t index, std::string& stringValue)
{
	try
	{
		if(isTeam() && !_saveTeam) return;
		bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(stringValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
			_bl->db->savePeerVariableAsynchronous(data);
		}
		else
		{
			if(_peerID == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(stringValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			_bl->db->savePeerVariableAsynchronous(data);
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

void Peer::saveVariable(uint32_t index, std::vector<char>& binaryValue)
{
	try
	{
		if(isTeam() && !_saveTeam) return;
		bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
			_bl->db->savePeerVariableAsynchronous(data);
		}
		else
		{
			if(_peerID == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
			_bl->db->savePeerVariableAsynchronous(data);
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

void Peer::saveVariable(uint32_t index, std::vector<uint8_t>& binaryValue)
{
	try
	{
		if(isTeam() && !_saveTeam) return;
		bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
			_bl->db->savePeerVariableAsynchronous(data);
		}
		else
		{
			if(_peerID == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
			_bl->db->savePeerVariableAsynchronous(data);
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

void Peer::saveConfig()
{
	try
	{
		if(_peerID == 0 || (isTeam() && !_saveTeam)) return;
		for(std::unordered_map<uint32_t, ConfigDataBlock>::iterator i = binaryConfig.begin(); i != binaryConfig.end(); ++i)
		{
			std::string emptyString;
			std::vector<uint8_t> configData = i->second.getBinaryData();
			if(i->second.databaseId > 0) saveParameter(i->second.databaseId, configData);
			else saveParameter(0, i->first, configData);
		}
		for(std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator i = configCentral.begin(); i != configCentral.end(); ++i)
		{
			for(std::unordered_map<std::string, RpcConfigurationParameter>::iterator j = i->second.begin(); j != i->second.end(); ++j)
			{
				if(j->first.empty())
				{
					_bl->out.printError("Error: Parameter has no id.");
					continue;
				}
				std::vector<uint8_t> data = j->second.getBinaryData();
				if(j->second.databaseId > 0) saveParameter(j->second.databaseId, data);
				else saveParameter(0, ParameterGroup::Type::Enum::config, i->first, j->first, data);
			}
		}
		for(std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator i = valuesCentral.begin(); i != valuesCentral.end(); ++i)
		{
			for(std::unordered_map<std::string, RpcConfigurationParameter>::iterator j = i->second.begin(); j != i->second.end(); ++j)
			{
				if(j->first.empty())
				{
					_bl->out.printError("Error: Parameter has no id.");
					continue;
				}
				std::vector<uint8_t> data = j->second.getBinaryData();
				if(j->second.databaseId > 0) saveParameter(j->second.databaseId, data);
				else saveParameter(0, ParameterGroup::Type::Enum::variables, i->first, j->first, data);
			}
		}
		for(std::unordered_map<uint32_t, std::unordered_map<int32_t, std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>>>::iterator i = linksCentral.begin(); i != linksCentral.end(); ++i)
		{
			for(std::unordered_map<int32_t, std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>>::iterator j = i->second.begin(); j != i->second.end(); ++j)
			{
				for(std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator k = j->second.begin(); k != j->second.end(); ++k)
				{
					for(std::unordered_map<std::string, RpcConfigurationParameter>::iterator l = k->second.begin(); l != k->second.end(); ++l)
					{
						if(l->first.empty())
						{
							_bl->out.printError("Error: Parameter has no id.");
							continue;
						}
						std::vector<uint8_t> data = l->second.getBinaryData();
						if(l->second.databaseId > 0) saveParameter(l->second.databaseId, data);
						else saveParameter(0, ParameterGroup::Type::Enum::link, i->first, l->first, data, j->first, k->first);
					}
				}
			}
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

void Peer::loadConfig()
{
	try
	{
		struct ParameterInfo
		{
			ParameterGroup::Type::Enum parameterGroupType;
			uint32_t channel;
			int32_t remoteAddress;
			int32_t remoteChannel;
			std::string parameterName;
			RpcConfigurationParameter parameter;
			PFunction function;
		};

		Database::DataRow data;
		std::shared_ptr<BaseLib::Database::DataTable> rows = _bl->db->getPeerParameters(_peerID);
		std::shared_ptr<ParameterInfo> parameterGroupSelector;
		std::vector<std::shared_ptr<ParameterInfo>> parameters;
        parameters.reserve(rows->size());
		for(Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row)
		{
			uint64_t databaseId = row->second.at(0)->intValue;
			ParameterGroup::Type::Enum parameterGroupType = (ParameterGroup::Type::Enum)row->second.at(2)->intValue;

			if(parameterGroupType == ParameterGroup::Type::Enum::none)
			{
				uint32_t index = row->second.at(3)->intValue;
				ConfigDataBlock& config = binaryConfig[index];
				config.databaseId = databaseId;
				std::vector<uint8_t> configData;
				configData.insert(configData.begin(), row->second.at(7)->binaryValue->begin(), row->second.at(7)->binaryValue->end());
				config.setBinaryData(configData);
			}
			else
			{
				auto parameterInfo = std::make_shared<ParameterInfo>();
				parameterInfo->parameterGroupType = parameterGroupType;
				parameterInfo->channel = row->second.at(3)->intValue;
				parameterInfo->remoteAddress = row->second.at(4)->intValue;
				parameterInfo->remoteChannel = row->second.at(5)->intValue;
				parameterInfo->parameterName = row->second.at(6)->textValue;
				if(parameterInfo->parameterName.empty())
				{
					_bl->out.printCritical("Critical: Added central config parameter without id. Device: " + std::to_string(_peerID) + " Channel: " + std::to_string(parameterInfo->channel));
					data.clear();
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(std::string(""))));
					_bl->db->deletePeerParameter(_peerID, data);
					continue;
				}

				parameterInfo->parameter.databaseId = databaseId;
				std::vector<uint8_t> parameterData;
				parameterData.insert(parameterData.begin(), row->second.at(7)->binaryValue->begin(), row->second.at(7)->binaryValue->end());
				parameterInfo->parameter.setBinaryData(parameterData);
				if(!_rpcDevice)
				{
					_bl->out.printCritical("Critical: No xml-rpc device found for peer " + std::to_string(_peerID) + ".");
					continue;
				}

                { // Rooms / Categories
                    parameterInfo->parameter.setRoom((uint64_t)row->second.at(8)->intValue);

                    std::vector<std::string> categoryStrings = BaseLib::HelperFunctions::splitAll(row->second.at(9)->textValue, ',');
                    for(auto categoryString : categoryStrings)
                    {
                        if(categoryString.empty()) continue;
                        uint64_t category = (uint64_t)BaseLib::Math::getNumber64(categoryString);
                        if(category != 0) parameterInfo->parameter.addCategory(category);
                    }
                }

				Functions::iterator functionIterator = _rpcDevice->functions.find(parameterInfo->channel);
				if(functionIterator == _rpcDevice->functions.end())
				{
					_bl->out.printError("Error: Added central config parameter with unknown channel. Device: " + std::to_string(_peerID) + " Channel: " + std::to_string(parameterInfo->channel));
					data.clear();
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn((int64_t)parameterInfo->parameterGroupType)));
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(parameterInfo->channel)));
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(parameterInfo->parameterName)));
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(parameterInfo->remoteAddress)));
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(parameterInfo->remoteChannel)));
					_bl->db->deletePeerParameter(_peerID, data);
					continue;
				}
				parameterInfo->function = functionIterator->second;
				if(parameterGroupType == ParameterGroup::Type::Enum::config && parameterInfo->function->parameterGroupSelector && !parameterInfo->function->alternativeFunctions.empty() && parameterInfo->parameterName == parameterInfo->function->parameterGroupSelector->id)
				{
					std::vector<uint8_t> parameterData = parameterInfo->parameter.getBinaryData();
					int32_t index = parameterInfo->function->parameterGroupSelector->convertFromPacket(parameterData)->integerValue;
					if(parameterInfo->function->parameterGroupSelector->logical->type != ILogical::Type::Enum::tBoolean && index > (signed)parameterInfo->function->alternativeFunctions.size())
					{
						_bl->out.printError("Error: Parameter group selector \"" + parameterInfo->parameterName + "\" has invalid value (" + std::to_string(parameterData.back()) + "). Peer: " + std::to_string(_peerID) + ".");
						continue;
					}
					parameterInfo->parameter.rpcParameter = parameterInfo->function->parameterGroupSelector;
					parameterGroupSelector = parameterInfo;
				}
				parameters.push_back(parameterInfo);
			}
		}

		for(std::vector<std::shared_ptr<ParameterInfo>>::iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			if(parameterGroupSelector && (*i)->function->parameterGroupSelector && !(*i)->function->alternativeFunctions.empty())
			{
				std::vector<uint8_t> parameterData = parameterGroupSelector->parameter.getBinaryData();
				int32_t index = parameterGroupSelector->parameter.rpcParameter->logical->type == ILogical::Type::Enum::tBoolean ? (int32_t)parameterGroupSelector->parameter.rpcParameter->convertFromPacket(parameterData)->booleanValue : parameterGroupSelector->parameter.rpcParameter->convertFromPacket(parameterData)->integerValue;
				if(index == 0)
				{
					if((*i)->parameterGroupType == ParameterGroup::Type::Enum::config)
					{
						Parameters::iterator parameterIterator = (*i)->function->configParameters->parameters.find((*i)->parameterName);
						if(parameterIterator != (*i)->function->configParameters->parameters.end()) (*i)->parameter.rpcParameter = parameterIterator->second;
					}
					else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::variables)
					{
						Parameters::iterator parameterIterator = (*i)->function->variables->parameters.find((*i)->parameterName);
						if(parameterIterator != (*i)->function->variables->parameters.end()) (*i)->parameter.rpcParameter = parameterIterator->second;
					}
					else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::link)
					{
						Parameters::iterator parameterIterator = (*i)->function->linkParameters->parameters.find((*i)->parameterName);
						if(parameterIterator != (*i)->function->linkParameters->parameters.end()) (*i)->parameter.rpcParameter = parameterIterator->second;
					}
				}
				else
				{
					index--;
					PFunction alternativeFunction = (*i)->function->alternativeFunctions.at(index);

					if((*i)->parameterGroupType == ParameterGroup::Type::Enum::config)
					{
						Parameters::iterator parameterIterator = alternativeFunction->configParameters->parameters.find((*i)->parameterName);
						if(parameterIterator != alternativeFunction->configParameters->parameters.end()) (*i)->parameter.rpcParameter = parameterIterator->second;
					}
					else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::variables)
					{
						Parameters::iterator parameterIterator = alternativeFunction->variables->parameters.find((*i)->parameterName);
						if(parameterIterator != alternativeFunction->variables->parameters.end()) (*i)->parameter.rpcParameter = parameterIterator->second;
					}
					else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::link)
					{
						Parameters::iterator parameterIterator = alternativeFunction->linkParameters->parameters.find((*i)->parameterName);
						if(parameterIterator != alternativeFunction->linkParameters->parameters.end()) (*i)->parameter.rpcParameter = parameterIterator->second;
					}
				}
			}
			else
			{
				if((*i)->parameterGroupType == ParameterGroup::Type::Enum::config)
				{
					Parameters::iterator parameterIterator = (*i)->function->configParameters->parameters.find((*i)->parameterName);
					if(parameterIterator != (*i)->function->configParameters->parameters.end()) (*i)->parameter.rpcParameter = parameterIterator->second;
				}
				else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::variables)
				{
					Parameters::iterator parameterIterator = (*i)->function->variables->parameters.find((*i)->parameterName);
					if(parameterIterator != (*i)->function->variables->parameters.end()) (*i)->parameter.rpcParameter = parameterIterator->second;
				}
				else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::link)
				{
					Parameters::iterator parameterIterator = (*i)->function->linkParameters->parameters.find((*i)->parameterName);
					if(parameterIterator != (*i)->function->linkParameters->parameters.end()) (*i)->parameter.rpcParameter = parameterIterator->second;
				}
			}

			if(!(*i)->parameter.rpcParameter)
			{
				if(!parameterGroupSelector || !(*i)->function->parameterGroupSelector || (*i)->function->alternativeFunctions.empty()) _bl->out.printWarning("Warning: Deleting parameter " + (*i)->parameterName + ", because no corresponding RPC parameter was found. Peer: " + std::to_string(_peerID) + " Channel: " + std::to_string((*i)->channel) + " Parameter set type: " + std::to_string((uint32_t)((*i)->parameterGroupType)));
				Database::DataRow data;
				data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_peerID)));
				data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn((int32_t)((*i)->parameterGroupType))));
				data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn((*i)->channel)));
				data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn((*i)->parameterName)));
				if((*i)->parameterGroupType == ParameterGroup::Type::Enum::config)
				{
					configCentral[(*i)->channel].erase((*i)->parameterName);
				}
				else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::variables)
				{
					valuesCentral[(*i)->channel].erase((*i)->parameterName);
				}
				else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::link)
				{
					linksCentral[(*i)->channel][(*i)->remoteAddress][(*i)->remoteChannel].erase((*i)->parameterName);
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn((*i)->remoteAddress)));
					data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn((*i)->remoteChannel)));
				}
				_bl->db->deletePeerParameter(_peerID, data);
			}
			else
			{
				if((*i)->parameterGroupType == ParameterGroup::Type::Enum::config) configCentral[(*i)->channel].emplace((*i)->parameterName, (*i)->parameter);
				else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::variables)
				{
					if((*i)->parameter.rpcParameter->resetAfterRestart)
					{
						std::vector<uint8_t> parameterData;
						(*i)->parameter.rpcParameter->convertToPacket((*i)->parameter.rpcParameter->logical->getDefaultValue(), parameterData);
						(*i)->parameter.setBinaryData(parameterData);
					}
					valuesCentral[(*i)->channel][(*i)->parameterName] = (*i)->parameter;
				}
				else if((*i)->parameterGroupType == ParameterGroup::Type::Enum::link) linksCentral[(*i)->channel][(*i)->remoteAddress][(*i)->remoteChannel].emplace((*i)->parameterName, (*i)->parameter);
			}
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

void Peer::initializeTypeString()
{
	try
	{
		if(!_rpcDevice) return;
		if(!_typeString.empty())
		{
			_rpcTypeString = _typeString;
			return;
		}
		PSupportedDevice rpcDeviceType = _rpcDevice->getType(_deviceType, _firmwareVersion);
		if(rpcDeviceType) _rpcTypeString = rpcDeviceType->id;
		else if(_deviceType == 0) _rpcTypeString = "HM-RCV-50"; //Central
		else if(!_rpcDevice->supportedDevices.empty()) _rpcTypeString = _rpcDevice->supportedDevices.at(0)->id;
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

bool Peer::setVariableRoom(int32_t channel, std::string& variableName, uint64_t roomId)
{
	try
	{
        auto channelIterator = valuesCentral.find(channel);
        if(channelIterator == valuesCentral.end()) return false;
        auto variableIterator = channelIterator->second.find(variableName);
        if(variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;
        variableIterator->second.setRoom(roomId);

        Database::DataRow data;
        data.push_back(std::make_shared<Database::DataColumn>(roomId));
        data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.databaseId));
        _bl->db->savePeerParameterRoomAsynchronous(data);

        return true;
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
	return false;
}

void Peer::removeRoomFromVariables(uint64_t roomId)
{
    try
    {
        for(auto& channelIterator : valuesCentral)
        {
            for(auto& variableIterator : channelIterator.second)
            {
                if(!variableIterator.second.rpcParameter || variableIterator.second.databaseId == 0) continue;
                if(variableIterator.second.getRoom() == roomId)
                {
                    variableIterator.second.setRoom(0);

                    Database::DataRow data;
                    data.push_back(std::make_shared<Database::DataColumn>(roomId));
                    data.push_back(std::make_shared<Database::DataColumn>(variableIterator.second.databaseId));
                    _bl->db->savePeerParameterRoomAsynchronous(data);
                }
            }
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

uint64_t Peer::getVariableRoom(int32_t channel, const std::string& variableName)
{
	try
	{
        auto channelIterator = valuesCentral.find(channel);
        if(channelIterator == valuesCentral.end()) return 0;
        auto variableIterator = channelIterator->second.find(variableName);
        if(variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return 0;

        return variableIterator->second.getRoom();
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
	return 0;
}

bool Peer::addCategoryToVariable(int32_t channel, std::string& variableName, uint64_t categoryId)
{
	try
	{
        auto channelIterator = valuesCentral.find(channel);
        if(channelIterator == valuesCentral.end()) return false;
        auto variableIterator = channelIterator->second.find(variableName);
        if(variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

        variableIterator->second.addCategory(categoryId);

        Database::DataRow data;
        data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.getCategoryString()));
        data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.databaseId));
        _bl->db->savePeerParameterCategoriesAsynchronous(data);

        return true;
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
    return false;
}

bool Peer::removeCategoryFromVariable(int32_t channel, std::string& variableName, uint64_t categoryId)
{
	try
	{
        auto channelIterator = valuesCentral.find(channel);
        if(channelIterator == valuesCentral.end()) return false;
        auto variableIterator = channelIterator->second.find(variableName);
        if(variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

        variableIterator->second.removeCategory(categoryId);

        Database::DataRow data;
        data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.getCategoryString()));
        data.push_back(std::make_shared<Database::DataColumn>(variableIterator->second.databaseId));
        _bl->db->savePeerParameterCategoriesAsynchronous(data);

        return true;
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
    return false;
}

void Peer::removeCategoryFromVariables(uint64_t categoryId)
{
    try
    {
        for(auto& channelIterator : valuesCentral)
        {
            for(auto& variableIterator : channelIterator.second)
            {
                if(!variableIterator.second.rpcParameter || variableIterator.second.databaseId == 0) continue;
                variableIterator.second.removeCategory(categoryId);

                Database::DataRow data;
                data.push_back(std::make_shared<Database::DataColumn>(variableIterator.second.getCategoryString()));
                data.push_back(std::make_shared<Database::DataColumn>(variableIterator.second.databaseId));
                _bl->db->savePeerParameterCategoriesAsynchronous(data);
            }
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

std::set<uint64_t> Peer::getVariableCategories(int32_t channel, std::string& variableName)
{
	try
	{
        auto channelIterator = valuesCentral.find(channel);
        if(channelIterator == valuesCentral.end()) std::set<uint64_t>();
        auto variableIterator = channelIterator->second.find(variableName);
        if(variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) std::set<uint64_t>();

        return variableIterator->second.getCategories();
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
	return std::set<uint64_t>();
}

bool Peer::variableHasCategory(int32_t channel, const std::string& variableName, uint64_t categoryId)
{
	try
	{
        auto channelIterator = valuesCentral.find(channel);
        if(channelIterator == valuesCentral.end()) return false;
        auto variableIterator = channelIterator->second.find(variableName);
        if(variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

        return variableIterator->second.hasCategory(categoryId);
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
	return false;
}

bool Peer::variableHasCategories(int32_t channel, const std::string& variableName)
{
    try
    {
        auto channelIterator = valuesCentral.find(channel);
        if(channelIterator == valuesCentral.end()) return false;
        auto variableIterator = channelIterator->second.find(variableName);
        if(variableIterator == channelIterator->second.end() || !variableIterator->second.rpcParameter || variableIterator->second.databaseId == 0) return false;

        return variableIterator->second.hasCategories();
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
    return false;
}

//RPC methods
PVariable Peer::getAllConfig(PRpcClientInfo clientInfo)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(!clientInfo) clientInfo.reset(new RpcClientInfo());
		PVariable config(new Variable(VariableType::tStruct));

		config->structValue->insert(StructElement("FAMILY", PVariable(new Variable((uint32_t)getCentral()->deviceFamily()))));
		config->structValue->insert(StructElement("ID", PVariable(new Variable((uint32_t)_peerID))));
		config->structValue->insert(StructElement("ADDRESS", PVariable(new Variable(_serialNumber))));
		config->structValue->insert(StructElement("TYPE", PVariable(new Variable(_rpcTypeString))));
		config->structValue->insert(StructElement("TYPE_ID", PVariable(new Variable(_deviceType))));
		config->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(-1)))));
		PVariable channels(new Variable(VariableType::tArray));
		for(Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i)
		{
			if(!i->second) continue;
			if(!i->second->countFromVariable.empty() && configCentral[0].find(i->second->countFromVariable) != configCentral[0].end())
			{
				std::vector<uint8_t> parameterData = configCentral[0][i->second->countFromVariable].getBinaryData();
				if(parameterData.size() > 0 && i->first >= i->second->channel + parameterData.at(parameterData.size() - 1)) continue;
			}
			PVariable channel(new Variable(VariableType::tStruct));
			channel->structValue->insert(StructElement("INDEX", PVariable(new Variable(i->first))));
			channel->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(i->first)))));
			channel->structValue->insert(StructElement("TYPE", PVariable(new Variable(i->second->type))));

			PVariable parameters(new Variable(VariableType::tStruct));
			channel->structValue->insert(StructElement("PARAMSET", parameters));
			channels->arrayValue->push_back(channel);

			PParameterGroup parameterGroup = getParameterSet(i->first, ParameterGroup::Type::config);
			if(!parameterGroup) continue;

			for(Parameters::iterator j = parameterGroup->parameters.begin(); j != parameterGroup->parameters.end(); ++j)
			{
				if(!j->second || j->second->id.empty() || !j->second->visible) continue;
				if(!j->second->visible && !j->second->service && !j->second->internal  && !j->second->transform)
				{
					_bl->out.printDebug("Debug: Omitting parameter " + j->second->id + " because of it's ui flag.");
					continue;
				}
				std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator configCentralIterator = configCentral.find(i->first);
				if(configCentralIterator == configCentral.end()) continue;
				std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = configCentralIterator->second.find(j->second->id);
				if(parameterIterator == configCentralIterator->second.end()) continue;
#ifdef CCU2
				if(j->second->logical->type == ILogical::Type::tInteger64) continue;
#endif

				PVariable element(new Variable(VariableType::tStruct));
				PVariable value;
				if(j->second->readable)
				{
					std::vector<uint8_t> parameterData = parameterIterator->second.getBinaryData();
					if(!convertFromPacketHook(j->second, parameterData, value)) value = j->second->convertFromPacket(parameterData);
					if(j->second->password) value.reset(new Variable(value->type));
					if(!value) continue;
					element->structValue->insert(StructElement("VALUE", value));
				}

				if(j->second->logical->type == ILogical::Type::tBoolean)
				{
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("BOOL")))));
				}
				else if(j->second->logical->type == ILogical::Type::tString)
				{
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRING")))));
				}
				else if(j->second->logical->type == ILogical::Type::tInteger)
				{
					LogicalInteger* parameter = (LogicalInteger*)j->second->logical.get();
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER")))));
					element->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
					element->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));

					if(!parameter->specialValuesStringMap.empty())
					{
						PVariable specialValues(new Variable(VariableType::tArray));
						for(std::unordered_map<std::string, int32_t>::iterator j = parameter->specialValuesStringMap.begin(); j != parameter->specialValuesStringMap.end(); ++j)
						{
							PVariable specialElement(new Variable(VariableType::tStruct));
							specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
							specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
							specialValues->arrayValue->push_back(specialElement);
						}
						element->structValue->insert(StructElement("SPECIAL", specialValues));
					}
				}
				else if(j->second->logical->type == ILogical::Type::tEnum)
				{
					LogicalEnumeration* parameter = (LogicalEnumeration*)j->second->logical.get();
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ENUM")))));
					element->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
					element->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));

					PVariable valueList(new Variable(VariableType::tArray));
					for(std::vector<EnumerationValue>::iterator j = parameter->values.begin(); j != parameter->values.end(); ++j)
					{
						valueList->arrayValue->push_back(PVariable(new Variable(j->id)));
					}
					element->structValue->insert(StructElement("VALUE_LIST", valueList));
				}
				else if(j->second->logical->type == ILogical::Type::tFloat)
				{
					LogicalDecimal* parameter = (LogicalDecimal*)j->second->logical.get();
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("FLOAT")))));
					element->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
					element->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));

					if(!parameter->specialValuesStringMap.empty())
					{
						PVariable specialValues(new Variable(VariableType::tArray));
						for(std::unordered_map<std::string, double>::iterator j = parameter->specialValuesStringMap.begin(); j != parameter->specialValuesStringMap.end(); ++j)
						{
							PVariable specialElement(new Variable(VariableType::tStruct));
							specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
							specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
							specialValues->arrayValue->push_back(specialElement);
						}
						element->structValue->insert(StructElement("SPECIAL", specialValues));
					}
				}
				else if(j->second->logical->type == ILogical::Type::tArray)
				{
					if(!clientInfo->initNewFormat) continue;
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ARRAY")))));
				}
				else if(j->second->logical->type == ILogical::Type::tStruct)
				{
					if(!clientInfo->initNewFormat) continue;
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRUCT")))));
				}
				parameters->structValue->insert(StructElement(j->second->id, element));
			}
		}
		config->structValue->insert(StructElement("CHANNELS", channels));

		return config;
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

PVariable Peer::getAllValues(PRpcClientInfo clientInfo, bool returnWriteOnly, bool checkAcls)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(!clientInfo) clientInfo.reset(new RpcClientInfo());
		PVariable values(new Variable(VariableType::tStruct));

		auto central = getCentral();
		if(!central) return Variable::createError(-32500, "Could not get central.");

		values->structValue->insert(StructElement("FAMILY", PVariable(new Variable((uint32_t)getCentral()->deviceFamily()))));
		values->structValue->insert(StructElement("ID", PVariable(new Variable((uint32_t)_peerID))));
		values->structValue->insert(StructElement("ADDRESS", PVariable(new Variable(_serialNumber))));
		values->structValue->insert(StructElement("TYPE", PVariable(new Variable(_rpcTypeString))));
		values->structValue->insert(StructElement("TYPE_ID", PVariable(new Variable(_deviceType))));
		values->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(-1)))));
		PVariable channels(new Variable(VariableType::tArray));
		for(Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i)
		{
			if(!i->second) continue;
			if(!i->second->countFromVariable.empty() && configCentral[0].find(i->second->countFromVariable) != configCentral[0].end())
			{
				std::vector<uint8_t> parameterData = configCentral[0][i->second->countFromVariable].getBinaryData();
				if(parameterData.size() > 0 && i->first >= i->second->channel + parameterData.at(parameterData.size() - 1)) continue;
			}
			PVariable channel(new Variable(VariableType::tStruct));
			channel->structValue->insert(StructElement("INDEX", PVariable(new Variable(i->first))));
			channel->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(i->first)))));
			channel->structValue->insert(StructElement("TYPE", PVariable(new Variable(i->second->type))));

			PVariable parameters(new Variable(VariableType::tStruct));
			channel->structValue->insert(StructElement("PARAMSET", parameters));
			channels->arrayValue->push_back(channel);

			PParameterGroup parameterGroup = getParameterSet(i->first, ParameterGroup::Type::variables);
			if(!parameterGroup) continue;

			for(Parameters::iterator j = parameterGroup->parameters.begin(); j != parameterGroup->parameters.end(); ++j)
			{
				if(checkAcls && !clientInfo->acls->checkVariableReadAccess(central->getPeer(_peerID), i->first, j->first)) continue;

				if(!j->second || j->second->id.empty() || !j->second->visible) continue;
				if(!j->second->visible && !j->second->service && !j->second->internal  && !j->second->transform)
				{
					_bl->out.printDebug("Debug: Omitting parameter " + j->second->id + " because of it's ui flag.");
					continue;
				}
				if(!j->second->readable && !j->second->transmitted && !returnWriteOnly) continue;
#ifdef CCU2
				if(j->second->logical->type == ILogical::Type::tInteger64) continue;
#endif
				if(clientInfo->clientType == RpcClientType::ccu2 && !j->second->ccu2Visible) continue;

				std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator valuesCentralIterator = valuesCentral.find(i->first);
				if(valuesCentralIterator == valuesCentral.end()) continue;
				std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = valuesCentralIterator->second.find(j->second->id);
				if(parameterIterator == valuesCentralIterator->second.end()) continue;

				if(getAllValuesHook2(clientInfo, j->second, i->first, parameters)) continue;

				PVariable element(new Variable(VariableType::tStruct));
				PVariable value;
				if(j->second->readable || j->second->transmitted)
				{
					std::vector<uint8_t> parameterData = parameterIterator->second.getBinaryData();
					if(j->second->password || parameterData.empty()) value.reset(new Variable(j->second->logical->type));
					else
					{
						if(!convertFromPacketHook(j->second, parameterData, value)) value = j->second->convertFromPacket(parameterData);
					}
					if(!value) continue;
					element->structValue->insert(StructElement("VALUE", value));
				}

				element->structValue->insert(StructElement("READABLE", PVariable(new Variable(j->second->readable))));
				element->structValue->insert(StructElement("WRITEABLE", PVariable(new Variable(j->second->writeable))));
                element->structValue->insert(StructElement("TRANSMITTED", PVariable(new Variable(j->second->transmitted))));
				element->structValue->insert(StructElement("UNIT", PVariable(new Variable(j->second->unit))));
				if(j->second->logical->type == ILogical::Type::tBoolean)
				{
					if(value) value->type = VariableType::tBoolean; //For some families/variables "convertFromPacket" returns wrong type
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("BOOL")))));
				}
				else if(j->second->logical->type == ILogical::Type::tString)
				{
					if(value) value->type = VariableType::tString; //For some families/variables "convertFromPacket" returns wrong type
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRING")))));
				}
				else if(j->second->logical->type == ILogical::Type::tAction)
				{
					if(value) value->type = VariableType::tBoolean; //For some families/variables "convertFromPacket" returns wrong type
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ACTION")))));
				}
				else if(j->second->logical->type == ILogical::Type::tInteger)
				{
					if(value) value->type = VariableType::tInteger; //For some families/variables "convertFromPacket" returns wrong type
					LogicalInteger* parameter = (LogicalInteger*)j->second->logical.get();
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER")))));
					element->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
					element->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));

					if(!parameter->specialValuesStringMap.empty())
					{
						PVariable specialValues(new Variable(VariableType::tArray));
						for(std::unordered_map<std::string, int32_t>::iterator j = parameter->specialValuesStringMap.begin(); j != parameter->specialValuesStringMap.end(); ++j)
						{
							PVariable specialElement(new Variable(VariableType::tStruct));
							specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
							specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
							specialValues->arrayValue->push_back(specialElement);
						}
						element->structValue->insert(StructElement("SPECIAL", specialValues));
					}
				}
				else if(j->second->logical->type == ILogical::Type::tInteger64)
				{
					if(value) value->type = VariableType::tInteger64; //For some families/variables "convertFromPacket" returns wrong type
					LogicalInteger64* parameter = (LogicalInteger64*)j->second->logical.get();
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER64")))));
					element->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
					element->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));

					if(!parameter->specialValuesStringMap.empty())
					{
						PVariable specialValues(new Variable(VariableType::tArray));
						for(std::unordered_map<std::string, int64_t>::iterator j = parameter->specialValuesStringMap.begin(); j != parameter->specialValuesStringMap.end(); ++j)
						{
							PVariable specialElement(new Variable(VariableType::tStruct));
							specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
							specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
							specialValues->arrayValue->push_back(specialElement);
						}
						element->structValue->insert(StructElement("SPECIAL", specialValues));
					}
				}
				else if(j->second->logical->type == ILogical::Type::tEnum)
				{
					if(value) value->type = VariableType::tInteger; //For some families/variables "convertFromPacket" returns wrong type
					LogicalEnumeration* parameter = (LogicalEnumeration*)j->second->logical.get();
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ENUM")))));
					element->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
					element->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));

					PVariable valueList(new Variable(VariableType::tArray));
					for(std::vector<EnumerationValue>::iterator j = parameter->values.begin(); j != parameter->values.end(); ++j)
					{
						valueList->arrayValue->push_back(PVariable(new Variable(j->id)));
					}
					element->structValue->insert(StructElement("VALUE_LIST", valueList));
				}
				else if(j->second->logical->type == ILogical::Type::tFloat)
				{
					if(value) value->type = VariableType::tFloat; //For some families/variables "convertFromPacket" returns wrong type
					LogicalDecimal* parameter = (LogicalDecimal*)j->second->logical.get();
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("FLOAT")))));
					element->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
					element->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));

					if(!parameter->specialValuesStringMap.empty())
					{
						PVariable specialValues(new Variable(VariableType::tArray));
						for(std::unordered_map<std::string, double>::iterator j = parameter->specialValuesStringMap.begin(); j != parameter->specialValuesStringMap.end(); ++j)
						{
							PVariable specialElement(new Variable(VariableType::tStruct));
							specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
							specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
							specialValues->arrayValue->push_back(specialElement);
						}
						element->structValue->insert(StructElement("SPECIAL", specialValues));
					}
				}
				else if(j->second->logical->type == ILogical::Type::tArray)
				{
					if(!clientInfo->initNewFormat) continue;
					if(value) value->type = VariableType::tArray; //For some families/variables "convertFromPacket" returns wrong type
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ARRAY")))));
				}
				else if(j->second->logical->type == ILogical::Type::tStruct)
				{
					if(!clientInfo->initNewFormat) continue;
					if(value) value->type = VariableType::tStruct; //For some families/variables "convertFromPacket" returns wrong type
					element->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRUCT")))));
				}
				parameters->structValue->insert(StructElement(j->second->id, element));
			}
		}
		values->structValue->insert(StructElement("CHANNELS", channels));

		return values;
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

PVariable Peer::getConfigParameter(PRpcClientInfo clientInfo, uint32_t channel, std::string name)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");
		std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator channelIterator = configCentral.find(channel);
		if(channelIterator == configCentral.end()) return Variable::createError(-2, "Unknown channel.");
		std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = channelIterator->second.find(name);
		if(parameterIterator == channelIterator->second.end()) return Variable::createError(-5, "Unknown parameter.");

		//Check if channel still exists in device description
		Functions::iterator functionIterator = _rpcDevice->functions.find(channel);
		if(functionIterator == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel (2).");

		PParameterGroup parameterGroup = getParameterSet(channel, ParameterGroup::Type::Enum::config);
		PParameter parameter = parameterGroup->parameters.at(name);
		if(!parameter) return Variable::createError(-5, "Unknown parameter.");
		if(!parameter->readable) return Variable::createError(-6, "Parameter is not readable.");
		std::vector<uint8_t> parameterData = parameterIterator->second.getBinaryData();
		PVariable variable;
		if(!convertFromPacketHook(parameter, parameterData, variable)) variable = parameter->convertFromPacket(parameterData);
		if(parameter->password) variable.reset(new Variable(variable->type));
		return variable;
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

PVariable Peer::getDeviceDescription(PRpcClientInfo clientInfo, int32_t channel, std::map<std::string, bool> fields)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		PVariable description(new Variable(VariableType::tStruct));

		if(channel == -1) //Base device
		{
			BaseLib::DeviceDescription::PSupportedDevice supportedDevice = _rpcDevice->getType(_deviceType, _firmwareVersion);

            std::shared_ptr<ICentral> central = getCentral();
            if(!central) return description;
            std::string language = clientInfo ? clientInfo->language : "en-US";
            std::string filename = _rpcDevice->getFilename();

			if(fields.empty() || fields.find("FAMILY") != fields.end()) description->structValue->insert(StructElement("FAMILY", PVariable(new Variable((uint32_t)getCentral()->deviceFamily()))));
			if(fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable((uint32_t)_peerID))));
			if(fields.empty() || fields.find("ADDRESS") != fields.end()) description->structValue->insert(StructElement("ADDRESS", PVariable(new Variable(_serialNumber))));
			if(fields.empty() || fields.find("NAME") != fields.end()) description->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(-1)))));
			if(supportedDevice && !supportedDevice->serialPrefix.empty() && (fields.empty() || fields.find("SERIAL_PREFIX") != fields.end())) description->structValue->insert(StructElement("SERIAL_PREFIX", PVariable(new Variable(supportedDevice->serialPrefix))));

            if(supportedDevice)
            {
                std::string descriptionText = central->getTranslations()->getTypeDescription(filename, language, supportedDevice->id);
                if(!descriptionText.empty() && fields.find("DESCRIPTION") != fields.end()) description->structValue->insert(StructElement("DESCRIPTION", PVariable(new Variable(descriptionText))));
                std::string longDescriptionText = central->getTranslations()->getTypeLongDescription(filename, language, supportedDevice->id);
                if(!longDescriptionText.empty() && fields.find("LONG_DESCRIPTION") != fields.end()) description->structValue->insert(StructElement("LONG_DESCRIPTION", PVariable(new Variable(longDescriptionText))));
            }

			if(fields.empty() || fields.find("PAIRING_METHOD") != fields.end()) description->structValue->insert(StructElement("PAIRING_METHOD", std::make_shared<Variable>(_rpcDevice->pairingMethod)));

			PVariable variable = PVariable(new Variable(VariableType::tArray));
			PVariable variable2 = PVariable(new Variable(VariableType::tArray));
			if(fields.empty() || fields.find("CHILDREN") != fields.end()) description->structValue->insert(StructElement("CHILDREN", variable));
			if(fields.empty() || fields.find("CHANNELS") != fields.end()) description->structValue->insert(StructElement("CHANNELS", variable2));

			if(fields.empty() || fields.find("CHILDREN") != fields.end() || fields.find("CHANNELS") != fields.end())
			{
				for(Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i)
				{
					if(!i->second->visible) continue;
					if(!i->second->countFromVariable.empty() && configCentral[0].find(i->second->countFromVariable) != configCentral[0].end())
					{
						std::vector<uint8_t> parameterData = configCentral[0][i->second->countFromVariable].getBinaryData();
						if(parameterData.size() > 0 && i->first >= i->second->channel + parameterData.at(parameterData.size() - 1)) continue;
					}
					if(fields.empty() || fields.find("CHILDREN") != fields.end()) variable->arrayValue->push_back(PVariable(new Variable(_serialNumber + ":" + std::to_string(i->first))));
					if(fields.empty() || fields.find("CHANNELS") != fields.end()) variable2->arrayValue->push_back(PVariable(new Variable(i->first)));
				}
			}

			if(fields.empty() || fields.find("FIRMWARE") != fields.end())
			{
				if(_firmwareVersion != -1) description->structValue->insert(StructElement("FIRMWARE", PVariable(new Variable(getFirmwareVersionString(_firmwareVersion)))));
				else if(!_firmwareVersionString.empty()) description->structValue->insert(StructElement("FIRMWARE", PVariable(new Variable(_firmwareVersionString))));
				else description->structValue->insert(StructElement("FIRMWARE", PVariable(new Variable(std::string("?")))));
			}

			if((fields.empty() || fields.find("AVAILABLE_FIRMWARE") != fields.end()) && (_firmwareVersion != -1 || !_firmwareVersionString.empty()))
			{
				int32_t newFirmwareVersion = getNewFirmwareVersion();
				if(newFirmwareVersion > _firmwareVersion) description->structValue->insert(StructElement("AVAILABLE_FIRMWARE", PVariable(new Variable(getFirmwareVersionString(newFirmwareVersion)))));
			}

			if(fields.empty() || fields.find("FLAGS") != fields.end())
			{
				int32_t uiFlags = 0;
				if(_rpcDevice->visible) uiFlags += 1;
				if(_rpcDevice->internal) uiFlags += 2;
				if(!_rpcDevice->deletable || isTeam()) uiFlags += 8;
				description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			}

			if(fields.empty() || fields.find("INTERFACE") != fields.end()) description->structValue->insert(StructElement("INTERFACE", PVariable(new Variable(getCentral()->getSerialNumber()))));

			if(fields.empty() || fields.find("PARAMSETS") != fields.end())
			{
				variable = PVariable(new Variable(VariableType::tArray));
				description->structValue->insert(StructElement("PARAMSETS", variable));
				variable->arrayValue->push_back(PVariable(new Variable(std::string("MASTER")))); //Always MASTER
			}

			if(fields.empty() || fields.find("PARENT") != fields.end()) description->structValue->insert(StructElement("PARENT", PVariable(new Variable(std::string("")))));

			if(!_ip.empty() && (fields.empty() || fields.find("IP_ADDRESS") != fields.end())) description->structValue->insert(StructElement("IP_ADDRESS", PVariable(new Variable(_ip))));

			if(fields.empty() || fields.find("PHYSICAL_ADDRESS") != fields.end()) description->structValue->insert(StructElement("PHYSICAL_ADDRESS", PVariable(new Variable(_address))));

			//Compatibility
			if(fields.empty() || fields.find("RF_ADDRESS") != fields.end()) description->structValue->insert(StructElement("RF_ADDRESS", PVariable(new Variable(_address))));
			//Compatibility
			if(fields.empty() || fields.find("ROAMING") != fields.end()) description->structValue->insert(StructElement("ROAMING", PVariable(new Variable((int32_t)0))));

			if(fields.empty() || fields.find("RX_MODE") != fields.end()) description->structValue->insert(StructElement("RX_MODE", PVariable(new Variable((int32_t)_rpcDevice->receiveModes))));

			if(!_rpcTypeString.empty() && (fields.empty() || fields.find("TYPE") != fields.end())) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(_rpcTypeString))));

			if(fields.empty() || fields.find("TYPE_ID") != fields.end()) description->structValue->insert(StructElement("TYPE_ID", PVariable(new Variable(_deviceType))));

			if(fields.empty() || fields.find("VERSION") != fields.end()) description->structValue->insert(StructElement("VERSION", PVariable(new Variable(_rpcDevice->version))));

			if(fields.find("WIRELESS") != fields.end()) description->structValue->insert(StructElement("WIRELESS", PVariable(new Variable(wireless()))));

            auto room = getRoom(-1);
			if(fields.find("ROOM") != fields.end() && room != 0) description->structValue->emplace("ROOM", std::make_shared<Variable>(room));

            auto categories = getCategories(-1);
			if(fields.find("CATEGORIES") != fields.end() && !categories.empty())
			{
				PVariable categoriesResult = std::make_shared<Variable>(VariableType::tArray);
                categoriesResult->arrayValue->reserve(categories.size());
				for(auto category : categories)
				{
                    categoriesResult->arrayValue->push_back(std::make_shared<Variable>(category));
				}
				description->structValue->emplace("CATEGORIES", categoriesResult);
			}
		}
		else
		{
			if(_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
			PFunction rpcFunction = _rpcDevice->functions.at(channel);
			if(!rpcFunction->countFromVariable.empty() && configCentral[0].find(rpcFunction->countFromVariable) != configCentral[0].end())
			{
				std::vector<uint8_t> parameterData = configCentral[0][rpcFunction->countFromVariable].getBinaryData();
				if(parameterData.size() > 0 && channel >= (int32_t)rpcFunction->channel + parameterData.at(parameterData.size() - 1)) return Variable::createError(-2, "Channel index larger than defined.");
			}
			if(!rpcFunction->visible) return description;

			if(fields.empty() || fields.find("FAMILYID") != fields.end()) description->structValue->insert(StructElement("FAMILY", PVariable(new Variable((uint32_t)getCentral()->deviceFamily()))));
			if(fields.empty() || fields.find("ID") != fields.end()) description->structValue->insert(StructElement("ID", PVariable(new Variable((uint32_t)_peerID))));
			if(fields.empty() || fields.find("CHANNEL") != fields.end()) description->structValue->insert(StructElement("CHANNEL", PVariable(new Variable(channel))));
			if(fields.empty() || fields.find("NAME") != fields.end()) description->structValue->insert(StructElement("NAME", PVariable(new Variable(getName(channel)))));
			if(fields.empty() || fields.find("ADDRESS") != fields.end()) description->structValue->insert(StructElement("ADDRESS", PVariable(new Variable(_serialNumber + ":" + std::to_string(channel)))));

			if(fields.empty() || fields.find("AES_ACTIVE") != fields.end())
			{
				int32_t aesActive = 0;
				if(configCentral.find(channel) != configCentral.end() && configCentral.at(channel).find("AES_ACTIVE") != configCentral.at(channel).end())
				{
					std::vector<uint8_t> parameterData = configCentral.at(channel).at("AES_ACTIVE").getBinaryData();
					if(!parameterData.empty() && parameterData.at(0) != 0)
					{
						aesActive = 1;
					}
				}
				//Integer for compatability
				description->structValue->insert(StructElement("AES_ACTIVE", PVariable(new Variable(aesActive))));
			}

			if(fields.empty() || fields.find("DIRECTION") != fields.end() || fields.find("LINK_SOURCE_ROLES") != fields.end() || fields.find("LINK_TARGET_ROLES") != fields.end())
			{
				int32_t direction = 0;
				std::ostringstream linkSourceRoles;
				std::ostringstream linkTargetRoles;
				for(LinkFunctionTypes::iterator k = rpcFunction->linkSenderFunctionTypes.begin(); k != rpcFunction->linkSenderFunctionTypes.end(); ++k)
				{
					//Probably only one direction is supported, but just in case I use the "or"
					if(!k->empty())
					{
						if(direction & 1) linkSourceRoles << " ";
						linkSourceRoles << *k;
						direction |= 1;
					}
				}
				for(LinkFunctionTypes::iterator k = rpcFunction->linkReceiverFunctionTypes.begin(); k != rpcFunction->linkReceiverFunctionTypes.end(); ++k)
				{
					//Probably only one direction is supported, but just in case I use the "or"
					if(!k->empty())
					{
						if(direction & 2) linkTargetRoles << " ";
						linkTargetRoles << *k;
						direction |= 2;
					}
				}

				//Overwrite direction when manually set
				if(rpcFunction->direction != Function::Direction::Enum::none) direction = (int32_t)rpcFunction->direction;
				if(fields.empty() || fields.find("DIRECTION") != fields.end()) description->structValue->insert(StructElement("DIRECTION", PVariable(new Variable(direction))));
				if(fields.empty() || fields.find("LINK_SOURCE_ROLES") != fields.end()) description->structValue->insert(StructElement("LINK_SOURCE_ROLES", PVariable(new Variable(linkSourceRoles.str()))));
				if(fields.empty() || fields.find("LINK_TARGET_ROLES") != fields.end()) description->structValue->insert(StructElement("LINK_TARGET_ROLES", PVariable(new Variable(linkTargetRoles.str()))));
			}

			if(fields.empty() || fields.find("FLAGS") != fields.end())
			{
				int32_t uiFlags = 0;
				if(rpcFunction->visible) uiFlags += 1;
				if(rpcFunction->internal) uiFlags += 2;
				if(rpcFunction->deletable || isTeam()) uiFlags += 8;
				description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			}

			if(fields.empty() || fields.find("GROUP") != fields.end())
			{
				int32_t groupedWith = getChannelGroupedWith(channel);
				if(groupedWith > -1)
				{
					description->structValue->insert(StructElement("GROUP", PVariable(new Variable(_serialNumber + ":" + std::to_string(groupedWith)))));
				}
			}

			if(fields.empty() || fields.find("INDEX") != fields.end()) description->structValue->insert(StructElement("INDEX", PVariable(new Variable(channel))));

			if(fields.empty() || fields.find("PARAMSETS") != fields.end())
			{
				PVariable variable = PVariable(new Variable(VariableType::tArray));
				description->structValue->insert(StructElement("PARAMSETS", variable));
				if(!rpcFunction->configParameters->parameters.empty() || !rpcFunction->configParameters->id.empty()) variable->arrayValue->push_back(PVariable(new Variable(std::string("MASTER"))));
				if(!rpcFunction->variables->parameters.empty() || !rpcFunction->variables->id.empty()) variable->arrayValue->push_back(PVariable(new Variable(std::string("VALUES"))));
				if(!rpcFunction->linkParameters->parameters.empty() || !rpcFunction->linkParameters->id.empty()) variable->arrayValue->push_back(PVariable(new Variable(std::string("LINK"))));
			}
			//if(rpcChannel->parameterSets.find(Rpc::ParameterSet::Type::Enum::link) != rpcChannel->parameterSets.end()) variable->arrayValue->push_back(PVariable(new Variable(rpcChannel->parameterSets.at(Rpc::ParameterSet::Type::Enum::link)->typeString())));
			//if(rpcChannel->parameterSets.find(Rpc::ParameterSet::Type::Enum::master) != rpcChannel->parameterSets.end()) variable->arrayValue->push_back(PVariable(new Variable(rpcChannel->parameterSets.at(Rpc::ParameterSet::Type::Enum::master)->typeString())));
			//if(rpcChannel->parameterSets.find(Rpc::ParameterSet::Type::Enum::values) != rpcChannel->parameterSets.end()) variable->arrayValue->push_back(PVariable(new Variable(rpcChannel->parameterSets.at(Rpc::ParameterSet::Type::Enum::values)->typeString())));

			if(fields.empty() || fields.find("PARENT") != fields.end()) description->structValue->insert(StructElement("PARENT", PVariable(new Variable(_serialNumber))));

			if(!_rpcTypeString.empty() && (fields.empty() || fields.find("PARENT_TYPE") != fields.end())) description->structValue->insert(StructElement("PARENT_TYPE", PVariable(new Variable(_rpcTypeString))));

			if(fields.empty() || fields.find("TYPE") != fields.end()) description->structValue->insert(StructElement("TYPE", PVariable(new Variable(rpcFunction->type))));

			if(fields.empty() || fields.find("VERSION") != fields.end()) description->structValue->insert(StructElement("VERSION", PVariable(new Variable(_rpcDevice->version))));

            auto room = getRoom(channel);
            if(fields.find("ROOM") != fields.end() && room != 0) description->structValue->emplace("ROOM", std::make_shared<Variable>(room));

            auto categories = getCategories(channel);
            if(fields.find("CATEGORIES") != fields.end() && !categories.empty())
            {
                PVariable categoriesResult = std::make_shared<Variable>(VariableType::tArray);
                categoriesResult->arrayValue->reserve(categories.size());
                for(auto category : categories)
                {
                    categoriesResult->arrayValue->push_back(std::make_shared<Variable>(category));
                }
                description->structValue->emplace("CATEGORIES", categoriesResult);
            }
		}
		return description;
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

std::shared_ptr<std::vector<PVariable>> Peer::getDeviceDescriptions(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields)
{
	try
	{
		std::shared_ptr<std::vector<PVariable>> descriptions(new std::vector<PVariable>());
		PVariable description = getDeviceDescription(clientInfo, -1, fields);
		if(!description->errorStruct && !description->structValue->empty()) descriptions->push_back(description);

		if(channels)
		{
			for(Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i)
			{
				if(!i->second->countFromVariable.empty() && configCentral[0].find(i->second->countFromVariable) != configCentral[0].end())
				{
					std::vector<uint8_t> parameterData = configCentral[0][i->second->countFromVariable].getBinaryData();
					if(parameterData.size() > 0 && i->first >= i->second->channel + parameterData.at(parameterData.size() - 1)) continue;
				}
				description = getDeviceDescription(clientInfo, (int32_t)i->first, fields);
				if(!description->errorStruct && !description->structValue->empty()) descriptions->push_back(description);
			}
		}

		return descriptions;
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
    return std::shared_ptr<std::vector<PVariable>>();
}

PVariable Peer::getDeviceInfo(PRpcClientInfo clientInfo, std::map<std::string, bool> fields)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		PVariable info(new Variable(VariableType::tStruct));

		info->structValue->insert(StructElement("ID", PVariable(new Variable((int32_t)_peerID))));

		if(wireless())
		{
			if(fields.empty() || fields.find("RSSI") != fields.end())
			{
				if(valuesCentral.find(0) != valuesCentral.end() && valuesCentral.at(0).find("RSSI_DEVICE") != valuesCentral.at(0).end() && valuesCentral.at(0).at("RSSI_DEVICE").rpcParameter)
				{
					std::vector<uint8_t> parameterData = valuesCentral.at(0).at("RSSI_DEVICE").getBinaryData();
					info->structValue->insert(StructElement("RSSI", valuesCentral.at(0).at("RSSI_DEVICE").rpcParameter->convertFromPacket(parameterData)));
				}
			}
		}

		return info;
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
    return PVariable();
}

PVariable Peer::getLink(PRpcClientInfo clientInfo, int32_t channel, int32_t flags, bool avoidDuplicates)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		PVariable array(new Variable(VariableType::tArray));
		std::shared_ptr<ICentral> central = getCentral();
		if(!central) return array; //central actually should always be set at this point
		PVariable element;
		bool groupFlag = false;
		if(flags & 0x01) groupFlag = true;
		if(channel > -1 && !groupFlag) //Get link of single channel
		{
			if(_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
			bool isSender = false;
			//Return if there are no link roles defined
			PFunction rpcFunction = _rpcDevice->functions.at(channel);
			if(!rpcFunction->linkSenderFunctionTypes.empty()) isSender = true;
			else if(rpcFunction->linkReceiverFunctionTypes.empty()) return array;
			//Return if no peers are paired to the channel
			_peersMutex.lock();
			std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>>::iterator peersIterator = _peers.find(channel);
			if(peersIterator == _peers.end() || peersIterator->second.empty())
			{
				_peersMutex.unlock();
				return array;
			}
			std::vector<std::shared_ptr<BasicPeer>> peers = peersIterator->second;
			_peersMutex.unlock();
			for(std::vector<std::shared_ptr<BasicPeer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if((*i)->isVirtual) continue;
				if((*i)->hasSender) isSender = (*i)->isSender;
				else (*i)->isSender = isSender; //Todo: Remove in future versions. Sets isSender of the peer if previously unset.
				std::shared_ptr<Peer> remotePeer;
				if((*i)->id != 0) remotePeer = central->getPeer((*i)->id);
				else if((*i)->address != 0) remotePeer = central->getPeer((*i)->address);
				else if(!(*i)->serialNumber.empty()) remotePeer = central->getPeer((*i)->serialNumber);
				if(!remotePeer)
				{
					_bl->out.printDebug("Debug: Can't return link description for peer with id " + std::to_string((*i)->id) + ". The peer is not paired to Homegear.");
					continue;
				}
				uint64_t peerID = remotePeer->getID();
				bool peerKnowsMe = false;
				if(remotePeer->getPeer((*i)->channel, _peerID, channel)) peerKnowsMe = true;

				//Don't continue if peer is sender and exists in central's peer array to avoid generation of duplicate results when requesting all links (only generate results when we are sender)
				if(!isSender && peerKnowsMe && avoidDuplicates && peerID != _peerID) return array;
				//If we are receiver this point is only reached, when the sender is not paired to this central

				std::string peerSerialNumber = remotePeer->getSerialNumber();
				int32_t brokenFlags = 0;
				/*if(peerID == 0 || peerSerialNumber.empty())
				{
					if(peerKnowsMe || (*i)->id == _peerID) //Link to myself with non-existing (virtual) channel (e. g. switches use this)
					{
						(*i)->id = remotePeer->getID();
						(*i)->serialNumber = remotePeer->getSerialNumber();
						peerID = (*i)->id;
						peerSerialNumber = remotePeer->getSerialNumber();
					}
					else
					{
						//Peer not paired to central
						std::ostringstream stringstream;
						stringstream << '@' << std::dec << (*i)->id;
						peerSerialNumber = stringstream.str();
						if(isSender) brokenFlags = 2; //LINK_FLAG_RECEIVER_BROKEN
						else brokenFlags = 1; //LINK_FLAG_SENDER_BROKEN
					}
				}*/
				//Relevent for switches
				if(peerID == _peerID && _rpcDevice->functions.find((*i)->channel) == _rpcDevice->functions.end())
				{
					if(isSender) brokenFlags = 2 | 4; //LINK_FLAG_RECEIVER_BROKEN | PEER_IS_ME
					else brokenFlags = 1 | 4; //LINK_FLAG_SENDER_BROKEN | PEER_IS_ME
				}
				if(brokenFlags == 0 && remotePeer && remotePeer->serviceMessages->getUnreach()) brokenFlags = 2;
				if(serviceMessages->getUnreach()) brokenFlags |= 1;
				element.reset(new Variable(VariableType::tStruct));
				element->structValue->insert(StructElement("DESCRIPTION", PVariable(new Variable((*i)->linkDescription))));
				element->structValue->insert(StructElement("FLAGS", PVariable(new Variable(brokenFlags))));
				element->structValue->insert(StructElement("NAME", PVariable(new Variable((*i)->linkName))));
				if(isSender)
				{
					element->structValue->insert(StructElement("RECEIVER", PVariable(new Variable(peerSerialNumber + ":" + std::to_string((*i)->channel)))));
					element->structValue->insert(StructElement("RECEIVER_ID", PVariable(new Variable((int32_t)remotePeer->getID()))));
					element->structValue->insert(StructElement("RECEIVER_CHANNEL", PVariable(new Variable((*i)->channel))));
					if(flags & 4)
					{
						PVariable paramset;
						if(!(brokenFlags & 2) && remotePeer) paramset = remotePeer->getParamset(clientInfo, (*i)->channel, ParameterGroup::Type::Enum::link, _peerID, channel, false);
						else paramset.reset(new Variable(VariableType::tStruct));
						if(paramset->errorStruct) paramset.reset(new Variable(VariableType::tStruct));
						element->structValue->insert(StructElement("RECEIVER_PARAMSET", paramset));
					}
					if(flags & 16)
					{
						PVariable description;
						if(!(brokenFlags & 2) && remotePeer) description = remotePeer->getDeviceDescription(clientInfo, (*i)->channel, std::map<std::string, bool>());
						else description.reset(new Variable(VariableType::tStruct));
						if(description->errorStruct) description.reset(new Variable(VariableType::tStruct));
						element->structValue->insert(StructElement("RECEIVER_DESCRIPTION", description));
					}
					element->structValue->insert(StructElement("SENDER", PVariable(new Variable(_serialNumber + ":" + std::to_string(channel)))));
					element->structValue->insert(StructElement("SENDER_ID", PVariable(new Variable((int32_t)_peerID))));
					element->structValue->insert(StructElement("SENDER_CHANNEL", PVariable(new Variable(channel))));
					if(flags & 2)
					{
						PVariable paramset;
						if(!(brokenFlags & 1)) paramset = getParamset(clientInfo, channel, ParameterGroup::Type::Enum::link, peerID, (*i)->channel, false);
						else paramset.reset(new Variable(VariableType::tStruct));
						if(paramset->errorStruct) paramset.reset(new Variable(VariableType::tStruct));
						element->structValue->insert(StructElement("SENDER_PARAMSET", paramset));
					}
					if(flags & 8)
					{
						PVariable description;
						if(!(brokenFlags & 1)) description = getDeviceDescription(clientInfo, channel, std::map<std::string, bool>());
						else description.reset(new Variable(VariableType::tStruct));
						if(description->errorStruct) description.reset(new Variable(VariableType::tStruct));
						element->structValue->insert(StructElement("SENDER_DESCRIPTION", description));
					}
				}
				else //When sender is broken
				{
					element->structValue->insert(StructElement("RECEIVER", PVariable(new Variable(_serialNumber + ":" + std::to_string(channel)))));
					element->structValue->insert(StructElement("RECEIVER_ID", PVariable(new Variable((int32_t)_peerID))));
					element->structValue->insert(StructElement("RECEIVER_CHANNEL", PVariable(new Variable(channel))));
					if(flags & 4)
					{
						PVariable paramset;
						if(!(brokenFlags & 2) && remotePeer) paramset = getParamset(clientInfo, channel, ParameterGroup::Type::Enum::link, peerID, (*i)->channel, false);
						else paramset.reset(new Variable(VariableType::tStruct));
						if(paramset->errorStruct) paramset.reset(new Variable(VariableType::tStruct));
						element->structValue->insert(StructElement("RECEIVER_PARAMSET", paramset));
					}
					if(flags & 16)
					{
						PVariable description;
						if(!(brokenFlags & 2)) description = getDeviceDescription(clientInfo, channel, std::map<std::string, bool>());
						else description.reset(new Variable(VariableType::tStruct));
						if(description->errorStruct) description.reset(new Variable(VariableType::tStruct));
						element->structValue->insert(StructElement("RECEIVER_DESCRIPTION", description));
					}
					element->structValue->insert(StructElement("SENDER", PVariable(new Variable(peerSerialNumber + ":" + std::to_string((*i)->channel)))));
					element->structValue->insert(StructElement("SENDER_ID", PVariable(new Variable((int32_t)remotePeer->getID()))));
					element->structValue->insert(StructElement("SENDER_CHANNEL", PVariable(new Variable((*i)->channel))));
					if(flags & 2)
					{
						PVariable paramset;
						if(!(brokenFlags & 1) && remotePeer) paramset = remotePeer->getParamset(clientInfo, (*i)->channel, ParameterGroup::Type::Enum::link, _peerID, channel, false);
						else paramset.reset(new Variable(VariableType::tStruct));
						if(paramset->errorStruct) paramset.reset(new Variable(VariableType::tStruct));
						element->structValue->insert(StructElement("SENDER_PARAMSET", paramset));
					}
					if(flags & 8)
					{
						PVariable description;
						if(!(brokenFlags & 1) && remotePeer) description = remotePeer->getDeviceDescription(clientInfo, (*i)->channel, std::map<std::string, bool>());
						else description.reset(new Variable(VariableType::tStruct));
						if(description->errorStruct) description.reset(new Variable(VariableType::tStruct));
						element->structValue->insert(StructElement("SENDER_DESCRIPTION", description));
					}
				}
				array->arrayValue->push_back(element);
			}
		}
		else
		{
			if(channel > -1 && groupFlag) //Get links for each grouped channel
			{
				if(_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
				PFunction rpcFunction = _rpcDevice->functions.at(channel);
				if(rpcFunction->grouped)
				{
					element = getLink(clientInfo, channel, flags & 0xFFFFFFFE, avoidDuplicates);
					array->arrayValue->insert(array->arrayValue->end(), element->arrayValue->begin(), element->arrayValue->end());

					int32_t groupedWith = getChannelGroupedWith(channel);
					if(groupedWith > -1)
					{
						element = getLink(clientInfo, groupedWith, flags & 0xFFFFFFFE, avoidDuplicates);
						array->arrayValue->insert(array->arrayValue->end(), element->arrayValue->begin(), element->arrayValue->end());
					}
				}
				else
				{
					element = getLink(clientInfo, channel, flags & 0xFFFFFFFE, avoidDuplicates);
					array->arrayValue->insert(array->arrayValue->end(), element->arrayValue->begin(), element->arrayValue->end());
				}
			}
			else //Get links for all channels
			{
				for(Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i)
				{
					element.reset(new Variable(VariableType::tArray));
					element = getLink(clientInfo, i->first, flags, avoidDuplicates);
					array->arrayValue->insert(array->arrayValue->end(), element->arrayValue->begin(), element->arrayValue->end());
				}
			}
		}
		return array;
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

PVariable Peer::getLinkInfo(PRpcClientInfo clientInfo, int32_t senderChannel, uint64_t receiverID, int32_t receiverChannel)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		std::shared_ptr<BasicPeer> remotePeer = getPeer(senderChannel, receiverID, receiverChannel);
		if(!remotePeer) return Variable::createError(-2, "No peer found for sender channel.");
		PVariable response(new Variable(VariableType::tStruct));
		response->structValue->insert(StructElement("DESCRIPTION", PVariable(new Variable(remotePeer->linkDescription))));
		response->structValue->insert(StructElement("NAME", PVariable(new Variable(remotePeer->linkName))));
		return response;
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

PVariable Peer::getLinkPeers(PRpcClientInfo clientInfo, int32_t channel, bool returnID)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		PVariable array(new Variable(VariableType::tArray));
		if(channel > -1)
		{
			if(_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
			PFunction rpcFunction = _rpcDevice->functions.at(channel);
			//Return if there are no link roles defined
			if(rpcFunction->linkSenderFunctionTypes.empty() && rpcFunction->linkReceiverFunctionTypes.empty()) return array;
			std::shared_ptr<ICentral> central(getCentral());
			if(!central) return array; //central actually should always be set at this point
			_peersMutex.lock();
			std::unordered_map<int32_t, std::vector<std::shared_ptr<BasicPeer>>>::iterator peersIterator = _peers.find(channel);
			if(peersIterator == _peers.end() || peersIterator->second.empty())
			{
				//Return if no peers are paired to the channel
				_peersMutex.unlock();
				return array;
			}
			std::vector<std::shared_ptr<BasicPeer>> peers = peersIterator->second;
			_peersMutex.unlock();
			for(std::vector<std::shared_ptr<BaseLib::Systems::BasicPeer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if((*i)->isVirtual) continue;
				std::shared_ptr<Peer> peer(central->getPeer((*i)->id));
				if(returnID && !peer) continue;
				bool peerKnowsMe = false;
				if(peer && peer->getPeer(channel, _peerID)) peerKnowsMe = true;

				std::string peerSerial = (*i)->serialNumber;
				if((*i)->serialNumber.empty() || (*i)->id == 0)
				{
					if(peerKnowsMe || (*i)->id == _peerID)
					{
						(*i)->serialNumber = peer->getSerialNumber();
						(*i)->id = peer->getID();
						peerSerial = (*i)->serialNumber;
					}
					else
					{
						//Peer not paired to central
						std::ostringstream stringstream;
						stringstream << '@' << std::dec << (*i)->id;
						peerSerial = stringstream.str();
					}
				}
				if(returnID)
				{
					PVariable address(new Variable(VariableType::tArray));
					array->arrayValue->push_back(address);
					address->arrayValue->push_back(PVariable(new Variable(peer->getID())));
					address->arrayValue->push_back(PVariable(new Variable((*i)->channel)));
				}
				else array->arrayValue->push_back(PVariable(new Variable(peerSerial + ":" + std::to_string((*i)->channel))));
			}
		}
		else
		{
			for(Functions::iterator i = _rpcDevice->functions.begin(); i != _rpcDevice->functions.end(); ++i)
			{
				PVariable linkPeers = getLinkPeers(clientInfo, i->first, returnID);
				array->arrayValue->insert(array->arrayValue->end(), linkPeers->arrayValue->begin(), linkPeers->arrayValue->end());
			}
		}
		return array;
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

PVariable Peer::getParamset(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, bool checkAcls)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(channel < 0) channel = 0;
		if(remoteChannel < 0) remoteChannel = 0;
		Functions::iterator functionIterator = _rpcDevice->functions.find(channel);
		if(functionIterator == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
		if(type == ParameterGroup::Type::none) type = ParameterGroup::Type::link;
		PParameterGroup parameterGroup = getParameterSet(channel, type);
		if(!parameterGroup) return Variable::createError(-3, "Unknown parameter set.");
		PVariable variables(new Variable(VariableType::tStruct));

		auto central = getCentral();
		if(!central) return Variable::createError(-32500, "Could not get central.");

		for(Parameters::iterator i = parameterGroup->parameters.begin(); i != parameterGroup->parameters.end(); ++i)
		{
			if(i->second->id.empty() || !i->second->visible) continue;
			if(!i->second->visible && !i->second->service && !i->second->internal && !i->second->transform)
			{
				_bl->out.printDebug("Debug: Omitting parameter " + i->second->id + " because of it's ui flag.");
				continue;
			}
			if(clientInfo->clientType == RpcClientType::ccu2 && !i->second->ccu2Visible) continue;

			PVariable element;
			if(type == ParameterGroup::Type::Enum::variables)
			{
				if(checkAcls && !clientInfo->acls->checkVariableReadAccess(central->getPeer(_peerID), channel, i->first)) continue;

				if(!i->second->readable && !i->second->transmitted) continue;
				std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator valuesIterator = valuesCentral.find(channel);
				if(valuesIterator == valuesCentral.end()) continue;
				std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = valuesIterator->second.find(i->second->id);
				if(parameterIterator == valuesIterator->second.end()) continue;
#ifdef CCU2
				if(parameterIterator->second.rpcParameter && parameterIterator->second.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif
				if(getParamsetHook2(clientInfo, i->second, channel, variables)) continue;
				std::vector<uint8_t> parameterData = parameterIterator->second.getBinaryData();
				if(!convertFromPacketHook(i->second, parameterData, element)) element = i->second->convertFromPacket(parameterData);
			}
			else if(type == ParameterGroup::Type::Enum::config)
			{
				std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator configIterator = configCentral.find(channel);
				if(configIterator == configCentral.end()) continue;
				std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = configIterator->second.find(i->second->id);
				if(parameterIterator == configIterator->second.end()) continue;
#ifdef CCU2
				if(parameterIterator->second.rpcParameter && parameterIterator->second.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif
				std::vector<uint8_t> parameterData = parameterIterator->second.getBinaryData();
				if(!convertFromPacketHook(i->second, parameterData, element)) element = i->second->convertFromPacket(parameterData);
				if(i->second->password) element.reset(new Variable(element->type));
			}
			else if(type == ParameterGroup::Type::Enum::link)
			{
				std::shared_ptr<BasicPeer> remotePeer;
				if(remoteID == 0) remoteID = 0xFFFFFFFFFFFFFFFF; //Remote peer is central
				remotePeer = getPeer(channel, remoteID, remoteChannel);
				if(!remotePeer) return Variable::createError(-3, "Not paired to this peer.");
				if(remotePeer->channel != remoteChannel) continue;
				std::unordered_map<uint32_t, std::unordered_map<int32_t, std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>>>::iterator linkIterator = linksCentral.find(channel);
				if(linkIterator == linksCentral.end()) continue;
				std::unordered_map<int32_t, std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>>::iterator addressIterator = linkIterator->second.find(remotePeer->address);
				if(addressIterator == linkIterator->second.end()) continue;
				std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator remoteChannelIterator = addressIterator->second.find(remotePeer->channel);
				if(remoteChannelIterator == addressIterator->second.end()) continue;
				std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = remoteChannelIterator->second.find(i->second->id);
				if(parameterIterator == remoteChannelIterator->second.end()) continue;
#ifdef CCU2
				if(parameterIterator->second.rpcParameter && parameterIterator->second.rpcParameter->logical->type == ILogical::Type::tInteger64) continue;
#endif
				std::vector<uint8_t> parameterData = parameterIterator->second.getBinaryData();
				if(!convertFromPacketHook(i->second, parameterData, element)) element = i->second->convertFromPacket(parameterData);
			}

			if(!element) continue;
			if(element->type == VariableType::tVoid) continue;
			variables->structValue->insert(StructElement(i->second->id, element));
		}
		return variables;
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

PVariable Peer::getParamsetDescription(PRpcClientInfo clientInfo, int32_t channel, PParameterGroup parameterGroup, bool checkAcls)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(!clientInfo) clientInfo.reset(new RpcClientInfo());

		auto central = getCentral();
		if(!central) return Variable::createError(-32500, "Could not get central.");

		PVariable descriptions(new Variable(VariableType::tStruct));
		uint32_t index = 0;
		for(Parameters::iterator i = parameterGroup->parameters.begin(); i != parameterGroup->parameters.end(); ++i)
		{
			if(parameterGroup->type() == ParameterGroup::Type::variables && checkAcls && !clientInfo->acls->checkVariableReadAccess(central->getPeer(_peerID), channel, i->first)) continue;

			if(!i->second || i->second->id.empty() || !i->second->visible) continue;
			if(!i->second->visible && !i->second->service && !i->second->internal  && !i->second->transform)
			{
				_bl->out.printDebug("Debug: Omitting parameter " + i->second->id + " because of it's ui flag.");
				continue;
			}

			PVariable description = getVariableDescription(clientInfo, i, channel, parameterGroup->type(), index);
			if(!description || description->errorStruct) continue;

			index++;
			descriptions->structValue->insert(StructElement(i->second->id, description));
		}
		return descriptions;
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

PVariable Peer::getParamsetDescription(PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, bool checkAcls)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(channel < 0) channel = 0;
		if(type == ParameterGroup::Type::none) type = ParameterGroup::Type::link;
		PParameterGroup parameterGroup = getParameterSet(channel, type);
		if(!parameterGroup) return Variable::createError(-3, "Unknown parameter set.");
		if(type == ParameterGroup::Type::link && remoteID > 0)
		{
			std::shared_ptr<BaseLib::Systems::BasicPeer> remotePeer = getPeer(channel, remoteID, remoteChannel);
			if(!remotePeer) return Variable::createError(-2, "Unknown remote peer.");
		}

		return Peer::getParamsetDescription(clientInfo, channel, parameterGroup, checkAcls);
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

PVariable Peer::getParamsetId(PRpcClientInfo clientInfo, uint32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(_rpcDevice->functions.find(channel) == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
		PFunction rpcFunction = _rpcDevice->functions.at(channel);
		std::shared_ptr<BasicPeer> remotePeer;
		if(type == ParameterGroup::Type::link && remoteID > 0)
		{
			remotePeer = getPeer(channel, remoteID, remoteChannel);
			if(!remotePeer) return Variable::createError(-2, "Unknown remote peer.");
		}

		std::string id;
		if(type == ParameterGroup::Type::Enum::config) id = rpcFunction->configParameters->id;
		else if(type == ParameterGroup::Type::Enum::variables) id = rpcFunction->variables->id;
		else if(type == ParameterGroup::Type::Enum::link) id = rpcFunction->linkParameters->id;
		int32_t pos = id.find_last_of("--");
		if(pos > 0) id = id.substr(0, pos - 1);
		return PVariable(new Variable(id));
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

PVariable Peer::getServiceMessages(PRpcClientInfo clientInfo, bool returnID)
{
	if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
	if(!serviceMessages) return Variable::createError(-32500, "Service messages are not initialized.");
	return serviceMessages->get(clientInfo, returnID);
}

PVariable Peer::getValue(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");
		if(valueKey == "IP_ADDRESS") return PVariable(new Variable(_ip));
		else if(valueKey == "PEER_ID") return PVariable(new Variable((int32_t)_peerID));
		std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator channelIterator = valuesCentral.find(channel);
		if(channelIterator == valuesCentral.end()) return Variable::createError(-2, "Unknown channel.");
		std::unordered_map<std::string, RpcConfigurationParameter>::iterator parameterIterator = channelIterator->second.find(valueKey);
		if(parameterIterator == channelIterator->second.end()) return Variable::createError(-5, "Unknown parameter.");

		//Check if channel still exists in device description
		Functions::iterator functionIterator = _rpcDevice->functions.find(channel);
		if(functionIterator == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel (2).");

		PParameterGroup parameterGroup = getParameterSet(channel, ParameterGroup::Type::Enum::variables);
		PParameter parameter = parameterGroup->parameters.at(valueKey);
		if(!parameter) return Variable::createError(-5, "Unknown parameter.");
		if(!parameter->readable && !parameter->transmitted) return Variable::createError(-6, "Parameter is not readable.");
		PVariable variable;
		if(requestFromDevice)
		{
			variable = getValueFromDevice(parameter, channel, asynchronous);
			if(parameter->password) variable.reset(new Variable(variable->type));
			if((!asynchronous && variable->type != VariableType::tVoid) || variable->errorStruct) return variable;
		}
		std::vector<uint8_t> parameterData = parameterIterator->second.getBinaryData();
		if(!convertFromPacketHook(parameter, parameterData, variable)) variable = parameter->convertFromPacket(parameterData);
		if(parameter->password) variable.reset(new Variable(variable->type));
		return variable;
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

PVariable Peer::getVariableDescription(PRpcClientInfo clientInfo, Parameters::iterator& parameterIterator, int32_t channel, ParameterGroup::Type::Enum type, int32_t index)
{
	try
	{
		if(!parameterIterator->second || parameterIterator->second->id.empty() || !parameterIterator->second->visible) return Variable::createError(-5, "Unknown parameter.");
		if(!parameterIterator->second->visible && !parameterIterator->second->service && !parameterIterator->second->internal  && !parameterIterator->second->transform)
		{
			_bl->out.printDebug("Debug: Omitting parameter " + parameterIterator->second->id + " because of it's ui flag.");
			 return Variable::createError(-5, "Unknown parameter (1).");
		}
#ifdef CCU2
			if(parameterIterator->second->logical->type == ILogical::Type::tInteger64) continue;
#endif
		if(clientInfo->clientType == RpcClientType::ccu2 && !parameterIterator->second->ccu2Visible) return Variable::createError(-5, "Parameter is invisible on the CCU2.");

		PVariable description = std::make_shared<Variable>(VariableType::tStruct);

		int32_t operations = 0;
		if(parameterIterator->second->readable) operations += 4;
		if(parameterIterator->second->writeable) operations += 2;
        if(parameterIterator->second->transmitted) operations += 1;
		int32_t uiFlags = 0;
		if(parameterIterator->second->visible) uiFlags += 1;
		if(parameterIterator->second->internal) uiFlags += 2;
		if(parameterIterator->second->transform) uiFlags += 4;
		if(parameterIterator->second->service) uiFlags += 8;
		if(parameterIterator->second->sticky) uiFlags += 0x10;
		if(parameterIterator->second->logical->type == ILogical::Type::tBoolean)
		{
			LogicalBoolean* parameter = (LogicalBoolean*)parameterIterator->second->logical.get();

			if(!parameterIterator->second->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameterIterator->second->control))));
			if(parameter->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(parameter->defaultValue))));
			description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			description->structValue->insert(StructElement("ID", PVariable(new Variable(parameterIterator->second->id))));
			description->structValue->insert(StructElement("MAX", PVariable(new Variable(true))));
			description->structValue->insert(StructElement("MIN", PVariable(new Variable(false))));
			description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
			if(index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
			description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("BOOL")))));
		}
		else if(parameterIterator->second->logical->type == ILogical::Type::tString)
		{
			LogicalString* parameter = (LogicalString*)parameterIterator->second->logical.get();

			if(!parameterIterator->second->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameterIterator->second->control))));
			if(parameter->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(parameter->defaultValue))));
			description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			description->structValue->insert(StructElement("ID", PVariable(new Variable(parameterIterator->second->id))));
			description->structValue->insert(StructElement("MAX", PVariable(new Variable(std::string("")))));
			description->structValue->insert(StructElement("MIN", PVariable(new Variable(std::string("")))));
			description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
			if(index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
			description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRING")))));
		}
		else if(parameterIterator->second->logical->type == ILogical::Type::tAction)
		{
			LogicalAction* parameter = (LogicalAction*)parameterIterator->second->logical.get();

			if(!parameterIterator->second->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameterIterator->second->control))));
			if(parameter->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(parameter->defaultValue)))); //CCU needs this, otherwise updates are not processed in programs
			description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			description->structValue->insert(StructElement("ID", PVariable(new Variable(parameterIterator->second->id))));
			description->structValue->insert(StructElement("MAX", PVariable(new Variable(true))));
			description->structValue->insert(StructElement("MIN", PVariable(new Variable(false))));
			description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations & 0xFE)))); //Remove read
			if(index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
			description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ACTION")))));
		}
		else if(parameterIterator->second->logical->type == ILogical::Type::tInteger)
		{
			LogicalInteger* parameter = (LogicalInteger*)parameterIterator->second->logical.get();

			if(!parameterIterator->second->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameterIterator->second->control))));
			if(parameter->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(parameter->defaultValue))));
			description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			description->structValue->insert(StructElement("ID", PVariable(new Variable(parameterIterator->second->id))));
			description->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));
			description->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
			description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));

			if(!parameter->specialValuesStringMap.empty())
			{
				PVariable specialValues(new Variable(VariableType::tArray));
				for(std::unordered_map<std::string, int32_t>::iterator j = parameter->specialValuesStringMap.begin(); j != parameter->specialValuesStringMap.end(); ++j)
				{
					PVariable specialElement(new Variable(VariableType::tStruct));
					specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
					specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
					specialValues->arrayValue->push_back(specialElement);
				}
				description->structValue->insert(StructElement("SPECIAL", specialValues));
			}

			if(index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
			description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER")))));
		}
		else if(parameterIterator->second->logical->type == ILogical::Type::tInteger64)
		{
			LogicalInteger64* parameter = (LogicalInteger64*)parameterIterator->second->logical.get();

			if(!parameterIterator->second->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameterIterator->second->control))));
			if(parameter->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(parameter->defaultValue))));
			description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			description->structValue->insert(StructElement("ID", PVariable(new Variable(parameterIterator->second->id))));
			description->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));
			description->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
			description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));

			if(!parameter->specialValuesStringMap.empty())
			{
				PVariable specialValues(new Variable(VariableType::tArray));
				for(std::unordered_map<std::string, int64_t>::iterator j = parameter->specialValuesStringMap.begin(); j != parameter->specialValuesStringMap.end(); ++j)
				{
					PVariable specialElement(new Variable(VariableType::tStruct));
					specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
					specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
					specialValues->arrayValue->push_back(specialElement);
				}
				description->structValue->insert(StructElement("SPECIAL", specialValues));
			}

			if(index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
			description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("INTEGER64")))));
		}
		else if(parameterIterator->second->logical->type == ILogical::Type::tEnum)
		{
			LogicalEnumeration* parameter = (LogicalEnumeration*)parameterIterator->second->logical.get();

			if(!parameterIterator->second->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameterIterator->second->control))));
			description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(parameter->defaultValueExists ? parameter->defaultValue : 0))));
			description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			description->structValue->insert(StructElement("ID", PVariable(new Variable(parameterIterator->second->id))));
			description->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));
			description->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
			description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
			if(index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
			description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ENUM")))));

			PVariable valueList(new Variable(VariableType::tArray));
			for(std::vector<EnumerationValue>::iterator j = parameter->values.begin(); j != parameter->values.end(); ++j)
			{
				valueList->arrayValue->push_back(PVariable(new Variable(j->id)));
			}
			description->structValue->insert(StructElement("VALUE_LIST", valueList));
		}
		else if(parameterIterator->second->logical->type == ILogical::Type::tFloat)
		{
			LogicalDecimal* parameter = (LogicalDecimal*)parameterIterator->second->logical.get();

			if(!parameterIterator->second->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameterIterator->second->control))));
			if(parameter->defaultValueExists) description->structValue->insert(StructElement("DEFAULT", PVariable(new Variable(parameter->defaultValue))));
			description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			description->structValue->insert(StructElement("ID", PVariable(new Variable(parameterIterator->second->id))));
			description->structValue->insert(StructElement("MAX", PVariable(new Variable(parameter->maximumValue))));
			description->structValue->insert(StructElement("MIN", PVariable(new Variable(parameter->minimumValue))));
			description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));

			if(!parameter->specialValuesStringMap.empty())
			{
				PVariable specialValues(new Variable(VariableType::tArray));
				for(std::unordered_map<std::string, double>::iterator j = parameter->specialValuesStringMap.begin(); j != parameter->specialValuesStringMap.end(); ++j)
				{
					PVariable specialElement(new Variable(VariableType::tStruct));
					specialElement->structValue->insert(StructElement("ID", PVariable(new Variable(j->first))));
					specialElement->structValue->insert(StructElement("VALUE", PVariable(new Variable(j->second))));
					specialValues->arrayValue->push_back(specialElement);
				}
				description->structValue->insert(StructElement("SPECIAL", specialValues));
			}

			if(index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
			description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("FLOAT")))));
		}
		else if(parameterIterator->second->logical->type == ILogical::Type::tArray)
		{
			if(!clientInfo->initNewFormat) return Variable::createError(-5, "Parameter is unsupported by this client.");
			if(!parameterIterator->second->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameterIterator->second->control))));
			description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			description->structValue->insert(StructElement("ID", PVariable(new Variable(parameterIterator->second->id))));
			description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
			if(index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
			description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("ARRAY")))));
		}
		else if(parameterIterator->second->logical->type == ILogical::Type::tStruct)
		{
			if(!clientInfo->initNewFormat) return Variable::createError(-5, "Parameter is unsupported by this client.");
			if(!parameterIterator->second->control.empty()) description->structValue->insert(StructElement("CONTROL", PVariable(new Variable(parameterIterator->second->control))));
			description->structValue->insert(StructElement("FLAGS", PVariable(new Variable(uiFlags))));
			description->structValue->insert(StructElement("ID", PVariable(new Variable(parameterIterator->second->id))));
			description->structValue->insert(StructElement("OPERATIONS", PVariable(new Variable(operations))));
			if(index != -1) description->structValue->insert(StructElement("TAB_ORDER", PVariable(new Variable(index))));
			description->structValue->insert(StructElement("TYPE", PVariable(new Variable(std::string("STRUCT")))));
		}

		description->structValue->insert(StructElement("UNIT", PVariable(new Variable(parameterIterator->second->unit))));
		if(!parameterIterator->second->formFieldType.empty()) description->structValue->insert(StructElement("FORM_FIELD_TYPE", PVariable(new Variable(parameterIterator->second->formFieldType))));
		if(parameterIterator->second->formPosition != -1) description->structValue->insert(StructElement("FORM_POSITION", PVariable(new Variable(parameterIterator->second->formPosition))));

        if(type == ParameterGroup::Type::Enum::variables)
        {
            std::unordered_map<uint32_t, std::unordered_map<std::string, RpcConfigurationParameter>>::iterator valuesCentralIterator = valuesCentral.find(channel);
            if(valuesCentralIterator == valuesCentral.end()) return Variable::createError(-5, "Unknown parameter (2).");
            std::unordered_map<std::string, RpcConfigurationParameter>::iterator valueParameterIterator = valuesCentralIterator->second.find(parameterIterator->second->id);
            if(valueParameterIterator == valuesCentralIterator->second.end()) return Variable::createError(-5, "Unknown parameter (3).");

            auto room = valueParameterIterator->second.getRoom();
            if(room != 0) description->structValue->emplace("ROOM", std::make_shared<Variable>(room));

            auto categories = valueParameterIterator->second.getCategories();
            if(!categories.empty())
            {
                PVariable categoriesResult = std::make_shared<Variable>(VariableType::tArray);
                categoriesResult->arrayValue->reserve(categories.size());
                for(auto category : categories)
                {
                    categoriesResult->arrayValue->push_back(std::make_shared<Variable>(category));
                }
                description->structValue->emplace("CATEGORIES", categoriesResult);
            }
        }

        std::shared_ptr<ICentral> central = getCentral();
        if(!central) return description;
		std::string language = clientInfo ? clientInfo->language : "en-US";
		std::string filename = _rpcDevice->getFilename();
		auto parameterTranslations = central->getTranslations()->getParameterTranslations(filename, language, parameterIterator->second->parent()->type(), parameterIterator->second->parent()->id, parameterIterator->second->id);
		if(!parameterTranslations.first.empty()) description->structValue->insert(StructElement("LABEL", std::shared_ptr<Variable>(new Variable(parameterTranslations.first))));
		if(!parameterTranslations.second.empty()) description->structValue->insert(StructElement("DESCRIPTION", std::shared_ptr<Variable>(new Variable(parameterTranslations.second))));

		return description;
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

PVariable Peer::getVariableDescription(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");

		PParameterGroup parameterGroup = getParameterSet(channel, ParameterGroup::Type::Enum::variables);
		if(!parameterGroup) return Variable::createError(-2, "Unknown channel.");

		Parameters::iterator parameterIterator = parameterGroup->parameters.find(valueKey);
		if(parameterIterator == parameterGroup->parameters.end()) return Variable::createError(-5, "Unknown parameter.");

		return getVariableDescription(clientInfo, parameterIterator, channel, ParameterGroup::Type::Enum::variables, -1);
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

PVariable Peer::getVariablesInCategory(PRpcClientInfo clientInfo, uint64_t categoryId, bool checkAcls)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");

		auto central = getCentral();
		if(!central) return Variable::createError(-32500, "Could not get central.");
		auto me = central->getPeer(_peerID);
		if(!me) return Variable::createError(-32500, "Could not get peer object.");

		auto channels = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

		for(auto& channelIterator : valuesCentral)
		{
			auto variables = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
			variables->arrayValue->reserve(channelIterator.second.size());
			for(auto& variableIterator : channelIterator.second)
			{
				if(checkAcls && !clientInfo->acls->checkVariableReadAccess(me, channelIterator.first, variableIterator.first)) continue;
				if(variableIterator.second.hasCategory(categoryId)) variables->arrayValue->push_back(std::make_shared<BaseLib::Variable>(variableIterator.first));
			}
			if(!variables->arrayValue->empty()) channels->structValue->emplace(std::to_string(channelIterator.first), variables);
		}

		return channels;
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

PVariable Peer::getVariablesInRoom(PRpcClientInfo clientInfo, uint64_t roomId, bool checkAcls)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(!_rpcDevice) return Variable::createError(-32500, "Unknown application error.");

		auto central = getCentral();
		if(!central) return Variable::createError(-32500, "Could not get central.");
		auto me = central->getPeer(_peerID);
		if(!me) return Variable::createError(-32500, "Could not get peer object.");

		auto channels = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

		for(auto& channelIterator : valuesCentral)
		{
			auto variables = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
			variables->arrayValue->reserve(channelIterator.second.size());
			for(auto& variableIterator : channelIterator.second)
			{
				if(checkAcls && !clientInfo->acls->checkVariableReadAccess(me, channelIterator.first, variableIterator.first)) continue;
				if(variableIterator.second.getRoom() == roomId) variables->arrayValue->push_back(std::make_shared<BaseLib::Variable>(variableIterator.first));
			}
			if(!variables->arrayValue->empty()) channels->structValue->emplace(std::to_string(channelIterator.first), variables);
		}

		return channels;
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

PVariable Peer::reportValueUsage(PRpcClientInfo clientInfo)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		return PVariable(new Variable(!serviceMessages->getConfigPending()));
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

PVariable Peer::rssiInfo(PRpcClientInfo clientInfo)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(!wireless()) return Variable::createError(-100, "Peer is not a wireless peer.");
		if(valuesCentral.find(0) == valuesCentral.end() || valuesCentral.at(0).find("RSSI_DEVICE") == valuesCentral.at(0).end() || !valuesCentral.at(0).at("RSSI_DEVICE").rpcParameter)
		{
			return Variable::createError(-101, "Peer has no rssi information.");
		}
		PVariable response(new Variable(VariableType::tStruct));
		PVariable rpcArray(new Variable(VariableType::tArray));

		std::vector<uint8_t> parameterData;
		PVariable element;
		if(valuesCentral.at(0).find("RSSI_PEER") != valuesCentral.at(0).end() && valuesCentral.at(0).at("RSSI_PEER").rpcParameter)
		{
			parameterData = valuesCentral.at(0).at("RSSI_PEER").getBinaryData();
			element = valuesCentral.at(0).at("RSSI_PEER").rpcParameter->convertFromPacket(parameterData);
			if(element->integerValue == 0) element->integerValue = 65536;
			rpcArray->arrayValue->push_back(element);
		}
		else
		{
			element = PVariable(new Variable(65536));
			rpcArray->arrayValue->push_back(element);
		}

		parameterData = valuesCentral.at(0).at("RSSI_DEVICE").getBinaryData();
		element = valuesCentral.at(0).at("RSSI_DEVICE").rpcParameter->convertFromPacket(parameterData);
		if(element->integerValue == 0) element->integerValue = 65536;
		rpcArray->arrayValue->push_back(element);

		std::shared_ptr<ICentral> central = getCentral();
		if(!central) return Variable::createError(-32500, "Central is nullptr.");
		response->structValue->insert(StructElement(central->getSerialNumber(), rpcArray));
		return response;
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

PVariable Peer::setLinkInfo(PRpcClientInfo clientInfo, int32_t senderChannel, uint64_t receiverID, int32_t receiverChannel, std::string name, std::string description)
{
	try
	{
		std::shared_ptr<BasicPeer> remotePeer = getPeer(senderChannel, receiverID, receiverChannel);
		if(!remotePeer)	return Variable::createError(-2, "No peer found for sender channel..");
		remotePeer->linkDescription = description;
		remotePeer->linkName = name;
		savePeers();
		return PVariable(new Variable(VariableType::tVoid));
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

PVariable Peer::setId(PRpcClientInfo clientInfo, uint64_t newPeerId)
{
	try
	{
		if(newPeerId == 0 || newPeerId >= 0x40000000) return Variable::createError(-100, "New peer ID is invalid.");
		if(newPeerId == _peerID) return Variable::createError(-100, "New peer ID is the same as the old one.");

		std::shared_ptr<ICentral> central(getCentral());
		if(central)
		{
			std::shared_ptr<Peer> peer = central->getPeer(newPeerId);
			if(peer) return Variable::createError(-101, "New peer ID is already in use.");
			if(!_bl->db->setPeerID(_peerID, newPeerId)) return Variable::createError(-32500, "Error setting id. See log for more details.");
			_peerID = newPeerId;
			if(serviceMessages) serviceMessages->setPeerId(newPeerId);
			return PVariable(new Variable(VariableType::tVoid));
		}
		else return Variable::createError(-32500, "Application error. Central could not be found.");
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
    return Variable::createError(-32500, "Unknown application error. See error log for more details.");
}

PVariable Peer::setValue(PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, PVariable value, bool wait)
{
	try
	{
		if(_disposing) return Variable::createError(-32500, "Peer is disposing.");
		if(valueKey.empty()) return Variable::createError(-5, "Value key is empty.");
		if(channel == 0 && serviceMessages->set(valueKey, value->booleanValue)) return PVariable(new Variable(VariableType::tVoid));
		auto channelIterator = valuesCentral.find(channel);
		if(channelIterator == valuesCentral.end()) return Variable::createError(-2, "Unknown channel.");
		auto parameterIterator = channelIterator->second.find(valueKey);
		if(parameterIterator == channelIterator->second.end()) return Variable::createError(-5, "Unknown parameter.");
		RpcConfigurationParameter& parameter = parameterIterator->second;
		PParameter rpcParameter = parameter.rpcParameter;
		if(!rpcParameter) return Variable::createError(-5, "Unknown parameter.");

		value->setType(rpcParameter->logical->type);

		//Perform operation on value
		if(value->stringValue.size() > 2 && value->stringValue.at(1) == '='
				&& (value->stringValue.at(0) == '+' || value->stringValue.at(0) == '-' || value->stringValue.at(0) == '*' || value->stringValue.at(0) == '/'))
		{
			std::vector<uint8_t> parameterData = parameter.getBinaryData();
			PVariable currentValue;
			if(!convertFromPacketHook(rpcParameter, parameterData, currentValue)) currentValue = rpcParameter->convertFromPacket(parameterData);
			if(rpcParameter->logical->type == ILogical::Type::Enum::tFloat)
			{
				std::string numberPart = value->stringValue.substr(2);
				double factor = Math::getDouble(numberPart);
				if(factor == 0) return Variable::createError(-1, "Factor is \"0\" or no valid number.");
				if(value->stringValue.at(0) == '+') value->floatValue = currentValue->floatValue + factor;
				else if(value->stringValue.at(0) == '-') value->floatValue = currentValue->floatValue - factor;
				else if(value->stringValue.at(0) == '*') value->floatValue = currentValue->floatValue * factor;
				else if(value->stringValue.at(0) == '/') value->floatValue = currentValue->floatValue / factor;
				value->type = VariableType::tFloat;
				value->stringValue.clear();
			}
			else if(rpcParameter->logical->type == ILogical::Type::Enum::tInteger)
			{
				std::string numberPart = value->stringValue.substr(2);
				int32_t factor = Math::getNumber(numberPart);
				if(factor == 0) return Variable::createError(-1, "Factor is \"0\" or no valid number.");
				if(value->stringValue.at(0) == '+') value->integerValue = currentValue->integerValue + factor;
				else if(value->stringValue.at(0) == '-') value->integerValue = currentValue->integerValue - factor;
				else if(value->stringValue.at(0) == '*') value->integerValue = currentValue->integerValue * factor;
				else if(value->stringValue.at(0) == '/') value->integerValue = currentValue->integerValue / factor;
				value->integerValue64 = value->integerValue;
				value->type = VariableType::tInteger;
				value->stringValue.clear();
			}
			else if(rpcParameter->logical->type == ILogical::Type::Enum::tInteger64)
			{
				std::string numberPart = value->stringValue.substr(2);
				int32_t factor = Math::getNumber(numberPart);
				if(factor == 0) return Variable::createError(-1, "Factor is \"0\" or no valid number.");
				if(value->stringValue.at(0) == '+') value->integerValue64 = currentValue->integerValue64 + factor;
				else if(value->stringValue.at(0) == '-') value->integerValue64 = currentValue->integerValue64 - factor;
				else if(value->stringValue.at(0) == '*') value->integerValue64 = currentValue->integerValue64 * factor;
				else if(value->stringValue.at(0) == '/') value->integerValue64 = currentValue->integerValue64 / factor;
				value->integerValue = value->integerValue64;
				value->type = VariableType::tInteger64;
				value->stringValue.clear();
			}
		}
		else if(value->stringValue == "!") // Toggle boolean
		{
			std::vector<uint8_t> parameterData = parameter.getBinaryData();
			PVariable currentValue;
			if(!convertFromPacketHook(rpcParameter, parameterData, currentValue)) currentValue = rpcParameter->convertFromPacket(parameterData);
			if(rpcParameter->logical->type == ILogical::Type::Enum::tBoolean)
			{
				value->booleanValue = !currentValue->booleanValue;
				value->type = VariableType::tBoolean;
				value->stringValue.clear();
			}
		}
		return PVariable(new Variable(VariableType::tVoid));
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
    return Variable::createError(-32500, "Unknown application error. See error log for more details.");
}

//End RPC methods
}
}
