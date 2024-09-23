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

#include "SupportedDevice.h"
#include "../BaseLib.h"

namespace BaseLib {
namespace DeviceDescription {

SupportedDevice::SupportedDevice(BaseLib::SharedObjects *baseLib) {
  _bl = baseLib;
}

SupportedDevice::SupportedDevice(BaseLib::SharedObjects *baseLib, xml_node *node) : SupportedDevice(baseLib) {
  for (xml_attribute *attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
    std::string attributeName(attr->name());
    std::string attributeValue(attr->value());
    if (attributeName == "id") {
      id = attributeValue;
    } else if (attributeName == "productId") {
      productId = attributeValue;
    } else _bl->out.printWarning("Warning: Unknown attribute for \"supportedDevice\": " + attributeName);
  }
  for (xml_node *subNode = node->first_node(); subNode; subNode = subNode->next_sibling()) {
    std::string nodeName(subNode->name());
    std::string value(subNode->value());
    if (nodeName == "hardwareVersion") hardwareVersion = value;
    else if (nodeName == "manufacturer") manufacturer = value;
    else if (nodeName == "longDescription") longDescription = value;
    else if (nodeName == "serialPrefix") serialPrefix = value;
    else if (nodeName == "description") description = value;
    else if (nodeName == "typeNumber") typeNumber = Math::getUnsignedNumber64(value);
    else if (nodeName == "minFirmwareVersion") minFirmwareVersion = Math::getUnsignedNumber(value);
    else if (nodeName == "maxFirmwareVersion") maxFirmwareVersion = Math::getUnsignedNumber(value);
    else _bl->out.printWarning("Warning: Unknown node in \"supportedDevice\": " + nodeName);
  }
}

bool SupportedDevice::matches(uint64_t typeNumber, uint32_t firmwareVersion) {
  try {
    if (typeNumber == this->typeNumber && checkFirmwareVersion(firmwareVersion)) return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return false;
}

bool SupportedDevice::matches(const std::string &typeId) {
  try {
    if (id == typeId) return true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool SupportedDevice::checkFirmwareVersion(int32_t version) {
  if (version < 0) return true;
  if ((unsigned)version >= minFirmwareVersion && (maxFirmwareVersion == 0 || (unsigned)version <= maxFirmwareVersion)) return true;
  return false;
}

}
}
