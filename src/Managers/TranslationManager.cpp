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

#include "TranslationManager.h"

namespace BaseLib
{

std::mutex TranslationManager::_translationsMutex;
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> TranslationManager::_translations;

void TranslationManager::load(const std::string &filepath) {
  std::lock_guard<std::mutex> translationsGuard(_translationsMutex);

  if (!Io::directoryExists(filepath)) return;

  auto languageDirectories = Io::getDirectories(filepath);
  for (auto &directory : languageDirectories) {
    auto language = directory.substr(0, directory.size() - 1);
    auto &translations = _translations[language];

    auto files = Io::getFiles(filepath + directory);
    for (auto &file : files) {
      auto content = Io::getFileContent(filepath + directory + file);

      std::istringstream stringStream(content);
      for (std::string line; std::getline(stringStream, line);) {
        if (line.empty() || line.at(0) == '-' || line.at(0) == '#') continue;
        else {
          auto pair = HelperFunctions::splitFirst(line, ':');
          HelperFunctions::trim(pair.first);
          HelperFunctions::trim(pair.second);
          if (pair.second.size() < 3) continue;
          pair.second = pair.second.substr(1, pair.second.size() - 2);
          translations.emplace(pair.first, pair.second);
        }
      }
    }
  }
}

std::string TranslationManager::getTranslation(const std::string &key, const std::string &language, const std::list<std::string> &variables) {
  std::lock_guard<std::mutex> translationsGuard(_translationsMutex);
  auto languageIterator = _translations.find(language);
  if (languageIterator == _translations.end() && language.size() > 2) {
    auto pair = HelperFunctions::splitFirst(language, '-');
    languageIterator = _translations.find(pair.first);
  }

  if (languageIterator == _translations.end() && language != "en") {
    languageIterator = _translations.find("en");
  }

  if (languageIterator == _translations.end()) {
    return key;
  }

  auto translationIterator = languageIterator->second.find(key);
  if (translationIterator == languageIterator->second.end()) {
    return key;
  }

  auto translation = translationIterator->second;
  uint32_t i = 0;
  for (auto &variable : variables) {
    HelperFunctions::stringReplace(translation, "%variable" + std::to_string(i++) + "%", variable);
  }

  return translation;
}

BaseLib::PVariable TranslationManager::getTranslations(const std::string &key, const std::list<std::string> &variables) {
  auto translations = std::make_shared<Variable>(VariableType::tStruct);

  std::lock_guard<std::mutex> translationsGuard(_translationsMutex);
  for (auto &language : _translations) {
    auto translationIterator = language.second.find(key);
    if (translationIterator == language.second.end()) {
      translations->structValue->emplace(language.first, std::make_shared<Variable>(key));
    }

    auto translation = translationIterator->second;
    uint32_t i = 0;
    for (auto &variable : variables) {
      HelperFunctions::stringReplace(translation, "%variable" + std::to_string(i++) + "%", variable);
    }

    translations->structValue->emplace(language.first, std::make_shared<Variable>(translation));
  }

  return translations;
}

}