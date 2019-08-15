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

#include "ICentral.h"
#include "../BaseLib.h"
#include "../Security/Acls.h"

namespace BaseLib
{
namespace Systems
{

ICentral::ICentral(int32_t deviceFamily, BaseLib::SharedObjects* baseLib, ICentralEventSink* eventHandler)
{
	_bl = baseLib;
	_deviceFamily = deviceFamily;
	setEventHandler(eventHandler);
	_initialized = false;
	_disposing = false;
    _pairing = false;
    _timeLeftInPairingMode = 0;
	_translations = std::make_shared<DeviceTranslations>(baseLib, deviceFamily);
}

ICentral::ICentral(int32_t deviceFamily, BaseLib::SharedObjects* baseLib, uint32_t deviceId, std::string serialNumber, int32_t address, ICentralEventSink* eventHandler) : ICentral(deviceFamily, baseLib, eventHandler)
{
	_deviceId = deviceId;
	_serialNumber = serialNumber;
	_address = address;
}

ICentral::~ICentral()
{
}

void ICentral::dispose(bool wait)
{
	_disposing = true;
	_peers.clear();
	_peersBySerial.clear();
	_peersById.clear();
}

// {{{ Event handling
	void ICentral::raiseAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onAddWebserverEventHandler(eventHandler, eventHandlers);
	}

	void ICentral::raiseRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRemoveWebserverEventHandler(eventHandlers);
	}

	void ICentral::raiseRPCEvent(std::string& source, uint64_t id, int32_t channel, std::string& deviceAddress, std::shared_ptr<std::vector<std::string>>& valueKeys, std::shared_ptr<std::vector<PVariable>>& values)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRPCEvent(source, id, channel, deviceAddress, valueKeys, values);
	}

	void ICentral::raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRPCUpdateDevice(id, channel, address, hint);
	}

	void ICentral::raiseRPCNewDevices(std::vector<uint64_t>& ids, PVariable deviceDescriptions)
	{
        // {{{ Default implementation for getPairingState
        {
            std::lock_guard<std::mutex> newPeersGuard(_newPeersMutex);
            std::list<int64_t> entriesToRemove;
            for(auto& entry : _newPeersDefault)
            {
                if(entry.first < BaseLib::HelperFunctions::getTimeSeconds() - 60) entriesToRemove.emplace_back(entry.first);
            }

            for(auto entry : entriesToRemove)
            {
                _newPeersDefault.erase(entry);
            }

            auto time = BaseLib::HelperFunctions::getTimeSeconds();
            for(auto id : ids)
            {
                auto pairingState = std::make_shared<PairingState>();
                pairingState->peerId = id;
                pairingState->state = "success";
                _newPeersDefault[time].emplace_back(std::move(pairingState));
            }
        }
        // }}}

		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRPCNewDevices(ids, deviceDescriptions);
	}

	void ICentral::raiseRPCDeleteDevices(std::vector<uint64_t>& ids, PVariable deviceAddresses, PVariable deviceInfo)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRPCDeleteDevices(ids, deviceAddresses, deviceInfo);
	}

	void ICentral::raiseEvent(std::string& source, uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>>& variables, std::shared_ptr<std::vector<PVariable>>& values)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onEvent(source, peerId, channel, variables, values);
	}

	void ICentral::raiseRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRunScript(scriptInfo, wait);
	}

	BaseLib::PVariable ICentral::raiseInvokeRpc(std::string& methodName, BaseLib::PArray& parameters)
	{
		if(_eventHandler) return ((ICentralEventSink*)_eventHandler)->onInvokeRpc(methodName, parameters);
		else return std::make_shared<BaseLib::Variable>();
	}

	uint64_t ICentral::raiseGetRoomIdByName(std::string& name)
	{
		if(_eventHandler) return ((ICentralEventSink*)_eventHandler)->onGetRoomIdByName(name);
		return 0;
	}
// }}}

// {{{ Peer event handling
	void ICentral::onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers)
	{
		raiseAddWebserverEventHandler(eventHandler, eventHandlers);
	}

	void ICentral::onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers)
	{
		raiseRemoveWebserverEventHandler(eventHandlers);
	}

	void ICentral::onRPCEvent(std::string& source, uint64_t id, int32_t channel, std::string& deviceAddress, std::shared_ptr<std::vector<std::string>>& valueKeys, std::shared_ptr<std::vector<PVariable>>& values)
	{
		raiseRPCEvent(source, id, channel, deviceAddress, valueKeys, values);
	}

	void ICentral::onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
	{
		raiseRPCUpdateDevice(id, channel, address, hint);
	}

	void ICentral::onEvent(std::string& source, uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>>& variables, std::shared_ptr<std::vector<PVariable>>& values)
	{
		raiseEvent(source, peerId, channel, variables, values);
	}

	void ICentral::onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait)
	{
		raiseRunScript(scriptInfo, wait);
	}

	BaseLib::PVariable ICentral::onInvokeRpc(std::string& methodName, BaseLib::PArray& parameters)
	{
		return raiseInvokeRpc(methodName, parameters);
	}
// }}}

