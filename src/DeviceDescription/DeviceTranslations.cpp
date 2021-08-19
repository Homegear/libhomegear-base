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

#include "DeviceTranslations.h"
#include "../BaseLib.h"

namespace BaseLib {

namespace DeviceDescription {

DeviceTranslations::DeviceTranslations(BaseLib::SharedObjects *baseLib, int32_t family) {
  _bl = baseLib;
  _family = family;
}

void DeviceTranslations::clear() {
  _deviceTranslations.clear();
}

std::shared_ptr<HomegearDeviceTranslation> DeviceTranslations::load(const std::string &filename, const std::string &language) {
  try {
    std::string filepath = _bl->settings.deviceDescriptionPath() + std::to_string((int32_t)_family) + "/l10n/" + language + '/' + filename;
    if (!Io::fileExists(filepath)) {
      filepath = _bl->settings.deviceDescriptionPath() + std::to_string((int32_t)_family) + "/l10n/en/" + filename;
      if (!Io::fileExists(filepath)) {
        _bl->out.printDebug("Not loading XML RPC device translation " + filepath + ": Translation not found.");
        return PHomegearDeviceTranslation();
      }
    }
    if (filepath.size() < 5) return PHomegearDeviceTranslation();
    std::string extension = filepath.substr(filepath.size() - 4, 4);
    HelperFunctions::toLower(extension);
    if (extension != ".xml") return PHomegearDeviceTranslation();
    if (_bl->debugLevel >= 5) _bl->out.printDebug("Loading XML RPC device translation " + filepath);
    PHomegearDeviceTranslation device = std::make_shared<HomegearDeviceTranslation>(_bl, filepath);
    if (device->loaded()) return device;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return PHomegearDeviceTranslation();
}

std::unordered_set<std::string> DeviceTranslations::getLanguages() {
  try {
    std::string translationsPath = _bl->settings.deviceDescriptionPath() + std::to_string((int32_t)_family) + "/l10n/";
    auto languageDirectories = Io::getDirectories(translationsPath);
    std::unordered_set<std::string> languages;
    for (auto &element : languageDirectories) {
      if (element.empty()) continue;
      languages.emplace(element.substr(0, element.size() - 1));
    }
    return languages;
  } catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::unordered_set<std::string>();
}

std::unordered_map<std::string, PHomegearDeviceTranslation> DeviceTranslations::getTranslations(const std::string &filename) {
  try {
    std::unordered_map<std::string, PHomegearDeviceTranslation> translations;

    auto languages = getLanguages();
    for (auto &language : languages) {
      std::lock_guard<std::mutex> deviceTranslationsGuard(_deviceTranslationsMutex);
      auto languageIterator = _deviceTranslations.find(language);
      if (languageIterator == _deviceTranslations.end()) {
        auto translation = load(filename, language);
        if (!translation) continue;
        _deviceTranslations[language].emplace(filename, translation);
        languageIterator = _deviceTranslations.find(language);
        if (languageIterator == _deviceTranslations.end()) continue;
      }

      auto translationIterator = languageIterator->second.find(filename);
      if (translationIterator == languageIterator->second.end()) {
        auto translation = load(filename, language);
        if (!translation) continue;
        languageIterator->second.emplace(filename, translation);
        translations.emplace(language, translation);
      } else translations.emplace(language, translationIterator->second);
    }

    return translations;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::unordered_map<std::string, PHomegearDeviceTranslation>();
}

PHomegearDeviceTranslation DeviceTranslations::getTranslation(const std::string &filename, const std::string &language) {
  try {
    auto currentLanguage = language;
    if (currentLanguage.empty()) currentLanguage = "en";
    std::lock_guard<std::mutex> deviceTranslationsGuard(_deviceTranslationsMutex);
    auto languageIterator = _deviceTranslations.find(currentLanguage);
    if (languageIterator == _deviceTranslations.end()) {
      auto translation = load(filename, currentLanguage);
      if (!translation) {
        if (currentLanguage != "en") {
          currentLanguage = "en";
          translation = load(filename, currentLanguage);
          if (!translation) return PHomegearDeviceTranslation();
        } else {
          return PHomegearDeviceTranslation();
        }
      }
      _deviceTranslations[currentLanguage].emplace(filename, translation);
      languageIterator = _deviceTranslations.find(currentLanguage);
      if (languageIterator == _deviceTranslations.end()) {
        return PHomegearDeviceTranslation();
      }
    }

    auto translationIterator = languageIterator->second.find(filename);
    if (translationIterator == languageIterator->second.end()) {
      auto translation = load(filename, currentLanguage);
      if (!translation) return PHomegearDeviceTranslation();
      languageIterator->second.emplace(filename, translation);
      return translation;
    } else return translationIterator->second;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return PHomegearDeviceTranslation();
}

PVariable DeviceTranslations::getTypeDescription(const std::string &filename, const std::string &language, const std::string &deviceId) {
  try {
    if (language.empty()) {
      auto typeDescription = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
      auto translations = getTranslations(filename);
      for (auto &translation : translations) {
        auto idIterator = translation.second->typeDescriptions.find(deviceId);
        if (idIterator != translation.second->typeDescriptions.end()) {
          typeDescription->structValue->emplace(translation.first, std::make_shared<BaseLib::Variable>(idIterator->second));
        }
      }
      return typeDescription;
    } else {
      PHomegearDeviceTranslation translation = getTranslation(filename, language);
      if (!translation) return std::make_shared<BaseLib::Variable>("");

      auto idIterator = translation->typeDescriptions.find(deviceId);
      if (idIterator != translation->typeDescriptions.end()) return std::make_shared<BaseLib::Variable>(idIterator->second);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::make_shared<BaseLib::Variable>("");
}

PVariable DeviceTranslations::getTypeLongDescription(const std::string &filename, const std::string &language, const std::string &deviceId) {
  try {
    if (language.empty()) {
      auto typeDescription = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
      auto translations = getTranslations(filename);
      for (auto &translation : translations) {
        auto idIterator = translation.second->typeLongDescriptions.find(deviceId);
        if (idIterator != translation.second->typeLongDescriptions.end()) {
          typeDescription->structValue->emplace(translation.first, std::make_shared<BaseLib::Variable>(idIterator->second));
        }
      }
      return typeDescription;
    } else {
      PHomegearDeviceTranslation translation = getTranslation(filename, language);
      if (!translation) return std::make_shared<BaseLib::Variable>("");

      auto idIterator = translation->typeLongDescriptions.find(deviceId);
      if (idIterator != translation->typeLongDescriptions.end()) return std::make_shared<BaseLib::Variable>(idIterator->second);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::make_shared<BaseLib::Variable>("");
}

PVariable DeviceTranslations::getParameterLabel(const std::string &filename, const std::string &language, ParameterGroup::Type::Enum parameterGroupType, const std::string &parameterGroupId, const std::string &parameterId) {
  try {
    if (language.empty()) {
      auto label = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
      auto translations = getTranslations(filename);
      for (auto &translation : translations) {
        std::unordered_map<std::string, std::unordered_map<std::string, HomegearDeviceTranslation::ParameterTranslation>>::iterator parameterGroupIterator;

        if (parameterGroupType == ParameterGroup::Type::Enum::config) {
          parameterGroupIterator = translation.second->configParameters.find(parameterGroupId);
          if (parameterGroupIterator == translation.second->configParameters.end()) continue;
        } else if (parameterGroupType == ParameterGroup::Type::Enum::variables) {
          parameterGroupIterator = translation.second->variables.find(parameterGroupId);
          if (parameterGroupIterator == translation.second->variables.end()) continue;
        } else if (parameterGroupType == ParameterGroup::Type::Enum::link) {
          parameterGroupIterator = translation.second->linkParameters.find(parameterGroupId);
          if (parameterGroupIterator == translation.second->linkParameters.end()) continue;
        }

        auto parameterIterator = parameterGroupIterator->second.find(parameterId);
        if (parameterIterator == parameterGroupIterator->second.end()) continue;

        label->structValue->emplace(translation.first, std::make_shared<BaseLib::Variable>(parameterIterator->second.label));
      }
      return label;
    } else {
      PHomegearDeviceTranslation translation = getTranslation(filename, language);
      if (!translation) return std::make_shared<BaseLib::Variable>("");

      std::unordered_map<std::string, std::unordered_map<std::string, HomegearDeviceTranslation::ParameterTranslation>>::iterator parameterGroupIterator;

      if (parameterGroupType == ParameterGroup::Type::Enum::config) {
        parameterGroupIterator = translation->configParameters.find(parameterGroupId);
        if (parameterGroupIterator == translation->configParameters.end()) return std::make_shared<BaseLib::Variable>("");
      } else if (parameterGroupType == ParameterGroup::Type::Enum::variables) {
        parameterGroupIterator = translation->variables.find(parameterGroupId);
        if (parameterGroupIterator == translation->variables.end()) return std::make_shared<BaseLib::Variable>("");
      } else if (parameterGroupType == ParameterGroup::Type::Enum::link) {
        parameterGroupIterator = translation->linkParameters.find(parameterGroupId);
        if (parameterGroupIterator == translation->linkParameters.end()) return std::make_shared<BaseLib::Variable>("");
      }

      auto parameterIterator = parameterGroupIterator->second.find(parameterId);
      if (parameterIterator == parameterGroupIterator->second.end()) return std::make_shared<BaseLib::Variable>("");

      return std::make_shared<BaseLib::Variable>(parameterIterator->second.label);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::make_shared<BaseLib::Variable>("");
}

PVariable DeviceTranslations::getParameterDescription(const std::string &filename, const std::string &language, ParameterGroup::Type::Enum parameterGroupType, const std::string &parameterGroupId, const std::string &parameterId) {
  try {
    if (language.empty()) {
      auto description = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
      auto translations = getTranslations(filename);
      for (auto &translation : translations) {
        std::unordered_map<std::string, std::unordered_map<std::string, HomegearDeviceTranslation::ParameterTranslation>>::iterator parameterGroupIterator;

        if (parameterGroupType == ParameterGroup::Type::Enum::config) {
          parameterGroupIterator = translation.second->configParameters.find(parameterGroupId);
          if (parameterGroupIterator == translation.second->configParameters.end()) continue;
        } else if (parameterGroupType == ParameterGroup::Type::Enum::variables) {
          parameterGroupIterator = translation.second->variables.find(parameterGroupId);
          if (parameterGroupIterator == translation.second->variables.end()) continue;
        } else if (parameterGroupType == ParameterGroup::Type::Enum::link) {
          parameterGroupIterator = translation.second->linkParameters.find(parameterGroupId);
          if (parameterGroupIterator == translation.second->linkParameters.end()) continue;
        }

        auto parameterIterator = parameterGroupIterator->second.find(parameterId);
        if (parameterIterator == parameterGroupIterator->second.end()) continue;

        description->structValue->emplace(translation.first, std::make_shared<BaseLib::Variable>(parameterIterator->second.description));
      }
      return description;
    } else {
      PHomegearDeviceTranslation translation = getTranslation(filename, language);
      if (!translation) return std::make_shared<BaseLib::Variable>("");

      std::unordered_map<std::string, std::unordered_map<std::string, HomegearDeviceTranslation::ParameterTranslation>>::iterator parameterGroupIterator;

      if (parameterGroupType == ParameterGroup::Type::Enum::config) {
        parameterGroupIterator = translation->configParameters.find(parameterGroupId);
        if (parameterGroupIterator == translation->configParameters.end()) return std::make_shared<BaseLib::Variable>("");
      } else if (parameterGroupType == ParameterGroup::Type::Enum::variables) {
        parameterGroupIterator = translation->variables.find(parameterGroupId);
        if (parameterGroupIterator == translation->variables.end()) return std::make_shared<BaseLib::Variable>("");
      } else if (parameterGroupType == ParameterGroup::Type::Enum::link) {
        parameterGroupIterator = translation->linkParameters.find(parameterGroupId);
        if (parameterGroupIterator == translation->linkParameters.end()) return std::make_shared<BaseLib::Variable>("");
      }

      auto parameterIterator = parameterGroupIterator->second.find(parameterId);
      if (parameterIterator == parameterGroupIterator->second.end()) return std::make_shared<BaseLib::Variable>("");

      return std::make_shared<BaseLib::Variable>(parameterIterator->second.description);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::make_shared<BaseLib::Variable>("");
}

}
}
