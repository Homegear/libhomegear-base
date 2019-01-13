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

#include "UiVariable.h"
#include "../../BaseLib.h"

namespace BaseLib
{
namespace DeviceDescription
{

UiVariable::UiVariable(BaseLib::SharedObjects* baseLib)
{
    _bl = baseLib;
}

UiVariable::UiVariable(BaseLib::SharedObjects* baseLib, xml_node<>* node) : UiVariable(baseLib)
{
    for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
    {
        std::string nodeName(subNode->name());
        std::string nodeValue(subNode->value());
        if(nodeName == "family")
        {
            if(nodeValue != "*") familyId = Math::getNumber(nodeValue);
        }
        else if(nodeName == "deviceTypeId")
        {
            if(nodeValue != "*") deviceTypeId = Math::getNumber(nodeValue);
        }
        else if(nodeName == "channel")
        {
            if(nodeValue != "*") channel = Math::getNumber(nodeValue);
        }
        else if(nodeName == "name")
        {
            if(nodeValue != "*") name = nodeValue;
        }
        else if(nodeName == "visualizeInOverview") visualizeInOverview = (nodeValue == "true");
        else if(nodeName == "unit") unit = nodeValue;
        else if(nodeName == "minimumValue")
        {
            if(nodeValue.find('.') != std::string::npos) minimumValue = std::make_shared<BaseLib::Variable>(Math::getDouble(nodeValue));
            else minimumValue = std::make_shared<BaseLib::Variable>(Math::getNumber64(nodeValue));
        }
        else if(nodeName == "maximumValue")
        {
            if(nodeValue.find('.') != std::string::npos) maximumValue = std::make_shared<BaseLib::Variable>(Math::getDouble(nodeValue));
            else maximumValue = std::make_shared<BaseLib::Variable>(Math::getNumber64(nodeValue));
        }
        else if(nodeName == "minimumValueScaled")
        {
            if(nodeValue.find('.') != std::string::npos) minimumValueScaled = std::make_shared<BaseLib::Variable>(Math::getDouble(nodeValue));
            else minimumValueScaled = std::make_shared<BaseLib::Variable>(Math::getNumber64(nodeValue));
        }
        else if(nodeName == "maximumValueScaled")
        {
            if(nodeValue.find('.') != std::string::npos) maximumValueScaled = std::make_shared<BaseLib::Variable>(Math::getDouble(nodeValue));
            else maximumValueScaled = std::make_shared<BaseLib::Variable>(Math::getNumber64(nodeValue));
        }
        else if(nodeName == "rendering")
        {
            for(xml_node<>* conditionNode = subNode->first_node("condition"); conditionNode; conditionNode = conditionNode->next_sibling("condition"))
            {
                rendering.emplace_back(std::make_shared<UiCondition>(baseLib, conditionNode));
            }
        }
        else _bl->out.printWarning("Warning: Unknown node in \"UiVariable\": " + nodeName);
    }
}

UiVariable::UiVariable(UiVariable const& rhs)
{
    _bl = rhs._bl;

    familyId = rhs.familyId;
    deviceTypeId = rhs.deviceTypeId;
    channel = rhs.channel;
    name = rhs.name;
    visualizeInOverview = rhs.visualizeInOverview;
    unit = rhs.unit;
    if(rhs.minimumValue)
    {
        minimumValue = std::make_shared<Variable>();
        *minimumValue = *rhs.minimumValue;
    }
    if(rhs.maximumValue)
    {
        maximumValue = std::make_shared<Variable>();
        *maximumValue = *rhs.maximumValue;
    }
    if(rhs.minimumValueScaled)
    {
        minimumValueScaled = std::make_shared<Variable>();
        *minimumValueScaled = *rhs.minimumValueScaled;
    }
    if(rhs.maximumValueScaled)
    {
        maximumValueScaled = std::make_shared<Variable>();
        *maximumValueScaled = *rhs.maximumValueScaled;
    }
    peerId = rhs.peerId;

    for(auto& rhsCondition : rhs.rendering)
    {
        auto condition = std::make_shared<UiCondition>(_bl);
        *condition = *rhsCondition;
        rendering.emplace_back(condition);
    }
}

UiVariable& UiVariable::operator=(const UiVariable& rhs)
{
    if(&rhs == this) return *this;

    _bl = rhs._bl;

    familyId = rhs.familyId;
    deviceTypeId = rhs.deviceTypeId;
    channel = rhs.channel;
    name = rhs.name;
    visualizeInOverview = rhs.visualizeInOverview;
    unit = rhs.unit;
    if(rhs.minimumValue)
    {
        minimumValue = std::make_shared<Variable>();
        *minimumValue = *rhs.minimumValue;
    }
    if(rhs.maximumValue)
    {
        maximumValue = std::make_shared<Variable>();
        *maximumValue = *rhs.maximumValue;
    }
    if(rhs.minimumValueScaled)
    {
        minimumValueScaled = std::make_shared<Variable>();
        *minimumValueScaled = *rhs.minimumValueScaled;
    }
    if(rhs.maximumValueScaled)
    {
        maximumValueScaled = std::make_shared<Variable>();
        *maximumValueScaled = *rhs.maximumValueScaled;
    }
    peerId = rhs.peerId;

    for(auto& rhsCondition : rhs.rendering)
    {
        auto condition = std::make_shared<UiCondition>(_bl);
        *condition = *rhsCondition;
        rendering.emplace_back(condition);
    }

    return *this;
}

}
}
