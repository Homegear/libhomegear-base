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

#ifndef DEVICEFUNCTION_H_
#define DEVICEFUNCTION_H_

#include "ParameterGroup.h"
#include <string>
#include <set>

namespace BaseLib
{
namespace DeviceDescription
{

class Function;

typedef std::set<std::string> LinkFunctionTypes;
typedef std::shared_ptr<Function> PFunction;
typedef std::map<uint32_t, PFunction> Functions;

/**
 * Class defining a device function or channel. One device can have multiple functions.
 */
class Function
{
public:
	struct Direction
	{
		enum Enum { none = 0, sender = 1, receiver = 2 };
	};

	Function(BaseLib::SharedObjects* baseLib);
	Function(BaseLib::SharedObjects* baseLib, xml_node* node, uint32_t& channel);
	virtual ~Function() {}

	//Attributes
	uint32_t channel = 0;
	std::string type;
	uint32_t channelCount = 1;

	//Properties
	bool encryptionEnabledByDefault = false;
	bool visible = true;
	bool deletable = false;
	bool internal = false;
	std::string countFromVariable;
	int32_t dynamicChannelCountIndex = -1;
	double dynamicChannelCountSize = 1;
	int32_t physicalChannelIndexOffset = 0;
	bool grouped = false;
	Direction::Enum direction = Direction::Enum::none;
	bool forceEncryption = false;
	std::string defaultLinkScenarioElementId;
	std::string defaultGroupedLinkScenarioElementId1;
	std::string defaultGroupedLinkScenarioElementId2;
	bool hasGroup = false;
	std::string groupId;
	std::unordered_map<std::string, BaseLib::PVariable> linkSenderAttributes;
    std::unordered_map<std::string, BaseLib::PVariable> linkReceiverAttributes;
	LinkFunctionTypes linkSenderFunctionTypes;
	LinkFunctionTypes linkReceiverFunctionTypes;

	//Elements
	std::string configParametersId;
	std::string variablesId;
	std::string linkParametersId;
	std::vector<PFunction> alternativeFunctions;

	//Helpers
	PParameter parameterGroupSelector;
	PConfigParameters configParameters;
	PVariables variables;
	PLinkParameters linkParameters;

	bool parameterSetDefined(ParameterGroup::Type::Enum type);
	PParameterGroup getParameterGroup(ParameterGroup::Type::Enum type);
protected:
	BaseLib::SharedObjects* _bl = nullptr;
};
}
}

#endif
