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

#ifndef DEVICEFAMILY_H_
#define DEVICEFAMILY_H_

#include "IDeviceFamily.h"

namespace BaseLib {
namespace Systems {

class DeviceFamily : public IDeviceFamily {
 public:
  DeviceFamily(BaseLib::SharedObjects *bl, IFamilyEventSink *eventHandler, int32_t id, std::string name);

  virtual bool init();
  virtual void dispose();

  virtual bool lifetick();

  virtual std::shared_ptr<DeviceDescription::Devices> getRpcDevices();
  virtual void load();
  virtual void save(bool full);
  virtual std::shared_ptr<ICentral> getCentral();
  virtual std::string handleCliCommand(std::string &command);
  virtual bool hasPhysicalInterface();
  virtual std::shared_ptr<PhysicalInterfaces> physicalInterfaces();

  /*
   * Executed when Homegear is fully started.
   */
  virtual void homegearStarted();

  /*
   * Executed before Homegear starts shutting down.
   */
  virtual void homegearShuttingDown();

  // {{{ RPC
  virtual std::shared_ptr<Variable> getPairingInfo() = 0;
  virtual std::shared_ptr<Variable> getParamsetDescription(PRpcClientInfo clientInfo, int32_t deviceId, int32_t firmwareVersion, int32_t channel, ParameterGroup::Type::Enum type);
  virtual PVariable listKnownDeviceTypes(PRpcClientInfo clientInfo, bool channels, std::set<std::string> &fields);
  // }}}
 protected:
  std::shared_ptr<ICentral> _central;
  std::shared_ptr<PhysicalInterfaces> _physicalInterfaces;

  std::shared_ptr<DeviceDescription::Devices> _rpcDevices;

  virtual std::shared_ptr<ICentral> initializeCentral(uint32_t deviceId, int32_t address, std::string serialNumber) = 0;
  virtual void createCentral() = 0;
 private:
  DeviceFamily(const DeviceFamily &) = delete;
  DeviceFamily &operator=(const DeviceFamily &) = delete;
};

}
}
#endif
