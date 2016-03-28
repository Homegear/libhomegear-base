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

#include "ICentral.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Systems
{

ICentral::ICentral(int32_t deviceFamily, BaseLib::Obj* baseLib, ICentralEventSink* eventHandler)
{
	_bl = baseLib;
	_deviceFamily = deviceFamily;
	setEventHandler(eventHandler);
}

ICentral::ICentral(int32_t deviceFamily, BaseLib::Obj* baseLib, uint32_t deviceId, std::string serialNumber, int32_t address, ICentralEventSink* eventHandler) : ICentral(deviceFamily, baseLib, eventHandler)
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
	_currentPeer.reset();
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

	void ICentral::raiseRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRPCEvent(id, channel, deviceAddress, valueKeys, values);
	}

	void ICentral::raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRPCUpdateDevice(id, channel, address, hint);
	}

	void ICentral::raiseRPCNewDevices(PVariable deviceDescriptions)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRPCNewDevices(deviceDescriptions);
	}

	void ICentral::raiseRPCDeleteDevices(PVariable deviceAddresses, PVariable deviceInfo)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRPCDeleteDevices(deviceAddresses, deviceInfo);
	}

	void ICentral::raiseEvent(uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onEvent(peerId, channel, variables, values);
	}

	void ICentral::raiseRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait)
	{
		if(_eventHandler) ((ICentralEventSink*)_eventHandler)->onRunScript(scriptInfo, wait);
	}

	int32_t ICentral::raiseIsAddonClient(int32_t clientId)
	{
		if(_eventHandler) return ((ICentralEventSink*)_eventHandler)->onIsAddonClient(clientId);
		return -1;
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

	void ICentral::onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values)
	{
		raiseRPCEvent(id, channel, deviceAddress, valueKeys, values);
	}

	void ICentral::onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
	{
		raiseRPCUpdateDevice(id, channel, address, hint);
	}

	void ICentral::onEvent(uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
	{
		raiseEvent(peerId, channel, variables, values);
	}

	void ICentral::onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait)
	{
		raiseRunScript(scriptInfo, wait);
	}

	int32_t ICentral::onIsAddonClient(int32_t clientId)
	{
		return raiseIsAddonClient(clientId);
	}
// }}}

