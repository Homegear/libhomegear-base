/* Copyright 2013-2016 Sathya Laufer
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Homegear.  If not, see
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

#include "FamilySettings.h"
#include "../BaseLib.h"

namespace BaseLib
{

namespace Systems
{

FamilySettings::FamilySettings(BaseLib::Obj* bl, int32_t familyId)
{
	_bl = bl;
	_familyId = familyId;
}

FamilySettings::~FamilySettings()
{
	dispose();
}

void FamilySettings::dispose()
{
	_settings.clear();
	_physicalInterfaceSettings.clear();
}

FamilySettings::PFamilySetting FamilySettings::get(std::string& name)
{
	_settingsMutex.lock();
	try
	{
		std::map<std::string, PFamilySetting>::iterator settingIterator = _settings.find(name);
		if(settingIterator != _settings.end())
		{
			PFamilySetting setting = settingIterator->second;
			_settingsMutex.unlock();
			return setting;
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
    _settingsMutex.unlock();
    return PFamilySetting();
}

std::string FamilySettings::getString(std::string name)
{
	_settingsMutex.lock();
	try
	{
		std::map<std::string, PFamilySetting>::iterator settingIterator = _settings.find(name);
		if(settingIterator != _settings.end())
		{
			std::string setting = settingIterator->second->stringValue;
			_settingsMutex.unlock();
			return setting;
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
    _settingsMutex.unlock();
    return "";
}

int32_t FamilySettings::getNumber(std::string name)
{
	_settingsMutex.lock();
	try
	{
		std::map<std::string, PFamilySetting>::iterator settingIterator = _settings.find(name);
		if(settingIterator != _settings.end())
		{
			int32_t setting = settingIterator->second->integerValue;
			_settingsMutex.unlock();
			return setting;
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
    _settingsMutex.unlock();
    return 0;
}

std::vector<char> FamilySettings::getBinary(std::string name)
{
	_settingsMutex.lock();
	try
	{
		std::map<std::string, PFamilySetting>::iterator settingIterator = _settings.find(name);
		if(settingIterator != _settings.end())
		{
			std::vector<char> setting = settingIterator->second->binaryValue;
			_settingsMutex.unlock();
			return setting;
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
    _settingsMutex.unlock();
    return std::vector<char>();
}

void FamilySettings::set(std::string& setting, std::string& value)
{
	try
	{
		if(setting.empty()) return;
		{
			std::lock_guard<std::mutex> settingGuard(_settingsMutex);
			std::map<std::string, PFamilySetting>::iterator settingIterator = _settings.find(setting);
			if(settingIterator != _settings.end())
			{
				settingIterator->second->stringValue = value;
				settingIterator->second->integerValue = 0;
				settingIterator->second->binaryValue.clear();
			}
		}
		Database::DataRow data;
		std::string name = setting;
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_familyId)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(0)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(name)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_familyId)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(0)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(name)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(value)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
		_bl->db->saveFamilyVariableAsynchronous(_familyId, data);
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

void FamilySettings::set(std::string& setting, int32_t value)
{
	try
	{
		if(setting.empty()) return;
		{
			std::lock_guard<std::mutex> settingGuard(_settingsMutex);
			std::map<std::string, PFamilySetting>::iterator settingIterator = _settings.find(setting);
			if(settingIterator != _settings.end())
			{
				settingIterator->second->stringValue.clear();
				settingIterator->second->integerValue = value;
				settingIterator->second->binaryValue.clear();
			}
		}
		if(setting.empty()) return;
		Database::DataRow data;
		std::string name = setting;
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_familyId)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(1)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(name)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_familyId)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(1)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(name)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(value)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
		_bl->db->saveFamilyVariableAsynchronous(_familyId, data);
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

void FamilySettings::set(std::string& setting, std::vector<char>& value)
{
	try
	{
		if(setting.empty()) return;
		{
			std::lock_guard<std::mutex> settingGuard(_settingsMutex);
			std::map<std::string, PFamilySetting>::iterator settingIterator = _settings.find(setting);
			if(settingIterator != _settings.end())
			{
				settingIterator->second->stringValue.clear();
				settingIterator->second->integerValue = 0;
				settingIterator->second->binaryValue = value;
			}
		}
		if(setting.empty()) return;
		Database::DataRow data;
		std::string name = setting;
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_familyId)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(2)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(name)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(_familyId)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(2)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(name)));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn()));
		data.push_back(std::shared_ptr<Database::DataColumn>(new Database::DataColumn(value)));
		_bl->db->saveFamilyVariableAsynchronous(_familyId, data);
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

std::map<std::string, PPhysicalInterfaceSettings> FamilySettings::getPhysicalInterfaceSettings()
{
	return _physicalInterfaceSettings;
}

void FamilySettings::load(std::string filename)
{
	try
	{
		DisposableLockGuard settingsGuard(_settingsMutex);
		_settings.clear();
		char input[1024];
		FILE *fin;
		int32_t len, ptr;
		bool found = false;
		bool generalSettings = false;

		if (!(fin = fopen(filename.c_str(), "r")))
		{
			_bl->out.printError("Unable to open family setting file: " + filename + ". " + strerror(errno));
			return;
		}

		std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings(new BaseLib::Systems::PhysicalInterfaceSettings());
		while (fgets(input, 1024, fin))
		{
			if(input[0] == '#' || input[0] == '-' || input[0] == '_') continue;
			len = strlen(input);
			if (len < 2) continue;
			if (input[len-1] == '\n') input[len-1] = '\0';
			ptr = 0;
			if(input[0] == '[')
			{
				while(ptr < len)
				{
					if (input[ptr] == ']')
					{
						input[ptr] = '\0';
						if(!settings->type.empty())
						{
							if(settings->id.empty()) settings->id = settings->type;
							_physicalInterfaceSettings[settings->id] = settings;
						}
						settings.reset(new BaseLib::Systems::PhysicalInterfaceSettings());
						std::string title(&input[1]);
						_bl->out.printDebug("Debug: Loading section \"" + title + "\"");
						BaseLib::HelperFunctions::toLower(title);
						if(title == "general") generalSettings = true;
						else generalSettings = false;
						break;
					}
					ptr++;
				}
				continue;
			}
			found = false;
			while(ptr < len)
			{
				if (input[ptr] == '=')
				{
					found = true;
					input[ptr++] = '\0';
					break;
				}
				ptr++;
			}
			if(found)
			{
				std::string name(input);
				BaseLib::HelperFunctions::toLower(name);
				BaseLib::HelperFunctions::trim(name);
				std::string value(&input[ptr]);
				BaseLib::HelperFunctions::trim(value);

				if(generalSettings)
				{
					if(_settings.find(name) != _settings.end()) _bl->out.printWarning("Warning: Setting defined twice: " + name);
					PFamilySetting setting(new FamilySetting());
					setting->stringValue = value;
					setting->integerValue = Math::getNumber(value);
					setting->binaryValue = _bl->hf.getBinary(value);
					_settings[name] = std::move(setting);
					_bl->out.printDebug("Debug: Family setting " + name + " set to " + value);
				}
				else processStringSetting(name, value, settings);
			}
		}
		if(!settings->type.empty())
		{
			if(settings->id.empty()) settings->id = settings->type;
			_physicalInterfaceSettings[settings->id] = settings;
		}

		fclose(fin);

		settingsGuard.dispose();
		loadFromDatabase();
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

void FamilySettings::loadFromDatabase()
{
	try
	{
		DisposableLockGuard settingsGuard(_settingsMutex);
		std::shared_ptr<BaseLib::Database::DataTable> rows = _bl->db->getFamilyVariables(_familyId);
		if(!rows) return;
		for(Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row)
		{
			PFamilySetting setting(new FamilySetting());
			setting->integerValue = row->second.at(4)->intValue;
			setting->stringValue = std::move(row->second.at(5)->textValue);
			setting->binaryValue = std::move(*(row->second.at(6)->binaryValue));

			std::pair<std::string, std::string> interfacePair = HelperFunctions::splitFirst(row->second.at(3)->textValue, '.');
			if(interfacePair.second.empty())
			{
				std::string& name = BaseLib::HelperFunctions::toLower(interfacePair.first);
				_settings[name] = std::move(setting);
			}
			else
			{
				std::map<std::string, PPhysicalInterfaceSettings>::iterator settingsIterator = _physicalInterfaceSettings.find(interfacePair.first);
				PPhysicalInterfaceSettings settings;
				if(settingsIterator != _physicalInterfaceSettings.end()) settings = settingsIterator->second;
				else
				{
					settings.reset(new PhysicalInterfaceSettings());
					settings->id = interfacePair.first;
					_physicalInterfaceSettings[settings->id] = settings;
				}
				HelperFunctions::toLower(interfacePair.second);
				processDatabaseSetting(interfacePair.second, setting, settings);
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

void FamilySettings::processStringSetting(std::string& name, std::string& value, PPhysicalInterfaceSettings& settings)
{
	try
	{
		if(name == "id")
		{
			if(!value.empty())
			{
				settings->id = value;
				_bl->out.printDebug("Debug: id set to " + settings->id);
			}
		}
		else if(name == "default")
		{
			if(value == "true") settings->isDefault = true;
			_bl->out.printDebug("Debug: default set to " + std::to_string(settings->isDefault));
		}
		else if(name == "devicetype")
		{
			if(settings->type.length() > 0) _bl->out.printWarning("Warning: deviceType for \"" + settings->id + "\" in family with ID \"" + std::to_string(_familyId) + "\" is set multiple times within one device block. Is the interface header missing (e. g. \"[My Interface]\")?");
			BaseLib::HelperFunctions::toLower(value);
			settings->type = value;
			_bl->out.printDebug("Debug: deviceType set to " + settings->type);
		}
		else if(name == "device")
		{
			settings->device = value;
			_bl->out.printDebug("Debug: device set to " + settings->device);
		}
		else if(name == "responsedelay")
		{
			settings->responseDelay = BaseLib::Math::getNumber(value);
			if(settings->responseDelay > 10000) settings->responseDelay = 10000;
			_bl->out.printDebug("Debug: responseDelay set to " + std::to_string(settings->responseDelay));
		}
		else if(name == "txpowersetting")
		{
			settings->txPowerSetting = BaseLib::Math::getNumber(value);
			_bl->out.printDebug("Debug: txPowerSetting set to " + std::to_string(settings->txPowerSetting));
		}
		else if(name == "oscillatorfrequency")
		{
			settings->oscillatorFrequency = BaseLib::Math::getNumber(value);
			if(settings->oscillatorFrequency < 0) settings->oscillatorFrequency = -1;
			_bl->out.printDebug("Debug: oscillatorFrequency set to " + std::to_string(settings->oscillatorFrequency));
		}
		else if(name == "oneway")
		{
			BaseLib::HelperFunctions::toLower(value);
			if(value == "true") settings->oneWay = true;
			_bl->out.printDebug("Debug: oneWay set to " + std::to_string(settings->oneWay));
		}
		else if(name == "enablerxvalue")
		{
			int32_t number = BaseLib::Math::getNumber(value);
			settings->enableRXValue = number;
			_bl->out.printDebug("Debug: enableRXValue set to " + std::to_string(settings->enableRXValue));
		}
		else if(name == "enabletxvalue")
		{
			int32_t number = BaseLib::Math::getNumber(value);
			settings->enableTXValue = number;
			_bl->out.printDebug("Debug: enableTXValue set to " + std::to_string(settings->enableTXValue));
		}
		else if(name == "fastsending")
		{
			BaseLib::HelperFunctions::toLower(value);
			if(value == "true") settings->fastSending = true;
			_bl->out.printDebug("Debug: fastSending set to " + std::to_string(settings->fastSending));
		}
		else if(name == "waitforbus")
		{
			settings->waitForBus = BaseLib::Math::getNumber(value);
			if(settings->waitForBus < 60) settings->waitForBus = 60;
			else if(settings->waitForBus > 210) settings->waitForBus = 210;
			_bl->out.printDebug("Debug: waitForBus set to " + std::to_string(settings->waitForBus));
		}
		else if(name == "interval")
		{
			settings->interval = BaseLib::Math::getNumber(value);
			if(settings->interval > 100000) settings->interval = 100000;
			_bl->out.printDebug("Debug: interval set to " + std::to_string(settings->interval));
		}
		else if(name == "timeout")
		{
			settings->timeout = BaseLib::Math::getNumber(value);
			if(settings->timeout > 100000) settings->timeout = 100000;
			_bl->out.printDebug("Debug: timeout set to " + std::to_string(settings->timeout));
		}
		else if(name == "watchdogtimeout")
		{
			settings->watchdogTimeout = BaseLib::Math::getNumber(value);
			if(settings->watchdogTimeout > 100000) settings->timeout = 100000;
			_bl->out.printDebug("Debug: watchdogTimeout set to " + std::to_string(settings->watchdogTimeout));
		}
		else if(name == "sendfix")
		{
			BaseLib::HelperFunctions::toLower(value);
			if(value == "true") settings->sendFix = true;
			_bl->out.printDebug("Debug: sendFix set to " + std::to_string(settings->sendFix));
		}
		else if(name == "interruptpin")
		{
			int32_t number = BaseLib::Math::getNumber(value);
			if(number >= 0)
			{
				settings->interruptPin = number;
				_bl->out.printDebug("Debug: interruptPin set to " + std::to_string(settings->interruptPin));
			}
		}
		else if(name == "gpio1")
		{
			int32_t number = BaseLib::Math::getNumber(value);
			if(number > 0)
			{
				settings->gpio[1].number = number;
				_bl->out.printDebug("Debug: GPIO1 set to " + std::to_string(settings->gpio[1].number));
			}
		}
		else if(name == "gpio2")
		{
			int32_t number = BaseLib::Math::getNumber(value);
			if(number > 0)
			{
				settings->gpio[2].number = number;
				_bl->out.printDebug("Debug: GPIO2 set to " + std::to_string(settings->gpio[2].number));
			}
		}
		else if(name == "gpio3")
		{
			int32_t number = BaseLib::Math::getNumber(value);
			if(number > 0)
			{
				settings->gpio[3].number = number;
				_bl->out.printDebug("Debug: GPIO3 set to " + std::to_string(settings->gpio[3].number));
			}
		}
		else if(name == "stackposition")
		{
			int32_t number = BaseLib::Math::getNumber(value);
			if(number > 0)
			{
				settings->stackPosition = number;
				_bl->out.printDebug("Debug: stackPosition set to " + std::to_string(settings->stackPosition));
			}
		}
		else if(name == "host")
		{
			settings->host = value;
			_bl->out.printDebug("Debug: Host set to " + settings->host);
		}
		else if(name == "port")
		{
			settings->port = value;
			_bl->out.printDebug("Debug: Port set to " + settings->port);
		}
		else if(name == "listenport")
		{
			settings->listenPort = value;
			_bl->out.printDebug("Debug: listenPort set to " + settings->listenPort);
		}
		else if(name == "portkeepalive")
		{
			settings->portKeepAlive = value;
			_bl->out.printDebug("Debug: PortKeepAlive set to " + settings->portKeepAlive);
		}
		else if(name == "oldrfkey")
		{
			_bl->out.printError("Error: OldRFKey now needs to be set in section [General] of homematicbidcos.conf.");
			PFamilySetting setting(new FamilySetting);
			setting->stringValue = value;
			if(_settings.find("oldrfkey") == _settings.end()) _settings["oldrfkey"] = setting;
		}
		else if(name == "rfkey")
		{
			_bl->out.printError("Error: RFKey now needs to be set in section [General] of homematicbidcos.conf.");
			PFamilySetting setting(new FamilySetting);
			setting->stringValue = value;
			if(_settings.find("rfkey") == _settings.end()) _settings["rfkey"] = setting;
		}
		else if(name == "currentrfkeyindex")
		{
			_bl->out.printError("Error: CurrentRFKeyIndex now needs to be set in section [General] of homematicbidcos.conf.");
			PFamilySetting setting(new FamilySetting);
			setting->stringValue = value;
			if(_settings.find("currentrfkeyindex") == _settings.end()) _settings["currentrfkeyindex"] = setting;
		}
		else if(name == "lankey")
		{
			settings->lanKey = value;
			_bl->out.printDebug("Debug: LANKey set to " + settings->lanKey);
		}
		else if(name == "ssl")
		{
			BaseLib::HelperFunctions::toLower(value);
			if(value == "true") settings->ssl = true;
			_bl->out.printDebug("Debug: SSL set to " + std::to_string(settings->ssl));
		}
		else if(name == "cafile")
		{
			settings->caFile = value;
			_bl->out.printDebug("Debug: CAFile set to " + settings->caFile);
		}
		else if(name == "verifycertificate")
		{
			BaseLib::HelperFunctions::toLower(value);
			if(value == "false") settings->verifyCertificate = false;
			_bl->out.printDebug("Debug: VerifyCertificate set to " + std::to_string(settings->verifyCertificate));
		}
		else if(name == "listenthreadpriority")
		{
			settings->listenThreadPriority = BaseLib::Math::getNumber(value);
			if(settings->listenThreadPriority > 99) settings->listenThreadPriority = 99;
			if(settings->listenThreadPriority < 0) settings->listenThreadPriority = 0;
			_bl->out.printDebug("Debug: ListenThreadPriority set to " + std::to_string(settings->listenThreadPriority));
		}
		else if(name == "listenthreadpolicy")
		{
			settings->listenThreadPolicy = ThreadManager::getThreadPolicyFromString(value);
			settings->listenThreadPriority = ThreadManager::parseThreadPriority(settings->listenThreadPriority, settings->listenThreadPolicy);
			_bl->out.printDebug("Debug: ListenThreadPolicy set to " + std::to_string(settings->listenThreadPolicy));
		}
		else if(name == "ttsprogram")
		{
			settings->ttsProgram = value;
			_bl->out.printDebug("Debug: ttsProgram set to " + settings->ttsProgram);
		}
		else if(name == "datapath")
		{
			settings->dataPath = value;
			if(settings->dataPath.back() != '/') settings->dataPath.push_back('/');
			_bl->out.printDebug("Debug: dataPath set to " + settings->dataPath);
		}
		else if(name == "user")
		{
			settings->user = value;
			_bl->out.printDebug("Debug: user set");
		}
		else if(name == "password")
		{
			settings->password = value;
			_bl->out.printDebug("Debug: password set");
		}
		else
		{
			_bl->out.printWarning("Warning: Unknown physical interface setting: " + name);
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

void FamilySettings::processDatabaseSetting(std::string& name, PFamilySetting& value, PPhysicalInterfaceSettings& settings)
{
	try
	{
		if(name == "default")
		{
			if(value->integerValue) settings->isDefault = true;
			_bl->out.printDebug("Debug: default set to " + std::to_string(settings->isDefault));
		}
		else if(name == "devicetype")
		{
			if(settings->type.length() > 0) _bl->out.printWarning("Warning: deviceType for \"" + settings->id + "\" in family with ID \"" + std::to_string(_familyId) + "\" is set multiple times within one device block. Is the interface header missing (e. g. \"[My Interface]\")?");
			BaseLib::HelperFunctions::toLower(value->stringValue);
			settings->type = value->stringValue;
			_bl->out.printDebug("Debug: deviceType set to " + settings->type);
		}
		else if(name == "device")
		{
			settings->device = value->stringValue;
			_bl->out.printDebug("Debug: device set to " + settings->device);
		}
		else if(name == "responsedelay")
		{
			settings->responseDelay = value->integerValue;
			if(settings->responseDelay > 10000) settings->responseDelay = 10000;
			_bl->out.printDebug("Debug: responseDelay set to " + std::to_string(settings->responseDelay));
		}
		else if(name == "txpowersetting")
		{
			settings->txPowerSetting = value->integerValue;
			_bl->out.printDebug("Debug: txPowerSetting set to " + std::to_string(settings->txPowerSetting));
		}
		else if(name == "oscillatorfrequency")
		{
			settings->oscillatorFrequency = value->integerValue;
			if(settings->oscillatorFrequency < 0) settings->oscillatorFrequency = -1;
			_bl->out.printDebug("Debug: oscillatorFrequency set to " + std::to_string(settings->oscillatorFrequency));
		}
		else if(name == "oneway")
		{
			settings->oneWay = value->integerValue;
			_bl->out.printDebug("Debug: oneWay set to " + std::to_string(settings->oneWay));
		}
		else if(name == "enablerxvalue")
		{
			int32_t number = value->integerValue;
			settings->enableRXValue = number;
			_bl->out.printDebug("Debug: enableRXValue set to " + std::to_string(settings->enableRXValue));
		}
		else if(name == "enabletxvalue")
		{
			int32_t number = value->integerValue;
			settings->enableTXValue = number;
			_bl->out.printDebug("Debug: enableTXValue set to " + std::to_string(settings->enableTXValue));
		}
		else if(name == "fastsending")
		{
			settings->fastSending = value->integerValue;
			_bl->out.printDebug("Debug: fastSending set to " + std::to_string(settings->fastSending));
		}
		else if(name == "waitforbus")
		{
			settings->waitForBus = value->integerValue;
			if(settings->waitForBus < 60) settings->waitForBus = 60;
			else if(settings->waitForBus > 210) settings->waitForBus = 210;
			_bl->out.printDebug("Debug: waitForBus set to " + std::to_string(settings->waitForBus));
		}
		else if(name == "interval")
		{
			settings->interval = value->integerValue;
			if(settings->interval > 100000) settings->interval = 100000;
			_bl->out.printDebug("Debug: interval set to " + std::to_string(settings->interval));
		}
		else if(name == "timeout")
		{
			settings->timeout = value->integerValue;
			if(settings->timeout > 100000) settings->timeout = 100000;
			_bl->out.printDebug("Debug: timeout set to " + std::to_string(settings->timeout));
		}
		else if(name == "watchdogtimeout")
		{
			settings->watchdogTimeout = value->integerValue;
			if(settings->watchdogTimeout > 100000) settings->timeout = 100000;
			_bl->out.printDebug("Debug: watchdogTimeout set to " + std::to_string(settings->watchdogTimeout));
		}
		else if(name == "sendfix")
		{
			settings->sendFix = value->integerValue;
			_bl->out.printDebug("Debug: sendFix set to " + std::to_string(settings->sendFix));
		}
		else if(name == "interruptpin")
		{
			int32_t number = value->integerValue;
			if(number >= 0)
			{
				settings->interruptPin = number;
				_bl->out.printDebug("Debug: interruptPin set to " + std::to_string(settings->interruptPin));
			}
		}
		else if(name == "gpio1")
		{
			int32_t number = value->integerValue;
			if(number > 0)
			{
				settings->gpio[1].number = number;
				_bl->out.printDebug("Debug: GPIO1 set to " + std::to_string(settings->gpio[1].number));
			}
		}
		else if(name == "gpio2")
		{
			int32_t number = value->integerValue;
			if(number > 0)
			{
				settings->gpio[2].number = number;
				_bl->out.printDebug("Debug: GPIO2 set to " + std::to_string(settings->gpio[2].number));
			}
		}
		else if(name == "gpio3")
		{
			int32_t number = value->integerValue;
			if(number > 0)
			{
				settings->gpio[3].number = number;
				_bl->out.printDebug("Debug: GPIO3 set to " + std::to_string(settings->gpio[3].number));
			}
		}
		else if(name == "stackposition")
		{
			int32_t number = value->integerValue;
			if(number > 0)
			{
				settings->stackPosition = number;
				_bl->out.printDebug("Debug: stackPosition set to " + std::to_string(settings->stackPosition));
			}
		}
		else if(name == "host")
		{
			settings->host = value->stringValue;
			_bl->out.printDebug("Debug: Host set to " + settings->host);
		}
		else if(name == "port")
		{
			settings->port = value->stringValue;
			_bl->out.printDebug("Debug: Port set to " + settings->port);
		}
		else if(name == "listenport")
		{
			settings->listenPort = value->stringValue;
			_bl->out.printDebug("Debug: listenPort set to " + settings->listenPort);
		}
		else if(name == "portkeepalive")
		{
			settings->portKeepAlive = value->stringValue;
			_bl->out.printDebug("Debug: PortKeepAlive set to " + settings->portKeepAlive);
		}
		else if(name == "lankey")
		{
			settings->lanKey = value->stringValue;
			_bl->out.printDebug("Debug: LANKey set to " + settings->lanKey);
		}
		else if(name == "ssl")
		{
			settings->ssl = value->integerValue;
			_bl->out.printDebug("Debug: SSL set to " + std::to_string(settings->ssl));
		}
		else if(name == "cafile")
		{
			settings->caFile = value->stringValue;
			_bl->out.printDebug("Debug: CAFile set to " + settings->caFile);
		}
		else if(name == "verifycertificate")
		{
			settings->verifyCertificate = value->integerValue;
			_bl->out.printDebug("Debug: VerifyCertificate set to " + std::to_string(settings->verifyCertificate));
		}
		else if(name == "listenthreadpriority")
		{
			settings->listenThreadPriority = value->integerValue;
			if(settings->listenThreadPriority > 99) settings->listenThreadPriority = 99;
			if(settings->listenThreadPriority < 0) settings->listenThreadPriority = 0;
			_bl->out.printDebug("Debug: ListenThreadPriority set to " + std::to_string(settings->listenThreadPriority));
		}
		else if(name == "listenthreadpolicy")
		{
			settings->listenThreadPolicy = ThreadManager::getThreadPolicyFromString(value->stringValue);
			settings->listenThreadPriority = ThreadManager::parseThreadPriority(settings->listenThreadPriority, settings->listenThreadPolicy);
			_bl->out.printDebug("Debug: ListenThreadPolicy set to " + std::to_string(settings->listenThreadPolicy));
		}
		else if(name == "ttsprogram")
		{
			settings->ttsProgram = value->stringValue;
			_bl->out.printDebug("Debug: ttsProgram set to " + settings->ttsProgram);
		}
		else if(name == "datapath")
		{
			settings->dataPath = value->stringValue;
			if(settings->dataPath.back() != '/') settings->dataPath.push_back('/');
			_bl->out.printDebug("Debug: dataPath set to " + settings->dataPath);
		}
		else if(name == "user")
		{
			settings->user = value->stringValue;
			_bl->out.printDebug("Debug: user set");
		}
		else if(name == "password")
		{
			settings->password = value->stringValue;
			_bl->out.printDebug("Debug: password set");
		}
		else
		{
			_bl->out.printWarning("Warning: Unknown physical interface setting: " + name);
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

}
}
