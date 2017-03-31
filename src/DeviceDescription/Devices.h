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

#ifndef DEVICES_H_
#define DEVICES_H_

#include <vector>
#include <memory>

#include "../Systems/Packet.h"
#include "../Sockets/RpcClientInfo.h"
#include "HomegearDevice.h"
#include "../IEvents.h"

namespace BaseLib
{

class SharedObjects;

namespace DeviceDescription
{

/**
 * Class to work with the device description files of one device family. It is used to load all device description files, list all device descriptions and to find the correct description for a device.
 */
class Devices : public IEvents
{
public:
	// {{{ Event handling
		class IDevicesEventSink : public IEventSinkBase
		{
		public:
			virtual void onDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output) = 0;
		};
	// }}}

	Devices(BaseLib::SharedObjects* baseLib, IDevicesEventSink* eventHandler, int32_t family);
	virtual ~Devices() {}
	bool empty() { return _devices.empty(); }
	void clear();
	void load();
	void load(std::string& xmlPath);
	std::shared_ptr<HomegearDevice> loadFile(std::string& filepath);
	uint32_t getTypeNumberFromTypeId(const std::string& typeId);
	std::shared_ptr<HomegearDevice> find(uint32_t typeNumber, uint32_t firmwareVersion, int32_t countFromSysinfo = -1);

	// {{{ RPC
	std::shared_ptr<Variable> getParamsetDescription(PRpcClientInfo clientInfo, int32_t deviceId, int32_t firmwareVersion, int32_t channel, ParameterGroup::Type::Enum type);
	PVariable listKnownDeviceType(PRpcClientInfo clientInfo, std::shared_ptr<HomegearDevice>& device, PSupportedDevice deviceType, int32_t channel, std::set<std::string>& fields);
	PVariable listKnownDeviceTypes(PRpcClientInfo clientInfo, bool channels, std::set<std::string>& fields);
	// }}}
protected:
	BaseLib::SharedObjects* _bl = nullptr;
	int32_t _family = -1;
	std::vector<std::shared_ptr<HomegearDevice>> _devices;
	std::vector<std::shared_ptr<HomegearDevice>> _dynamicDevices;

	std::shared_ptr<HomegearDevice> loadHomeMatic(std::string& filepath);
};

}
}
#endif /* DEVICES_H_ */