void ICentral::getPeers(std::vector<std::shared_ptr<Peer>>& peers, std::shared_ptr<std::set<uint64_t>> knownDevices)
{
	try
	{
		_peersMutex.lock();
		for(std::map<uint64_t, std::shared_ptr<Peer>>::iterator i = _peersById.begin(); i != _peersById.end(); ++i)
		{
			if(knownDevices && knownDevices->find(i->first) != knownDevices->end()) continue; //only add unknown devices
			peers.push_back(i->second);
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
}

std::shared_ptr<Peer> ICentral::getPeer(int32_t address)
{
	try
	{
		_peersMutex.lock();
		if(_peers.find(address) != _peers.end())
		{
			std::shared_ptr<Peer> peer(_peers.at(address));
			_peersMutex.unlock();
			return peer;
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
    return std::shared_ptr<Peer>();
}

std::shared_ptr<Peer> ICentral::getPeer(uint64_t id)
{
	try
	{
		_peersMutex.lock();
		if(_peersById.find(id) != _peersById.end())
		{
			std::shared_ptr<Peer> peer(_peersById.at(id));
			_peersMutex.unlock();
			return peer;
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
    return std::shared_ptr<Peer>();
}

std::shared_ptr<Peer> ICentral::getPeer(std::string serialNumber)
{
	try
	{
		_peersMutex.lock();
		if(_peersBySerial.find(serialNumber) != _peersBySerial.end())
		{
			std::shared_ptr<Peer> peer(_peersBySerial.at(serialNumber));
			_peersMutex.unlock();
			return peer;
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
    return std::shared_ptr<Peer>();
}

bool ICentral::peerSelected()
{
	 return (bool)_currentPeer;
}

bool ICentral::peerExists(int32_t address)
{
	try
	{
		_peersMutex.lock();
		if(_peers.find(address) != _peers.end())
		{
			_peersMutex.unlock();
			return true;
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
    return false;
}

bool ICentral::peerExists(uint64_t id)
{
	try
	{
		_peersMutex.lock();
		if(_peersById.find(id) != _peersById.end())
		{
			_peersMutex.unlock();
			return true;
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
    return false;
}

bool ICentral::peerExists(std::string serialNumber)
{
	try
	{
		_peersMutex.lock();
		if(_peersBySerial.find(serialNumber) != _peersBySerial.end())
		{
			_peersMutex.unlock();
			return true;
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
    return false;
}

void ICentral::setPeerId(uint64_t oldPeerId, uint64_t newPeerId)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(oldPeerId);
		if(!peer) return;
		std::lock_guard<std::mutex> peersGuard(_peersMutex);
		if(_peersById.find(oldPeerId) != _peersById.end()) _peersById.erase(oldPeerId);
		_peersById[newPeerId] = peer;
		for(std::map<uint64_t, std::shared_ptr<Peer>>::iterator i = _peersById.begin(); i != _peersById.end(); ++i)
		{
			i->second->updatePeer(oldPeerId, newPeerId);
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

int32_t ICentral::deviceFamily()
{
	return _deviceFamily;
}

void ICentral::homegearStarted()
{
	try
	{
		std::vector<std::shared_ptr<Peer>> peers;
		getPeers(peers);
		for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
		{
			(*i)->homegearStarted();
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

void ICentral::homegearShuttingDown()
{
	try
	{
		std::vector<std::shared_ptr<Peer>> peers;
		getPeers(peers);
		for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
		{
			(*i)->homegearShuttingDown();
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
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
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
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
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
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
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
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
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
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
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
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

//RPC methods
PVariable ICentral::getAllConfig(PRpcClientInfo clientInfo, uint64_t peerId)
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
			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because listDevices takes very long and we don't want to lock _peersMutex too long
			getPeers(peers);

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				//getAllValues really needs a lot of resources, so wait a little bit after each device
				std::this_thread::sleep_for(std::chrono::milliseconds(3));
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

PVariable ICentral::getAllValues(PRpcClientInfo clientInfo, uint64_t peerId, bool returnWriteOnly)
{
	try
	{
		PVariable array(new Variable(VariableType::tArray));

		if(peerId > 0)
		{
			std::shared_ptr<Peer> peer = getPeer(peerId);
			if(!peer) return Variable::createError(-2, "Unknown device.");
			PVariable values = peer->getAllValues(clientInfo, returnWriteOnly);
			if(!values) return Variable::createError(-32500, "Unknown application error. Values is nullptr.");
			if(values->errorStruct) return values;
			array->arrayValue->push_back(values);
		}
		else
		{
			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because listDevices takes very long and we don't want to lock _peersMutex too long
			getPeers(peers);

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				//getAllValues really needs a lot of resources, so wait a little bit after each device
				std::this_thread::sleep_for(std::chrono::milliseconds(3));
				PVariable values = (*i)->getAllValues(clientInfo, returnWriteOnly);
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

PVariable ICentral::getDeviceDescription(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(serialNumber));
		if(!peer) return Variable::createError(-2, "Unknown device.");

		return peer->getDeviceDescription(clientInfo, channel, std::map<std::string, bool>());
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

PVariable ICentral::getDeviceDescription(PRpcClientInfo clientInfo, uint64_t id, int32_t channel)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(!peer) return Variable::createError(-2, "Unknown device.");

		return peer->getDeviceDescription(clientInfo, channel, std::map<std::string, bool>());
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

PVariable ICentral::getLinks(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, int32_t flags)
{
	try
	{
		if(serialNumber.empty()) return getLinks(clientInfo, 0, -1, flags);
		std::shared_ptr<Peer> peer(getPeer(serialNumber));
		if(!peer) return Variable::createError(-2, "Unknown device.");
		return getLinks(clientInfo, peer->getID(), channel, flags);
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

PVariable ICentral::getLinks(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, int32_t flags)
{
	try
	{
		PVariable array(new Variable(VariableType::tArray));
		PVariable element(new Variable(VariableType::tArray));
		if(peerId == 0)
		{
			try
			{
				std::vector<std::shared_ptr<Peer>> peers;
				//Copy all peers first, because getLinks takes very long and we don't want to lock _peersMutex too long
				getPeers(peers);

				for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
				{
					//listDevices really needs a lot of ressources, so wait a little bit after each device
					std::this_thread::sleep_for(std::chrono::milliseconds(3));
					element = (*i)->getLink(clientInfo, channel, flags, true);
					array->arrayValue->insert(array->arrayValue->begin(), element->arrayValue->begin(), element->arrayValue->end());
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

PVariable ICentral::getName(PRpcClientInfo clientInfo, uint64_t id)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer) return PVariable(new Variable(peer->getName()));
		return Variable::createError(-2, "Unknown device.");
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

PVariable ICentral::getParamset(PRpcClientInfo clientInfo, std::string serialNumber, int32_t channel, ParameterGroup::Type::Enum type, std::string remoteSerialNumber, int32_t remoteChannel)
{
	try
	{
		/*if(serialNumber == "BidCoS-RF" && (channel == 0 || channel == -1) && type == RPC::ParameterSet::Type::Enum::master)
		{
			PVariable paramset(new Variable(VariableType::rpcStruct));
			paramset->structValue->insert(RPC::RPCStructElement("AES_KEY", PVariable(new Variable(1))));
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
			return peer->getParamset(clientInfo, channel, type, remoteId, remoteChannel);
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getParamset(PRpcClientInfo clientInfo, uint64_t peerId, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(peerId));
		if(!peer) return Variable::createError(-2, "Unknown device.");
		return peer->getParamset(clientInfo, channel, type, remoteId, remoteChannel);
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
			if(peer) return peer->getParamsetDescription(clientInfo, channel, type, remoteId, remoteChannel);
			return Variable::createError(-2, "Unknown device.");
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable ICentral::getParamsetDescription(PRpcClientInfo clientInfo, uint64_t id, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteId, int32_t remoteChannel)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer) return peer->getParamsetDescription(clientInfo, channel, type, remoteId, remoteChannel);
		return Variable::createError(-2, "Unknown device.");
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

PVariable ICentral::getPeerId(PRpcClientInfo clientInfo, int32_t filterType, std::string filterValue)
{
	try
	{
		PVariable ids(new BaseLib::Variable(BaseLib::VariableType::tArray));
		if(filterType == 1) //Serial number
		{
			std::shared_ptr<Peer> peer = getPeer(filterValue);
			if(peer) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
		}
		else if(filterType == 2) //Physical address
		{
			int32_t address = Math::getNumber(filterValue);
			if(address != 0)
			{
				std::shared_ptr<Peer> peer = getPeer(address);
				if(peer) ids->arrayValue->push_back(PVariable(new Variable((int32_t)peer->getID())));
			}
		}
		else if(filterType == 3) //Type id
		{
			uint32_t type = (uint32_t)Math::getNumber(filterValue);
			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because getServiceMessages takes very long and we don't want to lock _peersMutex too long
			getPeers(peers);

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if(!*i) continue;
				if((*i)->getDeviceType().type() == type) ids->arrayValue->push_back(PVariable(new Variable((int32_t)(*i)->getID())));
			}
		}
		else if(filterType == 4) //Type string
		{
			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because getServiceMessages takes very long and we don't want to lock _peersMutex too long
			getPeers(peers);

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if(!*i) continue;
				if((*i)->getTypeString() == filterValue) ids->arrayValue->push_back(PVariable(new Variable((int32_t)(*i)->getID())));
			}
		}
		else if(filterType == 5) //Name
		{
			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because getServiceMessages takes very long and we don't want to lock _peersMutex too long
			getPeers(peers);

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if(!*i) continue;
				if((*i)->getName().find(filterValue) != std::string::npos) ids->arrayValue->push_back(PVariable(new Variable((int32_t)(*i)->getID())));
			}
		}
		else if(filterType == 6) //Pending config
		{
			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because getServiceMessages takes very long and we don't want to lock _peersMutex too long
			getPeers(peers);

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if(!*i) continue;
				if((*i)->serviceMessages->getConfigPending()) ids->arrayValue->push_back(PVariable(new Variable((int32_t)(*i)->getID())));
			}
		}
		else if(filterType == 7) //Unreachable
		{
			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because getServiceMessages takes very long and we don't want to lock _peersMutex too long
			getPeers(peers);

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if(!*i) continue;
				if((*i)->serviceMessages->getUnreach()) ids->arrayValue->push_back(PVariable(new Variable((int32_t)(*i)->getID())));
			}
		}
		else if(filterType == 8) //Reachable
		{
			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because getServiceMessages takes very long and we don't want to lock _peersMutex too long
			getPeers(peers);

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if(!*i) continue;
				if(!(*i)->serviceMessages->getUnreach()) ids->arrayValue->push_back(PVariable(new Variable((int32_t)(*i)->getID())));
			}
		}
		else if(filterType == 9) //Low battery
		{
			std::vector<std::shared_ptr<Peer>> peers;
			//Copy all peers first, because getServiceMessages takes very long and we don't want to lock _peersMutex too long
			getPeers(peers);

			for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
			{
				if(!*i) continue;
				if((*i)->serviceMessages->getLowbat()) ids->arrayValue->push_back(PVariable(new Variable((int32_t)(*i)->getID())));
			}
		}
		return ids;
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

PVariable ICentral::getServiceMessages(PRpcClientInfo clientInfo, bool returnId)
{
	try
	{
		std::vector<std::shared_ptr<Peer>> peers;
		//Copy all peers first, because getServiceMessages takes very long and we don't want to lock _peersMutex too long
		getPeers(peers);

		PVariable serviceMessages(new Variable(VariableType::tArray));
		for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
		{
			if(!*i) continue;
			//getServiceMessages really needs a lot of ressources, so wait a little bit after each device
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
			PVariable messages = (*i)->getServiceMessages(clientInfo, returnId);
			if(!messages->arrayValue->empty()) serviceMessages->arrayValue->insert(serviceMessages->arrayValue->end(), messages->arrayValue->begin(), messages->arrayValue->end());
		}
		return serviceMessages;
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

PVariable ICentral::listDevices(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields)
{
	return listDevices(clientInfo, channels, fields, std::shared_ptr<std::set<std::uint64_t>>());
}

PVariable ICentral::listDevices(PRpcClientInfo clientInfo, bool channels, std::map<std::string, bool> fields, std::shared_ptr<std::set<uint64_t>> knownDevices)
{
	try
	{
		PVariable array(new Variable(VariableType::tArray));

		std::vector<std::shared_ptr<Peer>> peers;
		//Copy all peers first, because listDevices takes very long and we don't want to lock _peersMutex too long
		getPeers(peers, knownDevices);

		for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
		{
			//listDevices really needs a lot of resources, so wait a little bit after each device
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
			std::shared_ptr<std::vector<PVariable>> descriptions = (*i)->getDeviceDescriptions(clientInfo, channels, fields);
			if(!descriptions) continue;
			for(std::vector<PVariable>::iterator j = descriptions->begin(); j != descriptions->end(); ++j)
			{
				array->arrayValue->push_back(*j);
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

PVariable ICentral::rssiInfo(PRpcClientInfo clientInfo)
{
	try
	{
		PVariable response(new Variable(VariableType::tStruct));

		std::vector<std::shared_ptr<Peer>> peers;
		//Copy all peers first, because rssiInfo takes very long and we don't want to lock _peersMutex too long
		getPeers(peers);

		for(std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin(); i != peers.end(); ++i)
		{
			//rssiInfo really needs a lot of resources, so wait a little bit after each device
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
			PVariable element = (*i)->rssiInfo(clientInfo);
			if(!element || element->errorStruct) continue;
			response->structValue->insert(StructElement((*i)->getSerialNumber(), element));
		}

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

PVariable ICentral::setName(PRpcClientInfo clientInfo, uint64_t id, std::string name)
{
	try
	{
		std::shared_ptr<Peer> peer(getPeer(id));
		if(peer)
		{
			peer->setName(name);
			return PVariable(new Variable(VariableType::tVoid));
		}
		return Variable::createError(-2, "Unknown device.");
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
//End RPC methods

}
}
