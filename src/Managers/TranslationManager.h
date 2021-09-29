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

#ifndef LIBHOMEGEAR_BASE_SRC_MANAGERS_TRANSLATIONMANAGER_H_
#define LIBHOMEGEAR_BASE_SRC_MANAGERS_TRANSLATIONMANAGER_H_

#include "../HelperFunctions/HelperFunctions.h"
#include "../HelperFunctions/Io.h"

namespace BaseLib {

class TranslationManager {
 public:
  TranslationManager() = delete;

  /**
   * Loads a translation file.
   *
   * @param filepath The path to the translation directory containing language subdirectories which contain translation files in YAML format (i. e. "en-US/my-translation.yaml").
   * @return No return value.
   */
  static void load(const std::string &filepath);

  /**
   * Returns a translation from one of the global translation files.
   *
   * @param key The translation to lookup.
   * @param language The language to return.
   * @param variables Optional variables to replace in the translation string.
   * @return The translation or an empty string if no translation was found.
   */
  static std::string getTranslation(const std::string &key, const std::string &language, const std::list<std::string> &variables = std::list<std::string>());

  /**
   * Returns all available translations for a key as a Struct.
   *
   * @param key The translation to lookup for every language.
   * @param variables Optional variables to replace in the translation string.
   * @return The translations or an empty Struct if no translations were found.
   */
  static BaseLib::PVariable getTranslations(const std::string &key, const std::list<std::string> &variables = std::list<std::string>());
 private:
  static std::mutex _translationsMutex;
  static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _translations;
};

}

#endif //LIBHOMEGEAR_BASE_SRC_MANAGERS_TRANSLATIONMANAGER_H_
