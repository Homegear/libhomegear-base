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
        if(nodeName == "id" || nodeName == "uniqueUiElementId") id = value;
        else if(nodeName == "type")
        {
            if(value == "simple") type = Type::simple;
            else if(value == "complex") type = Type::complex;
            else _bl->out.printWarning("Warning: Unknown value for homegearUiElement\\type: " + value);
        }
        else if(nodeName == "control") control = value;
        else if(nodeName == "role") role = Math::getUnsignedNumber64(value, true);
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
        else if(nodeName == "grid")
        {
            grid = std::make_shared<UiGrid>(baseLib, subNode);
        }
        else if(nodeName == "controls")
        {
            for(xml_node<>* controlNode = subNode->first_node("control"); controlNode; controlNode = controlNode->next_sibling("control"))
            {
                controls.push_back(std::make_shared<UiControl>(baseLib, controlNode));
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
        else _bl->out.printWarning("Warning: Unknown node in \"homegearUiElement\": " + nodeName);
    }
}

HomegearUiElement::HomegearUiElement(HomegearUiElement const& rhs)
{
    _bl = rhs._bl;

    id = rhs.id;
    type = rhs.type;
    control = rhs.control;
    role = rhs.role;

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

    for(auto& control : rhs.controls)
    {
        auto uiControl = std::make_shared<UiControl>(_bl);
        *uiControl = *control;
        controls.emplace_back(uiControl);
    }

    if(rhs.grid)
    {
        grid = std::make_shared<UiGrid>(_bl);
        *grid = *rhs.grid;
    }

    metadata = rhs.metadata;
}

HomegearUiElement& HomegearUiElement::operator=(const HomegearUiElement& rhs)
{
    if(&rhs == this) return *this;

    _bl = rhs._bl;

    id = rhs.id;
    type = rhs.type;
    control = rhs.control;
    role = rhs.role;

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

    for(auto& control : rhs.controls)
    {
        auto uiControl = std::make_shared<UiControl>(_bl);
        *uiControl = *control;
        controls.emplace_back(uiControl);
    }

    if(rhs.grid)
    {
        grid = std::make_shared<UiGrid>(_bl);
        *grid = *rhs.grid;
    }

    metadata = rhs.metadata;

    return *this;
}

