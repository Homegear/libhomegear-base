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

#ifndef FAMILYSETTINGS_H_
#define FAMILYSETTINGS_H_

#include "PhysicalInterfaceSettings.h"

#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <cstring>
#include <vector>

namespace BaseLib {

class SharedObjects;

namespace Systems {

class FamilySettings {
 public:
  struct FamilySetting {
    std::string stringValue;
    int32_t integerValue;
    std::vector<char> binaryValue;
  };
  typedef std::shared_ptr<FamilySetting> PFamilySetting;

  FamilySettings(BaseLib::SharedObjects *bl, int32_t familyId);
  virtual ~FamilySettings();
  void dispose();
  void load(const std::string& filename);
  bool changed();
  PFamilySetting get(std::string name);
  std::string getString(std::string name);
  int32_t getNumber(std::string name);
  std::vector<char> getBinary(std::string name);
  void set(std::string name, const std::string &value);
  void set(std::string name, int32_t value);
  void set(std::string name, const std::vector<char> &value);
  void deleteFromDatabase(std::string name);
  std::map<std::string, PPhysicalInterfaceSettings> getPhysicalInterfaceSettings();
 private:
  BaseLib::SharedObjects *_bl = nullptr;
  int32_t _familyId = -1;
  std::mutex _settingsMutex;
  std::map<std::string, PFamilySetting> _settings;
  std::map<std::string, PPhysicalInterfaceSettings> _physicalInterfaceSettings;

  void loadFromDatabase();
  void processStringSetting(std::string &name, std::string &value, PPhysicalInterfaceSettings &settings);
  void processDatabaseSetting(std::string &name, PFamilySetting &value, PPhysicalInterfaceSettings &settings);
};

}

}

#endif
