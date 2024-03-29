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

#include "UiIcon.h"
#include "../../BaseLib.h"

namespace BaseLib {
namespace DeviceDescription {

UiIcon::UiIcon(BaseLib::SharedObjects *baseLib) {
  _bl = baseLib;
}

UiIcon::UiIcon(BaseLib::SharedObjects *baseLib, xml_node *node) : UiIcon(baseLib) {
  for (xml_attribute *attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
    std::string name(attr->name());
    std::string value(attr->value());
    if (name == "id") id = value;
    else _bl->out.printWarning("Warning: Unknown attribute for \"icon\": " + std::string(attr->name()));
  }
  for (xml_node *subNode = node->first_node(); subNode; subNode = subNode->next_sibling()) {
    std::string name(subNode->name());
    std::string value(subNode->value());
    if (name == "name") this->name = value;
    else if (name == "color") color = value;
    else _bl->out.printWarning("Warning: Unknown node in \"icon\": " + name);
  }
}

UiIcon::UiIcon(UiIcon const &rhs) {
  _bl = rhs._bl;

  id = rhs.id;

  name = rhs.name;
  color = rhs.color;
}

PUiIcon UiIcon::fromJson(BaseLib::SharedObjects *baseLib, const std::string &id, const PVariable &json) {
  auto icon = std::make_shared<UiIcon>(baseLib);
  icon->id = id;

  auto iterator = json->structValue->find("color");
  if (iterator != json->structValue->end()) icon->color = iterator->second->stringValue;

  iterator = json->structValue->find("name");
  if (iterator != json->structValue->end()) icon->name = iterator->second->stringValue;

  return icon;
}

UiIcon &UiIcon::operator=(const UiIcon &rhs) {
  if (&rhs == this) return *this;

  _bl = rhs._bl;

  id = rhs.id;

  name = rhs.name;
  color = rhs.color;

  return *this;
}

}
}
