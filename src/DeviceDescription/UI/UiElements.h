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

#ifndef UIELEMENTS_H_
#define UIELEMENTS_H_

#include "HomegearUiElements.h"
#include "../../Variable.h"

#include <vector>
#include <memory>
#include <mutex>

namespace BaseLib {

class SharedObjects;

namespace DeviceDescription {

/**
 * Class holding information on how UI elements look like. It is used to load all UI elements.
 */
class UiElements {
 public:
  struct UiVariableInfo {
    uint64_t peerId = 0;
    int32_t channel = -1;
    std::string name;
    PVariable value;
    std::string unit;
    PVariable minimumValue;
    PVariable maximumValue;
    PVariable minimumValueScaled;
    PVariable maximumValueScaled;
    PVariable rendering;
  } __attribute__((aligned(128)));
  typedef std::shared_ptr<UiVariableInfo> PUiVariableInfo;

  struct UiPeerInfo {
    std::vector<std::vector<PUiVariableInfo>> inputPeers;
    std::vector<std::vector<PUiVariableInfo>> outputPeers;
  } __attribute__((aligned(64)));
  typedef std::shared_ptr<UiPeerInfo> PUiPeerInfo;

  explicit UiElements(BaseLib::SharedObjects *baseLib);
  virtual ~UiElements() = default;
  void clear();

  PVariable getUiElements(const std::string &language);
  PHomegearUiElement getUiElement(const std::string &language, const std::string &id);

  /**
   * Returns an UI element and assigns peer IDs to it.
   *
   * @param language The language to return (e. g. "en-US")
   * @param id The ID of the UI element
   * @param peerInfo The peer IDs to assign
   * @return Returns the UI element on success.
   */
  PHomegearUiElement getUiElement(const std::string &language, const std::string &id, PUiPeerInfo peerInfo);
 protected:
  BaseLib::SharedObjects *_bl = nullptr;
  std::mutex _uiInfoMutex;
  std::unordered_map<std::string, std::unordered_map<std::string, PHomegearUiElement>> _uiInfo;

  void load(const std::string &language);
};

}
}
#endif
