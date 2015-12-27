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

#include "Licensing.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Licensing
{

Licensing::Licensing(BaseLib::Obj* bl)
{
	_bl = bl;
}

Licensing::~Licensing()
{
}

void Licensing::dispose()
{
	try
	{
		if(_disposed) return;
		_disposed = true;
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

void Licensing::load()
{
	loadVariables();
}

void Licensing::loadVariables()
{
	try
	{
		if(!_bl->db) return;
		std::shared_ptr<BaseLib::Database::DataTable> rows = _bl->db->getLicenseVariables(_moduleId);
		for(BaseLib::Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row)
		{
			_variableDatabaseIds[row->second.at(2)->intValue] = row->second.at(0)->intValue;
			uint64_t index = row->second.at(2)->intValue;
			std::shared_ptr<std::vector<char>> data = row->second.at(5)->binaryValue;
			if(!data || data->empty()) continue;
			std::string stringData(&data->at(0), data->size());
			std::pair<std::string, std::string> parsedData = BaseLib::HelperFunctions::splitLast(stringData, ',');
			_licenseData[index].licenseKey = parsedData.first;
			_licenseData[index].activationKey = parsedData.second;
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

std::string Licensing::getLicenseKey(int32_t familyId, int32_t deviceId)
{
	try
	{
		std::lock_guard<std::mutex> devicesGuard(_devicesMutex);
		DeviceStates::iterator familyIterator = _devices.find(familyId);
		if(familyIterator == _devices.end()) return "";
		std::map<int32_t, PDeviceInfo>::iterator deviceIterator = familyIterator->second.find(deviceId);
		if(deviceIterator == familyIterator->second.end() || !deviceIterator->second) return "";
		return deviceIterator->second->licenseKey;
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
    return "";
}

void Licensing::removeLicense(int32_t familyId, int32_t deviceId)
{
	try
	{
		removeDevice(familyId, deviceId);
		uint64_t mapKey = getMapKey(familyId, deviceId);
		_licenseData[mapKey].licenseKey.clear();
		_licenseData[mapKey].activationKey.clear();
		std::map<uint32_t, uint32_t>::iterator databaseIdIterator = _variableDatabaseIds.find(mapKey);
		if(databaseIdIterator != _variableDatabaseIds.end()) databaseIdIterator->second = 0;
		_bl->db->deleteLicenseVariable(_moduleId, mapKey);
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

void Licensing::saveVariable(uint64_t index, int32_t intValue)
{
	try
	{
		if(!_bl->db) return;
		std::map<uint32_t, uint32_t>::iterator databaseIdIterator = _variableDatabaseIds.find(index);
		Database::DataRow data;
		if(databaseIdIterator != _variableDatabaseIds.end())
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(databaseIdIterator->second)));
			_bl->db->saveLicenseVariable(_moduleId, data);
		}
		else
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_moduleId)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			_bl->db->saveLicenseVariable(_moduleId, data);
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

void Licensing::saveVariable(uint64_t index, int64_t intValue)
{
	try
	{
		if(!_bl->db) return;
		std::map<uint32_t, uint32_t>::iterator databaseIdIterator = _variableDatabaseIds.find(index);
		Database::DataRow data;
		if(databaseIdIterator != _variableDatabaseIds.end())
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(databaseIdIterator->second)));
			_bl->db->saveLicenseVariable(_moduleId, data);
		}
		else
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_moduleId)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(intValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			_bl->db->saveLicenseVariable(_moduleId, data);
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

void Licensing::saveVariable(uint64_t index, std::string& stringValue)
{
	try
	{
		if(!_bl->db) return;
		std::map<uint32_t, uint32_t>::iterator databaseIdIterator = _variableDatabaseIds.find(index);
		Database::DataRow data;
		if(databaseIdIterator != _variableDatabaseIds.end())
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(stringValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(databaseIdIterator->second)));
			_bl->db->saveLicenseVariable(_moduleId, data);
		}
		else
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_moduleId)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(stringValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			_bl->db->saveLicenseVariable(_moduleId, data);
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

void Licensing::saveVariable(uint64_t index, std::vector<uint8_t>& binaryValue)
{
	try
	{
		if(!_bl->db) return;
		std::map<uint32_t, uint32_t>::iterator databaseIdIterator = _variableDatabaseIds.find(index);
		Database::DataRow data;
		if(databaseIdIterator != _variableDatabaseIds.end())
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(databaseIdIterator->second)));
			_bl->db->saveLicenseVariable(_moduleId, data);
		}
		else
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_moduleId)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(binaryValue)));
			_bl->db->saveLicenseVariable(_moduleId, data);
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

void Licensing::saveVariable(uint64_t index, LicenseData& licenseData)
{
	try
	{
		if(!_bl->db) return;
		if(licenseData.licenseKey.empty() || licenseData.activationKey.empty()) return;
		std::vector<uint8_t> blob;
		blob.reserve(licenseData.licenseKey.size() + 1 + licenseData.activationKey.size());
		blob.insert(blob.end(), licenseData.licenseKey.begin(), licenseData.licenseKey.end());
		blob.push_back(',');
		blob.insert(blob.end(), licenseData.activationKey.begin(), licenseData.activationKey.end());

		std::map<uint32_t, uint32_t>::iterator databaseIdIterator = _variableDatabaseIds.find(index);
		Database::DataRow data;
		_licenseData[index].licenseKey = licenseData.licenseKey;
		_licenseData[index].activationKey = licenseData.activationKey;
		if(databaseIdIterator != _variableDatabaseIds.end() && databaseIdIterator->second > 0)
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(blob)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(databaseIdIterator->second)));
			_bl->db->saveLicenseVariable(_moduleId, data);
		}
		else
		{
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_moduleId)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(index)));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
			data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(blob)));
			_bl->db->saveLicenseVariable(_moduleId, data);
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

