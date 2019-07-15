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

void UiElements::load(const std::string& language)
{
    try
    {
        std::lock_guard<std::mutex> uiInfoGuard(_uiInfoMutex);
        auto& uiInfo = _uiInfo[language];
        auto& uiInfoEnglish = _uiInfo["en-US"];

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
                        if(elementIterator->second->type == HomegearUiElement::Type::complex)
                        {
                            _bl->out.printWarning("Warning: Only elements of type simple can be referenced in complex elements. Element \"" + uiElement.second->id + "\" is referencing complex element \"" + elementIterator->second->id + "\".");
                        }
                        else control->uiElement = elementIterator->second;
                    }
                    else
                    {
                        elementIterator = uiInfoEnglish.find(control->id);
                        if(elementIterator != uiInfo.end())
                        {
                            if(elementIterator->second->type == HomegearUiElement::Type::complex)
                            {
                                _bl->out.printWarning("Warning: Only elements of type simple can be referenced in complex elements. Element \"" + uiElement.second->id + "\" is referencing complex element \"" + elementIterator->second->id + "\".");
                            }
                            else control->uiElement = elementIterator->second;
                        }
                    }
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

PHomegearUiElement UiElements::getUiElement(const std::string& language, const std::string& id)
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
    return PHomegearUiElement();
}

