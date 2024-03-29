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

#ifndef HOMEGEARUIELEMENT_H_
#define HOMEGEARUIELEMENT_H_

#include "UiVariable.h"
#include "UiIcon.h"
#include "UiText.h"
#include "UiGrid.h"
#include "../../Variable.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <list>

using namespace rapidxml;

namespace BaseLib {

class SharedObjects;

namespace DeviceDescription {

class HomegearUiElement;

/**
 * Helper type for Packet pointers.
 */
typedef std::shared_ptr<HomegearUiElement> PHomegearUiElement;

/**
 * Class defining a physical packet.
 */
class HomegearUiElement {
 public:
  enum class Type {
    undefined,
    simple,
    complex
  };

  //Elements
  std::string id;
  Type type = Type::undefined;
  std::string control;
  std::string description;
  std::unordered_map<std::string, PUiIcon> icons;
  std::unordered_map<std::string, PUiText> texts;
  std::list<PUiVariable> variableInputs;
  std::list<PUiVariable> variableOutputs;
  std::unordered_map<std::string, PVariable> metadata;
  PUiGrid grid;
  std::list<PUiControl> controls;

  HomegearUiElement(BaseLib::SharedObjects *baseLib);
  HomegearUiElement(BaseLib::SharedObjects *baseLib, xml_node *node);
  HomegearUiElement(HomegearUiElement const &rhs);
  virtual ~HomegearUiElement() = default;

  HomegearUiElement &operator=(const HomegearUiElement &rhs);

  PVariable getElementInfo(bool addHelpInfo = false);
 protected:
  BaseLib::SharedObjects *_bl = nullptr;
};
}
}

#endif
