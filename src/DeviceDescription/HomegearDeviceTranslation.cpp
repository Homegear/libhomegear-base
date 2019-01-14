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

#include "HomegearDeviceTranslation.h"
#include "../BaseLib.h"
#include "../Encoding/RapidXml/rapidxml_print.hpp"

namespace BaseLib
{
namespace DeviceDescription
{
HomegearDeviceTranslation::HomegearDeviceTranslation(BaseLib::SharedObjects* baseLib, std::string xmlFilename)
{
	try
	{
		_bl = baseLib;
		load(xmlFilename);
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

HomegearDeviceTranslation::~HomegearDeviceTranslation() {

}

void HomegearDeviceTranslation::load(std::string xmlFilename)
{
	xml_document<> doc;
	try
	{
		std::ifstream fileStream(xmlFilename, std::ios::in | std::ios::binary);
		if(fileStream)
		{
			uint32_t length;
			fileStream.seekg(0, std::ios::end);
			length = fileStream.tellg();
			fileStream.seekg(0, std::ios::beg);
			std::vector<char> buffer(length + 1);
			fileStream.read(&buffer[0], length);
			fileStream.close();
			buffer[length] = '\0';
			doc.parse<parse_no_entity_translation | parse_validate_closing_tags>(&buffer[0]);
			if(!doc.first_node("homegearDeviceTranslation"))
			{
				_bl->out.printError("Error: Device translation XML file \"" + xmlFilename + "\" does not start with \"homegearDeviceTranslation\".");
				doc.clear();
				return;
			}
			parseXML(doc.first_node("homegearDeviceTranslation"));
		}
		else _bl->out.printError("Error reading file " + xmlFilename + ": " + strerror(errno));

		_loaded = true;
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printError("Error: Could not parse file \"" + xmlFilename + "\": " + ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    doc.clear();
}

void HomegearDeviceTranslation::parseXML(xml_node<>* node)
{
	try
	{
		for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
		{
			std::string attributeName(attr->name());
			std::string attributeValue(attr->value());
			if(attributeName == "lang") lang = attributeValue;
			else if(attributeName == "xmlns") {}
			else _bl->out.printWarning("Warning: Unknown attribute for \"homegearDeviceTranslation\": " + attributeName);
		}
		for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
		{
			std::string nodeName(subNode->name());
			if(nodeName == "supportedDevices")
			{
				for(xml_node<>* typeNode = subNode->first_node("device"); typeNode; typeNode = typeNode->next_sibling("device"))
				{
                    xml_attribute<>* idAttr = typeNode->first_attribute("id");
                    if(!idAttr) continue;
                    std::string deviceId(idAttr->value());

                    xml_node<>* descriptionNode = typeNode->first_node("description");
                    if(descriptionNode) typeDescriptions.emplace(deviceId, std::string(descriptionNode->value()));

                    descriptionNode = typeNode->first_node("longDescription");
                    if(descriptionNode) typeLongDescriptions.emplace(deviceId, std::string(descriptionNode->value()));
				}
			}
			else if(nodeName == "parameterGroups")
			{
				for(xml_node<>* parameterGroupNode = subNode->first_node(); parameterGroupNode; parameterGroupNode = parameterGroupNode->next_sibling())
				{
                    std::string parameterGroupName(parameterGroupNode->name());
                    if(parameterGroupName != "configParameters" && parameterGroupName != "variables" && parameterGroupName != "linkParameters") _bl->out.printWarning("Warning: Unknown parameter group: " + parameterGroupName);

                    xml_attribute<>* idAttr = parameterGroupNode->first_attribute("id");
                    if(!idAttr) continue;
                    std::string parameterGroupId(idAttr->value());

                    for(xml_node<>* parameterNode = parameterGroupNode->first_node(); parameterNode; parameterNode = parameterNode->next_sibling())
                    {
                        idAttr = parameterNode->first_attribute("id");
                        if(!idAttr) continue;
                        std::string parameterId(idAttr->value());

                        ParameterTranslation parameterTranslation;

                        xml_node<>* labelNode = parameterNode->first_node("label");
                        if(!labelNode) continue;
                        parameterTranslation.label = std::string(labelNode->value());

                        xml_node<>* descriptionNode = parameterNode->first_node("description");
                        if(!descriptionNode) continue;
                        parameterTranslation.description = std::string(descriptionNode->value());

                        if(parameterGroupName == "configParameters") configParameters[parameterGroupId].emplace(parameterId, std::move(parameterTranslation));
                        else if(parameterGroupName == "variables") variables[parameterGroupId].emplace(parameterId, std::move(parameterTranslation));
                        else if(parameterGroupName == "linkParameters") linkParameters[parameterGroupId].emplace(parameterId, std::move(parameterTranslation));
                    }
				}
			}
			else _bl->out.printWarning("Warning: Unknown node name for \"homegearDeviceTranslation\": " + nodeName);
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

}
}
