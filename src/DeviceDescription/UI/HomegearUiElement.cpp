/* Copyright 2013-2017 Sathya Laufer
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

#include "HomegearUiElement.h"
#include "../../BaseLib.h"

namespace BaseLib
{
namespace DeviceDescription
{

HomegearUiElement::HomegearUiElement(BaseLib::SharedObjects* baseLib)
{
    _bl = baseLib;
}

HomegearUiElement::HomegearUiElement(BaseLib::SharedObjects* baseLib, xml_node<>* node) : HomegearUiElement(baseLib)
{
    for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
    {
        std::string nodeName(subNode->name());
        std::string value(subNode->value());
        if(nodeName == "id") id = value;
        else if(nodeName == "type")
        {
            if(value == "simple") type = Type::simple;
            else if(value == "complex") type = Type::complex;
            else _bl->out.printWarning("Warning: Unknown value for homegearUiElement\\type: " + value);
        }
        else if(nodeName == "control") control = value;
        else if(nodeName == "unit") unit = value;
        else if(nodeName == "icon") icon = value;
        else if(nodeName == "texts")
        {
            for(xml_node<>* textsNode = subNode->first_node(); textsNode; textsNode = textsNode->next_sibling())
            {
                std::string textNodeName(textsNode->name());
                if(textNodeName == "text") texts.push_back(std::string(textsNode->value()));
                else _bl->out.printWarning("Warning: Unknown subnode for \"homegearUiElement\\texts\": " + textNodeName);
            }
        }
        else if(nodeName == "variableInputs")
        {
            for(xml_node<>* variableNode = subNode->first_node("variable"); variableNode; variableNode = variableNode->next_sibling("variable"))
            {
                variableInputs.push_back(std::make_shared<UiVariable>(baseLib, variableNode));
            }
        }
        else if(nodeName == "variableOutputs")
        {
            for(xml_node<>* variableNode = subNode->first_node("variable"); variableNode; variableNode = variableNode->next_sibling("variable"))
            {
                variableOutputs.push_back(std::make_shared<UiVariable>(baseLib, variableNode));
            }
        }
        else if(nodeName == "metadata")
        {
            for(xml_node<>* metadataNode = subNode->first_node(); metadataNode; metadataNode = metadataNode->next_sibling())
            {
                std::string metadataNodeName(metadataNode->name());
                metadata.emplace(metadataNodeName, std::string(metadataNode->value()));
            }
        }
        else if(nodeName == "width") width = Math::getNumber(value);
        else if(nodeName == "height") height = Math::getNumber(value);
        else if(nodeName == "cols") cols = Math::getNumber(value);
        else if(nodeName == "rows") rows = Math::getNumber(value);
        else if(nodeName == "controls")
        {
            for(xml_node<>* controlNode = subNode->first_node("control"); controlNode; controlNode = controlNode->next_sibling("control"))
            {
                controls.push_back(std::make_shared<UiControl>(baseLib, controlNode));
            }
        }
        else _bl->out.printWarning("Warning: Unknown node in \"homegearUiElement\": " + nodeName);
    }
}

HomegearUiElement::HomegearUiElement(HomegearUiElement const& rhs)
{
    _bl = rhs._bl;

    id = rhs.id;
    type = rhs.type;
    control = rhs.control;
    unit = rhs.unit;
    icon = rhs.icon;
    texts = rhs.texts;
    variableInputs.clear();
    variableOutputs.clear();

    for(auto& variableInput : rhs.variableInputs)
    {
        auto uiVariable = std::make_shared<UiVariable>(_bl);
        *uiVariable = *variableInput;
        variableInputs.emplace_back(uiVariable);
    }

    for(auto& variableOutput : rhs.variableOutputs)
    {
        auto uiVariable = std::make_shared<UiVariable>(_bl);
        *uiVariable = *variableOutput;
        variableOutputs.emplace_back(uiVariable);
    }

    metadata = rhs.metadata;
    width = rhs.width;
    height = rhs.height;
    cols = rhs.cols;
    rows = rhs.rows;

    for(auto& control : rhs.controls)
    {
        auto uiControl = std::make_shared<UiControl>(_bl);
        *uiControl = *control;
        controls.emplace_back(uiControl);
    }
}

HomegearUiElement& HomegearUiElement::operator=(const HomegearUiElement& rhs)
{
    if(&rhs == this) return *this;

    _bl = rhs._bl;

    id = rhs.id;
    type = rhs.type;
    control = rhs.control;
    unit = rhs.unit;
    icon = rhs.icon;
    texts = rhs.texts;
    variableInputs.clear();
    variableOutputs.clear();

    for(auto& variableInput : rhs.variableInputs)
    {
        auto uiVariable = std::make_shared<UiVariable>(_bl);
        *uiVariable = *variableInput;
        variableInputs.emplace_back(uiVariable);
    }

    for(auto& variableOutput : rhs.variableOutputs)
    {
        auto uiVariable = std::make_shared<UiVariable>(_bl);
        *uiVariable = *variableOutput;
        variableOutputs.emplace_back(uiVariable);
    }

    metadata = rhs.metadata;
    width = rhs.width;
    height = rhs.height;
    cols = rhs.cols;
    rows = rhs.rows;

    for(auto& control : rhs.controls)
    {
        auto uiControl = std::make_shared<UiControl>(_bl);
        *uiControl = *control;
        controls.emplace_back(uiControl);
    }

    return *this;
}

}
}
