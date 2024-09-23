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

#include "Function.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace DeviceDescription
{

Function::Function(BaseLib::SharedObjects* baseLib)
{
	_bl = baseLib;
	configParameters.reset(new ConfigParameters(baseLib));
	linkParameters.reset(new LinkParameters(baseLib));
	variables.reset(new Variables(baseLib));
}

Function::Function(BaseLib::SharedObjects* baseLib, xml_node* node, uint32_t& channelIndex) : Function(baseLib)
{
	for(xml_attribute* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		std::string attributeName(attr->name());
		std::string attributeValue(attr->value());
		if(attributeName == "channel")
		{
			channel = Math::getNumber(attributeValue);
			channelIndex = channel;
		}
		else if(attributeName == "type") type = attributeValue;
		else if(attributeName == "channelCount") channelCount = Math::getNumber(attributeValue);
		else if(attributeName == "xmlns") {}
		else _bl->out.printWarning("Warning: Unknown attribute for \"function\": " + attributeName);
	}
	for(xml_node* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
	{
		std::string nodeName(subNode->name());
		if(nodeName == "properties")
		{
			for(xml_node* propertyNode = subNode->first_node(); propertyNode; propertyNode = propertyNode->next_sibling())
			{
				std::string propertyName(propertyNode->name());
				std::string propertyValue(propertyNode->value());
				if(propertyName == "encryptionEnabledByDefault") { if(propertyValue == "true") encryptionEnabledByDefault = true; }
				else if(propertyName == "visible") { if(propertyValue == "false") visible = false; }
				else if(propertyName == "internal") { if(propertyValue == "true") internal = true; }
				else if(propertyName == "deletable") { if(propertyValue == "true") deletable = true; }
				else if(propertyName == "dynamicChannelCount")
				{
					std::pair<std::string, std::string> splitValue = HelperFunctions::splitFirst(propertyValue, ':');
					if(!splitValue.first.empty()) dynamicChannelCountIndex = Math::getUnsignedNumber(splitValue.first);
					if(!splitValue.second.empty()) dynamicChannelCountSize = Math::getDouble(splitValue.second);
				}
				else if(propertyName == "countFromVariable") countFromVariable = propertyValue;
				else if(propertyName == "physicalChannelIndexOffset") physicalChannelIndexOffset = Math::getNumber(propertyValue);
				else if(propertyName == "grouped") { if(propertyValue == "true") grouped = true; }
				else if(propertyName == "direction")
				{
					if(propertyValue == "sender") direction = Direction::Enum::sender;
					else if(propertyValue == "receiver") direction = Direction::Enum::receiver;
					else _bl->out.printWarning("Warning: Unknown direction for \"function\": " + propertyValue);
				}
				else if(propertyName == "forceEncryption") { if(propertyValue == "true") forceEncryption = true; }
				else if(propertyName == "defaultLinkScenarioElementId") defaultLinkScenarioElementId = propertyValue;
				else if(propertyName == "defaultGroupedLinkScenarioElementId1") defaultGroupedLinkScenarioElementId1 = propertyValue;
				else if(propertyName == "defaultGroupedLinkScenarioElementId2") defaultGroupedLinkScenarioElementId2 = propertyValue;
				else if(propertyName == "hasGroup") { if(propertyValue == "true") hasGroup = true; }
				else if(propertyName == "groupId") groupId = propertyValue;
				else if(propertyName == "linkSenderFunctionTypes")
				{
                    for(xml_attribute* attr = propertyNode->first_attribute(); attr; attr = attr->next_attribute())
                    {
                        linkSenderAttributes.emplace(std::string(attr->name()), Rpc::JsonDecoder::decode(std::string(attr->value())));
                    }
					for(xml_node* functionTypeNode = propertyNode->first_node("type"); functionTypeNode; functionTypeNode = functionTypeNode->next_sibling())
					{
						std::string functionTypeValue(functionTypeNode->value());
						if(functionTypeValue.empty()) continue;
						linkSenderFunctionTypes.insert(functionTypeValue);
					}
				}
				else if(propertyName == "linkReceiverFunctionTypes")
				{
                    for(xml_attribute* attr = propertyNode->first_attribute(); attr; attr = attr->next_attribute())
                    {
                        linkReceiverAttributes.emplace(std::string(attr->name()), Rpc::JsonDecoder::decode(std::string(attr->value())));
                    }
					for(xml_node* functionTypeNode = propertyNode->first_node("type"); functionTypeNode; functionTypeNode = functionTypeNode->next_sibling())
					{
						std::string functionTypeValue(functionTypeNode->value());
						if(functionTypeValue.empty()) continue;
						linkReceiverFunctionTypes.insert(functionTypeValue);
					}
				}
				else _bl->out.printWarning("Warning: Unknown function property: " + propertyName);
			}
		}
		else if(nodeName == "configParameters") configParametersId = std::string(subNode->value());
		else if(nodeName == "variables") variablesId = std::string(subNode->value());
		else if(nodeName == "linkParameters") linkParametersId = std::string(subNode->value());
		else if(nodeName == "alternativeFunction")
		{
			uint32_t channel = 0;
			alternativeFunctions.push_back(PFunction(new Function(baseLib, subNode, channel)));
		}
		else _bl->out.printWarning("Warning: Unknown node in \"function\": " + nodeName);
	}
}

bool Function::parameterSetDefined(ParameterGroup::Type::Enum type)
{
	return (type == ParameterGroup::Type::config && !configParameters->parameters.empty()) || (type == ParameterGroup::Type::variables && !variables->parameters.empty()) || (type == ParameterGroup::Type::link && !linkParameters->parameters.empty());
}

PParameterGroup Function::getParameterGroup(ParameterGroup::Type::Enum type)
{
	if(type == ParameterGroup::Type::Enum::variables) return variables;
	else if(type == ParameterGroup::Type::Enum::config) return configParameters;
	else if(type == ParameterGroup::Type::Enum::link) return linkParameters;
	return PParameterGroup();
}

}
}
