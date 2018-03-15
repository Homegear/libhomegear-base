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

#include "UiElements.h"
#include "../../BaseLib.h"

namespace BaseLib
{

namespace DeviceDescription
{

UiElements::UiElements(BaseLib::SharedObjects* baseLib)
{
    _bl = baseLib;
}

void UiElements::clear()
{
    std::lock_guard<std::mutex> uiInfoGuard(_uiInfoMutex);
    _uiInfo.clear();
}

void UiElements::load(std::string& language)
{
    try
    {
        std::lock_guard<std::mutex> uiInfoGuard(_uiInfoMutex);
        auto& uiInfo = _uiInfo[language];

        Io io;
        auto familyDirectories = io.getDirectories(_bl->settings.deviceDescriptionPath());

        for(auto& directory : familyDirectories)
        {
            std::string path;
            if(directory == "uiBase/") path = _bl->settings.deviceDescriptionPath() + directory + language + '/';
            else path = _bl->settings.deviceDescriptionPath() + directory + "ui/" + language + '/';
            if(!io.directoryExists(path))
            {
                path = _bl->settings.deviceDescriptionPath() + directory + "ui/en-US/";
                if(!io.directoryExists(path)) continue;
            }
            auto files = io.getFiles(path, false);
            for(auto& file : files)
            {
                std::string extension = file.substr(file.size() - 4, 4);
                HelperFunctions::toLower(extension);
                if(extension != ".xml") continue;
                if(_bl->debugLevel >= 5) _bl->out.printDebug("Loading UI info " + path + file);
                auto uiElements = std::make_shared<HomegearUiElements>(_bl, path + file);
                if(uiElements->loaded())
                {
                    auto elements = uiElements->getUiElements();
                    uiInfo.insert(elements.begin(), elements.end());
                }
            }
        }

        for(auto& uiElement : uiInfo)
        {
            if(uiElement.second->type == HomegearUiElement::Type::complex)
            {
                for(auto& control : uiElement.second->controls)
                {
                    auto elementIterator = uiInfo.find(control->id);
                    if(elementIterator != uiInfo.end())
                    {
                        if(elementIterator->second->type = HomegearUiElement::Type::complex)
                        {
                            _bl->out.printWarning("Warning: Only elements of type simple can be referenced in complex elements. Element \"" + uiElement.second->id + "\" is referencing complex element \"" + elementIterator->second->id + "\".");
                        }
                        else control->uiElement = elementIterator->second;
                    }
                }
            }
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

PHomegearUiElement UiElements::getUiElement(std::string& language, std::string& id)
{
    try
    {
        std::unique_lock<std::mutex> uiInfoGuard(_uiInfoMutex);
        auto uiInfoIterator = _uiInfo.find(language);
        if(uiInfoIterator == _uiInfo.end() || uiInfoIterator->second.empty())
        {
            uiInfoGuard.unlock();
            load(language);
            uiInfoGuard.lock();
        }

        auto uiElementIterator = _uiInfo[language].find(id);
        if(uiElementIterator != _uiInfo[language].end()) return uiElementIterator->second;
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
    return PHomegearUiElement();
}

PVariable UiElements::getUiElements(std::string& language)
{
    try
    {
        std::unique_lock<std::mutex> uiInfoGuard(_uiInfoMutex);
        auto uiInfoIterator = _uiInfo.find(language);
        if(uiInfoIterator == _uiInfo.end() || uiInfoIterator->second.empty())
        {
            uiInfoGuard.unlock();
            load(language);
            uiInfoGuard.lock();
        }

        auto& uiInfo = _uiInfo[language];
        auto uiElements = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);

        for(auto& element : uiInfo)
        {
            auto uiElement = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            uiElement->structValue->emplace("type", std::make_shared<BaseLib::Variable>((int32_t)element.second->type));
            uiElement->structValue->emplace("control", std::make_shared<BaseLib::Variable>(element.second->control));
            uiElement->structValue->emplace("unit", std::make_shared<BaseLib::Variable>(element.second->unit));
            uiElement->structValue->emplace("icon", std::make_shared<BaseLib::Variable>(element.second->icon));
            uiElement->structValue->emplace("width", std::make_shared<BaseLib::Variable>(element.second->width));
            uiElement->structValue->emplace("height", std::make_shared<BaseLib::Variable>(element.second->height));
            uiElement->structValue->emplace("cols", std::make_shared<BaseLib::Variable>(element.second->cols));
            uiElement->structValue->emplace("rows", std::make_shared<BaseLib::Variable>(element.second->rows));

            auto texts = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
            texts->arrayValue->reserve(element.second->texts.size());
            for(auto& text : element.second->texts)
            {
                texts->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(text));
            }
            uiElements->structValue->emplace("texts", texts);

            auto inputs = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
            inputs->arrayValue->reserve(element.second->variableInputs.size());
            for(auto& variableInput : element.second->variableInputs)
            {
                auto input = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
                input->structValue->emplace("familyId", std::make_shared<BaseLib::Variable>(variableInput->familyId));
                input->structValue->emplace("deviceTypeId", std::make_shared<BaseLib::Variable>(variableInput->deviceTypeId));
                input->structValue->emplace("channel", std::make_shared<BaseLib::Variable>(variableInput->channel));
                input->structValue->emplace("name", std::make_shared<BaseLib::Variable>(variableInput->name));

                inputs->arrayValue->emplace_back(input);
            }
            uiElements->structValue->emplace("variableInputs", inputs);

            auto outputs = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
            outputs->arrayValue->reserve(element.second->variableOutputs.size());
            for(auto& variableOutput : element.second->variableOutputs)
            {
                auto output = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
                output->structValue->emplace("familyId", std::make_shared<BaseLib::Variable>(variableOutput->familyId));
                output->structValue->emplace("deviceTypeId", std::make_shared<BaseLib::Variable>(variableOutput->deviceTypeId));
                output->structValue->emplace("channel", std::make_shared<BaseLib::Variable>(variableOutput->channel));
                output->structValue->emplace("name", std::make_shared<BaseLib::Variable>(variableOutput->name));

                inputs->arrayValue->emplace_back(output);
            }
            uiElements->structValue->emplace("variableOutputs", outputs);

            auto metadata = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
            metadata->arrayValue->reserve(element.second->metadata.size());
            for(auto& entry : element.second->metadata)
            {
                metadata->structValue->emplace(entry.first, std::make_shared<BaseLib::Variable>(entry.second));
            }
            uiElements->structValue->emplace("metadata", metadata);

            uiElements->structValue->emplace(element.first, uiElement);
        }

        return uiElements;
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
    return Variable::createError(-32500, "Unknown application error.");
}

}
}
