/* Copyright 2013-2019 Homegear GmbH
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

#include <memory>
#include "../BaseLib.h"

namespace BaseLib {

namespace Systems {

FamilySettings::FamilySettings(BaseLib::SharedObjects *bl, int32_t familyId) {
  _bl = bl;
  _familyId = familyId;
}

FamilySettings::~FamilySettings() {
  dispose();
}

void FamilySettings::dispose() {
  _settings.clear();
  _physicalInterfaceSettings.clear();
}

FamilySettings::PFamilySetting FamilySettings::get(std::string name) {
  BaseLib::HelperFunctions::toLower(name);
  _settingsMutex.lock();
  try {
    auto settingIterator = _settings.find(name);
    if (settingIterator != _settings.end()) {
      PFamilySetting setting = settingIterator->second;
      _settingsMutex.unlock();
      return setting;
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  _settingsMutex.unlock();
  return PFamilySetting();
}

std::string FamilySettings::getString(std::string name) {
  _settingsMutex.lock();
  try {
    auto settingIterator = _settings.find(name);
    if (settingIterator != _settings.end()) {
      std::string setting = settingIterator->second->stringValue;
      _settingsMutex.unlock();
      return setting;
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  _settingsMutex.unlock();
  return "";
}

int32_t FamilySettings::getNumber(std::string name) {
  _settingsMutex.lock();
  try {
    auto settingIterator = _settings.find(name);
    if (settingIterator != _settings.end()) {
      int32_t setting = settingIterator->second->integerValue;
      _settingsMutex.unlock();
      return setting;
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  _settingsMutex.unlock();
  return 0;
}

std::vector<char> FamilySettings::getBinary(std::string name) {
  _settingsMutex.lock();
  try {
    auto settingIterator = _settings.find(name);
    if (settingIterator != _settings.end()) {
      std::vector<char> setting = settingIterator->second->binaryValue;
      _settingsMutex.unlock();
      return setting;
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  _settingsMutex.unlock();
  return std::vector<char>();
}

void FamilySettings::set(std::string name, const std::string &value) {
  try {
    BaseLib::HelperFunctions::toLower(name);
    if (name.empty()) return;
    {
      std::lock_guard<std::mutex> settingGuard(_settingsMutex);
      auto settingIterator = _settings.find(name);
      if (settingIterator != _settings.end()) {
        settingIterator->second->stringValue = value;
        settingIterator->second->integerValue = 0;
        settingIterator->second->binaryValue.clear();
      } else {
        PFamilySetting setting(new FamilySetting());
        setting->stringValue = value;
        _settings[name] = setting;
      }
    }
    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(_familyId));
    data.push_back(std::make_shared<Database::DataColumn>(0));
    data.push_back(std::make_shared<Database::DataColumn>(name));
    data.push_back(std::make_shared<Database::DataColumn>(_familyId));
    data.push_back(std::make_shared<Database::DataColumn>(0));
    data.push_back(std::make_shared<Database::DataColumn>(name));
    data.push_back(std::make_shared<Database::DataColumn>());
    data.push_back(std::make_shared<Database::DataColumn>(value));
    data.push_back(std::make_shared<Database::DataColumn>());
    _bl->db->saveFamilyVariableAsynchronous(_familyId, data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void FamilySettings::set(std::string name, int32_t value) {
  try {
    BaseLib::HelperFunctions::toLower(name);
    if (name.empty()) return;
    {
      std::lock_guard<std::mutex> settingGuard(_settingsMutex);
      auto settingIterator = _settings.find(name);
      if (settingIterator != _settings.end()) {
        settingIterator->second->stringValue.clear();
        settingIterator->second->integerValue = value;
        settingIterator->second->binaryValue.clear();
      } else {
        PFamilySetting setting(new FamilySetting());
        setting->integerValue = value;
        _settings[name] = setting;
      }
    }
    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(_familyId));
    data.push_back(std::make_shared<Database::DataColumn>(1));
    data.push_back(std::make_shared<Database::DataColumn>(name));
    data.push_back(std::make_shared<Database::DataColumn>(_familyId));
    data.push_back(std::make_shared<Database::DataColumn>(1));
    data.push_back(std::make_shared<Database::DataColumn>(name));
    data.push_back(std::make_shared<Database::DataColumn>(value));
    data.push_back(std::make_shared<Database::DataColumn>());
    data.push_back(std::make_shared<Database::DataColumn>());
    _bl->db->saveFamilyVariableAsynchronous(_familyId, data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void FamilySettings::set(std::string name, const std::vector<char> &value) {
  try {
    BaseLib::HelperFunctions::toLower(name);
    if (name.empty()) return;
    {
      std::lock_guard<std::mutex> settingGuard(_settingsMutex);
      auto settingIterator = _settings.find(name);
      if (settingIterator != _settings.end()) {
        settingIterator->second->stringValue.clear();
        settingIterator->second->integerValue = 0;
        settingIterator->second->binaryValue = value;
      } else {
        PFamilySetting setting(new FamilySetting());
        setting->binaryValue = value;
        _settings[name] = setting;
      }
    }
    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(_familyId));
    data.push_back(std::make_shared<Database::DataColumn>(2));
    data.push_back(std::make_shared<Database::DataColumn>(name));
    data.push_back(std::make_shared<Database::DataColumn>(_familyId));
    data.push_back(std::make_shared<Database::DataColumn>(2));
    data.push_back(std::make_shared<Database::DataColumn>(name));
    data.push_back(std::make_shared<Database::DataColumn>());
    data.push_back(std::make_shared<Database::DataColumn>());
    data.push_back(std::make_shared<Database::DataColumn>(value));
    _bl->db->saveFamilyVariableAsynchronous(_familyId, data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void FamilySettings::deleteFromDatabase(std::string name) {
  try {
    BaseLib::HelperFunctions::toLower(name);
    if (name.empty()) return;
    Database::DataRow data;
    data.push_back(std::make_shared<Database::DataColumn>(_familyId));
    data.push_back(std::make_shared<Database::DataColumn>(name));
    _bl->db->deleteFamilyVariable(data);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::map<std::string, PPhysicalInterfaceSettings> FamilySettings::getPhysicalInterfaceSettings() {
  return _physicalInterfaceSettings;
}

void FamilySettings::load(const std::string& filename) {
  try {
    std::unique_lock<std::mutex> settingsGuard(_settingsMutex);
    _settings.clear();
    std::array<char, 1024> input{};
    FILE *fin = nullptr;
    int32_t len = 0, ptr = 0;
    bool found = false;
    bool generalSettings = false;

    if (!(fin = fopen(filename.c_str(), "r"))) {
      _bl->out.printError("Unable to open family setting file: " + filename + ". " + strerror(errno));
      return;
    }

    auto settings = std::make_shared<BaseLib::Systems::PhysicalInterfaceSettings>();
    std::string title;
    while (fgets(input.data(), input.size(), fin)) {
      if (input[0] == '#' || input[0] == '-' || input[0] == '_') continue;
      len = strnlen(input.data(), input.size());
      if (len < 2) continue;
      if (input.at(len - 1) == '\n') input.at(len - 1) = '\0';
      ptr = 0;
      if (input[0] == '[') {
        while (ptr < len) {
          if (input[ptr] == ']') {
            input[ptr] = '\0';
            if (!settings->type.empty()) {
              if (settings->id.empty()) settings->id = settings->type;
              _physicalInterfaceSettings[settings->id] = settings;
            } else if (!title.empty() && !generalSettings && (settings->type.empty() || settings->id.empty())) {
              _bl->out.printError("Error: Physical interface with title \"" + title + "\" has no type or id set. Please control your settings in \"" + filename + "\".");
            }
            settings = std::make_shared<BaseLib::Systems::PhysicalInterfaceSettings>();
            title = std::string(&input[1]);
            _bl->out.printDebug("Debug: Loading section \"" + title + "\"");
            BaseLib::HelperFunctions::toLower(title);
            generalSettings = (title == "general");
            break;
          }
          ptr++;
        }
        continue;
      }
      found = false;
      while (ptr < len) {
        if (input[ptr] == '=') {
          found = true;
          input[ptr++] = '\0';
          break;
        }
        ptr++;
      }
      if (found) {
        std::string name(input.data());
        BaseLib::HelperFunctions::toLower(name);
        BaseLib::HelperFunctions::trim(name);
        std::string value(&input[ptr]);
        BaseLib::HelperFunctions::trim(value);

        if (generalSettings) {
          if (_settings.find(name) != _settings.end()) _bl->out.printWarning("Warning: Setting defined twice: " + name);
          PFamilySetting setting(new FamilySetting());
          setting->stringValue = value;
          if (value == "true" || value == "false") setting->integerValue = (int32_t)(value == "true");
          else setting->integerValue = Math::getNumber(value);
          setting->binaryValue = BaseLib::HelperFunctions::getBinary(value);
          HelperFunctions::toLower(value);
          _settings[name] = std::move(setting);
          _bl->out.printDebug("Debug: Family setting " + name + " set to " + value);
        } else processStringSetting(name, value, settings);
      }
    }
    if (!settings->type.empty()) {
      if (settings->id.empty()) settings->id = settings->type;
      _physicalInterfaceSettings[settings->id] = settings;
    } else if (!title.empty() && !generalSettings && (settings->type.empty() || settings->id.empty())) {
      _bl->out.printError("Error: Physical interface with title \"" + title + "\" has no type or id set. Please control your settings in \"" + filename + "\".");
    }

    fclose(fin);

    settingsGuard.unlock();
    loadFromDatabase();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void FamilySettings::loadFromDatabase() {
  try {
    std::lock_guard<std::mutex> settingsGuard(_settingsMutex);
    std::shared_ptr<BaseLib::Database::DataTable> rows = _bl->db->getFamilyVariables(_familyId);
    if (!rows) return;
    for (auto &row : *rows) {
      PFamilySetting setting(new FamilySetting());
      setting->integerValue = row.second.at(4)->intValue;
      setting->stringValue = std::move(row.second.at(5)->textValue);
      setting->binaryValue = std::move(*(row.second.at(6)->binaryValue));

      std::pair<std::string, std::string> interfacePair = HelperFunctions::splitFirst(row.second.at(3)->textValue, '.');
      if (interfacePair.second.empty()) {
        std::string &name = BaseLib::HelperFunctions::toLower(interfacePair.first);
        _settings[name] = std::move(setting);
      } else {
        _settings[row.second.at(3)->textValue] = setting;
        auto settingsIterator = _physicalInterfaceSettings.find(interfacePair.first);
        PPhysicalInterfaceSettings settings;
        if (settingsIterator != _physicalInterfaceSettings.end()) settings = settingsIterator->second;
        else {
          settings.reset(new PhysicalInterfaceSettings());
          settings->id = interfacePair.first;
          _physicalInterfaceSettings[settings->id] = settings;
        }
        HelperFunctions::toLower(interfacePair.second);
        processDatabaseSetting(interfacePair.second, setting, settings);
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void FamilySettings::processStringSetting(std::string &name, std::string &value, PPhysicalInterfaceSettings &settings) {
  try {
    { //Insert into "all" map
      auto setting = Rpc::JsonDecoder::decode(value);
      settings->all.emplace(name, setting);
    }

    if (name == "id") {
      if (!value.empty()) {
        settings->id = value;
        _bl->out.printDebug("Debug: id set to " + settings->id);
      }
    } else if (name == "default") {
      settings->isDefault = (value == "true");
      _bl->out.printDebug("Debug: default set to " + std::to_string(settings->isDefault));
    } else if (name == "rawpacketevents") {
      settings->rawPacketEvents = (value == "true");
      _bl->out.printDebug("Debug: rawPacketEvents set to " + std::to_string(settings->rawPacketEvents));
    } else if (name == "devicetype") {
      if (settings->type.length() > 0)
        _bl->out.printWarning("Warning: deviceType for \"" + settings->id + "\" in family with ID \"" + std::to_string(_familyId) + "\" is set multiple times within one device block. Is the interface header missing (e. g. \"[My Interface]\")?");
      BaseLib::HelperFunctions::toLower(value);
      settings->type = value;
      _bl->out.printDebug("Debug: deviceType set to " + settings->type);
    } else if (name == "device") {
      settings->device = value;
      _bl->out.printDebug("Debug: device set to " + settings->device);
    } else if (name == "baudrate") {
      settings->baudrate = BaseLib::Math::getNumber(value);
      _bl->out.printDebug("Debug: baudrate set to " + std::to_string(settings->baudrate));
    } else if (name == "openwriteonly") {
      settings->openWriteonly = (value == "true");
      _bl->out.printDebug("Debug: openWriteonly set to " + std::to_string(settings->openWriteonly));
    } else if (name == "responsedelay") {
      settings->responseDelay = BaseLib::Math::getNumber(value);
      if (settings->responseDelay > 10000) settings->responseDelay = 10000;
      _bl->out.printDebug("Debug: responseDelay set to " + std::to_string(settings->responseDelay));
    } else if (name == "txpowersetting") {
      settings->txPowerSetting = BaseLib::Math::getNumber(value);
      _bl->out.printDebug("Debug: txPowerSetting set to " + std::to_string(settings->txPowerSetting));
    } else if (name == "oscillatorfrequency") {
      settings->oscillatorFrequency = BaseLib::Math::getNumber(value);
      if (settings->oscillatorFrequency < 0) settings->oscillatorFrequency = -1;
      _bl->out.printDebug("Debug: oscillatorFrequency set to " + std::to_string(settings->oscillatorFrequency));
    } else if (name == "oneway") {
      BaseLib::HelperFunctions::toLower(value);
      if (value == "true") settings->oneWay = true;
      _bl->out.printDebug("Debug: oneWay set to " + std::to_string(settings->oneWay));
    } else if (name == "enablerxvalue") {
      int32_t number = BaseLib::Math::getNumber(value);
      settings->enableRXValue = number;
      _bl->out.printDebug("Debug: enableRXValue set to " + std::to_string(settings->enableRXValue));
    } else if (name == "enabletxvalue") {
      int32_t number = BaseLib::Math::getNumber(value);
      settings->enableTXValue = number;
      _bl->out.printDebug("Debug: enableTXValue set to " + std::to_string(settings->enableTXValue));
    } else if (name == "fastsending") {
      BaseLib::HelperFunctions::toLower(value);
      if (value == "true") settings->fastSending = true;
      _bl->out.printDebug("Debug: fastSending set to " + std::to_string(settings->fastSending));
    } else if (name == "waitforbus") {
      settings->waitForBus = BaseLib::Math::getNumber(value);
      if (settings->waitForBus < 60) settings->waitForBus = 60;
      else if (settings->waitForBus > 210) settings->waitForBus = 210;
      _bl->out.printDebug("Debug: waitForBus set to " + std::to_string(settings->waitForBus));
    } else if (name == "interval") {
      settings->interval = BaseLib::Math::getNumber(value);
      if (settings->interval > 100000) settings->interval = 100000;
      _bl->out.printDebug("Debug: interval set to " + std::to_string(settings->interval));
    } else if (name == "timeout") {
      settings->timeout = BaseLib::Math::getNumber(value);
      if (settings->timeout > 100000) settings->timeout = 100000;
      _bl->out.printDebug("Debug: timeout set to " + std::to_string(settings->timeout));
    } else if (name == "watchdogtimeout") {
      settings->watchdogTimeout = BaseLib::Math::getNumber(value);
      if (settings->watchdogTimeout > 100000) settings->watchdogTimeout = 100000;
      _bl->out.printDebug("Debug: watchdogTimeout set to " + std::to_string(settings->watchdogTimeout));
    } else if (name == "sendfix") {
      BaseLib::HelperFunctions::toLower(value);
      if (value == "true") settings->sendFix = true;
      _bl->out.printDebug("Debug: sendFix set to " + std::to_string(settings->sendFix));
    } else if (name == "interruptpin") {
      int32_t number = BaseLib::Math::getNumber(value);
      if (number >= 0) {
        settings->interruptPin = number;
        _bl->out.printDebug("Debug: interruptPin set to " + std::to_string(settings->interruptPin));
      }
    } else if (name == "gpio1") {
      int32_t number = BaseLib::Math::getNumber(value);
      if (number > 0) {
        settings->gpio[1].number = number;
        _bl->out.printDebug("Debug: GPIO1 set to " + std::to_string(settings->gpio[1].number));
      }
    } else if (name == "gpio2") {
      int32_t number = BaseLib::Math::getNumber(value);
      if (number > 0) {
        settings->gpio[2].number = number;
        _bl->out.printDebug("Debug: GPIO2 set to " + std::to_string(settings->gpio[2].number));
      }
    } else if (name == "gpio3") {
      int32_t number = BaseLib::Math::getNumber(value);
      if (number > 0) {
        settings->gpio[3].number = number;
        _bl->out.printDebug("Debug: GPIO3 set to " + std::to_string(settings->gpio[3].number));
      }
    } else if (name == "stackposition") {
      int32_t number = BaseLib::Math::getNumber(value);
      if (number > 0) {
        settings->stackPosition = number;
        _bl->out.printDebug("Debug: stackPosition set to " + std::to_string(settings->stackPosition));
      }
    } else if (name == "host") {
      settings->host = value;
      _bl->out.printDebug("Debug: host set to " + settings->host);
    } else if (name == "port") {
      settings->port = value;
      _bl->out.printDebug("Debug: port set to " + settings->port);
    } else if (name == "port2") {
      settings->port2 = value;
      _bl->out.printDebug("Debug: port2 set to " + settings->port2);
    } else if (name == "port3") {
      settings->port3 = value;
      _bl->out.printDebug("Debug: port3 set to " + settings->port3);
    } else if (name == "port4") {
      settings->port4 = value;
      _bl->out.printDebug("Debug: port4 set to " + settings->port4);
    } else if (name == "listenip") {
      settings->listenIp = value;
      _bl->out.printDebug("Debug: listenIp set to " + settings->listenIp);
    } else if (name == "listenport") {
      settings->listenPort = value;
      _bl->out.printDebug("Debug: listenPort set to " + settings->listenPort);
    } else if (name == "portkeepalive") {
      settings->portKeepAlive = value;
      _bl->out.printDebug("Debug: portKeepAlive set to " + settings->portKeepAlive);
    } else if (name == "address") {
      settings->address = BaseLib::Math::getNumber(value);
      _bl->out.printDebug("Debug: address set to " + std::to_string(settings->address));
    } else if (name == "oldrfkey") {
      _bl->out.printError("Error: OldRFKey now needs to be set in section [General] of homematicbidcos.conf.");
      PFamilySetting setting(new FamilySetting);
      setting->stringValue = value;
      if (_settings.find("oldrfkey") == _settings.end()) _settings["oldrfkey"] = setting;
    } else if (name == "rfkey") {
      _bl->out.printError("Error: RFKey now needs to be set in section [General] of homematicbidcos.conf.");
      PFamilySetting setting(new FamilySetting);
      setting->stringValue = value;
      if (_settings.find("rfkey") == _settings.end()) _settings["rfkey"] = setting;
    } else if (name == "currentrfkeyindex") {
      _bl->out.printError("Error: CurrentRFKeyIndex now needs to be set in section [General] of homematicbidcos.conf.");
      PFamilySetting setting(new FamilySetting);
      setting->stringValue = value;
      if (_settings.find("currentrfkeyindex") == _settings.end()) _settings["currentrfkeyindex"] = setting;
    } else if (name == "lankey") {
      settings->lanKey = value;
      _bl->out.printDebug("Debug: lanKey set to " + settings->lanKey);
    } else if (name == "ssl") {
      BaseLib::HelperFunctions::toLower(value);
      if (value == "true") settings->ssl = true;
      _bl->out.printDebug("Debug: ssl set to " + std::to_string(settings->ssl));
    } else if (name == "cafile") {
      settings->caFile = value;
      _bl->out.printDebug("Debug: caFile set to " + settings->caFile);
    } else if (name == "certfile") {
      settings->certFile = value;
      _bl->out.printDebug("Debug: certFile set to " + settings->certFile);
    } else if (name == "keyfile") {
      settings->keyFile = value;
      _bl->out.printDebug("Debug: keyFile set to " + settings->keyFile);
    } else if (name == "verifycertificate") {
      BaseLib::HelperFunctions::toLower(value);
      if (value == "false") settings->verifyCertificate = false;
      _bl->out.printDebug("Debug: verifyCertificate set to " + std::to_string(settings->verifyCertificate));
    } else if (name == "useidforhostnameverification") {
      BaseLib::HelperFunctions::toLower(value);
      settings->useIdForHostnameVerification = (value == "true");
      _bl->out.printDebug("Debug: useIdForHostnameVerification set to " + std::to_string(settings->useIdForHostnameVerification));
    } else if (name == "listenthreadpriority") {
      settings->listenThreadPriority = BaseLib::Math::getNumber(value);
      if (settings->listenThreadPriority > 99) settings->listenThreadPriority = 99;
      if (settings->listenThreadPriority < 0) settings->listenThreadPriority = 0;
      _bl->out.printDebug("Debug: listenThreadPriority set to " + std::to_string(settings->listenThreadPriority));
    } else if (name == "listenthreadpolicy") {
      settings->listenThreadPolicy = ThreadManager::getThreadPolicyFromString(value);
      settings->listenThreadPriority = ThreadManager::parseThreadPriority(settings->listenThreadPriority, settings->listenThreadPolicy);
      _bl->out.printDebug("Debug: listenThreadPolicy set to " + std::to_string(settings->listenThreadPolicy));
    } else if (name == "ttsprogram") {
      settings->ttsProgram = value;
      _bl->out.printDebug("Debug: ttsProgram set to " + settings->ttsProgram);
    } else if (name == "datapath") {
      settings->dataPath = value;
      if (settings->dataPath.back() != '/') settings->dataPath.push_back('/');
      _bl->out.printDebug("Debug: dataPath set to " + settings->dataPath);
    } else if (name == "user") {
      settings->user = value;
      _bl->out.printDebug("Debug: user set");
    } else if (name == "password") {
      settings->password = value;
      _bl->out.printDebug("Debug: password set");
    } else if (name == "passwords21") {
      settings->passwordS21 = value;
      _bl->out.printDebug("Debug: password S2 1 set");
    } else if (name == "passwords22") {
      settings->passwordS22 = value;
      _bl->out.printDebug("Debug: password S2 2 set");
    } else if (name == "passwords23") {
      settings->passwordS23 = value;
      _bl->out.printDebug("Debug: password S2 3 set");
    } else if (name == "additionalcommands") {
      settings->additionalCommands = value;
      _bl->out.printDebug("Debug: additionalCommands set to " + settings->additionalCommands);
    } else if (name == "mode") {
      settings->mode = value;
      _bl->out.printDebug("Debug: mode set to " + settings->mode);
    } else if (name == "serialnumber") {
      settings->serialNumber = value;
      _bl->out.printDebug("Debug: serialNumber set to " + settings->serialNumber);
    } else if (name == "uuid") {
      settings->uuid = value;
      _bl->out.printDebug("Debug: uuid set to " + settings->uuid);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void FamilySettings::processDatabaseSetting(std::string &name, PFamilySetting &value, PPhysicalInterfaceSettings &settings) {
  try {
    { //Insert into "all" map
      if (!value->stringValue.empty()) settings->all.emplace(name, std::make_shared<Variable>(value->stringValue));
      else if (!value->binaryValue.empty()) settings->all.emplace(name, std::make_shared<Variable>(value->binaryValue));
      else settings->all.emplace(name, std::make_shared<Variable>(value->integerValue));
    }

    if (name == "default") {
      settings->isDefault = (bool)value->integerValue;
      _bl->out.printDebug("Debug: default set to " + std::to_string(settings->isDefault));
    } else if (name == "rawpacketevents") {
      settings->rawPacketEvents = (bool)value->integerValue;
      _bl->out.printDebug("Debug: rawPacketEvents set to " + std::to_string(settings->rawPacketEvents));
    } else if (name == "devicetype") {
      if (settings->type.length() > 0)
        _bl->out.printWarning("Warning: deviceType for \"" + settings->id + "\" in family with ID \"" + std::to_string(_familyId) + "\" is set multiple times within one device block. Is the interface header missing (e. g. \"[My Interface]\")?");
      BaseLib::HelperFunctions::toLower(value->stringValue);
      settings->type = value->stringValue;
      _bl->out.printDebug("Debug: deviceType set to " + settings->type);
    } else if (name == "device") {
      settings->device = value->stringValue;
      _bl->out.printDebug("Debug: device set to " + settings->device);
    } else if (name == "baudrate") {
      settings->baudrate = value->integerValue;
      _bl->out.printDebug("Debug: baudrate set to " + std::to_string(settings->baudrate));
    } else if (name == "openwriteonly") {
      settings->openWriteonly = (bool)value->integerValue;
      _bl->out.printDebug("Debug: openWriteonly set to " + std::to_string(settings->openWriteonly));
    } else if (name == "responsedelay") {
      settings->responseDelay = value->integerValue;
      if (settings->responseDelay > 10000) settings->responseDelay = 10000;
      _bl->out.printDebug("Debug: responseDelay set to " + std::to_string(settings->responseDelay));
    } else if (name == "txpowersetting") {
      settings->txPowerSetting = value->integerValue;
      _bl->out.printDebug("Debug: txPowerSetting set to " + std::to_string(settings->txPowerSetting));
    } else if (name == "oscillatorfrequency") {
      settings->oscillatorFrequency = value->integerValue;
      if (settings->oscillatorFrequency < 0) settings->oscillatorFrequency = -1;
      _bl->out.printDebug("Debug: oscillatorFrequency set to " + std::to_string(settings->oscillatorFrequency));
    } else if (name == "oneway") {
      settings->oneWay = (bool)value->integerValue;
      _bl->out.printDebug("Debug: oneWay set to " + std::to_string(settings->oneWay));
    } else if (name == "enablerxvalue") {
      int32_t number = value->integerValue;
      settings->enableRXValue = number;
      _bl->out.printDebug("Debug: enableRXValue set to " + std::to_string(settings->enableRXValue));
    } else if (name == "enabletxvalue") {
      int32_t number = value->integerValue;
      settings->enableTXValue = number;
      _bl->out.printDebug("Debug: enableTXValue set to " + std::to_string(settings->enableTXValue));
    } else if (name == "fastsending") {
      settings->fastSending = (bool)value->integerValue;
      _bl->out.printDebug("Debug: fastSending set to " + std::to_string(settings->fastSending));
    } else if (name == "waitforbus") {
      settings->waitForBus = value->integerValue;
      if (settings->waitForBus < 60) settings->waitForBus = 60;
      else if (settings->waitForBus > 210) settings->waitForBus = 210;
      _bl->out.printDebug("Debug: waitForBus set to " + std::to_string(settings->waitForBus));
    } else if (name == "interval") {
      settings->interval = value->integerValue;
      if (settings->interval > 100000) settings->interval = 100000;
      _bl->out.printDebug("Debug: interval set to " + std::to_string(settings->interval));
    } else if (name == "timeout") {
      settings->timeout = value->integerValue;
      if (settings->timeout > 100000) settings->timeout = 100000;
      _bl->out.printDebug("Debug: timeout set to " + std::to_string(settings->timeout));
    } else if (name == "watchdogtimeout") {
      settings->watchdogTimeout = value->integerValue;
      if (settings->watchdogTimeout > 100000) settings->watchdogTimeout = 100000;
      _bl->out.printDebug("Debug: watchdogTimeout set to " + std::to_string(settings->watchdogTimeout));
    } else if (name == "sendfix") {
      settings->sendFix = value->integerValue;
      _bl->out.printDebug("Debug: sendFix set to " + std::to_string(settings->sendFix));
    } else if (name == "interruptpin") {
      int32_t number = value->integerValue;
      if (number >= 0) {
        settings->interruptPin = number;
        _bl->out.printDebug("Debug: interruptPin set to " + std::to_string(settings->interruptPin));
      }
    } else if (name == "gpio1") {
      int32_t number = value->integerValue;
      if (number > 0) {
        settings->gpio[1].number = number;
        _bl->out.printDebug("Debug: GPIO1 set to " + std::to_string(settings->gpio[1].number));
      }
    } else if (name == "gpio2") {
      int32_t number = value->integerValue;
      if (number > 0) {
        settings->gpio[2].number = number;
        _bl->out.printDebug("Debug: GPIO2 set to " + std::to_string(settings->gpio[2].number));
      }
    } else if (name == "gpio3") {
      int32_t number = value->integerValue;
      if (number > 0) {
        settings->gpio[3].number = number;
        _bl->out.printDebug("Debug: GPIO3 set to " + std::to_string(settings->gpio[3].number));
      }
    } else if (name == "stackposition") {
      int32_t number = value->integerValue;
      if (number > 0) {
        settings->stackPosition = number;
        _bl->out.printDebug("Debug: stackPosition set to " + std::to_string(settings->stackPosition));
      }
    } else if (name == "host") {
      settings->host = value->stringValue;
      _bl->out.printDebug("Debug: host set to " + settings->host);
    } else if (name == "port") {
      settings->port = value->stringValue;
      _bl->out.printDebug("Debug: port set to " + settings->port);
    } else if (name == "port2") {
      settings->port2 = value->stringValue;
      _bl->out.printDebug("Debug: port2 set to " + settings->port2);
    } else if (name == "port3") {
      settings->port3 = value->stringValue;
      _bl->out.printDebug("Debug: port3 set to " + settings->port3);
    } else if (name == "port4") {
      settings->port4 = value->stringValue;
      _bl->out.printDebug("Debug: port4 set to " + settings->port4);
    } else if (name == "listenip") {
      settings->listenIp = value->stringValue;
      _bl->out.printDebug("Debug: listenIp set to " + settings->listenIp);
    } else if (name == "listenport") {
      settings->listenPort = value->stringValue;
      _bl->out.printDebug("Debug: listenPort set to " + settings->listenPort);
    } else if (name == "portkeepalive") {
      settings->portKeepAlive = value->stringValue;
      _bl->out.printDebug("Debug: portKeepAlive set to " + settings->portKeepAlive);
    } else if (name == "address") {
      settings->address = value->integerValue;
      _bl->out.printDebug("Debug: address set to " + std::to_string(settings->address));
    } else if (name == "lankey") {
      settings->lanKey = value->stringValue;
      _bl->out.printDebug("Debug: lanKey set to " + settings->lanKey);
    } else if (name == "ssl") {
      settings->ssl = value->integerValue;
      _bl->out.printDebug("Debug: ssl set to " + std::to_string(settings->ssl));
    } else if (name == "cafile") {
      settings->caFile = value->stringValue;
      _bl->out.printDebug("Debug: caFile set to " + settings->caFile);
    } else if (name == "certfile") {
      settings->certFile = value->stringValue;
      _bl->out.printDebug("Debug: certFile set to " + settings->certFile);
    } else if (name == "keyfile") {
      settings->keyFile = value->stringValue;
      _bl->out.printDebug("Debug: keyFile set to " + settings->keyFile);
    } else if (name == "verifycertificate") {
      settings->verifyCertificate = value->integerValue;
      _bl->out.printDebug("Debug: verifyCertificate set to " + std::to_string(settings->verifyCertificate));
    } else if (name == "useidforhostnameverification") {
      settings->useIdForHostnameVerification = value->integerValue;
      _bl->out.printDebug("Debug: useIdForHostnameVerification set to " + std::to_string(settings->verifyCertificate));
    } else if (name == "listenthreadpriority") {
      settings->listenThreadPriority = value->integerValue;
      if (settings->listenThreadPriority > 99) settings->listenThreadPriority = 99;
      if (settings->listenThreadPriority < 0) settings->listenThreadPriority = 0;
      _bl->out.printDebug("Debug: listenThreadPriority set to " + std::to_string(settings->listenThreadPriority));
    } else if (name == "listenthreadpolicy") {
      settings->listenThreadPolicy = ThreadManager::getThreadPolicyFromString(value->stringValue);
      settings->listenThreadPriority = ThreadManager::parseThreadPriority(settings->listenThreadPriority, settings->listenThreadPolicy);
      _bl->out.printDebug("Debug: listenThreadPolicy set to " + std::to_string(settings->listenThreadPolicy));
    } else if (name == "ttsprogram") {
      settings->ttsProgram = value->stringValue;
      _bl->out.printDebug("Debug: ttsProgram set to " + settings->ttsProgram);
    } else if (name == "datapath") {
      settings->dataPath = value->stringValue;
      if (settings->dataPath.back() != '/') settings->dataPath.push_back('/');
      _bl->out.printDebug("Debug: dataPath set to " + settings->dataPath);
    } else if (name == "user") {
      settings->user = value->stringValue;
      _bl->out.printDebug("Debug: user set");
    } else if (name == "password") {
      settings->password = value->stringValue;
      _bl->out.printDebug("Debug: password set");
    } else if (name == "additionalcommands") {
      settings->additionalCommands = value->stringValue;
      _bl->out.printDebug("Debug: additionalCommands set to " + settings->additionalCommands);
    } else if (name == "mode") {
      settings->mode = value->stringValue;
      _bl->out.printDebug("Debug: mode set to " + settings->mode);
    } else if (name == "serialnumber") {
      settings->serialNumber = value->stringValue;
      _bl->out.printDebug("Debug: serialNumber set to " + settings->serialNumber);
    } else if (name == "uuid") {
      settings->uuid = value->stringValue;
      _bl->out.printDebug("Debug: uuid set to " + settings->uuid);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}
}
