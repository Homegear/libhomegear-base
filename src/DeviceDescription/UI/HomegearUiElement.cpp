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
        else if(nodeName == "icons")
        {
            for(xml_node<>* iconNode = subNode->first_node("icon"); iconNode; iconNode = iconNode->next_sibling("icon"))
            {
                auto icon = std::make_shared<UiIcon>(baseLib, iconNode);
                if(!icon->id.empty()) icons.emplace(icon->id, std::move(icon));
            }
        }
        else if(nodeName == "texts")
        {
            for(xml_node<>* textNode = subNode->first_node("text"); textNode; textNode = textNode->next_sibling("text"))
            {
                auto text = std::make_shared<UiText>(baseLib, textNode);
                if(!text->id.empty()) texts.emplace(text->id, std::move(text));
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

    for(auto& icon : rhs.icons)
    {
        auto uiIcon = std::make_shared<UiIcon>(_bl);
        *uiIcon = *(icon.second);
        icons.emplace(uiIcon->id, std::move(uiIcon));
    }

    for(auto& text : rhs.texts)
    {
        auto uiText = std::make_shared<UiText>(_bl);
        *uiText = *(text.second);
        texts.emplace(uiText->id, std::move(uiText));
    }

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

    for(auto& icon : rhs.icons)
    {
        auto uiIcon = std::make_shared<UiIcon>(_bl);
        *uiIcon = *(icon.second);
        icons.emplace(uiIcon->id, std::move(uiIcon));
    }

    for(auto& text : rhs.texts)
    {
        auto uiText = std::make_shared<UiText>(_bl);
        *uiText = *(text.second);
        texts.emplace(uiText->id, std::move(uiText));
    }

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

PVariable HomegearUiElement::getElementInfo()
{
    auto uiElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
    uiElement->structValue->emplace("id", std::make_shared<BaseLib::Variable>(id));
    uiElement->structValue->emplace("type", std::make_shared<BaseLib::Variable>((int32_t)type));

    if(!icons.empty())
    {
        auto iconElements = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
        for(auto& icon : icons)
        {
            auto iconElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

            iconElement->structValue->emplace("name", std::make_shared<BaseLib::Variable>(icon.second->name));
            if(!icon.second->color.empty()) iconElement->structValue->emplace("color", std::make_shared<BaseLib::Variable>(icon.second->color));

            iconElements->structValue->emplace(icon.first, iconElement);
        }
        uiElement->structValue->emplace("icons", iconElements);
    }

    if(type == Type::simple)
    {
        uiElement->structValue->emplace("control", std::make_shared<BaseLib::Variable>(control));

        auto inputs = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
        inputs->arrayValue->reserve(variableInputs.size());
        for(auto& variableInput : variableInputs)
        {
            auto input = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            if(variableInput->familyId != -1) input->structValue->emplace("familyId", std::make_shared<BaseLib::Variable>(variableInput->familyId));
            if(variableInput->deviceTypeId != -1) input->structValue->emplace("deviceTypeId", std::make_shared<BaseLib::Variable>(variableInput->deviceTypeId));
            if(variableInput->peerId != 0) input->structValue->emplace("peerId", std::make_shared<BaseLib::Variable>(variableInput->peerId));
            if(variableInput->channel != -1) input->structValue->emplace("channel", std::make_shared<BaseLib::Variable>(variableInput->channel));
            input->structValue->emplace("name", std::make_shared<BaseLib::Variable>(variableInput->name));
            input->structValue->emplace("visualize", std::make_shared<BaseLib::Variable>(variableInput->visualize));
            if(!variableInput->unit.empty()) input->structValue->emplace("unit", std::make_shared<BaseLib::Variable>(variableInput->unit));
            if(variableInput->minimumValue) input->structValue->emplace("minimumValue", variableInput->minimumValue);
            if(variableInput->maximumValue) input->structValue->emplace("maximumValue", variableInput->maximumValue);
            if(variableInput->minimumValueScaled) input->structValue->emplace("minimumValueScaled", variableInput->minimumValueScaled);
            if(variableInput->maximumValueScaled) input->structValue->emplace("maximumValueScaled", variableInput->maximumValueScaled);

            {
                auto conditionElements = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
                conditionElements->arrayValue->reserve(variableInput->conditions.size());
                for(auto& condition : variableInput->conditions)
                {
                    auto conditionElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
                    conditionElement->structValue->emplace("operator", std::make_shared<BaseLib::Variable>(condition->conditionOperator));
                    conditionElement->structValue->emplace("value", std::make_shared<BaseLib::Variable>(condition->conditionValue));

                    if(!condition->icons.empty())
                    {
                        auto iconElements = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
                        for(auto& icon : condition->icons)
                        {
                            auto iconElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

                            iconElement->structValue->emplace("name", std::make_shared<BaseLib::Variable>(icon.second->name));
                            if(!icon.second->color.empty()) iconElement->structValue->emplace("color", std::make_shared<BaseLib::Variable>(icon.second->color));

                            iconElements->structValue->emplace(icon.first, iconElement);
                        }
                        conditionElement->structValue->emplace("icons", iconElements);
                    }

                    if(!condition->texts.empty())
                    {
                        auto textsElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
                        for(auto& text : condition->texts)
                        {
                            auto textElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

                            textElement->structValue->emplace("content", std::make_shared<BaseLib::Variable>(text.second->content));
                            if(!text.second->color.empty()) textElement->structValue->emplace("color", std::make_shared<BaseLib::Variable>(text.second->color));

                            textsElement->structValue->emplace(text.first, textElement);
                        }
                        conditionElement->structValue->emplace("texts", textsElement);
                    }

                    conditionElements->arrayValue->emplace_back(conditionElement);
                }
                input->structValue->emplace("conditions", conditionElements);
            }

            inputs->arrayValue->emplace_back(input);
        }
        uiElement->structValue->emplace("variableInputs", inputs);

        auto outputs = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
        outputs->arrayValue->reserve(variableOutputs.size());
        for(auto& variableOutput : variableOutputs)
        {
            auto output = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            if(variableOutput->familyId != -1) output->structValue->emplace("familyId", std::make_shared<BaseLib::Variable>(variableOutput->familyId));
            if(variableOutput->deviceTypeId != -1) output->structValue->emplace("deviceTypeId", std::make_shared<BaseLib::Variable>(variableOutput->deviceTypeId));
            if(variableOutput->peerId != 0) output->structValue->emplace("peerId", std::make_shared<BaseLib::Variable>(variableOutput->peerId));
            if(variableOutput->channel != -1) output->structValue->emplace("channel", std::make_shared<BaseLib::Variable>(variableOutput->channel));
            output->structValue->emplace("name", std::make_shared<BaseLib::Variable>(variableOutput->name));
            if(variableOutput->minimumValue) output->structValue->emplace("minimumValue", variableOutput->minimumValue);
            if(variableOutput->maximumValue) output->structValue->emplace("maximumValue", variableOutput->maximumValue);

            outputs->arrayValue->emplace_back(output);
        }
        uiElement->structValue->emplace("variableOutputs", outputs);
    }
    else if(type == Type::complex)
    {
        uiElement->structValue->emplace("width", std::make_shared<BaseLib::Variable>(width));
        uiElement->structValue->emplace("height", std::make_shared<BaseLib::Variable>(height));
        uiElement->structValue->emplace("cols", std::make_shared<BaseLib::Variable>(cols));
        uiElement->structValue->emplace("rows", std::make_shared<BaseLib::Variable>(rows));

        auto controlElements = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
        controlElements->arrayValue->reserve(controls.size());
        for(auto& control : controls)
        {
            if(!control->uiElement) continue;
            auto controlElement = control->uiElement->getElementInfo();
            controlElement->structValue->emplace("posX", std::make_shared<BaseLib::Variable>(control->posX));
            controlElement->structValue->emplace("posY", std::make_shared<BaseLib::Variable>(control->posY));
            controlElement->structValue->emplace("colWidth", std::make_shared<BaseLib::Variable>(control->colWidth));
            controlElements->arrayValue->emplace_back(controlElement);
        }
        uiElement->structValue->emplace("controls", controlElements);
    }

    if(!texts.empty())
    {
        auto textsElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
        for(auto& text : texts)
        {
            auto textElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

            textElement->structValue->emplace("content", std::make_shared<BaseLib::Variable>(text.second->content));
            if(!text.second->color.empty()) textElement->structValue->emplace("color", std::make_shared<BaseLib::Variable>(text.second->color));

            textsElement->structValue->emplace(text.first, textElement);
        }
        uiElement->structValue->emplace("texts", textsElement);
    }

    auto metadataElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
    for(auto& entry : metadata)
    {
        metadataElement->structValue->emplace(entry.first, std::make_shared<BaseLib::Variable>(entry.second));
    }
    uiElement->structValue->emplace("metadata", metadataElement);

    return uiElement;
}

}
}
