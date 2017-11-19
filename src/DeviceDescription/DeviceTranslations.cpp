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

#include "DeviceTranslations.h"
#include "../BaseLib.h"

namespace BaseLib
{

namespace DeviceDescription
{

DeviceTranslations::DeviceTranslations(BaseLib::SharedObjects* baseLib, int32_t family)
{
	_bl = baseLib;
	_family = family;
}

void DeviceTranslations::clear()
{
	_deviceTranslations.clear();
}

std::shared_ptr<HomegearDeviceTranslation> DeviceTranslations::load(std::string& filename, std::string& language)
{
	try
	{
        std::string filepath = _bl->settings.deviceDescriptionPath() + std::to_string((int32_t)_family) + "/l10n/" + language + '/' + filename;
		if(!Io::fileExists(filepath))
		{
            filepath = _bl->settings.deviceDescriptionPath() + std::to_string((int32_t)_family) + "/l10n/en-US/" + filename;
            if(!Io::fileExists(filepath))
            {
                _bl->out.printDebug("Not loading XML RPC device translation " + filepath + ": Translation not found.");
                return PHomegearDeviceTranslation();
            }
		}
		if(filepath.size() < 5) return PHomegearDeviceTranslation();
		std::string extension = filepath.substr(filepath.size() - 4, 4);
		HelperFunctions::toLower(extension);
		if(extension != ".xml") return PHomegearDeviceTranslation();
		if(_bl->debugLevel >= 5) _bl->out.printDebug("Loading XML RPC device translation " + filepath);
		PHomegearDeviceTranslation device = std::make_shared<HomegearDeviceTranslation>(_bl, filepath);
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
    return PHomegearDeviceTranslation();
}

PHomegearDeviceTranslation DeviceTranslations::getTranslation(std::string& filename, std::string& language)
{
    try
    {
        if(language.empty()) language = "en-US";
        std::lock_guard<std::mutex> deviceTranslationsGuard(_deviceTranslationsMutex);
        auto languageIterator = _deviceTranslations.find(language);
        if(languageIterator == _deviceTranslations.end())
        {
            auto translation = load(filename, language);
            if (!translation) return PHomegearDeviceTranslation();
            _deviceTranslations[language].emplace(filename, translation);
        }
        else
        {
            auto translationIterator = languageIterator->second.find(filename);
            if (translationIterator == languageIterator->second.end())
            {
                auto translation = load(filename, language);
                if (!translation) return PHomegearDeviceTranslation();
                languageIterator->second.emplace(filename, translation);
                return translation;
            }
            else return translationIterator->second;
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
    return PHomegearDeviceTranslation();
}

std::string DeviceTranslations::getTypeDescription(std::string& filename, std::string& language, std::string& deviceId)
{
    try
    {
        PHomegearDeviceTranslation translation = getTranslation(filename, language);
        if(!translation) return "";

        auto idIterator = translation->typeDescriptions.find(deviceId);
        if(idIterator != translation->typeDescriptions.end()) return idIterator->second;
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
    return "";
}

std::string DeviceTranslations::getTypeLongDescription(std::string& filename, std::string& language, std::string& deviceId)
{
    try
    {
        PHomegearDeviceTranslation translation = getTranslation(filename, language);
        if(!translation) return "";

        auto idIterator = translation->typeLongDescriptions.find(deviceId);
        if(idIterator != translation->typeLongDescriptions.end()) return idIterator->second;
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
    return "";
}

std::pair<std::string, std::string> DeviceTranslations::getParameterTranslations(std::string& filename, std::string& language, ParameterGroup::Type::Enum parameterGroupType, std::string& parameterGroupId, std::string& parameterId)
{
    try
    {
        if(language.empty()) language = "en-US";
        PHomegearDeviceTranslation translation = getTranslation(filename, language);
        if(!translation) return std::make_pair("", "");

        std::unordered_map<std::string, std::unordered_map<std::string, HomegearDeviceTranslation::ParameterTranslation>>::iterator parameterGroupIterator;

        if(parameterGroupType == ParameterGroup::Type::Enum::config)
        {
            parameterGroupIterator = translation->configParameters.find(parameterGroupId);
            if(parameterGroupIterator == translation->configParameters.end()) return std::make_pair("", "");
        }
        else if(parameterGroupType == ParameterGroup::Type::Enum::variables)
        {
            parameterGroupIterator = translation->variables.find(parameterGroupId);
            if(parameterGroupIterator == translation->variables.end()) return std::make_pair("", "");
        }
        else if(parameterGroupType == ParameterGroup::Type::Enum::link)
        {
            parameterGroupIterator = translation->linkParameters.find(parameterGroupId);
            if(parameterGroupIterator == translation->linkParameters.end()) return std::make_pair("", "");
        }

        auto parameterIterator = parameterGroupIterator->second.find(parameterId);
        if(parameterIterator == parameterGroupIterator->second.end()) return std::make_pair("", "");

        return std::make_pair(parameterIterator->second.label, parameterIterator->second.description);
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
    return std::make_pair("", "");
}

}
}
