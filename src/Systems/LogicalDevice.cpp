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

#include "LogicalDevice.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Systems
{

LogicalDevice::LogicalDevice(int32_t deviceFamily, BaseLib::Obj* baseLib, IDeviceEventSink* eventHandler)
{
	_bl = baseLib;
	_deviceFamily = deviceFamily;
	setEventHandler(eventHandler);
}

LogicalDevice::LogicalDevice(int32_t deviceFamily, BaseLib::Obj* baseLib, uint32_t deviceID, std::string serialNumber, int32_t address, IDeviceEventSink* eventHandler) : LogicalDevice(deviceFamily, baseLib, eventHandler)
{
	_deviceID = deviceID;
	_serialNumber = serialNumber;
	_address = address;
}

LogicalDevice::~LogicalDevice()
{
}

void LogicalDevice::dispose(bool wait)
{
	_disposing = true;
	_central.reset();
	_currentPeer.reset();
	_peers.clear();
	_peersBySerial.clear();
	_peersByID.clear();
}

//Event handling
void LogicalDevice::raiseAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers)
{
	if(_eventHandler) ((IDeviceEventSink*)_eventHandler)->onAddWebserverEventHandler(eventHandler, eventHandlers);
}

void LogicalDevice::raiseRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers)
{
	if(_eventHandler) ((IDeviceEventSink*)_eventHandler)->onRemoveWebserverEventHandler(eventHandlers);
}

void LogicalDevice::raiseRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values)
{
	if(_eventHandler) ((IDeviceEventSink*)_eventHandler)->onRPCEvent(id, channel, deviceAddress, valueKeys, values);
}

void LogicalDevice::raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
{
	if(_eventHandler) ((IDeviceEventSink*)_eventHandler)->onRPCUpdateDevice(id, channel, address, hint);
}

void LogicalDevice::raiseRPCNewDevices(PVariable deviceDescriptions)
{
	if(_eventHandler) ((IDeviceEventSink*)_eventHandler)->onRPCNewDevices(deviceDescriptions);
}

void LogicalDevice::raiseRPCDeleteDevices(PVariable deviceAddresses, PVariable deviceInfo)
{
	if(_eventHandler) ((IDeviceEventSink*)_eventHandler)->onRPCDeleteDevices(deviceAddresses, deviceInfo);
}

void LogicalDevice::raiseEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
{
	if(_eventHandler) ((IDeviceEventSink*)_eventHandler)->onEvent(peerID, channel, variables, values);
}

void LogicalDevice::raiseRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval)
{
	if(_eventHandler) ((IDeviceEventSink*)_eventHandler)->onRunScript(script, peerId, args, keepAlive, interval);
}

int32_t LogicalDevice::raiseIsAddonClient(int32_t clientID)
{
	if(_eventHandler) return ((IDeviceEventSink*)_eventHandler)->onIsAddonClient(clientID);
	return -1;
}
//End event handling

//Peer event handling
void LogicalDevice::onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers)
{
	raiseAddWebserverEventHandler(eventHandler, eventHandlers);
}

void LogicalDevice::onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers)
{
	raiseRemoveWebserverEventHandler(eventHandlers);
}

void LogicalDevice::onRPCEvent(uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<PVariable>> values)
{
	raiseRPCEvent(id, channel, deviceAddress, valueKeys, values);
}

void LogicalDevice::onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint)
{
	raiseRPCUpdateDevice(id, channel, address, hint);
}

void LogicalDevice::onEvent(uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<PVariable>> values)
{
	raiseEvent(peerID, channel, variables, values);
}

void LogicalDevice::onRunScript(std::string& script, uint64_t peerId, const std::string& args, bool keepAlive, int32_t interval)
{
	raiseRunScript(script, peerId, args, keepAlive, interval);
}

int32_t LogicalDevice::onIsAddonClient(int32_t clientID)
{
	return raiseIsAddonClient(clientID);
}
//End Peer event handling

void LogicalDevice::getPeers(std::vector<std::shared_ptr<Peer>>& peers, std::shared_ptr<std::set<uint64_t>> knownDevices)
{
	try
	{
		_peersMutex.lock();
		for(std::map<uint64_t, std::shared_ptr<Peer>>::iterator i = _peersByID.begin(); i != _peersByID.end(); ++i)
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

std::shared_ptr<Peer> LogicalDevice::getPeer(int32_t address)
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

std::shared_ptr<Peer> LogicalDevice::getPeer(uint64_t id)
{
	try
	{
		_peersMutex.lock();
		if(_peersByID.find(id) != _peersByID.end())
		{
			std::shared_ptr<Peer> peer(_peersByID.at(id));
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

std::shared_ptr<Peer> LogicalDevice::getPeer(std::string serialNumber)
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

bool LogicalDevice::peerExists(int32_t address)
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

bool LogicalDevice::peerExists(uint64_t id)
{
	try
	{
		_peersMutex.lock();
		if(_peersByID.find(id) != _peersByID.end())
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

bool LogicalDevice::peerExists(std::string serialNumber)
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

void LogicalDevice::setPeerID(uint64_t oldPeerID, uint64_t newPeerID)
{
	try
	{
		std::shared_ptr<Peer> peer = getPeer(oldPeerID);
		if(!peer) return;
		_peersMutex.lock();
		if(_peersByID.find(oldPeerID) != _peersByID.end()) _peersByID.erase(oldPeerID);
		_peersByID[newPeerID] = peer;
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

int32_t LogicalDevice::deviceFamily()
{
	return _deviceFamily;
}

void LogicalDevice::homegearStarted()
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

void LogicalDevice::homegearShuttingDown()
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

void LogicalDevice::load()
{
	try
	{
		loadVariables();
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

void LogicalDevice::save(bool saveDevice)
{
	try
	{
		if(saveDevice)
		{
			uint64_t result = _bl->db->saveDevice(_deviceID, _address, _serialNumber, _deviceType, (uint32_t)_deviceFamily);
			if(_deviceID == 0) _deviceID = result;
		}
		saveVariables();
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

void LogicalDevice::saveVariable(uint32_t index, int64_t intValue)
{
	try
	{
		bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
			_bl->db->saveDeviceVariableAsynchronous(data);
		}
		else
		{
			if(_deviceID == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_deviceID)));
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

void LogicalDevice::saveVariable(uint32_t index, std::string& stringValue)
{
	try
	{
		bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(stringValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
			_bl->db->saveDeviceVariableAsynchronous(data);
		}
		else
		{
			if(_deviceID == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_deviceID)));
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

void LogicalDevice::saveVariable(uint32_t index, std::vector<uint8_t>& binaryValue)
{
	try
	{
		bool idIsKnown = _variableDatabaseIDs.find(index) != _variableDatabaseIDs.end();
		Database::DataRow data;
		if(idIsKnown)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_variableDatabaseIDs[index])));
			_bl->db->saveDeviceVariableAsynchronous(data);
		}
		else
		{
			if(_deviceID == 0) return;
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_deviceID)));
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

void LogicalDevice::deletePeersFromDatabase()
{
	try
	{
		_bl->db->deletePeers(_deviceID);
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

}
}