std::vector<std::shared_ptr<Peer>> ICentral::getPeers()
{
	try
	{
		std::vector<std::shared_ptr<Peer>> peers;
		std::lock_guard<std::mutex> peersGuard(_peersMutex);
		peers.reserve(_peersById.size());
		for(std::map<uint64_t, std::shared_ptr<Peer>>::iterator i = _peersById.begin(); i != _peersById.end(); ++i)
		{
			if(i->second) peers.push_back(i->second);
		}
		return peers;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return std::vector<std::shared_ptr<Peer>>();
}

std::shared_ptr<Peer> ICentral::getPeer(int32_t address)
{
	try
	{
		std::lock_guard<std::mutex> peersGuard(_peersMutex);
		std::unordered_map<int32_t, std::shared_ptr<Peer>>::iterator peerIterator = _peers.find(address);
		if(peerIterator != _peers.end())
		{
			std::shared_ptr<Peer> peer = peerIterator->second;
			return peer;
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return std::shared_ptr<Peer>();
}

std::shared_ptr<Peer> ICentral::getPeer(uint64_t id)
{
	try
	{
		std::lock_guard<std::mutex> peersGuard(_peersMutex);
		std::map<uint64_t, std::shared_ptr<Peer>>::iterator peerIterator = _peersById.find(id);
		if(peerIterator != _peersById.end())
		{
			std::shared_ptr<Peer> peer = peerIterator->second;
			return peer;
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return std::shared_ptr<Peer>();
}

std::shared_ptr<Peer> ICentral::getPeer(std::string serialNumber)
{
	try
	{
		std::lock_guard<std::mutex> peersGuard(_peersMutex);
		std::unordered_map<std::string, std::shared_ptr<Peer>>::iterator peerIterator = _peersBySerial.find(serialNumber);
		if(peerIterator != _peersBySerial.end())
		{
			std::shared_ptr<Peer> peer = peerIterator->second;
			return peer;
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return std::shared_ptr<Peer>();
}

bool ICentral::peerExists(int32_t address)
{
	try
	{
		std::lock_guard<std::mutex> peersGuard(_peersMutex);
		if(_peers.find(address) != _peers.end()) return true;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

bool ICentral::peerExists(uint64_t id)
{
	try
	{
		std::lock_guard<std::mutex> peersGuard(_peersMutex);
		if(_peersById.find(id) != _peersById.end()) return true;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

bool ICentral::peerExists(std::string serialNumber)
{
	try
	{
		std::lock_guard<std::mutex> peersGuard(_peersMutex);
		if(_peersBySerial.find(serialNumber) != _peersBySerial.end()) return true;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

void ICentral::setPeerId(uint64_t oldPeerId, uint64_t newPeerId)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(oldPeerId);
		if(!peer) return;
		{
			std::lock_guard<std::mutex> peersGuard(_peersMutex);
			if(_peersById.find(oldPeerId) != _peersById.end()) _peersById.erase(oldPeerId);
			_peersById[newPeerId] = peer;
		}

		std::vector<std::shared_ptr<Peer>> peers = getPeers();
		for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
		{
			(*i)->updatePeer(oldPeerId, newPeerId);
		}
	}
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

int32_t ICentral::deviceFamily()
{
	return _deviceFamily;
}

void ICentral::homegearStarted()
{
	try
	{
		std::vector<std::shared_ptr<Peer>> peers = getPeers();
		for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
		{
			(*i)->homegearStarted();
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void ICentral::homegearShuttingDown()
{
	try
	{
		std::vector<std::shared_ptr<Peer>> peers = getPeers();
		for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
		{
			(*i)->homegearShuttingDown();
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void ICentral::load()
{
	try
	{
		loadVariables();
		loadPeers();
	}
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void ICentral::save(bool full)
{
	try
	{
		if(full)
		{
			uint64_t result = _bl->db->saveDevice(_deviceId, _address, _serialNumber, 0xFFFFFFFD, (uint32_t)_deviceFamily);
			if(_deviceId == 0) _deviceId = result;
		}
		saveVariables();
		savePeers(full);
	}
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void ICentral::saveVariable(uint32_t index, int64_t intValue)
{
	try
	{
		bool idIsKnown = _variableDatabaseIds.find(index) != _variableDatabaseIds.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIds[index])));
			_bl->db->saveDeviceVariableAsynchronous(data);
		}
		else
		{
			if(_deviceId == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_deviceId)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			_bl->db->saveDeviceVariableAsynchronous(data);
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void ICentral::saveVariable(uint32_t index, std::string& stringValue)
{
	try
	{
		bool idIsKnown = _variableDatabaseIds.find(index) != _variableDatabaseIds.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(stringValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIds[index])));
			_bl->db->saveDeviceVariableAsynchronous(data);
		}
		else
		{
			if(_deviceId == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_deviceId)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(stringValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			_bl->db->saveDeviceVariableAsynchronous(data);
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void ICentral::saveVariable(uint32_t index, std::vector<uint8_t>& binaryValue)
{
	try
	{
		bool idIsKnown = _variableDatabaseIds.find(index) != _variableDatabaseIds.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIds[index])));
			_bl->db->saveDeviceVariableAsynchronous(data);
		}
		else
		{
			if(_deviceId == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_deviceId)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
			_bl->db->saveDeviceVariableAsynchronous(data);
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void ICentral::deletePeersFromDatabase()
{
	try
	{
		_bl->db->deletePeers(_deviceId);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

uint64_t ICentral::getPeerIdFromSerial(std::string& serialNumber)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(serialNumber);
		if(peer) return peer->getID();
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return 0;
}

//RPC methods
PVariable ICentral::addCategoryToChannel(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, uint64_t categoryId)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(peerId);
		if(!peer) return Variable::createError(-2, "Unknown device.");

		return std::make_shared<Variable>(peer->addCategory(channel, categoryId));
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::addChannelToRoom(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, uint64_t roomId)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(peerId);
		if(!peer) return Variable::createError(-2, "Unknown device.");

		return std::make_shared<Variable>(peer->setRoom(channel, roomId));
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getAllConfig(PRpcClientInfo clientInfo, uint64_t peerId, bool checkAcls)
{
	try
	{
		PVariable array(new Variable(VariableType::tArray));

		if(peerId > 0)
		{
			std::shared_ptr<Peer> peer = getPeer(peerId);
			if(!peer) return Variable::createError(-2, "Unknown device.");
			PVariable config = peer->getAllConfig(clientInfo);
			if(!config) return Variable::createError(-32500, "Unknown application error. Config is nullptr.");
			if(config->errorStruct) return config;
			array->arrayValue->push_back(config);
		}
		else
		{
			//Copy all peers first, because getAllConfig takes very long and we don't want to lock _peersMutex too long
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(*i)) continue;

				PVariable config = (*i)->getAllConfig(clientInfo);
				if(!config || config->errorStruct) continue;
				array->arrayValue->push_back(config);
			}
		}

		return array;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getAllValues(PRpcClientInfo clientInfo, BaseLib::PArray peerIds, bool returnWriteOnly, bool checkAcls)
{
	try
	{
		PVariable array(new Variable(VariableType::tArray));

		if(!peerIds->empty())
		{
            array->arrayValue->reserve(peerIds->size());

            for(auto& peerId : *peerIds)
            {
                std::shared_ptr<Peer> peer = getPeer((uint64_t)peerId->integerValue64);
                if(!peer)
                {
                    if(peerIds->size() == 1) return Variable::createError(-2, "Unknown device.");
                    else continue;
                }
                PVariable values = peer->getAllValues(clientInfo, returnWriteOnly, checkAcls);
                if(!values) return Variable::createError(-32500, "Unknown application error. Values is nullptr.");
                if(values->errorStruct) return values;
                array->arrayValue->push_back(values);
            }
		}
		else
		{
			//Copy all peers first, because getAllValues takes very long and we don't want to lock _peersMutex too long
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

            array->arrayValue->reserve(peers.size());
			for(auto& peer : peers)
			{
                if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				PVariable values = peer->getAllValues(clientInfo, returnWriteOnly, checkAcls);
				if(!values || values->errorStruct) continue;
				array->arrayValue->push_back(values);
			}
		}

		return array;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getChannelsInCategory(PRpcClientInfo clientInfo, uint64_t categoryId, bool checkAcls)
{
	try
	{
		PVariable result = std::make_shared<Variable>(VariableType::tStruct);
		std::vector<std::shared_ptr<Peer>> peers = getPeers();
		for(auto peer : peers)
		{
			if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

			auto channels = peer->getChannelsInCategory(categoryId);
			PVariable channelResult = std::make_shared<Variable>(VariableType::tArray);
			channelResult->arrayValue->reserve(channels.size());
			for(auto channel : channels)
			{
				channelResult->arrayValue->push_back(std::make_shared<Variable>(channel));
			}

            if(!channelResult->arrayValue->empty()) result->structValue->emplace(std::to_string(peer->getID()), channelResult);
		}
		return result;
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getChannelsInRoom(PRpcClientInfo clientInfo, uint64_t roomId, bool checkAcls)
{
	try
	{
		PVariable result = std::make_shared<Variable>(VariableType::tStruct);
		std::vector<std::shared_ptr<Peer>> peers = getPeers();
		for(auto peer : peers)
		{
			if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

			auto channels = peer->getChannelsInRoom(roomId);
			PVariable roomsResult = std::make_shared<Variable>(VariableType::tArray);
			roomsResult->arrayValue->reserve(channels.size());
			for(auto channel : channels)
			{
				roomsResult->arrayValue->push_back(std::make_shared<Variable>(channel));
			}

			if(!roomsResult->arrayValue->empty()) result->structValue->emplace(std::to_string(peer->getID()), roomsResult);
		}
		return result;
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getConfigParameter(PRpcClientInfo clientInfo, std::string serialNumber, uint32_t channel, std::string name)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(serialNumber));
		if(peer) return peer->getConfigParameter(clientInfo, channel, name);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getConfigParameter(PRpcClientInfo clientInfo, uint64_t id, uint32_t channel, std::string name)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer) return peer->getConfigParameter(clientInfo, channel, name);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getDeviceDescription(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, std::map<std::string, bool> fields)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(serialNumber));
		if(!peer) return Variable::createError(-2, "Unknown device.");

		return peer->getDeviceDescription(clientInfo, channel, fields);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getDeviceDescription(PRpcClientInfo clientInfo, uint64_t id, int32_t channel, std::map<std::string, bool> fields)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(!peer) return Variable::createError(-2, "Unknown device.");

		return peer->getDeviceDescription(clientInfo, channel, fields);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getDeviceInfo(PRpcClientInfo clientInfo, uint64_t id, std::map<std::string, bool> fields, bool checkAcls)
{
	try
	{
		if(id > 0)
		{
			std::shared_ptr<Peer> peer(getPeer(id));
			if(!peer) return Variable::createError(-2, "Unknown device.");

			return peer->getDeviceInfo(clientInfo, fields);
		}
		else
		{
			PVariable array(new Variable(VariableType::tArray));

			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because getDeviceInfo takes very long and we don't want to lock _peersMutex too long
			{
				std::lock_guard<std::mutex> peersGuard(_peersMutex);
				for(auto& peer : _peersById)
				{
					peers.push_back(peer.second);
				}
			}

			for(auto& peer : peers)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				PVariable info = peer->getDeviceInfo(clientInfo, fields);
				if(!info) continue;
				array->arrayValue->push_back(info);
			}

			return array;
		}
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getDevicesInCategory(PRpcClientInfo clientInfo, uint64_t categoryId, bool checkAcls)
{
	try
	{
		PVariable result = std::make_shared<Variable>(VariableType::tArray);
		std::vector<std::shared_ptr<Peer>> peers = getPeers();
		result->arrayValue->reserve(peers.size());
		for(auto peer : peers)
		{
			if(peer->hasCategory(-1, categoryId)) result->arrayValue->push_back(std::make_shared<Variable>(peer->getID()));
		}
		return result;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getDevicesInRoom(PRpcClientInfo clientInfo, uint64_t roomId, bool checkAcls)
{
	try
	{
		PVariable result = std::make_shared<Variable>(VariableType::tArray);
		std::vector<std::shared_ptr<Peer>> peers = getPeers();
		result->arrayValue->reserve(peers.size());
		for(auto peer : peers)
		{
			if(peer->getRoom(-1) == roomId) result->arrayValue->push_back(std::make_shared<Variable>(peer->getID()));
		}
		return result;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getInstallMode(PRpcClientInfo clientInfo)
{
	try
	{
		return PVariable(new Variable(_timeLeftInPairingMode));
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getLinkInfo(PRpcClientInfo clientInfo, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel)
{
	try
	{
		if(senderSerialNumber.empty()) return Variable::createError(-2, "Given sender address is empty.");
		if(receiverSerialNumber.empty()) return Variable::createError(-2, "Given receiver address is empty.");
		std::shared_ptr<Peer> sender(getPeer(senderSerialNumber));
		std::shared_ptr<Peer> receiver(getPeer(receiverSerialNumber));
		if(!sender) return Variable::createError(-2, "Sender device not found.");
		if(!receiver) return Variable::createError(-2, "Receiver device not found.");
		return sender->getLinkInfo(clientInfo, senderChannel, receiver->getID(), receiverChannel);
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getLinkInfo(PRpcClientInfo clientInfo, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel)
{
	try
	{
		if(senderId == 0) return Variable::createError(-2, "Sender id is not set.");
		if(receiverId == 0) return Variable::createError(-2, "Receiver id is not set.");
		std::shared_ptr<Peer> sender(getPeer(senderId));
		std::shared_ptr<Peer> receiver(getPeer(receiverId));
		if(!sender) return Variable::createError(-2, "Sender device not found.");
		if(!receiver) return Variable::createError(-2, "Receiver device not found.");
		return sender->getLinkInfo(clientInfo, senderChannel, receiver->getID(), receiverChannel);
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getLinkPeers(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(serialNumber));
		if(!peer) return Variable::createError(-2, "Unknown device.");
		return peer->getLinkPeers(clientInfo, channel, false);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getLinkPeers(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(peerId));
		if(!peer) return Variable::createError(-2, "Unknown device.");
		return peer->getLinkPeers(clientInfo, channel, true);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getLinks(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, int32_t flags)
{
	try
	{
		if(serialNumber.empty()) return getLinks(clientInfo, 0, -1, flags);
		std::shared_ptr<Peer> peer(getPeer(serialNumber));
		if(!peer) return Variable::createError(-2, "Unknown device.");
		return getLinks(clientInfo, peer->getID(), channel, flags, false);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getLinks(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, int32_t flags, bool checkAcls)
{
	try
	{
		PVariable array(new Variable(VariableType::tArray));
		PVariable element(new Variable(VariableType::tArray));
		if(peerId == 0)
		{
			//Copy all peers first, because getLinks takes very long and we don't want to lock _peersMutex too long
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

			for(auto& peer : peers)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				element = peer->getLink(clientInfo, channel, flags, true);
				array->arrayValue->insert(array->arrayValue->begin(), element->arrayValue->begin(), element->arrayValue->end());
			}
		}
		else
		{
			std::shared_ptr<Peer> peer(getPeer(peerId));
			if(!peer) return Variable::createError(-2, "Unknown device.");
			element = peer->getLink(clientInfo, channel, flags, false);
			array->arrayValue->insert(array->arrayValue->begin(), element->arrayValue->begin(), element->arrayValue->end());
		}
		return array;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getName(PRpcClientInfo clientInfo, uint64_t id, int32_t channel)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer) return PVariable(new Variable(peer->getName(channel)));
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getPairingState(PRpcClientInfo clientInfo)
{
	try
	{
        auto states = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

        states->structValue->emplace("pairingModeEnabled", std::make_shared<BaseLib::Variable>(_pairing));
        states->structValue->emplace("pairingModeEndTime", std::make_shared<BaseLib::Variable>(BaseLib::HelperFunctions::getTimeSeconds() + _timeLeftInPairingMode));

        auto newPeers = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

        {
            std::lock_guard<std::mutex> newPeersGuard(_newPeersMutex);
            for(auto& element : _newPeersDefault)
            {
                for(auto& peer : element.second)
                {
                    auto peerState = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
                    peerState->structValue->emplace("state", std::make_shared<BaseLib::Variable>(peer->state));
                    peerState->structValue->emplace("messageId", std::make_shared<BaseLib::Variable>(peer->messageId));
                    auto variables = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
                    variables->arrayValue->reserve(peer->variables.size());
                    for(auto& variable : peer->variables)
                    {
                        variables->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(variable));
                    }
                    peerState->structValue->emplace("variables", variables);
                    states->structValue->emplace(std::to_string(peer->peerId), std::move(peerState));
                }
            }
        }

        states->structValue->emplace("newPeers", newPeers);

        return states;
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getParamset(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel)
{
	try
	{
		/*if(serialNumber == "BidCoS-RF" && (channel == 0 || channel == -1) && type == Rpc::ParameterSet::Type::Enum::master)
		{
			PVariable paramset(new Variable(VariableType::rpcStruct));
			paramset->structValue->insert(Rpc::RPCStructElement("AES_KEY", PVariable(new Variable(1))));
			return paramset;
		}*/
		if(serialNumber == getSerialNumber() && (channel == 0 || channel == -1) && type == ParameterGroup::Type::Enum::config)
		{
			PVariable paramset(new Variable(VariableType::tStruct));
			return paramset;
		}
		else
		{
			std::shared_ptr<Peer> peer(getPeer(serialNumber));
			if(!peer) return Variable::createError(-2, "Unknown device.");
			uint64_t remoteId = 0;
			if(!remoteSerialNumber.empty())
			{
				std::shared_ptr<Peer> remotePeer(getPeer(remoteSerialNumber));
				if(!remotePeer)
				{
					if(remoteSerialNumber != getSerialNumber()) return Variable::createError(-3, "Remote peer is unknown.");
				}
				else remoteId = remotePeer->getID();
			}
			return peer->getParamset(clientInfo, channel, type, remoteId, remoteChannel, false);
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getParamset(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel, bool checkAcls)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(peerId));
		if(!peer) return Variable::createError(-2, "Unknown device.");
		return peer->getParamset(clientInfo, channel, type, remoteId, remoteChannel, checkAcls);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getParamsetDescription(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel)
{
	try
	{
		if(serialNumber == getSerialNumber() && (channel == 0 || channel == -1) && type == ParameterGroup::Type::Enum::config)
		{
			PVariable descriptions(new Variable(VariableType::tStruct));
			return descriptions;
		}
		else
		{
			std::shared_ptr<Peer> peer(getPeer(serialNumber));
			uint64_t remoteId = 0;
			if(!remoteSerialNumber.empty())
			{
				std::shared_ptr<Peer> remotePeer(getPeer(remoteSerialNumber));
				if(remotePeer) remoteId = remotePeer->getID();
			}
			if(peer) return peer->getParamsetDescription(clientInfo, channel, type, remoteId, remoteChannel, false);
			return Variable::createError(-2, "Unknown device.");
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getParamsetDescription(PRpcClientInfo clientInfo, uint64_t id, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel, bool checkAcls)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer) return peer->getParamsetDescription(clientInfo, channel, type, remoteId, remoteChannel, checkAcls);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getParamsetId(PRpcClientInfo clientInfo, std::string serialNumber, uint32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel)
{
	try
	{
		if(serialNumber == getSerialNumber())
		{
			if(channel > 0) return Variable::createError(-2, "Unknown channel.");
			if(type != ParameterGroup::Type::Enum::config) return Variable::createError(-3, "Unknown parameter set.");
			return PVariable(new Variable(std::string("rf_homegear_central_master")));
		}
		else
		{
			std::shared_ptr<Peer> peer(getPeer(serialNumber));
			uint64_t remoteId = 0;
			if(!remoteSerialNumber.empty())
			{
				std::shared_ptr<Peer> remotePeer(getPeer(remoteSerialNumber));
				if(remotePeer) remoteId = remotePeer->getID();
			}
			if(peer) return peer->getParamsetId(clientInfo, channel, type, remoteId, remoteChannel);
			return Variable::createError(-2, "Unknown device.");
		}
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getParamsetId(PRpcClientInfo clientInfo, uint64_t peerId, uint32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(peerId));
		if(!peer) return Variable::createError(-2, "Unknown device.");
		return peer->getParamsetId(clientInfo, channel, type, remoteId, remoteChannel);
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getPeerId(PRpcClientInfo clientInfo, int32_t filterType, std::string filterValue, bool checkAcls)
{
	try
	{
		PVariable ids(new BaseLib::Variable(BaseLib::VariableType::tArray));
		if(filterType == 1) //Serial number
		{
			std::shared_ptr<Peer> peer = getPeer(filterValue);
			if(peer && (!checkAcls || clientInfo->acls->checkDeviceReadAccess(peer))) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
		}
		else if(filterType == 2) //Physical address
		{
			int32_t address = Math::getNumber(filterValue);
			if(address != 0)
			{
				std::shared_ptr<Peer> peer = getPeer(address);
				if(peer && (!checkAcls || clientInfo->acls->checkDeviceReadAccess(peer))) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
			}
		}
		else if(filterType == 3) //Type id
		{
			uint32_t type = (uint32_t)Math::getNumber(filterValue);
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

			for(auto& peer : peers)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				if(peer->getDeviceType() == type) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
			}
		}
		else if(filterType == 4) //Type string
		{
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

			for(auto& peer : peers)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				if(peer->getTypeString() == filterValue) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
			}
		}
		else if(filterType == 5) //Name
		{
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

			for(auto& peer : peers)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				if(peer->getName().find(filterValue) != std::string::npos) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
			}
		}
		else if(filterType == 6) //Pending config
		{
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

			for(auto& peer : peers)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				if(peer->serviceMessages->getConfigPending()) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
			}
		}
		else if(filterType == 7) //Unreachable
		{
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

			for(auto& peer : peers)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				if(peer->serviceMessages->getUnreach()) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
			}
		}
		else if(filterType == 8) //Reachable
		{
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

			for(auto& peer : peers)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				if(!peer->serviceMessages->getUnreach()) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
			}
		}
		else if(filterType == 9) //Low battery
		{
			std::vector<std::shared_ptr<Peer>> peers = getPeers();

			for(auto& peer : peers)
			{
				if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

				if(peer->serviceMessages->getLowbat()) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
			}
		}
		return ids;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getPeerId(PRpcClientInfo clientInfo, int32_t address)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(address);
		if(!peer) return Variable::createError(-2, "Unknown device.");
		return PVariable(new Variable((int32_t)peer->getID()));
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getPeerId(PRpcClientInfo clientInfo, std::string serialNumber)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(serialNumber);
		if(!peer) return Variable::createError(-2, "Unknown device.");
		return PVariable(new Variable((int32_t)peer->getID()));
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getRolesInRoom(PRpcClientInfo clientInfo, uint64_t roomId, bool checkDeviceAcls, bool checkVariableAcls)
{
    try
    {
        PVariable variables = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

        std::vector<std::shared_ptr<Peer>> peers = getPeers();

        for(std::shared_ptr<Peer> peer : peers)
        {
            if(checkDeviceAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

            auto result = peer->getRolesInRoom(clientInfo, roomId, checkVariableAcls);
            if(!result->structValue->empty()) variables->structValue->emplace(std::to_string(peer->getID()), result);
        }

        return variables;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getServiceMessages(PRpcClientInfo clientInfo, bool returnId, bool checkAcls)
{
	try
	{
		std::vector<std::shared_ptr<Peer>> peers = getPeers();

		PVariable serviceMessages(new Variable(VariableType::tArray));
		for(auto& peer : peers)
		{
			if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

			PVariable messages = peer->getServiceMessages(clientInfo, returnId);
			if(!messages->arrayValue->empty()) serviceMessages->arrayValue->insert(serviceMessages->arrayValue->end(), messages->arrayValue->begin(), messages->arrayValue->end());
		}
		return serviceMessages;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getValue(PRpcClientInfo clientInfo, std::string serialNumber, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(serialNumber));
		if(peer) return peer->getValue(clientInfo, channel, valueKey, requestFromDevice, asynchronous);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getValue(PRpcClientInfo clientInfo, uint64_t id, uint32_t channel, std::string valueKey, bool requestFromDevice, bool asynchronous)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer) return peer->getValue(clientInfo, channel, valueKey, requestFromDevice, asynchronous);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getVariableDescription(PRpcClientInfo clientInfo, uint64_t id, uint32_t channel, std::string valueKey, const std::unordered_set<std::string>& fields)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer) return peer->getVariableDescription(clientInfo, channel, valueKey, fields);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getVariablesInCategory(PRpcClientInfo clientInfo, uint64_t categoryId, bool checkDeviceAcls, bool checkVariableAcls)
{
    try
    {
        PVariable variables = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

        std::vector<std::shared_ptr<Peer>> peers = getPeers();

        for(std::shared_ptr<Peer> peer : peers)
        {
            if(checkDeviceAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

            auto result = peer->getVariablesInCategory(clientInfo, categoryId, checkVariableAcls);
            if(!result->structValue->empty()) variables->structValue->emplace(std::to_string(peer->getID()), result);
        }

        return variables;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getVariablesInRole(PRpcClientInfo clientInfo, uint64_t roleId, bool checkDeviceAcls, bool checkVariableAcls)
{
	try
	{
		PVariable variables = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

		std::vector<std::shared_ptr<Peer>> peers = getPeers();

		for(std::shared_ptr<Peer> peer : peers)
		{
			if(checkDeviceAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

			auto result = peer->getVariablesInRole(clientInfo, roleId, checkVariableAcls);
			if(!result->structValue->empty()) variables->structValue->emplace(std::to_string(peer->getID()), result);
		}

		return variables;
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getVariablesInRoom(PRpcClientInfo clientInfo, uint64_t categoryId, bool checkDeviceAcls, bool checkVariableAcls)
{
    try
    {
        PVariable variables = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

        std::vector<std::shared_ptr<Peer>> peers = getPeers();

        for(std::shared_ptr<Peer> peer : peers)
        {
            if(checkDeviceAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

            auto result = peer->getVariablesInRoom(clientInfo, categoryId, checkVariableAcls);
            if(!result->structValue->empty()) variables->structValue->emplace(std::to_string(peer->getID()), result);
        }

        return variables;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::listDevices(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields, bool checkAcls)
{
	return listDevices(clientInfo, channels, fields, std::shared_ptr<std::set<std::uint64_t>>(), checkAcls);
}

PVariable ICentral::listDevices(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields, std::shared_ptr<std::set<uint64_t>> knownDevices, bool checkAcls)
{
	try
	{
		PVariable array(new Variable(VariableType::tArray));

		std::vector<std::shared_ptr<Peer>> peers = getPeers();

		for(std::shared_ptr<Peer> peer : peers)
		{
			if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

			if(knownDevices && knownDevices->find(peer->getID()) != knownDevices->end()) continue;
			std::shared_ptr<std::vector<PVariable>> descriptions = peer->getDeviceDescriptions(clientInfo, channels, fields);
			if(!descriptions) continue;
			for(PVariable description : *descriptions)
			{
				array->arrayValue->push_back(description);
			}
		}

		return array;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::listTeams(BaseLib::PRpcClientInfo clientInfo, bool checkAcls)
{
	try
	{
		PVariable array(new Variable(VariableType::tArray));

		std::vector<std::shared_ptr<Peer>> peers = getPeers();

		for(auto& peer : peers)
		{
			if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

			std::string serialNumber = peer->getSerialNumber();
			if(serialNumber.empty() || serialNumber.at(0) != '*') continue;
			auto descriptions = peer->getDeviceDescriptions(clientInfo, true, std::map<std::string, bool>());
			if(!descriptions) continue;
			for(auto description : *descriptions)
			{
				array->arrayValue->push_back(description);
			}
		}
		return array;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::putParamset(BaseLib::PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel, PVariable paramset)
{
	try
	{
		auto peer = getPeer(serialNumber);
		uint64_t remoteId = 0;
		if(!remoteSerialNumber.empty())
		{
			auto remotePeer = getPeer(remoteSerialNumber);
			if(!remotePeer) return Variable::createError(-3, "Remote peer is unknown.");
			remoteId = remotePeer->getID();
		}
		if(peer) return peer->putParamset(clientInfo, channel, type, remoteId, remoteChannel, paramset, false);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::putParamset(BaseLib::PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel, PVariable paramset, bool checkAcls)
{
	try
	{
		auto peer = getPeer(peerId);
		if(peer) return peer->putParamset(clientInfo, channel, type, remoteId, remoteChannel, paramset, checkAcls);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
	{
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::removeCategoryFromChannel(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, uint64_t categoryId)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(peerId);
		if(!peer) return Variable::createError(-2, "Unknown device.");

		return std::make_shared<Variable>(peer->removeCategory(channel, categoryId));
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::removeChannelFromRoom(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, uint64_t roomId)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(peerId);
		if(!peer) return Variable::createError(-2, "Unknown device.");
		if(peer->getRoom(channel) == roomId) peer->setRoom(channel, 0);

		return std::make_shared<Variable>();
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::reportValueUsage(PRpcClientInfo clientInfo, std::string serialNumber)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(serialNumber));
		if(!peer) return Variable::createError(-2, "Peer not found.");
		return peer->reportValueUsage(clientInfo);
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::rssiInfo(PRpcClientInfo clientInfo, bool checkAcls)
{
	try
	{
		PVariable response(new Variable(VariableType::tStruct));

		std::vector<std::shared_ptr<Peer>> peers = getPeers();

		for(auto& peer : peers)
		{
			if(checkAcls && !clientInfo->acls->checkDeviceReadAccess(peer)) continue;

			PVariable element = peer->rssiInfo(clientInfo);
			if(!element || element->errorStruct) continue;
			response->structValue->insert(StructElement(peer->getSerialNumber(), element));
		}

		return response;
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::setId(PRpcClientInfo clientInfo, uint64_t oldPeerId, uint64_t newPeerId)
{
	try
	{
		if(oldPeerId == 0 || oldPeerId >= 0x40000000) return Variable::createError(-100, "The current peer ID is invalid.");
		std::shared_ptr<Peer> peer(getPeer(oldPeerId));
		if(!peer) return Variable::createError(-2, "Peer not found.");
		PVariable result = peer->setId(clientInfo, newPeerId);
		if(result->errorStruct) return result;
		setPeerId(oldPeerId, newPeerId);

		// {{{ Send deleteDevices event
			PVariable deviceAddresses(new Variable(VariableType::tArray));
			deviceAddresses->arrayValue->push_back(PVariable(new Variable(peer->getSerialNumber())));

			PVariable deviceInfo(new Variable(VariableType::tStruct));
			deviceInfo->structValue->insert(StructElement("ID", PVariable(new Variable((int32_t)peer->getID()))));
			PVariable channels(new Variable(VariableType::tArray));
			deviceInfo->structValue->insert(StructElement("CHANNELS", channels));

			for(Functions::iterator i = peer->getRpcDevice()->functions.begin(); i != peer->getRpcDevice()->functions.end(); ++i)
			{
				deviceAddresses->arrayValue->push_back(PVariable(new Variable(peer->getSerialNumber() + ":" + std::to_string(i->first))));
				channels->arrayValue->push_back(PVariable(new Variable(i->first)));
			}

            auto oldIds = std::vector<uint64_t>{ oldPeerId };
			raiseRPCDeleteDevices(oldIds, deviceAddresses, deviceInfo);
		// }}}

		// {{{ Send newDevices event
			PVariable deviceDescriptions(new Variable(VariableType::tArray));
			deviceDescriptions->arrayValue = peer->getDeviceDescriptions(nullptr, true, std::map<std::string, bool>());
            auto newIds = std::vector<uint64_t>{ newPeerId };
			raiseRPCNewDevices(newIds, deviceDescriptions);
		// }}}

		return PVariable(new Variable(VariableType::tVoid));
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::setLinkInfo(PRpcClientInfo clientInfo, std::string senderSerialNumber, int32_t senderChannel, std::string receiverSerialNumber, int32_t receiverChannel, std::string name, std::string description)
{
	try
	{
		if(senderSerialNumber.empty()) return Variable::createError(-2, "Given sender address is empty.");
		if(receiverSerialNumber.empty()) return Variable::createError(-2, "Given receiver address is empty.");
		std::shared_ptr<Peer> sender(getPeer(senderSerialNumber));
		std::shared_ptr<Peer> receiver(getPeer(receiverSerialNumber));
		if(!sender) return Variable::createError(-2, "Sender device not found.");
		if(!receiver) return Variable::createError(-2, "Receiver device not found.");
		PVariable result1 = sender->setLinkInfo(clientInfo, senderChannel, receiver->getID(), receiverChannel, name, description);
		PVariable result2 = receiver->setLinkInfo(clientInfo, receiverChannel, sender->getID(), senderChannel, name, description);
		if(result1->errorStruct) return result1;
		if(result2->errorStruct) return result2;
		return PVariable(new Variable(VariableType::tVoid));
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::setLinkInfo(PRpcClientInfo clientInfo, uint64_t senderId, int32_t senderChannel, uint64_t receiverId, int32_t receiverChannel, std::string name, std::string description)
{
	try
	{
		if(senderId == 0) return Variable::createError(-2, "Sender id is not set.");
		if(receiverId == 0) return Variable::createError(-2, "Receiver id is not set.");
		std::shared_ptr<Peer> sender(getPeer(senderId));
		std::shared_ptr<Peer> receiver(getPeer(receiverId));
		if(!sender) return Variable::createError(-2, "Sender device not found.");
		if(!receiver) return Variable::createError(-2, "Receiver device not found.");
		PVariable result1 = sender->setLinkInfo(clientInfo, senderChannel, receiver->getID(), receiverChannel, name, description);
		PVariable result2 = receiver->setLinkInfo(clientInfo, receiverChannel, sender->getID(), senderChannel, name, description);
		if(result1->errorStruct) return result1;
		if(result2->errorStruct) return result2;
		return PVariable(new Variable(VariableType::tVoid));
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::setName(PRpcClientInfo clientInfo, uint64_t id, int32_t channel, std::string name)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer)
		{
			peer->setName(channel, name);
			return PVariable(new Variable(VariableType::tVoid));
		}
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::setValue(PRpcClientInfo clientInfo, std::string serialNumber, uint32_t channel, std::string valueKey, PVariable value, bool wait)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(serialNumber));
		if(peer) return peer->setValue(clientInfo, channel, valueKey, value, wait);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::setValue(PRpcClientInfo clientInfo, uint64_t id, uint32_t channel, std::string valueKey, PVariable value, bool wait)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer) return peer->setValue(clientInfo, channel, valueKey, value, wait);
		return Variable::createError(-2, "Unknown device.");
	}
	catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}
//End RPC methods

}
}