PVariable HomegearUiElement::getElementInfo()
{
    auto uiElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
    uiElement->structValue->emplace("uniqueUiElementId", std::make_shared<BaseLib::Variable>(id));
    uiElement->structValue->emplace("type", std::make_shared<BaseLib::Variable>(type == Type::complex ? "complex" : "simple"));

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
        uiElement->structValue->emplace("role", std::make_shared<BaseLib::Variable>(role));

        auto inputs = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
        inputs->arrayValue->reserve(variableInputs.size());
        for(auto& variableInput : variableInputs)
        {
            auto input = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            if(variableInput->familyId != -1) input->structValue->emplace("family", std::make_shared<BaseLib::Variable>(variableInput->familyId));
            if(variableInput->deviceTypeId != -1) input->structValue->emplace("deviceTypeId", std::make_shared<BaseLib::Variable>(variableInput->deviceTypeId));
            input->structValue->emplace("peer", std::make_shared<BaseLib::Variable>(variableInput->peerId));
            input->structValue->emplace("channel", std::make_shared<BaseLib::Variable>(variableInput->channel));
            input->structValue->emplace("name", std::make_shared<BaseLib::Variable>(variableInput->name));

            auto variableProperties = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            variableProperties->structValue->emplace("visualizeInOverview", std::make_shared<BaseLib::Variable>(variableInput->visualizeInOverview));
            if(!variableInput->unit.empty()) variableProperties->structValue->emplace("unit", std::make_shared<BaseLib::Variable>(variableInput->unit));
            if(variableInput->minimumValue) variableProperties->structValue->emplace("minimum", variableInput->minimumValue);
            if(variableInput->maximumValue) variableProperties->structValue->emplace("maximum", variableInput->maximumValue);
            if(variableInput->minimumValueScaled) variableProperties->structValue->emplace("minimumScaled", variableInput->minimumValueScaled);
            if(variableInput->maximumValueScaled) variableProperties->structValue->emplace("maximumScaled", variableInput->maximumValueScaled);
            input->structValue->emplace("properties", variableProperties);

            {
                auto conditionElements = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
                conditionElements->arrayValue->reserve(variableInput->rendering.size());
                for(auto& condition : variableInput->rendering)
                {
                    auto conditionContainerElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

                    auto conditionElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
                    conditionElement->structValue->emplace("operator", std::make_shared<BaseLib::Variable>(condition->conditionOperator));
                    conditionElement->structValue->emplace("value", std::make_shared<BaseLib::Variable>(condition->conditionValue));
                    conditionContainerElement->structValue->emplace("condition", conditionElement);

                    auto definitionsElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
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
                        definitionsElement->structValue->emplace("icons", iconElements);
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
                        definitionsElement->structValue->emplace("texts", textsElement);
                    }
                    conditionContainerElement->structValue->emplace("definitions", definitionsElement);

                    conditionElements->arrayValue->emplace_back(conditionContainerElement);
                }
                input->structValue->emplace("rendering", conditionElements);
            }

            inputs->arrayValue->emplace_back(input);
        }
        uiElement->structValue->emplace("variableInputs", inputs);

        auto outputs = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
        outputs->arrayValue->reserve(variableOutputs.size());
        for(auto& variableOutput : variableOutputs)
        {
            auto output = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            if(variableOutput->familyId != -1) output->structValue->emplace("family", std::make_shared<BaseLib::Variable>(variableOutput->familyId));
            if(variableOutput->deviceTypeId != -1) output->structValue->emplace("deviceTypeId", std::make_shared<BaseLib::Variable>(variableOutput->deviceTypeId));
            output->structValue->emplace("peer", std::make_shared<BaseLib::Variable>(variableOutput->peerId));
            output->structValue->emplace("channel", std::make_shared<BaseLib::Variable>(variableOutput->channel));
            output->structValue->emplace("name", std::make_shared<BaseLib::Variable>(variableOutput->name));

            auto variableProperties = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            if(variableOutput->minimumValue) variableProperties->structValue->emplace("minimum", variableOutput->minimumValue);
            if(variableOutput->maximumValue) variableProperties->structValue->emplace("maximum", variableOutput->maximumValue);
            if(variableOutput->minimumValueScaled) variableProperties->structValue->emplace("minimumScaled", variableOutput->minimumValueScaled);
            if(variableOutput->maximumValueScaled) variableProperties->structValue->emplace("maximumScaled", variableOutput->maximumValueScaled);
            output->structValue->emplace("properties", variableProperties);

            outputs->arrayValue->emplace_back(output);
        }
        uiElement->structValue->emplace("variableOutputs", outputs);
    }
    else if(type == Type::complex)
    {
        if(grid)
        {
            auto gridElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            gridElement->structValue->emplace("width", std::make_shared<BaseLib::Variable>(grid->width));
            gridElement->structValue->emplace("height", std::make_shared<BaseLib::Variable>(grid->height));
            gridElement->structValue->emplace("columns", std::make_shared<BaseLib::Variable>(grid->columns));
            gridElement->structValue->emplace("rows", std::make_shared<BaseLib::Variable>(grid->rows));
            uiElement->structValue->emplace("grid", gridElement);
        }

        uiElement->structValue->emplace("role", std::make_shared<BaseLib::Variable>(role));

        auto controlElements = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
        controlElements->arrayValue->reserve(controls.size());
        for(auto& control : controls)
        {
            if(!control->uiElement) continue;
            auto controlElement = control->uiElement->getElementInfo();

            auto cellElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            cellElement->structValue->emplace("x", std::make_shared<BaseLib::Variable>(control->x));
            cellElement->structValue->emplace("y", std::make_shared<BaseLib::Variable>(control->y));
            cellElement->structValue->emplace("columns", std::make_shared<BaseLib::Variable>(control->columns));
            cellElement->structValue->emplace("rows", std::make_shared<BaseLib::Variable>(control->rows));

            controlElement->structValue->emplace("cell", cellElement);

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
