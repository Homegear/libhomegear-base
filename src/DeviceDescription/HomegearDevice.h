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

#ifndef HOMEGEARDEVICE_H_
#define HOMEGEARDEVICE_H_

#include "DevicePacket.h"
#include "SupportedDevice.h"
#include "RunProgram.h"
#include "Function.h"

using namespace rapidxml;

namespace BaseLib
{
namespace DeviceDescription
{

class HomegearDevice;

/**
 * Helper type for HomegearDevice pointers.
 */
typedef std::shared_ptr<HomegearDevice> PHomegearDevice;

/**
 * Class defining a Homegear device. It is a direct representation of the device description XML file.
 */
class HomegearDevice
{
public:
	struct ReceiveModes
	{
		enum Enum { none = 0, always = 1, wakeOnRadio = 2, config = 4, wakeUp = 8, lazyConfig = 16, wakeUp2 = 32 };
	};

	HomegearDevice(BaseLib::SharedObjects* baseLib);
	HomegearDevice(BaseLib::SharedObjects* baseLib, xml_node<>* node);
	HomegearDevice(BaseLib::SharedObjects* baseLib, std::string xmlFilename, bool& oldFormat);
	HomegearDevice(BaseLib::SharedObjects* baseLib, std::string xmlFilename, std::vector<char>& xml);
	virtual ~HomegearDevice();

	bool loaded() { return _loaded; }

	// {{{ Shortcuts
	int32_t dynamicChannelCountIndex = -1;
	double dynamicChannelCountSize = 1;
	// }}}

	// {{{ Attributes
	int32_t version = 0;
	// }}}

	// {{{ Properties
	ReceiveModes::Enum receiveModes = ReceiveModes::none;
	bool encryption = false;
	uint32_t timeout = 0;
	uint32_t memorySize = 0;
    uint32_t memorySize2 = 0;
	bool visible = true;
	bool deletable = true;
	bool internal = false;
	bool needsTime = false;
	bool hasBattery = false;
	uint32_t addressSize = 0;
	std::string pairingMethod;
	std::string interface;
	// }}}

	// {{{ Elements
	SupportedDevices supportedDevices;
	PRunProgram runProgram;
	Functions functions;
	PacketsByMessageType packetsByMessageType;
	PacketsById packetsById;
	PacketsByFunction packetsByFunction1;
	PacketsByFunction packetsByFunction2;
	ValueRequestPackets valueRequestPackets;
	PHomegearDevice group;
	// }}}

	// {{{ Helpers
	void setPath(std::string& value);
	std::string getPath();
    void setFilename(std::string& value);
    std::string getFilename();
	int32_t getDynamicChannelCount();
	void setDynamicChannelCount(int32_t value);
	PSupportedDevice getType(uint32_t typeNumber);
	PSupportedDevice getType(uint32_t typeNumber, int32_t firmwareVersion);
	void save(std::string& filename);
	// }}}
protected:
	BaseLib::SharedObjects* _bl = nullptr;
	bool _loaded = false;

	// {{{ Helpers
	std::string _path;
    std::string _filename;
	int32_t _dynamicChannelCount = -1;
	// }}}

	void load(std::string xmlFilename, bool& oldFormat);
	void load(std::string xmlFilename, std::vector<char>& xml);
	void postProcessFunction(PFunction& function, std::map<std::string, PConfigParameters>& configParameters, std::map<std::string, PVariables>& variables, std::map<std::string, PLinkParameters>& linkParameters);
	void parseXML(xml_node<>* node);
	void postLoad();

	// {{{ Helpers
	void saveDevice(xml_document<>* doc, xml_node<>* parentNode, HomegearDevice* device);
	void saveFunction(xml_document<>* doc, xml_node<>* parentNode, PFunction& function, std::map<std::string, PConfigParameters>& configParameters, std::map<std::string, PVariables>& variables, std::map<std::string, PLinkParameters>& linkParameters);
	void saveParameter(xml_document<>* doc, xml_node<>* parentNode, PParameter& parameter);
	void saveScenario(xml_document<>* doc, xml_node<>* parentNode, PScenario& scenario);
	void saveParameterPacket(xml_document<>* doc, xml_node<>* parentNode, std::shared_ptr<Parameter::Packet>& packet);
	// }}}
};
}
}

#endif
