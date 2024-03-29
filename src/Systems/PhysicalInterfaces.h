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

#ifndef SYSTEMSPHYSICALINTERFACES_H_
#define SYSTEMSPHYSICALINTERFACES_H_

#include "../Variable.h"
#include "PhysicalInterfaceSettings.h"

#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <vector>
#include <atomic>
#include <functional>

namespace BaseLib {

class SharedObjects;

namespace Systems {

class IPhysicalInterface;

class PhysicalInterfaces {
 public:
  PhysicalInterfaces(BaseLib::SharedObjects *bl, int32_t familyId, std::map<std::string, PPhysicalInterfaceSettings> physicalInterfaceSettings);
  virtual ~PhysicalInterfaces();
  void dispose();

  uint32_t count();
  virtual void stopListening();
  virtual void startListening();
  bool lifetick();
  bool isOpen();
  void setup(int32_t userID, int32_t groupID, bool setPermissions);
  virtual PVariable listInterfaces();

  void setRawPacketEvent(std::function<void(int32_t familyId, const std::string &interfaceId, const BaseLib::PVariable &packet)> value) { _rawPacketEvent.swap(value); }
 protected:
  BaseLib::SharedObjects *_bl = nullptr;
  int32_t _familyId = -1;
  std::map<std::string, PPhysicalInterfaceSettings> _physicalInterfaceSettings;
  std::mutex _physicalInterfacesMutex;
  std::map<std::string, std::shared_ptr<IPhysicalInterface>> _physicalInterfaces;

  std::function<void(int32_t familyId, const std::string &interfaceId, const BaseLib::PVariable &packet)> _rawPacketEvent;

  virtual void create() {};

  void rawPacketEvent(int32_t familyId, const std::string &interfaceId, const BaseLib::PVariable &packet);
};

}

}
#endif