Licensing::DeviceStates Licensing::getDeviceStates()
{
	DeviceStates devices;
	_devicesMutex.lock();
	try
	{
		devices = _devices;
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
    _devicesMutex.unlock();
    return devices;
}

bool Licensing::getDeviceState(int32_t familyId, int32_t deviceId)
{
	_devicesMutex.lock();
	try
	{
		DeviceStates::iterator stateIterator1 = _devices.find(familyId);
		if(stateIterator1 == _devices.end())
		{
			_devicesMutex.unlock();
			return false;
		}
		std::map<int32_t, std::shared_ptr<DeviceInfo>>::iterator stateIterator2 = stateIterator1->second.find(deviceId);
		if(stateIterator2 == stateIterator1->second.end())
		{
			_devicesMutex.unlock();
			return false;
		}
		bool state = stateIterator2->second->state;
		_devicesMutex.unlock();
		return state;
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
    _devicesMutex.unlock();
    return false;
}

uint64_t Licensing::getMapKey(int32_t familyId, int32_t deviceId)
{
	return (((uint64_t)familyId) << 32) + (uint32_t)deviceId;
}

void Licensing::addDevice(int32_t familyId, int32_t deviceId, bool state, std::string licenseKey)
{
	_devicesMutex.lock();
	try
	{
		PDeviceInfo info(new DeviceInfo());
		info->moduleId = _moduleId;
		info->familyId = familyId;
		info->deviceId = deviceId;
		info->state = state;
		info->licenseKey = licenseKey;
		_devices[familyId][deviceId] = info;
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
    _devicesMutex.unlock();
}

void Licensing::updateDevice(int32_t familyId, int32_t deviceId, bool state, std::string licenseKey)
{
	addDevice(familyId, deviceId, state, licenseKey);
}

void Licensing::removeDevice(int32_t familyId, int32_t deviceId)
{
	_devicesMutex.lock();
	try
	{
		_devices[familyId].erase(deviceId);
		_devices.erase(familyId);
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
    _devicesMutex.unlock();
}

}
}
