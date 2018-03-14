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
#include "../BaseLib.h"

namespace BaseLib
{

namespace DeviceDescription
{

UiElements::UiElements(BaseLib::SharedObjects* baseLib, int32_t family)
{
    _bl = baseLib;
    _family = family;
}

void UiElements::clear()
{
    _uiInfo.clear();
}

PHomegearUiElements UiElements::load(std::string& filename, std::string& language)
{
    try
    {
        std::string filepath = _bl->settings.deviceDescriptionPath() + std::to_string((int32_t)_family) + "/ui/" + language + '/' + filename;
        if(!Io::fileExists(filepath))
        {
            filepath = _bl->settings.deviceDescriptionPath() + std::to_string((int32_t)_family) + "/ui/en-US/" + filename;
            if(!Io::fileExists(filepath))
            {
                _bl->out.printDebug("Not loading UI info " + filepath + ": UI info not found.");
                return PHomegearUiElements();
            }
        }
        if(filepath.size() < 5) return PHomegearUiElements();
        std::string extension = filepath.substr(filepath.size() - 4, 4);
        HelperFunctions::toLower(extension);
        if(extension != ".xml") return PHomegearUiElements();
        if(_bl->debugLevel >= 5) _bl->out.printDebug("Loading UI info " + filepath);
        auto device = std::make_shared<HomegearUiElements>(_bl, filepath);
        if(device->loaded()) return device;
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
    return PHomegearUiElements();
}

PHomegearUiElements UiElements::getUiInfo(std::string& filename, std::string& language)
{
    try
    {
        if(language.empty()) language = "en-US";
        std::lock_guard<std::mutex> uiInfoGuard(_uiInfoMutex);
        auto languageIterator = _uiInfo.find(language);
        if(languageIterator == _uiInfo.end())
        {
            auto ui = load(filename, language);
            if (!ui) return PHomegearUiElements();
            _uiInfo[language].emplace(filename, ui);
        }
        else
        {
            auto uiIterator = languageIterator->second.find(filename);
            if (uiIterator == languageIterator->second.end())
            {
                auto ui = load(filename, language);
                if (!ui) return PHomegearUiElements();
                languageIterator->second.emplace(filename, ui);
                return ui;
            }
            else return uiIterator->second;
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
    return PHomegearUiElements();
}

}
}