PHomegearUiElement UiElements::getUiElement(const std::string& language, const std::string& id, PUiPeerInfo peerInfo)
{
    try
    {
        auto uiElement = getUiElement(language, id);
        if(!uiElement) return uiElement;

        auto uiElementCopy = std::make_shared<HomegearUiElement>(_bl);
        *uiElementCopy = *uiElement;

        if(uiElementCopy->type == HomegearUiElement::Type::simple)
        {
            int32_t i = 0;
            for(auto& variableInput : uiElementCopy->variableInputs)
            {
                if(peerInfo->inputPeers.empty() || i >= (int32_t)peerInfo->inputPeers.at(0).size()) break;
                variableInput->peerId = peerInfo->inputPeers.at(0).at(i)->peerId;
                variableInput->channel = peerInfo->inputPeers.at(0).at(i)->channel;
                if(!peerInfo->inputPeers.at(0).at(i)->name.empty()) variableInput->name = peerInfo->inputPeers.at(0).at(i)->name;
                if(peerInfo->inputPeers.at(0).at(i)->minimumValue) variableInput->minimumValue = peerInfo->inputPeers.at(0).at(i)->minimumValue;
                if(peerInfo->inputPeers.at(0).at(i)->maximumValue) variableInput->maximumValue = peerInfo->inputPeers.at(0).at(i)->maximumValue;
                if(peerInfo->inputPeers.at(0).at(i)->minimumValueScaled) variableInput->minimumValueScaled = peerInfo->inputPeers.at(0).at(i)->minimumValueScaled;
                if(peerInfo->inputPeers.at(0).at(i)->maximumValueScaled) variableInput->maximumValueScaled = peerInfo->inputPeers.at(0).at(i)->maximumValueScaled;
                i++;
            }

            i = 0;
            for(auto& variableOutput : uiElementCopy->variableOutputs)
            {
                if(peerInfo->outputPeers.empty() || i >= (int32_t)peerInfo->outputPeers.at(0).size()) break;
                variableOutput->peerId = peerInfo->outputPeers.at(0).at(i)->peerId;
                variableOutput->channel = peerInfo->outputPeers.at(0).at(i)->channel;
                if(!peerInfo->outputPeers.at(0).at(i)->name.empty()) variableOutput->name = peerInfo->outputPeers.at(0).at(i)->name;
                if(peerInfo->outputPeers.at(0).at(i)->value) variableOutput->value = peerInfo->outputPeers.at(0).at(i)->value;
                if(peerInfo->outputPeers.at(0).at(i)->minimumValue) variableOutput->minimumValue = peerInfo->outputPeers.at(0).at(i)->minimumValue;
                if(peerInfo->outputPeers.at(0).at(i)->maximumValue) variableOutput->maximumValue = peerInfo->outputPeers.at(0).at(i)->maximumValue;
                if(peerInfo->outputPeers.at(0).at(i)->minimumValueScaled) variableOutput->minimumValueScaled = peerInfo->outputPeers.at(0).at(i)->minimumValueScaled;
                if(peerInfo->outputPeers.at(0).at(i)->maximumValueScaled) variableOutput->maximumValueScaled = peerInfo->outputPeers.at(0).at(i)->maximumValueScaled;
                i++;
            }
        }
        else if(uiElementCopy->type == HomegearUiElement::Type::complex)
        {
            int32_t i = 0;
            int32_t j = 0;
            for(auto& control : uiElementCopy->controls)
            {
                if(!control->uiElement) continue;

                if(i < (int32_t)peerInfo->inputPeers.size())
                {
                    j = 0;
                    for(auto& variableInput : control->uiElement->variableInputs)
                    {
                        if(j >= (int32_t)peerInfo->inputPeers.at(i).size()) break;
                        variableInput->peerId = peerInfo->inputPeers.at(i).at(j)->peerId;
                        variableInput->channel = peerInfo->inputPeers.at(i).at(j)->channel;
                        if(!peerInfo->inputPeers.at(i).at(j)->name.empty()) variableInput->name = peerInfo->inputPeers.at(i).at(j)->name;
                        if(peerInfo->inputPeers.at(i).at(j)->minimumValue) variableInput->minimumValue = peerInfo->inputPeers.at(i).at(j)->minimumValue;
                        if(peerInfo->inputPeers.at(i).at(j)->maximumValue) variableInput->maximumValue = peerInfo->inputPeers.at(i).at(j)->maximumValue;
                        if(peerInfo->inputPeers.at(i).at(j)->minimumValueScaled) variableInput->minimumValueScaled = peerInfo->inputPeers.at(i).at(j)->minimumValueScaled;
                        if(peerInfo->inputPeers.at(i).at(j)->maximumValueScaled) variableInput->maximumValueScaled = peerInfo->inputPeers.at(i).at(j)->maximumValueScaled;
                        j++;
                    }
                }

                if(i < (int32_t)peerInfo->outputPeers.size())
                {
                    j = 0;
                    for(auto& variableOutput : control->uiElement->variableOutputs)
                    {
                        if(j >= (int32_t)peerInfo->outputPeers.at(i).size()) break;
                        variableOutput->peerId = peerInfo->outputPeers.at(i).at(j)->peerId;
                        variableOutput->channel = peerInfo->outputPeers.at(i).at(j)->channel;
                        if(!peerInfo->outputPeers.at(i).at(j)->name.empty()) variableOutput->name = peerInfo->outputPeers.at(i).at(j)->name;
                        if(peerInfo->outputPeers.at(i).at(j)->value) variableOutput->value = peerInfo->outputPeers.at(i).at(j)->value;
                        if(peerInfo->outputPeers.at(i).at(j)->minimumValue) variableOutput->minimumValue = peerInfo->outputPeers.at(i).at(j)->minimumValue;
                        if(peerInfo->outputPeers.at(i).at(j)->maximumValue) variableOutput->maximumValue = peerInfo->outputPeers.at(i).at(j)->maximumValue;
                        if(peerInfo->outputPeers.at(i).at(j)->minimumValueScaled) variableOutput->minimumValueScaled = peerInfo->outputPeers.at(i).at(j)->minimumValueScaled;
                        if(peerInfo->outputPeers.at(i).at(j)->maximumValueScaled) variableOutput->maximumValueScaled = peerInfo->outputPeers.at(i).at(j)->maximumValueScaled;
                        j++;
                    }
                }

                i++;
            }
        }

        return uiElementCopy;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return PHomegearUiElement();
}

PVariable UiElements::getUiElements(const std::string& language)
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
            uiElements->structValue->emplace(element.first, element.second->getElementInfo());
        }

        return uiElements;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Variable::createError(-32500, "Unknown application error.");
}

}
}
