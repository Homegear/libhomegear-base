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

#include "HomegearDevice.h"
#include "../BaseLib.h"

#include "../Encoding/RapidXml/rapidxml_print.hpp"

namespace BaseLib {
namespace DeviceDescription {

void HomegearDevice::setPath(std::string &value) {
  _path = value;
}

std::string HomegearDevice::getPath() {
  return _path;
}

void HomegearDevice::setFilename(std::string &value) {
  _filename = value;
}

std::string HomegearDevice::getFilename() {
  return _filename;
}

int32_t HomegearDevice::getDynamicChannelCount() {
  return _dynamicChannelCount;
}

void HomegearDevice::setDynamicChannelCount(int32_t value) {
  try {
    _dynamicChannelCount = value;
    PFunction function;
    uint32_t index = 0;
    for (Functions::iterator i = functions.begin(); i != functions.end(); ++i) {
      if (!i->second) continue;
      if (i->second->dynamicChannelCountIndex > -1) {
        index = i->first;
        function = i->second;
        break;
      }
    }

    for (uint32_t i = index + 1; i < index + value; i++) {
      if (functions.find(i) == functions.end()) {
        functions[i] = function;
        for (Parameters::iterator j = function->variables->parameters.begin(); j != function->variables->parameters.end(); ++j) {
          for (std::vector<std::shared_ptr<Parameter::Packet>>::iterator k = j->second->getPackets.begin(); k != j->second->getPackets.end(); ++k) {
            PacketsById::iterator packet = packetsById.find((*k)->id);
            if (packet != packetsById.end()) {
              valueRequestPackets[i][(*k)->id] = packet->second;
            }
          }
        }
      } else _bl->out.printError("Error: Tried to add channel with the same index twice. Index: " + std::to_string(i));
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

HomegearDevice::HomegearDevice(BaseLib::SharedObjects *baseLib) {
  _bl = baseLib;
  runProgram.reset(new RunProgram(baseLib));
}

HomegearDevice::HomegearDevice(BaseLib::SharedObjects *baseLib, xml_node *node) : HomegearDevice(baseLib) {
  if (node) parseXML(node);
}

HomegearDevice::HomegearDevice(BaseLib::SharedObjects *baseLib, std::string xmlFilename, bool &oldFormat) : HomegearDevice(baseLib) {
  try {
    load(xmlFilename, oldFormat);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

HomegearDevice::HomegearDevice(BaseLib::SharedObjects *baseLib, std::string xmlFilename, std::vector<char> &xml) : HomegearDevice(baseLib) {
  try {
    load(xmlFilename, xml);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

HomegearDevice::~HomegearDevice() {

}

void HomegearDevice::load(std::string xmlFilename, std::vector<char> &xml) {
  if (xml.empty()) return;
  if (xml.at(xml.size() - 1) != '\0') {
    _bl->out.printError("Error: Passed XML does not end with null character.");
    return;
  }
  xml_document doc;
  try {
    _path = xmlFilename;
    _filename = BaseLib::HelperFunctions::splitLast(xmlFilename, '/').second;
    doc.parse<parse_no_entity_translation | parse_validate_closing_tags>(xml.data());
    if (!doc.first_node("homegearDevice")) {
      _bl->out.printError("Error: Device XML does not start with \"homegearDevice\".");
      doc.clear();
      return;
    }
    parseXML(doc.first_node("homegearDevice"));

    postLoad();
    _loaded = true;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  doc.clear();
}

void HomegearDevice::load(std::string xmlFilename, bool &oldFormat) {
  xml_document doc;
  try {
    _path = xmlFilename;
    _filename = BaseLib::HelperFunctions::splitLast(xmlFilename, '/').second;
    std::ifstream fileStream(xmlFilename, std::ios::in | std::ios::binary);
    if (fileStream) {
      uint32_t length;
      fileStream.seekg(0, std::ios::end);
      length = fileStream.tellg();
      fileStream.seekg(0, std::ios::beg);
      std::vector<char> buffer(length + 1);
      fileStream.read(&buffer[0], length);
      fileStream.close();
      buffer[length] = '\0';
      doc.parse<parse_no_entity_translation | parse_validate_closing_tags>(&buffer[0]);
      if (doc.first_node("device")) {
        oldFormat = true;
        doc.clear();
        return;
      } else if (!doc.first_node("homegearDevice")) {
        _bl->out.printError("Error: Device XML file \"" + xmlFilename + "\" does not start with \"homegearDevice\".");
        doc.clear();
        return;
      }
      parseXML(doc.first_node("homegearDevice"));
    } else _bl->out.printError("Error reading file " + xmlFilename + ": " + strerror(errno));

    postLoad();
    _loaded = true;
  }
  catch (const std::exception &ex) {
    _bl->out.printError("Error: Could not parse file \"" + xmlFilename + "\": " + ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  doc.clear();
}

void HomegearDevice::postLoad() {
  try {
    PParameter parameter(new Parameter(_bl, nullptr));
    parameter->id = "LAST_PACKET_RECEIVED";
    parameter->writeable = false;
    parameter->visible = false;
    parameter->logical.reset(new LogicalInteger(_bl));
    parameter->logical->type = ILogical::Type::Enum::tInteger;
    parameter->physical.reset(new PhysicalInteger(_bl));
    parameter->physical->operationType = IPhysical::OperationType::Enum::internal;
    parameter->physical->type = IPhysical::Type::Enum::tInteger;
    parameter->physical->groupId = "LAST_PACKET_RECEIVED";
    if (!functions[0]) functions[0].reset(new Function(_bl));
    functions[0]->variables->parameters[parameter->id] = parameter;

    if (!encryption) return;
    //For HomeMatic BidCoS: Set AES default value to "false", so a new AES key is set on pairing
    for (Functions::iterator i = functions.begin(); i != functions.end(); ++i) {
      if (!i->second || i->first == 0) continue;
      parameter = i->second->configParameters->getParameter("AES_ACTIVE");
      if (!parameter) {
        parameter.reset(new Parameter(_bl, nullptr));
        i->second->configParameters->parameters["AES_ACTIVE"] = parameter;
      }
      parameter->id = "AES_ACTIVE";
      parameter->internal = true;
      parameter->casts.clear();
      parameter->casts.push_back(std::shared_ptr<BooleanInteger>(new BooleanInteger(_bl)));
      parameter->logical.reset(new LogicalBoolean(_bl));
      parameter->logical->defaultValueExists = true;
      parameter->physical->operationType = IPhysical::OperationType::Enum::config;
      parameter->physical->type = IPhysical::Type::Enum::tInteger;
      parameter->physical->groupId = "AES_ACTIVE";
      parameter->physical->list = 1;
      parameter->physical->index = 8;
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void HomegearDevice::save(std::string &filename) {
  xml_document doc;
  try {
    if (Io::fileExists(filename)) {
      if (!Io::deleteFile(filename)) {
        doc.clear();
        _bl->out.printError("Error: File \"" + filename + "\" already exists and cannot be deleted.");
        return;
      }
    }

    xml_node *homegearDevice = doc.allocate_node(node_element, "homegearDevice");
    doc.append_node(homegearDevice);

    saveDevice(&doc, homegearDevice, this);

    std::ofstream fileStream(filename, std::ios::out | std::ios::binary);
    if (fileStream) {
      fileStream << doc;
    }
    fileStream.close();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  doc.clear();
}

void HomegearDevice::saveDevice(xml_document *doc, xml_node *parentNode, HomegearDevice *device) {
  try {
    std::string tempString = std::to_string(device->version);
    xml_attribute *versionAttr = doc->allocate_attribute("version", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
    parentNode->append_attribute(versionAttr);

    xml_node *node = doc->allocate_node(node_element, "supportedDevices");
    parentNode->append_node(node);
    for (SupportedDevices::iterator i = device->supportedDevices.begin(); i != device->supportedDevices.end(); ++i) {
      xml_node *subnode = doc->allocate_node(node_element, "device");
      node->append_node(subnode);

      xml_attribute *attr = doc->allocate_attribute("id", doc->allocate_string((*i)->id.c_str(), (*i)->id.size() + 1));
      subnode->append_attribute(attr);

      if (!(*i)->productId.empty()) {
        attr = doc->allocate_attribute("productId", doc->allocate_string((*i)->productId.c_str(), (*i)->productId.size() + 1));
        subnode->append_attribute(attr);
      }

      if (!(*i)->hardwareVersion.empty()) {
        xml_node *deviceNode = doc->allocate_node(node_element, "hardwareVersion", doc->allocate_string((*i)->hardwareVersion.c_str(), (*i)->hardwareVersion.size() + 1));
        subnode->append_node(deviceNode);
      }

      if (!(*i)->manufacturer.empty()) {
        xml_node *deviceNode = doc->allocate_node(node_element, "manufacturer", doc->allocate_string((*i)->manufacturer.c_str(), (*i)->manufacturer.size() + 1));
        subnode->append_node(deviceNode);
      }

      xml_node *deviceNode = doc->allocate_node(node_element, "description", doc->allocate_string((*i)->description.c_str(), (*i)->description.size() + 1));
      subnode->append_node(deviceNode);

      if (!(*i)->longDescription.empty()) {
        deviceNode = doc->allocate_node(node_element, "longDescription", doc->allocate_string((*i)->longDescription.c_str(), (*i)->longDescription.size() + 1));
        subnode->append_node(deviceNode);
      }

      if (!(*i)->serialPrefix.empty()) {
        deviceNode = doc->allocate_node(node_element, "serialPrefix", doc->allocate_string((*i)->serialPrefix.c_str(), (*i)->serialPrefix.size() + 1));
        subnode->append_node(deviceNode);
      }

      if ((*i)->typeNumber != (uint64_t)-1) {
        std::string typeNumber = "0x" + BaseLib::HelperFunctions::getHexString((*i)->typeNumber);
        char *pTypeNumber = doc->allocate_string(typeNumber.c_str(), typeNumber.size() + 1);
        deviceNode = doc->allocate_node(node_element, "typeNumber", pTypeNumber);
        subnode->append_node(deviceNode);
      }

      if ((*i)->minFirmwareVersion != 0) {
        tempString = "0x" + BaseLib::HelperFunctions::getHexString((*i)->minFirmwareVersion);
        deviceNode = doc->allocate_node(node_element, "minFirmwareVersion", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(deviceNode);
      }

      if ((*i)->maxFirmwareVersion != 0) {
        tempString = "0x" + BaseLib::HelperFunctions::getHexString((*i)->maxFirmwareVersion);
        deviceNode = doc->allocate_node(node_element, "maxFirmwareVersion", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(deviceNode);
      }
    }

    if (device->runProgram && !device->runProgram->path.empty()) {
      node = doc->allocate_node(node_element, "runProgram");
      parentNode->append_node(node);

      xml_node *subnode = doc->allocate_node(node_element, "path", doc->allocate_string(device->runProgram->path.c_str(), device->runProgram->path.size() + 1));
      node->append_node(subnode);

      if (device->runProgram->arguments.size() > 0) {
        subnode = doc->allocate_node(node_element, "arguments");
        node->append_node(subnode);
        for (std::vector<std::string>::iterator i = device->runProgram->arguments.begin(); i != device->runProgram->arguments.end(); ++i) {
          xml_node *argumentnode = doc->allocate_node(node_element, "argument", i->c_str());
          subnode->append_node(argumentnode);
        }
      }

      tempString = device->runProgram->startType == RunProgram::StartType::Enum::once ? "once" : (device->runProgram->startType == RunProgram::StartType::Enum::interval ? "interval" : "permanent");
      subnode = doc->allocate_node(node_element, "startType", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);

      if (device->runProgram->interval > 0) {
        tempString = std::to_string(device->runProgram->interval);
        subnode = doc->allocate_node(node_element, "interval", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }
    }

    // {{{ Properties
    node = doc->allocate_node(node_element, "properties");
    parentNode->append_node(node);

    if (device->receiveModes != ReceiveModes::Enum::always) {
      if (device->receiveModes & ReceiveModes::Enum::always) {
        tempString = "always";
        xml_node *subnode = doc->allocate_node(node_element, "receiveMode", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }
      if (device->receiveModes & ReceiveModes::Enum::wakeOnRadio) {
        tempString = "wakeOnRadio";
        xml_node *subnode = doc->allocate_node(node_element, "receiveMode", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }
      if (device->receiveModes & ReceiveModes::Enum::config) {
        tempString = "config";
        xml_node *subnode = doc->allocate_node(node_element, "receiveMode", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }
      if (device->receiveModes & ReceiveModes::Enum::wakeUp) {
        tempString = "wakeUp";
        xml_node *subnode = doc->allocate_node(node_element, "receiveMode", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }
      if (device->receiveModes & ReceiveModes::Enum::wakeUp2) {
        tempString = "wakeUp2";
        xml_node *subnode = doc->allocate_node(node_element, "receiveMode", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }
      if (device->receiveModes & ReceiveModes::Enum::lazyConfig) {
        tempString = "lazyConfig";
        xml_node *subnode = doc->allocate_node(node_element, "receiveMode", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }
    }

    if (device->encryption) {
      tempString = "true";
      xml_node *subnode = doc->allocate_node(node_element, "encryption", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (device->timeout > 0) {
      tempString = std::to_string(device->timeout);
      xml_node *subnode = doc->allocate_node(node_element, "timeout", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (device->memorySize != 0) {
      tempString = std::to_string(device->memorySize);
      xml_node *subnode = doc->allocate_node(node_element, "memorySize", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (device->memorySize2 != 0) {
      tempString = std::to_string(device->memorySize2);
      xml_node *subnode = doc->allocate_node(node_element, "memorySize2", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (!device->visible) {
      tempString = "false";
      xml_node *subnode = doc->allocate_node(node_element, "visible", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (!device->deletable) {
      tempString = "false";
      xml_node *subnode = doc->allocate_node(node_element, "deletable", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (device->internal) {
      tempString = "true";
      xml_node *subnode = doc->allocate_node(node_element, "internal", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (device->needsTime) {
      tempString = "true";
      xml_node *subnode = doc->allocate_node(node_element, "needsTime", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (device->hasBattery) {
      tempString = "true";
      xml_node *subnode = doc->allocate_node(node_element, "hasBattery", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (device->addressSize != 0) {
      tempString = std::to_string(device->addressSize);
      xml_node *subnode = doc->allocate_node(node_element, "addressSize", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      node->append_node(subnode);
    }

    if (!device->pairingMethod.empty()) {
      xml_node *subnode = doc->allocate_node(node_element, "pairingMethod", doc->allocate_string(device->pairingMethod.c_str(), device->pairingMethod.size() + 1));
      node->append_node(subnode);
    }

    if (!device->interface.empty()) {
      xml_node *subnode = doc->allocate_node(node_element, "interface", doc->allocate_string(device->interface.c_str(), device->interface.size() + 1));
      node->append_node(subnode);
    }
    // }}}

    std::map<std::string, PConfigParameters> configParameters;
    std::map<std::string, PVariables> variables;
    std::map<std::string, PLinkParameters> linkParameters;

    node = doc->allocate_node(node_element, "functions");
    parentNode->append_node(node);
    int32_t skip = 0;
    for (Functions::iterator i = device->functions.begin(); i != device->functions.end(); i++) {
      if (skip == 0) skip = i->second->channelCount - 1;
      else if (skip > 0) {
        skip--;
        continue;
      }
      xml_node *subnode = doc->allocate_node(node_element, "function");
      node->append_node(subnode);
      saveFunction(doc, subnode, i->second, configParameters, variables, linkParameters);
    }

    // {{{ Metadata
    if (metadata) {
      node = doc->allocate_node(node_element, "metadata");
      parentNode->append_node(node);
      HelperFunctions::variable2xml(doc, node, metadata);
    }
    // }}}

    // {{{ Packets
    node = doc->allocate_node(node_element, "packets");
    parentNode->append_node(node);
    for (PacketsById::iterator i = device->packetsById.begin(); i != device->packetsById.end(); ++i) {
      xml_node *subnode = doc->allocate_node(node_element, "packet");
      node->append_node(subnode);

      xml_attribute *attr = doc->allocate_attribute("id", doc->allocate_string(i->first.c_str(), i->first.size() + 1));
      subnode->append_attribute(attr);

      tempString = i->second->direction == Packet::Direction::Enum::fromCentral ? "fromCentral" : "toCentral";
      xml_node *packetNode = doc->allocate_node(node_element, "direction", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      subnode->append_node(packetNode);

      if (i->second->length != -1) {
        tempString = std::to_string(i->second->length);
        packetNode = doc->allocate_node(node_element, "length", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (i->second->type != -1) {
        tempString = "0x" + BaseLib::HelperFunctions::getHexString(i->second->type);
        packetNode = doc->allocate_node(node_element, "type", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (i->second->subtype != -1) {
        tempString = "0x" + BaseLib::HelperFunctions::getHexString(i->second->subtype);
        packetNode = doc->allocate_node(node_element, "subtype", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (i->second->subtypeIndex != -1) {
        tempString = (i->second->subtypeSize == 1.0 || i->second->subtypeSize == -1) ? std::to_string(i->second->subtypeIndex) : std::to_string(i->second->subtypeIndex) + ':' + Math::toString(i->second->subtypeSize, 1);
        packetNode = doc->allocate_node(node_element, "subtypeIndex", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (!i->second->function1.empty()) {
        packetNode = doc->allocate_node(node_element, "function1", doc->allocate_string(i->second->function1.c_str(), i->second->function1.size() + 1));
        subnode->append_node(packetNode);
      }

      if (!i->second->function2.empty()) {
        packetNode = doc->allocate_node(node_element, "function2", doc->allocate_string(i->second->function2.c_str(), i->second->function2.size() + 1));
        subnode->append_node(packetNode);
      }

      if (!i->second->metaString1.empty()) {
        packetNode = doc->allocate_node(node_element, "metaString1", doc->allocate_string(i->second->metaString1.c_str(), i->second->metaString1.size() + 1));
        subnode->append_node(packetNode);
      }

      if (!i->second->metaString2.empty()) {
        packetNode = doc->allocate_node(node_element, "metaString2", doc->allocate_string(i->second->metaString2.c_str(), i->second->metaString2.size() + 1));
        subnode->append_node(packetNode);
      }

      if (i->second->responseType != -1) {
        tempString = std::to_string(i->second->responseType);
        packetNode = doc->allocate_node(node_element, "responseType", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (i->second->responseSubtype != -1) {
        tempString = std::to_string(i->second->responseSubtype);
        packetNode = doc->allocate_node(node_element, "responseSubtype", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (!i->second->responseTypeId.empty()) {
        packetNode = doc->allocate_node(node_element, "responseTypeId", doc->allocate_string(i->second->responseTypeId.c_str(), i->second->responseTypeId.size() + 1));
        subnode->append_node(packetNode);
      }

      if (!i->second->responses.empty()) {
        packetNode = doc->allocate_node(node_element, "responses");
        subnode->append_node(packetNode);

        for (auto &response : i->second->responses) {
          xml_node *responseNode = doc->allocate_node(node_element, "response");
          packetNode->append_node(responseNode);

          if (!response->responseId.empty()) {
            xml_node *responseElementNode = doc->allocate_node(node_element, "responseId", doc->allocate_string(response->responseId.c_str(), response->responseId.size() + 1));
            responseNode->append_node(responseElementNode);
          }

          if (response->conditionOperator != DevicePacketResponse::ConditionOperator::Enum::none) {
            if (response->conditionOperator == DevicePacketResponse::ConditionOperator::Enum::e) tempString = "e";
            else if (response->conditionOperator == DevicePacketResponse::ConditionOperator::Enum::g) tempString = "g";
            else if (response->conditionOperator == DevicePacketResponse::ConditionOperator::Enum::l) tempString = "l";
            else if (response->conditionOperator == DevicePacketResponse::ConditionOperator::Enum::ge) tempString = "ge";
            else if (response->conditionOperator == DevicePacketResponse::ConditionOperator::Enum::le) tempString = "le";
            xml_node *responseElementNode = doc->allocate_node(node_element, "conditionOperator", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            responseNode->append_node(responseElementNode);
          }

          if (!response->conditionParameterId.empty()) {
            xml_node *responseElementNode = doc->allocate_node(node_element, "conditionParameterId", doc->allocate_string(response->conditionParameterId.c_str(), response->conditionParameterId.size() + 1));
            responseNode->append_node(responseElementNode);
          }

          if (response->conditionChannel != -1) {
            tempString = std::to_string(response->conditionChannel);
            xml_node *responseElementNode = doc->allocate_node(node_element, "conditionChannel", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            responseNode->append_node(responseElementNode);
          }

          if (response->conditionValue != -1) {
            tempString = std::to_string(response->conditionValue);
            xml_node *responseElementNode = doc->allocate_node(node_element, "conditionValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            responseNode->append_node(responseElementNode);
          }
        }
      }

      if (i->second->channel != -1) {
        tempString = std::to_string(i->second->channel);
        packetNode = doc->allocate_node(node_element, "channel", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (i->second->channelIndex != -1) {
        tempString = i->second->channelSize == 1.0 ? std::to_string(i->second->channelIndex) : std::to_string(i->second->channelIndex) + ':' + Math::toString(i->second->channelSize, 1);
        packetNode = doc->allocate_node(node_element, "channelIndex", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (i->second->doubleSend) {
        tempString = "true";
        packetNode = doc->allocate_node(node_element, "doubleSend", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (i->second->splitAfter != -1) {
        tempString = std::to_string(i->second->splitAfter);
        packetNode = doc->allocate_node(node_element, "splitAfter", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (i->second->maxPackets != -1) {
        tempString = std::to_string(i->second->maxPackets);
        packetNode = doc->allocate_node(node_element, "maxPackets", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(packetNode);
      }

      if (!i->second->binaryPayloads.empty()) {
        packetNode = doc->allocate_node(node_element, "binaryPayload");
        subnode->append_node(packetNode);

        for (BinaryPayloads::iterator j = i->second->binaryPayloads.begin(); j != i->second->binaryPayloads.end(); ++j) {
          xml_node *payloadNode = doc->allocate_node(node_element, "element");
          packetNode->append_node(payloadNode);

          if ((*j)->index != 0) {
            tempString = Math::toString((*j)->index, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "index", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->size != 1.0) {
            tempString = Math::toString((*j)->size, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "size", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->index2 != 0) {
            tempString = Math::toString((*j)->index2, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "index2", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->size2 != 0) {
            tempString = Math::toString((*j)->size2, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "size2", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->index2Offset != -1) {
            tempString = std::to_string((*j)->index2Offset);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "index2Offset", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->bitIndex != 0) {
            tempString = Math::toString((*j)->bitIndex, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "bitIndex", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->bitSize != 0) {
            tempString = Math::toString((*j)->bitSize, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "bitSize", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->parameterChannel != -1) {
            tempString = Math::toString((*j)->parameterChannel, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "parameterChannel", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->metaInteger1 != -1) {
            tempString = Math::toString((*j)->metaInteger1, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "metaInteger1", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->metaInteger2 != -1) {
            tempString = Math::toString((*j)->metaInteger2, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "metaInteger2", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->metaInteger3 != -1) {
            tempString = Math::toString((*j)->metaInteger3, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "metaInteger3", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->metaInteger4 != -1) {
            tempString = Math::toString((*j)->metaInteger4, 1);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "metaInteger4", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if (!(*j)->parameterId.empty()) {
            xml_node *payloadElementNode = doc->allocate_node(node_element, "parameterId", doc->allocate_string((*j)->parameterId.c_str(), (*j)->parameterId.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueInteger != -1) {
            tempString = std::to_string((*j)->constValueInteger);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueInteger", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueDecimal != -1) {
            tempString = std::to_string((*j)->constValueDecimal);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueDecimal", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if (!(*j)->constValueString.empty()) {
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueString", doc->allocate_string((*j)->constValueString.c_str(), (*j)->constValueString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->isSigned) {
            tempString = "true";
            xml_node *payloadElementNode = doc->allocate_node(node_element, "isSigned", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->omitIfSet) {
            tempString = std::to_string((*j)->omitIf);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "omitIf", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }
        }
      }

      if (!i->second->jsonPayloads.empty()) {
        packetNode = doc->allocate_node(node_element, "jsonPayload");
        subnode->append_node(packetNode);

        for (JsonPayloads::iterator j = i->second->jsonPayloads.begin(); j != i->second->jsonPayloads.end(); ++j) {
          xml_node *payloadNode = doc->allocate_node(node_element, "element");
          packetNode->append_node(payloadNode);

          xml_node *payloadElementNode = doc->allocate_node(node_element, "key", doc->allocate_string((*j)->key.c_str(), (*j)->key.size() + 1));
          payloadNode->append_node(payloadElementNode);

          if (!(*j)->subkey.empty()) {
            payloadElementNode = doc->allocate_node(node_element, "subkey", doc->allocate_string((*j)->subkey.c_str(), (*j)->subkey.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if (!(*j)->subsubkey.empty()) {
            payloadElementNode = doc->allocate_node(node_element, "subsubkey", doc->allocate_string((*j)->subsubkey.c_str(), (*j)->subsubkey.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if (!(*j)->parameterId.empty()) {
            payloadElementNode = doc->allocate_node(node_element, "parameterId", doc->allocate_string((*j)->parameterId.c_str(), (*j)->parameterId.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueBooleanSet) {
            tempString = (*j)->constValueBoolean ? "true" : "false";
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueBoolean", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueIntegerSet) {
            tempString = std::to_string((*j)->constValueInteger);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueInteger", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueDecimalSet) {
            tempString = std::to_string((*j)->constValueDecimal);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueDecimal", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueStringSet) {
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueString", doc->allocate_string((*j)->constValueString.c_str(), (*j)->constValueString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }
        }
      }

      if (!i->second->httpPayloads.empty()) {
        packetNode = doc->allocate_node(node_element, "httpPayload");
        subnode->append_node(packetNode);

        for (HttpPayloads::iterator j = i->second->httpPayloads.begin(); j != i->second->httpPayloads.end(); ++j) {
          xml_node *payloadNode = doc->allocate_node(node_element, "element");
          packetNode->append_node(payloadNode);

          xml_node *payloadElementNode = doc->allocate_node(node_element, "key", doc->allocate_string((*j)->key.c_str(), (*j)->key.size() + 1));
          payloadNode->append_node(payloadElementNode);

          if (!(*j)->parameterId.empty()) {
            payloadElementNode = doc->allocate_node(node_element, "parameterId", doc->allocate_string((*j)->parameterId.c_str(), (*j)->parameterId.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueBooleanSet) {
            tempString = (*j)->constValueBoolean ? "true" : "false";
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueBoolean", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueIntegerSet) {
            tempString = std::to_string((*j)->constValueInteger);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueInteger", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueDecimalSet) {
            tempString = std::to_string((*j)->constValueDecimal);
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueDecimal", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }

          if ((*j)->constValueStringSet) {
            xml_node *payloadElementNode = doc->allocate_node(node_element, "constValueString", doc->allocate_string((*j)->constValueString.c_str(), (*j)->constValueString.size() + 1));
            payloadNode->append_node(payloadElementNode);
          }
        }
      }
    }
    /// }}}

    /// {{{ ParameterGroups
    node = doc->allocate_node(node_element, "parameterGroups");
    parentNode->append_node(node);

    for (std::map<std::string, PConfigParameters>::iterator i = configParameters.begin(); i != configParameters.end(); ++i) {
      xml_node *subnode = doc->allocate_node(node_element, "configParameters");
      node->append_node(subnode);

      xml_attribute *attr = doc->allocate_attribute("id", doc->allocate_string(i->first.c_str(), i->first.size() + 1));
      subnode->append_attribute(attr);

      if (i->second->memoryAddressStart != -1) {
        tempString = std::to_string(i->second->memoryAddressStart);
        attr = doc->allocate_attribute("memoryAddressStart", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      if (i->second->memoryAddressStep != -1) {
        tempString = std::to_string(i->second->memoryAddressStep);
        attr = doc->allocate_attribute("memoryAddressStep", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      for (std::vector<PParameter>::iterator j = i->second->parametersOrdered.begin(); j != i->second->parametersOrdered.end(); ++j) {
        xml_node *parameterNode = doc->allocate_node(node_element, "parameter");
        subnode->append_node(parameterNode);
        saveParameter(doc, parameterNode, (*j));
      }

      for (Scenarios::iterator j = i->second->scenarios.begin(); j != i->second->scenarios.end(); ++j) {
        xml_node *parameterNode = doc->allocate_node(node_element, "scenario");
        subnode->append_node(parameterNode);
        saveScenario(doc, parameterNode, j->second);
      }
    }

    for (std::map<std::string, PVariables>::iterator i = variables.begin(); i != variables.end(); ++i) {
      xml_node *subnode = doc->allocate_node(node_element, "variables");
      node->append_node(subnode);

      xml_attribute *attr = doc->allocate_attribute("id", doc->allocate_string(i->first.c_str(), i->first.size() + 1));
      subnode->append_attribute(attr);

      if (i->second->memoryAddressStart != -1) {
        tempString = std::to_string(i->second->memoryAddressStart);
        attr = doc->allocate_attribute("memoryAddressStart", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      if (i->second->memoryAddressStep != -1) {
        tempString = std::to_string(i->second->memoryAddressStep);
        attr = doc->allocate_attribute("memoryAddressStep", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      for (std::vector<PParameter>::iterator j = i->second->parametersOrdered.begin(); j != i->second->parametersOrdered.end(); ++j) {
        xml_node *parameterNode = doc->allocate_node(node_element, "parameter");
        subnode->append_node(parameterNode);
        saveParameter(doc, parameterNode, (*j));
      }

      for (Scenarios::iterator j = i->second->scenarios.begin(); j != i->second->scenarios.end(); ++j) {
        xml_node *parameterNode = doc->allocate_node(node_element, "scenario");
        subnode->append_node(parameterNode);
        saveScenario(doc, parameterNode, j->second);
      }
    }

    for (std::map<std::string, PLinkParameters>::iterator i = linkParameters.begin(); i != linkParameters.end(); ++i) {
      xml_node *subnode = doc->allocate_node(node_element, "linkParameters");
      node->append_node(subnode);

      xml_attribute *attr = doc->allocate_attribute("id", doc->allocate_string(i->first.c_str(), i->first.size() + 1));
      subnode->append_attribute(attr);

      if (i->second->memoryAddressStart != -1) {
        tempString = std::to_string(i->second->memoryAddressStart);
        attr = doc->allocate_attribute("memoryAddressStart", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      if (i->second->memoryAddressStep != -1) {
        tempString = std::to_string(i->second->memoryAddressStep);
        attr = doc->allocate_attribute("memoryAddressStep", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      if (i->second->peerChannelMemoryOffset != -1) {
        tempString = std::to_string(i->second->peerChannelMemoryOffset);
        attr = doc->allocate_attribute("peerChannelMemoryOffset", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      if (i->second->channelMemoryOffset != -1) {
        tempString = std::to_string(i->second->channelMemoryOffset);
        attr = doc->allocate_attribute("channelMemoryOffset", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      if (i->second->peerAddressMemoryOffset != -1) {
        tempString = std::to_string(i->second->peerAddressMemoryOffset);
        attr = doc->allocate_attribute("peerAddressMemoryOffset", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      if (i->second->maxLinkCount != -1) {
        tempString = std::to_string(i->second->maxLinkCount);
        attr = doc->allocate_attribute("maxLinkCount", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_attribute(attr);
      }

      for (std::vector<PParameter>::iterator j = i->second->parametersOrdered.begin(); j != i->second->parametersOrdered.end(); ++j) {
        xml_node *parameterNode = doc->allocate_node(node_element, "parameter");
        subnode->append_node(parameterNode);
        saveParameter(doc, parameterNode, (*j));
      }

      for (Scenarios::iterator j = i->second->scenarios.begin(); j != i->second->scenarios.end(); ++j) {
        xml_node *parameterNode = doc->allocate_node(node_element, "scenario");
        subnode->append_node(parameterNode);
        saveScenario(doc, parameterNode, j->second);
      }
    }
    /// }}}


    if (device->group) {
      node = doc->allocate_node(node_element, "group");
      parentNode->append_node(node);
      saveDevice(doc, node, device->group.get());
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void HomegearDevice::saveParameter(xml_document *doc, xml_node *parentNode, PParameter &parameter) {
  try {
    std::string tempString;
    xml_attribute *attr = doc->allocate_attribute("id", doc->allocate_string(parameter->id.c_str(), parameter->id.size() + 1));
    parentNode->append_attribute(attr);

    // {{{ Properties
    xml_node *propertiesNode = doc->allocate_node(node_element, "properties");
    parentNode->append_node(propertiesNode);

    if (!parameter->readable) {
      tempString = "false";
      xml_node *node = doc->allocate_node(node_element, "readable", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->writeable) {
      tempString = "false";
      xml_node *node = doc->allocate_node(node_element, "writeable", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->readOnInit) {
      tempString = "true";
      xml_node *node = doc->allocate_node(node_element, "readOnInit", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->transmitted) {
      tempString = "false";
      xml_node *node = doc->allocate_node(node_element, "transmitted", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->addonWriteable) {
      tempString = "false";
      xml_node *node = doc->allocate_node(node_element, "addonWriteable", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->password) {
      tempString = "true";
      xml_node *node = doc->allocate_node(node_element, "password", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->visible) {
      tempString = "false";
      xml_node *node = doc->allocate_node(node_element, "visible", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->internal) {
      tempString = "true";
      xml_node *node = doc->allocate_node(node_element, "internal", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->parameterGroupSelector) {
      tempString = "true";
      xml_node *node = doc->allocate_node(node_element, "parameterGroupSelector", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->service) {
      tempString = "true";
      xml_node *node = doc->allocate_node(node_element, "service", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->serviceInverted) {
      tempString = "true";
      xml_node *node = doc->allocate_node(node_element, "serviceInverted", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->sticky) {
      tempString = "true";
      xml_node *node = doc->allocate_node(node_element, "sticky", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->transform) {
      tempString = "true";
      xml_node *node = doc->allocate_node(node_element, "transform", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->isSigned) {
      tempString = "true";
      xml_node *node = doc->allocate_node(node_element, "signed", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->control.empty()) {
      xml_node *node = doc->allocate_node(node_element, "control", doc->allocate_string(parameter->control.c_str(), parameter->control.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->unit.empty()) {
      xml_node *node = doc->allocate_node(node_element, "unit", doc->allocate_string(parameter->unit.c_str(), parameter->unit.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->unit_code != UnitCode::kUndefined) {
      tempString = std::to_string((int32_t)parameter->unit_code);
      xml_node *node = doc->allocate_node(node_element, "unitCode", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->formFieldType.empty()) {
      xml_node *node = doc->allocate_node(node_element, "formFieldType", doc->allocate_string(parameter->formFieldType.c_str(), parameter->formFieldType.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->formPosition != -1) {
      tempString = std::to_string(parameter->formPosition);
      xml_node *node = doc->allocate_node(node_element, "formPosition", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->metadata.empty()) {
      xml_node *node = doc->allocate_node(node_element, "metadata", doc->allocate_string(parameter->metadata.c_str(), parameter->metadata.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->ccu2Visible) {
      tempString = "false";
      xml_node *node = doc->allocate_node(node_element, "ccu2Visible", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (parameter->priority != -1) {
      tempString = std::to_string(parameter->priority);
      xml_node *node = doc->allocate_node(node_element, "priority", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      propertiesNode->append_node(node);
    }

    if (!parameter->casts.empty()) {
      xml_node *node = doc->allocate_node(node_element, "casts");
      propertiesNode->append_node(node);
      for (Casts::iterator i = parameter->casts.begin(); i != parameter->casts.end(); ++i) {
        {
          PDecimalIntegerScale decimalIntegerScale;
          decimalIntegerScale = std::dynamic_pointer_cast<DecimalIntegerScale>(*i);
          if (decimalIntegerScale) {
            xml_node *castNode = doc->allocate_node(node_element, "decimalIntegerScale");
            node->append_node(castNode);
            if (decimalIntegerScale->factor != 0) {
              tempString = Math::toString(decimalIntegerScale->factor, 6);
              xml_node *subnode = doc->allocate_node(node_element, "factor", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (decimalIntegerScale->offset != 0) {
              tempString = Math::toString(decimalIntegerScale->offset, 6);
              xml_node *subnode = doc->allocate_node(node_element, "offset", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }
            continue;
          }
        }

        {
          PDecimalStringScale decimalStringScale;
          decimalStringScale = std::dynamic_pointer_cast<DecimalStringScale>(*i);
          if (decimalStringScale) {
            xml_node *castNode = doc->allocate_node(node_element, "decimalStringScale");
            node->append_node(castNode);
            if (decimalStringScale->factor != 0) {
              tempString = Math::toString(decimalStringScale->factor, 6);
              xml_node *subnode = doc->allocate_node(node_element, "factor", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            continue;
          }
        }

        {
          PIntegerIntegerScale integerIntegerScale;
          integerIntegerScale = std::dynamic_pointer_cast<IntegerIntegerScale>(*i);
          if (integerIntegerScale) {
            xml_node *castNode = doc->allocate_node(node_element, "integerIntegerScale");
            node->append_node(castNode);
            if (integerIntegerScale->operation != IntegerIntegerScale::Operation::Enum::none) {
              tempString = integerIntegerScale->operation == IntegerIntegerScale::Operation::Enum::division ? "division" : "multiplication";
              xml_node *subnode = doc->allocate_node(node_element, "operation", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (integerIntegerScale->factor != 10) {
              tempString = std::to_string(integerIntegerScale->factor);
              xml_node *subnode = doc->allocate_node(node_element, "factor", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (integerIntegerScale->offset != 0) {
              tempString = std::to_string(integerIntegerScale->offset);
              xml_node *subnode = doc->allocate_node(node_element, "offset", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }
            continue;
          }
        }

        {
          PIntegerOffset integerOffset;
          integerOffset = std::dynamic_pointer_cast<IntegerOffset>(*i);
          if (integerOffset) {
            xml_node *castNode = doc->allocate_node(node_element, "integerOffset");
            node->append_node(castNode);

            if (!integerOffset->directionToPacket) {
              xml_node *subnode = doc->allocate_node(node_element, "direction", doc->allocate_string("fromPacket", 11));
              castNode->append_node(subnode);
            }

            tempString = std::to_string(integerOffset->offset);
            xml_node *subnode = doc->allocate_node(node_element, integerOffset->addOffset ? "addOffset" : "subtractFromOffset", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            castNode->append_node(subnode);
            continue;
          }
        }

        {
          PDecimalOffset decimalOffset;
          decimalOffset = std::dynamic_pointer_cast<DecimalOffset>(*i);
          if (decimalOffset) {
            xml_node *castNode = doc->allocate_node(node_element, "decimalOffset");
            node->append_node(castNode);

            if (!decimalOffset->directionToPacket) {
              xml_node *subnode = doc->allocate_node(node_element, "direction", doc->allocate_string("fromPacket", 11));
              castNode->append_node(subnode);
            }

            tempString = std::to_string(decimalOffset->offset);
            xml_node *subnode = doc->allocate_node(node_element, decimalOffset->addOffset ? "addOffset" : "subtractFromOffset", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            castNode->append_node(subnode);
            continue;
          }
        }

        {
          PIntegerIntegerMap integerIntegerMap;
          integerIntegerMap = std::dynamic_pointer_cast<IntegerIntegerMap>(*i);
          if (integerIntegerMap) {
            xml_node *castNode = doc->allocate_node(node_element, "integerIntegerMap");
            node->append_node(castNode);
            if (integerIntegerMap->direction != IntegerIntegerMap::Direction::Enum::none) {
              tempString = integerIntegerMap->direction == IntegerIntegerMap::Direction::Enum::fromDevice ? "fromDevice" : (integerIntegerMap->direction == IntegerIntegerMap::Direction::Enum::toDevice ? "toDevice" : "both");
              xml_node *subnode = doc->allocate_node(node_element, "direction", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }
            for (std::map<int32_t, int32_t>::iterator j = integerIntegerMap->integerValueMapFromDevice.begin(); j != integerIntegerMap->integerValueMapFromDevice.end(); ++j) {
              xml_node *subnode = doc->allocate_node(node_element, "value");
              castNode->append_node(subnode);

              tempString = std::to_string(j->first);
              xml_node *valueNode = doc->allocate_node(node_element, "physical", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              subnode->append_node(valueNode);

              tempString = std::to_string(j->second);
              valueNode = doc->allocate_node(node_element, "logical", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              subnode->append_node(valueNode);
            }
            continue;
          }
        }

        {
          PBooleanInteger booleanInteger;
          booleanInteger = std::dynamic_pointer_cast<BooleanInteger>(*i);
          if (booleanInteger) {
            xml_node *castNode = doc->allocate_node(node_element, "booleanInteger");
            node->append_node(castNode);

            if (booleanInteger->trueValue != 0) {
              tempString = std::to_string(booleanInteger->trueValue);
              xml_node *subnode = doc->allocate_node(node_element, "trueValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (booleanInteger->falseValue != 0) {
              tempString = std::to_string(booleanInteger->falseValue);
              xml_node *subnode = doc->allocate_node(node_element, "falseValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (booleanInteger->invert) {
              tempString = "true";
              xml_node *subnode = doc->allocate_node(node_element, "invert", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (booleanInteger->threshold != 1) {
              tempString = std::to_string(booleanInteger->threshold);
              xml_node *subnode = doc->allocate_node(node_element, "threshold", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }
            continue;
          }
        }

        {
          PBooleanDecimal booleanDecimal;
          booleanDecimal = std::dynamic_pointer_cast<BooleanDecimal>(*i);
          if (booleanDecimal) {
            xml_node *castNode = doc->allocate_node(node_element, "booleanDecimal");
            node->append_node(castNode);

            if (booleanDecimal->trueValue != 0) {
              tempString = std::to_string(booleanDecimal->trueValue);
              xml_node *subnode = doc->allocate_node(node_element, "trueValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (booleanDecimal->falseValue != 0) {
              tempString = std::to_string(booleanDecimal->falseValue);
              xml_node *subnode = doc->allocate_node(node_element, "falseValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (booleanDecimal->invert) {
              tempString = "true";
              xml_node *subnode = doc->allocate_node(node_element, "invert", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (booleanDecimal->threshold != 1) {
              tempString = std::to_string(booleanDecimal->threshold);
              xml_node *subnode = doc->allocate_node(node_element, "threshold", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }
            continue;
          }
        }

        {
          PBooleanString booleanString;
          booleanString = std::dynamic_pointer_cast<BooleanString>(*i);
          if (booleanString) {
            xml_node *castNode = doc->allocate_node(node_element, "booleanString");
            node->append_node(castNode);

            if (!booleanString->trueValue.empty()) {
              xml_node *subnode = doc->allocate_node(node_element, "trueValue", doc->allocate_string(booleanString->trueValue.c_str(), booleanString->trueValue.size() + 1));
              castNode->append_node(subnode);
            }

            if (!booleanString->falseValue.empty()) {
              xml_node *subnode = doc->allocate_node(node_element, "falseValue", doc->allocate_string(booleanString->falseValue.c_str(), booleanString->falseValue.size() + 1));
              castNode->append_node(subnode);
            }

            if (booleanString->invert) {
              tempString = "true";
              xml_node *subnode = doc->allocate_node(node_element, "invert", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            continue;
          }
        }

        {
          PDecimalConfigTime decimalConfigTime;
          decimalConfigTime = std::dynamic_pointer_cast<DecimalConfigTime>(*i);
          if (decimalConfigTime) {
            xml_node *castNode = doc->allocate_node(node_element, "decimalConfigTime");
            node->append_node(castNode);

            for (std::vector<double>::iterator j = decimalConfigTime->factors.begin(); j != decimalConfigTime->factors.end(); ++j) {
              xml_node *subnode = doc->allocate_node(node_element, "factors");
              castNode->append_node(subnode);

              tempString = std::to_string(*j);
              xml_node *factorNode = doc->allocate_node(node_element, "factor", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              subnode->append_node(factorNode);
            }

            if (decimalConfigTime->valueSize != 0) {
              tempString = Math::toString(decimalConfigTime->valueSize, 6);
              xml_node *subnode = doc->allocate_node(node_element, "valueSize", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }
            continue;
          }
        }

        {
          PIntegerTinyFloat integerTinyFloat;
          integerTinyFloat = std::dynamic_pointer_cast<IntegerTinyFloat>(*i);
          if (integerTinyFloat) {
            xml_node *castNode = doc->allocate_node(node_element, "integerTinyFloat");
            node->append_node(castNode);

            if (integerTinyFloat->mantissaStart != 5) {
              tempString = std::to_string(integerTinyFloat->mantissaStart);
              xml_node *subnode = doc->allocate_node(node_element, "mantissaStart", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (integerTinyFloat->mantissaSize != 11) {
              tempString = std::to_string(integerTinyFloat->mantissaSize);
              xml_node *subnode = doc->allocate_node(node_element, "mantissaSize", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (integerTinyFloat->exponentStart != 0) {
              tempString = std::to_string(integerTinyFloat->exponentStart);
              xml_node *subnode = doc->allocate_node(node_element, "exponentStart", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (integerTinyFloat->exponentSize != 5) {
              tempString = std::to_string(integerTinyFloat->exponentSize);
              xml_node *subnode = doc->allocate_node(node_element, "exponentSize", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }
            continue;
          }
        }

        {
          PStringUnsignedInteger stringUnsignedInteger;
          stringUnsignedInteger = std::dynamic_pointer_cast<StringUnsignedInteger>(*i);
          if (stringUnsignedInteger) {
            xml_node *castNode = doc->allocate_node(node_element, "stringUnsignedInteger");
            node->append_node(castNode);

            continue;
          }
        }

        {
          PBlindTest blindTest;
          blindTest = std::dynamic_pointer_cast<BlindTest>(*i);
          if (blindTest) {
            xml_node *castNode = doc->allocate_node(node_element, "blindTest");
            node->append_node(castNode);

            if (blindTest->value != 0) {
              tempString = std::to_string(blindTest->value);
              xml_node *subnode = doc->allocate_node(node_element, "value", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            continue;
          }
        }

        {
          POptionString optionString;
          optionString = std::dynamic_pointer_cast<OptionString>(*i);
          if (optionString) {
            xml_node *castNode = doc->allocate_node(node_element, "optionString");
            node->append_node(castNode);

            continue;
          }
        }

        {
          POptionInteger optionInteger;
          optionInteger = std::dynamic_pointer_cast<OptionInteger>(*i);
          if (optionInteger) {
            xml_node *castNode = doc->allocate_node(node_element, "optionInteger");
            node->append_node(castNode);
            for (std::map<int32_t, int32_t>::iterator j = optionInteger->valueMapFromDevice.begin(); j != optionInteger->valueMapFromDevice.end(); ++j) {
              xml_node *subnode = doc->allocate_node(node_element, "value");
              castNode->append_node(subnode);

              tempString = std::to_string(j->first);
              xml_node *valueNode = doc->allocate_node(node_element, "physical", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              subnode->append_node(valueNode);

              tempString = std::to_string(j->second);
              valueNode = doc->allocate_node(node_element, "logical", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              subnode->append_node(valueNode);
            }
            continue;
          }
        }

        {
          PStringJsonArrayDecimal stringJsonArrayDecimal;
          stringJsonArrayDecimal = std::dynamic_pointer_cast<StringJsonArrayDecimal>(*i);
          if (stringJsonArrayDecimal) {
            xml_node *castNode = doc->allocate_node(node_element, "stringJsonArrayDecimal");
            node->append_node(castNode);

            continue;
          }
        }

        {
          PRpcBinary rpcBinary;
          rpcBinary = std::dynamic_pointer_cast<RpcBinary>(*i);
          if (rpcBinary) {
            xml_node *castNode = doc->allocate_node(node_element, "rpcBinary");
            node->append_node(castNode);

            continue;
          }
        }

        {
          PToggle toggleCast;
          toggleCast = std::dynamic_pointer_cast<Toggle>(*i);
          if (toggleCast) {
            xml_node *castNode = doc->allocate_node(node_element, "toggle");
            node->append_node(castNode);

            xml_node *subnode = doc->allocate_node(node_element, "parameter", doc->allocate_string(toggleCast->parameter.c_str(), toggleCast->parameter.size() + 1));
            castNode->append_node(subnode);

            if (toggleCast->on != 200) {
              tempString = std::to_string(toggleCast->on);
              subnode = doc->allocate_node(node_element, "on", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }

            if (toggleCast->off != 0) {
              tempString = std::to_string(toggleCast->off);
              subnode = doc->allocate_node(node_element, "off", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
              castNode->append_node(subnode);
            }
            continue;
          }
        }

        {
          PCfm cfm;
          cfm = std::dynamic_pointer_cast<Cfm>(*i);
          if (cfm) {
            xml_node *castNode = doc->allocate_node(node_element, "cfm");
            node->append_node(castNode);

            continue;
          }
        }

        {
          PCcrtdnParty ccrtdnParty;
          ccrtdnParty = std::dynamic_pointer_cast<CcrtdnParty>(*i);
          if (ccrtdnParty) {
            xml_node *castNode = doc->allocate_node(node_element, "ccrtdnParty");
            node->append_node(castNode);

            continue;
          }
        }

        {
          PStringReplace stringReplace;
          stringReplace = std::dynamic_pointer_cast<StringReplace>(*i);
          if (stringReplace) {
            xml_node *castNode = doc->allocate_node(node_element, "stringReplace");
            node->append_node(castNode);

            if (!stringReplace->search.empty()) {
              xml_node *subnode = doc->allocate_node(node_element, "search", doc->allocate_string(stringReplace->search.c_str(), stringReplace->search.size() + 1));
              castNode->append_node(subnode);
            }

            if (!stringReplace->replace.empty()) {
              xml_node *subnode = doc->allocate_node(node_element, "replace", doc->allocate_string(stringReplace->replace.c_str(), stringReplace->replace.size() + 1));
              castNode->append_node(subnode);
            }

            continue;
          }
        }

        {
          PHexStringByteArray hexStringByteArray;
          hexStringByteArray = std::dynamic_pointer_cast<HexStringByteArray>(*i);
          if (hexStringByteArray) {
            xml_node *castNode = doc->allocate_node(node_element, "hexStringByteArray");
            node->append_node(castNode);
            continue;
          }
        }

        {
          PTimeStringSeconds timeStringSeconds;
          timeStringSeconds = std::dynamic_pointer_cast<TimeStringSeconds>(*i);
          if (timeStringSeconds) {
            xml_node *castNode = doc->allocate_node(node_element, "timeStringSeconds");
            node->append_node(castNode);
            continue;
          }
        }

        {
          PInvert invert;
          invert = std::dynamic_pointer_cast<Invert>(*i);
          if (invert) {
            xml_node *castNode = doc->allocate_node(node_element, "invert");
            node->append_node(castNode);
            continue;
          }
        }

        {
          PRound round;
          round = std::dynamic_pointer_cast<Round>(*i);
          if (round) {
            xml_node *castNode = doc->allocate_node(node_element, "round");
            node->append_node(castNode);

            tempString = std::to_string(round->decimalPlaces);
            xml_node *subnode = doc->allocate_node(node_element, "decimalPlaces", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            castNode->append_node(subnode);
            continue;
          }
        }

        {
          PGeneric generic;
          generic = std::dynamic_pointer_cast<Generic>(*i);
          if (generic) {
            xml_node *castNode = doc->allocate_node(node_element, "generic");
            node->append_node(castNode);

            if (!generic->type.empty()) castNode->append_attribute(doc->allocate_attribute("type", doc->allocate_string(generic->type.c_str(), generic->type.size() + 1)));

            continue;
          }
        }
      }
    }

    if (!parameter->roles.empty()) {
      xml_node *node = doc->allocate_node(node_element, "roles");
      propertiesNode->append_node(node);
      for (auto &role : parameter->roles) {
        tempString = std::to_string(role.first);
        xml_node *roleNode = doc->allocate_node(node_element, "role", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(roleNode);

        if (role.second.direction != RoleDirection::both) {
          tempString = std::to_string((int32_t)role.second.direction);
          attr = doc->allocate_attribute("direction", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          roleNode->append_attribute(attr);
        }
        if (role.second.invert) {
          tempString = "true";
          attr = doc->allocate_attribute("invert", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          roleNode->append_attribute(attr);
        }
        if (role.second.scale) {
          if (role.second.scaleInfo.valueSet) {
            tempString = std::to_string(role.second.scaleInfo.valueMin);
            attr = doc->allocate_attribute("valueMin", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            roleNode->append_attribute(attr);

            tempString = std::to_string(role.second.scaleInfo.valueMax);
            attr = doc->allocate_attribute("valueMax", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            roleNode->append_attribute(attr);
          }

          tempString = std::to_string(role.second.scaleInfo.scaleMin);
          attr = doc->allocate_attribute("scaleMin", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          roleNode->append_attribute(attr);

          tempString = std::to_string(role.second.scaleInfo.scaleMax);
          attr = doc->allocate_attribute("scaleMax", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          roleNode->append_attribute(attr);
        }
      }
    }
    // }}}

    // {{{ Logical
    if (parameter->logical->type == ILogical::Type::Enum::tBoolean) {
      xml_node *node = doc->allocate_node(node_element, "logicalBoolean");
      parentNode->append_node(node);

      if (parameter->logical->defaultValueExists) {
        tempString = parameter->logical->getDefaultValue()->booleanValue ? "true" : "false";
        xml_node *subnode = doc->allocate_node(node_element, "defaultValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (parameter->logical->setToValueOnPairingExists) {
        tempString = parameter->logical->getSetToValueOnPairing()->booleanValue ? "true" : "false";
        xml_node *subnode = doc->allocate_node(node_element, "setToValueOnPairing", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }
    } else if (parameter->logical->type == ILogical::Type::Enum::tAction) {
      xml_node *node = doc->allocate_node(node_element, "logicalAction");
      parentNode->append_node(node);
    } else if (parameter->logical->type == ILogical::Type::Enum::tInteger) {
      xml_node *node = doc->allocate_node(node_element, "logicalInteger");
      parentNode->append_node(node);

      LogicalInteger *logical = (LogicalInteger *)parameter->logical.get();

      if (logical->minimumValue != -2147483648) {
        tempString = std::to_string(logical->minimumValue);
        xml_node *subnode = doc->allocate_node(node_element, "minimumValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->maximumValue != 2147483647) {
        tempString = std::to_string(logical->maximumValue);
        xml_node *subnode = doc->allocate_node(node_element, "maximumValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->defaultValueExists) {
        tempString = std::to_string(logical->getDefaultValue()->integerValue);
        xml_node *subnode = doc->allocate_node(node_element, "defaultValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->setToValueOnPairingExists) {
        tempString = std::to_string(logical->getSetToValueOnPairing()->integerValue);
        xml_node *subnode = doc->allocate_node(node_element, "setToValueOnPairing", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (!logical->specialValuesStringMap.empty()) {
        xml_node *subnode = doc->allocate_node(node_element, "specialValues", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);

        for (std::unordered_map<std::string, int32_t>::iterator i = logical->specialValuesStringMap.begin(); i != logical->specialValuesStringMap.end(); ++i) {
          tempString = std::to_string(i->second);
          xml_node *specialValueNode = doc->allocate_node(node_element, "specialValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          subnode->append_node(specialValueNode);

          attr = doc->allocate_attribute("id", doc->allocate_string(i->first.c_str(), i->first.size() + 1));
          specialValueNode->append_attribute(attr);
        }
      }
    } else if (parameter->logical->type == ILogical::Type::Enum::tInteger64) {
      xml_node *node = doc->allocate_node(node_element, "logicalInteger64");
      parentNode->append_node(node);

      LogicalInteger64 *logical = (LogicalInteger64 *)parameter->logical.get();

      if (logical->minimumValue != (signed)0x8000000000000000ll) {
        tempString = std::to_string(logical->minimumValue);
        xml_node *subnode = doc->allocate_node(node_element, "minimumValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->maximumValue != (signed)0x7FFFFFFFFFFFFFFFll) {
        tempString = std::to_string(logical->maximumValue);
        xml_node *subnode = doc->allocate_node(node_element, "maximumValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->defaultValueExists) {
        tempString = std::to_string(logical->getDefaultValue()->integerValue64);
        xml_node *subnode = doc->allocate_node(node_element, "defaultValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->setToValueOnPairingExists) {
        tempString = std::to_string(logical->getSetToValueOnPairing()->integerValue64);
        xml_node *subnode = doc->allocate_node(node_element, "setToValueOnPairing", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (!logical->specialValuesStringMap.empty()) {
        xml_node *subnode = doc->allocate_node(node_element, "specialValues", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);

        for (std::unordered_map<std::string, int64_t>::iterator i = logical->specialValuesStringMap.begin(); i != logical->specialValuesStringMap.end(); ++i) {
          tempString = std::to_string(i->second);
          xml_node *specialValueNode = doc->allocate_node(node_element, "specialValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          subnode->append_node(specialValueNode);

          attr = doc->allocate_attribute("id", doc->allocate_string(i->first.c_str(), i->first.size() + 1));
          specialValueNode->append_attribute(attr);
        }
      }
    } else if (parameter->logical->type == ILogical::Type::Enum::tFloat) {
      xml_node *node = doc->allocate_node(node_element, "logicalFloat");
      parentNode->append_node(node);

      LogicalDecimal *logical = (LogicalDecimal *)parameter->logical.get();

      if (logical->minimumValue != 1.175494351e-38f) {
        tempString = std::to_string(logical->minimumValue);
        xml_node *subnode = doc->allocate_node(node_element, "minimumValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->maximumValue != 3.40282347e+38f) {
        tempString = std::to_string(logical->maximumValue);
        xml_node *subnode = doc->allocate_node(node_element, "maximumValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->defaultValueExists) {
        tempString = std::to_string(logical->getDefaultValue()->floatValue);
        xml_node *subnode = doc->allocate_node(node_element, "defaultValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->setToValueOnPairingExists) {
        tempString = std::to_string(logical->getSetToValueOnPairing()->floatValue);
        xml_node *subnode = doc->allocate_node(node_element, "setToValueOnPairing", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (!logical->specialValuesStringMap.empty()) {
        xml_node *subnode = doc->allocate_node(node_element, "specialValues", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);

        for (std::unordered_map<std::string, double>::iterator i = logical->specialValuesStringMap.begin(); i != logical->specialValuesStringMap.end(); ++i) {
          tempString = std::to_string(i->second);
          xml_node *specialValueNode = doc->allocate_node(node_element, "specialValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          subnode->append_node(specialValueNode);

          attr = doc->allocate_attribute("id", doc->allocate_string(i->first.c_str(), i->first.size() + 1));
          specialValueNode->append_attribute(attr);
        }
      }
    } else if (parameter->logical->type == ILogical::Type::Enum::tString) {
      xml_node *node = doc->allocate_node(node_element, "logicalString");
      parentNode->append_node(node);

      if (parameter->logical->defaultValueExists) {
        xml_node *subnode = doc->allocate_node(node_element, "defaultValue", doc->allocate_string(parameter->logical->getDefaultValue()->stringValue.c_str(), parameter->logical->getDefaultValue()->stringValue.size() + 1));
        node->append_node(subnode);
      }

      if (parameter->logical->setToValueOnPairingExists) {
        xml_node *subnode = doc->allocate_node(node_element, "setToValueOnPairing", doc->allocate_string(parameter->logical->getSetToValueOnPairing()->stringValue.c_str(), parameter->logical->getSetToValueOnPairing()->stringValue.size() + 1));
        node->append_node(subnode);
      }
    } else if (parameter->logical->type == ILogical::Type::Enum::tEnum) {
      xml_node *node = doc->allocate_node(node_element, "logicalEnumeration");
      parentNode->append_node(node);

      LogicalEnumeration *logical = (LogicalEnumeration *)parameter->logical.get();

      if (logical->defaultValueExists) {
        tempString = std::to_string(logical->getDefaultValue()->integerValue);
        xml_node *subnode = doc->allocate_node(node_element, "defaultValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      if (logical->setToValueOnPairingExists) {
        tempString = std::to_string(logical->getSetToValueOnPairing()->integerValue);
        xml_node *subnode = doc->allocate_node(node_element, "setToValueOnPairing", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);
      }

      for (std::vector<EnumerationValue>::iterator i = logical->values.begin(); i != logical->values.end(); ++i) {
        xml_node *subnode = doc->allocate_node(node_element, "value", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        node->append_node(subnode);

        xml_node *valueNode = doc->allocate_node(node_element, "id", doc->allocate_string(i->id.c_str(), i->id.size() + 1));
        subnode->append_node(valueNode);

        tempString = std::to_string(i->index);
        valueNode = doc->allocate_node(node_element, "index", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        subnode->append_node(valueNode);
      }
    } else if (parameter->logical->type == ILogical::Type::Enum::tArray) {
      xml_node *node = doc->allocate_node(node_element, "logicalArray");
      parentNode->append_node(node);
    } else if (parameter->logical->type == ILogical::Type::Enum::tStruct) {
      xml_node *node = doc->allocate_node(node_element, "logicalStruct");
      parentNode->append_node(node);
    }
    // }}}

    // {{{ Physical
    {
      xml_node *node = nullptr;
      if (parameter->physical->type == IPhysical::Type::Enum::tInteger) {
        node = doc->allocate_node(node_element, "physicalInteger");
        parentNode->append_node(node);
      } else if (parameter->physical->type == IPhysical::Type::Enum::tBoolean) {
        node = doc->allocate_node(node_element, "physicalBoolean");
        parentNode->append_node(node);
      } else if (parameter->physical->type == IPhysical::Type::Enum::tString) {
        node = doc->allocate_node(node_element, "physicalString");
        parentNode->append_node(node);
      } else if (parameter->physical->type == IPhysical::Type::Enum::none) {
        node = doc->allocate_node(node_element, "physical");
        parentNode->append_node(node);
      }

      if (node) {
        if (!parameter->physical->groupId.empty()) {
          attr = doc->allocate_attribute("groupId", doc->allocate_string(parameter->physical->groupId.c_str(), parameter->physical->groupId.size() + 1));
          node->append_attribute(attr);
        }

        if (parameter->physical->index != 0) {
          tempString = Math::toString(parameter->physical->index, 1);
          xml_node *subnode = doc->allocate_node(node_element, "index", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->sizeDefined) {
          tempString = Math::toString(parameter->physical->size, 1);
          xml_node *subnode = doc->allocate_node(node_element, "size", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->bitSize >= 0) {
          tempString = Math::toString(parameter->physical->bitSize);
          xml_node *subnode = doc->allocate_node(node_element, "bitSize", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->mask != -1) {
          tempString = std::to_string(parameter->physical->mask);
          xml_node *subnode = doc->allocate_node(node_element, "mask", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->list != -1) {
          tempString = std::to_string(parameter->physical->list);
          xml_node *subnode = doc->allocate_node(node_element, "list", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->operationType != IPhysical::OperationType::Enum::none) {
          if (parameter->physical->operationType == IPhysical::OperationType::Enum::command) tempString = "command";
          else if (parameter->physical->operationType == IPhysical::OperationType::Enum::centralCommand) tempString = "centralCommand";
          else if (parameter->physical->operationType == IPhysical::OperationType::Enum::internal) tempString = "internal";
          else if (parameter->physical->operationType == IPhysical::OperationType::Enum::config) tempString = "config";
          else if (parameter->physical->operationType == IPhysical::OperationType::Enum::configString) tempString = "configString";
          else if (parameter->physical->operationType == IPhysical::OperationType::Enum::store) tempString = "store";
          else if (parameter->physical->operationType == IPhysical::OperationType::Enum::memory) tempString = "memory";
          xml_node *subnode = doc->allocate_node(node_element, "operationType", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->endianess != IPhysical::Endianess::Enum::big) {
          tempString = "little";
          xml_node *subnode = doc->allocate_node(node_element, "endianess", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->memoryIndex != 0) {
          tempString = Math::toString(parameter->physical->memoryIndex, 1);
          xml_node *subnode = doc->allocate_node(node_element, "memoryIndex", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->memoryIndexOperation != IPhysical::MemoryIndexOperation::Enum::none) {
          tempString = parameter->physical->memoryIndexOperation == IPhysical::MemoryIndexOperation::Enum::addition ? "addition" : "subtraction";
          xml_node *subnode = doc->allocate_node(node_element, "memoryIndexOperation", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->memoryChannelStep != 0) {
          tempString = Math::toString(parameter->physical->memoryChannelStep, 1);
          xml_node *subnode = doc->allocate_node(node_element, "memoryChannelStep", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }

        if (parameter->physical->address != 0) {
          tempString = Math::toString(parameter->physical->address);
          xml_node *subnode = doc->allocate_node(node_element, "address", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
          node->append_node(subnode);
        }
      }
    }
    //}}}

    // {{{ Packets
    if (!parameter->getPackets.empty() || !parameter->setPackets.empty() || !parameter->eventPackets.empty()) {
      xml_node *node = doc->allocate_node(node_element, "packets");
      parentNode->append_node(node);

      for (std::vector<std::shared_ptr<Parameter::Packet>>::iterator i = parameter->getPackets.begin(); i != parameter->getPackets.end(); ++i) {
        saveParameterPacket(doc, node, *i);
      }

      for (std::vector<std::shared_ptr<Parameter::Packet>>::iterator i = parameter->setPackets.begin(); i != parameter->setPackets.end(); ++i) {
        saveParameterPacket(doc, node, *i);
      }

      for (std::vector<std::shared_ptr<Parameter::Packet>>::iterator i = parameter->eventPackets.begin(); i != parameter->eventPackets.end(); ++i) {
        saveParameterPacket(doc, node, *i);
      }
    }
    // }}}
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void HomegearDevice::saveParameterPacket(xml_document *doc, xml_node *parentNode, std::shared_ptr<Parameter::Packet> &packet) {
  try {
    xml_node *subnode = doc->allocate_node(node_element, "packet");
    parentNode->append_node(subnode);

    xml_attribute *attr = doc->allocate_attribute("id", doc->allocate_string(packet->id.c_str(), packet->id.size() + 1));
    subnode->append_attribute(attr);

    std::string tempString = (packet->type == Parameter::Packet::Type::Enum::get) ? "get" : ((packet->type == Parameter::Packet::Type::Enum::set) ? "set" : "event");
    xml_node *packetNode = doc->allocate_node(node_element, "type", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
    subnode->append_node(packetNode);

    if (!packet->responseId.empty()) {
      packetNode = doc->allocate_node(node_element, "responseId", doc->allocate_string(packet->responseId.c_str(), packet->responseId.size() + 1));
      subnode->append_node(packetNode);
    }

    if (!packet->autoReset.empty()) {
      packetNode = doc->allocate_node(node_element, "autoReset");
      subnode->append_node(packetNode);

      for (std::vector<std::string>::iterator j = packet->autoReset.begin(); j != packet->autoReset.end(); ++j) {
        xml_node *autoResetNode = doc->allocate_node(node_element, "parameterId", j->c_str());
        packetNode->append_node(autoResetNode);
      }
    }

    if (!packet->delayedAutoReset.first.empty()) {
      packetNode = doc->allocate_node(node_element, "delayedAutoReset");
      subnode->append_node(packetNode);

      xml_node *autoResetNode = doc->allocate_node(node_element, "resetDelayParameterId", doc->allocate_string(packet->delayedAutoReset.first.c_str(), packet->delayedAutoReset.first.size() + 1));
      packetNode->append_node(autoResetNode);

      tempString = std::to_string(packet->delayedAutoReset.second);
      autoResetNode = doc->allocate_node(node_element, "resetTo", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      packetNode->append_node(autoResetNode);
    }

    if (packet->conditionOperator != Parameter::Packet::ConditionOperator::Enum::none) {
      if (packet->conditionOperator == Parameter::Packet::ConditionOperator::Enum::e) tempString = "e";
      else if (packet->conditionOperator == Parameter::Packet::ConditionOperator::Enum::g) tempString = "g";
      else if (packet->conditionOperator == Parameter::Packet::ConditionOperator::Enum::l) tempString = "l";
      else if (packet->conditionOperator == Parameter::Packet::ConditionOperator::Enum::ge) tempString = "ge";
      else if (packet->conditionOperator == Parameter::Packet::ConditionOperator::Enum::le) tempString = "le";
      packetNode = doc->allocate_node(node_element, "conditionOperator", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      subnode->append_node(packetNode);
    }

    if (packet->conditionValue != -1) {
      tempString = std::to_string(packet->conditionValue);
      packetNode = doc->allocate_node(node_element, "conditionValue", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      subnode->append_node(packetNode);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void HomegearDevice::saveScenario(xml_document *doc, xml_node *parentNode, PScenario &scenario) {
  try {
    xml_attribute *attr = doc->allocate_attribute("id", doc->allocate_string(scenario->id.c_str(), scenario->id.size() + 1));
    parentNode->append_attribute(attr);

    for (ScenarioEntries::iterator i = scenario->scenarioEntries.begin(); i != scenario->scenarioEntries.end(); ++i) {
      xml_node *parameterNode = doc->allocate_node(node_element, "parameter", doc->allocate_string(i->second.c_str(), i->second.size() + 1));
      parentNode->append_node(parameterNode);

      attr = doc->allocate_attribute("id", doc->allocate_string(i->first.c_str(), i->first.size() + 1));
      parameterNode->append_attribute(attr);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void HomegearDevice::saveFunction(xml_document *doc,
                                  xml_node *parentNode,
                                  PFunction &function,
                                  std::map<std::string, PConfigParameters> &configParameters,
                                  std::map<std::string, PVariables> &variables,
                                  std::map<std::string, PLinkParameters> &linkParameters) {
  try {
    std::string tempString = std::to_string(function->channel);
    xml_attribute *attr = doc->allocate_attribute("channel", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
    parentNode->append_attribute(attr);

    attr = doc->allocate_attribute("type", doc->allocate_string(function->type.c_str(), function->type.size() + 1));
    parentNode->append_attribute(attr);

    if (function->channelCount > 1) {
      tempString = std::to_string(function->channelCount);
      attr = doc->allocate_attribute("channelCount", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
      parentNode->append_attribute(attr);
    }

    // {{{ Properties
    {
      xml_node *propertiesNode = doc->allocate_node(node_element, "properties");
      parentNode->append_node(propertiesNode);

      if (function->encryptionEnabledByDefault) {
        tempString = "true";
        xml_node *propertyNode = doc->allocate_node(node_element, "encryptionEnabledByDefault", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (!function->visible) {
        tempString = "false";
        xml_node *propertyNode = doc->allocate_node(node_element, "visible", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (function->internal) {
        tempString = "true";
        xml_node *propertyNode = doc->allocate_node(node_element, "internal", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (function->deletable) {
        tempString = "true";
        xml_node *propertyNode = doc->allocate_node(node_element, "deletable", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (function->dynamicChannelCountIndex != -1) {
        tempString = std::to_string(function->dynamicChannelCountIndex) + ':' + Math::toString(function->dynamicChannelCountSize, 1);
        xml_node *propertyNode = doc->allocate_node(node_element, "dynamicChannelCount", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (!function->countFromVariable.empty()) {
        xml_node *propertyNode = doc->allocate_node(node_element, "countFromVariable", doc->allocate_string(function->countFromVariable.c_str(), function->countFromVariable.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (function->physicalChannelIndexOffset != 0) {
        tempString = std::to_string(function->physicalChannelIndexOffset);
        xml_node *propertyNode = doc->allocate_node(node_element, "physicalChannelIndexOffset", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (function->grouped) {
        tempString = "true";
        xml_node *propertyNode = doc->allocate_node(node_element, "grouped", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (function->direction != Function::Direction::Enum::none) {
        tempString = function->direction == Function::Direction::Enum::sender ? "sender" : "receiver";
        xml_node *propertyNode = doc->allocate_node(node_element, "direction", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (function->forceEncryption) {
        tempString = "true";
        xml_node *propertyNode = doc->allocate_node(node_element, "forceEncryption", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (!function->defaultLinkScenarioElementId.empty()) {
        xml_node *propertyNode = doc->allocate_node(node_element, "defaultLinkScenarioElementId", doc->allocate_string(function->defaultLinkScenarioElementId.c_str(), function->defaultLinkScenarioElementId.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (!function->defaultGroupedLinkScenarioElementId1.empty()) {
        xml_node *propertyNode = doc->allocate_node(node_element, "defaultGroupedLinkScenarioElementId1", doc->allocate_string(function->defaultGroupedLinkScenarioElementId1.c_str(), function->defaultGroupedLinkScenarioElementId1.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (!function->defaultGroupedLinkScenarioElementId2.empty()) {
        xml_node *propertyNode = doc->allocate_node(node_element, "defaultGroupedLinkScenarioElementId2", doc->allocate_string(function->defaultGroupedLinkScenarioElementId2.c_str(), function->defaultGroupedLinkScenarioElementId2.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (function->hasGroup) {
        tempString = "true";
        xml_node *propertyNode = doc->allocate_node(node_element, "hasGroup", doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (!function->groupId.empty()) {
        xml_node *propertyNode = doc->allocate_node(node_element, "groupId", doc->allocate_string(function->groupId.c_str(), function->groupId.size() + 1));
        propertiesNode->append_node(propertyNode);
      }

      if (!function->linkSenderFunctionTypes.empty()) {
        xml_node *propertyNode = doc->allocate_node(node_element, "linkSenderFunctionTypes");
        propertiesNode->append_node(propertyNode);

        for (auto &linkSenderAttribute : function->linkSenderAttributes) {
          auto tempString2 = linkSenderAttribute.second->toString();
          propertyNode->append_attribute(doc->allocate_attribute(linkSenderAttribute.first.c_str(), doc->allocate_string(tempString2.c_str(), tempString2.size() + 1)));
        }

        for (LinkFunctionTypes::iterator j = function->linkSenderFunctionTypes.begin(); j != function->linkSenderFunctionTypes.end(); ++j) {
          xml_node *typeNode = doc->allocate_node(node_element, "type", j->c_str());
          propertyNode->append_node(typeNode);
        }
      }

      if (!function->linkReceiverFunctionTypes.empty()) {
        xml_node *propertyNode = doc->allocate_node(node_element, "linkReceiverFunctionTypes");
        propertiesNode->append_node(propertyNode);

        for (auto &linkReceiverAttribute : function->linkReceiverAttributes) {
          auto tempString2 = linkReceiverAttribute.second->toString();
          propertyNode->append_attribute(doc->allocate_attribute(linkReceiverAttribute.first.c_str(), doc->allocate_string(tempString2.c_str(), tempString2.size() + 1)));
        }

        for (LinkFunctionTypes::iterator j = function->linkReceiverFunctionTypes.begin(); j != function->linkReceiverFunctionTypes.end(); ++j) {
          xml_node *typeNode = doc->allocate_node(node_element, "type", j->c_str());
          propertyNode->append_node(typeNode);
        }
      }
    }
    // }}}

    if (!function->configParametersId.empty()) {
      xml_node *subnode = doc->allocate_node(node_element, "configParameters", doc->allocate_string(function->configParametersId.c_str(), function->configParametersId.size() + 1));
      parentNode->append_node(subnode);
      configParameters[function->configParametersId] = function->configParameters;
    }

    if (!function->variablesId.empty()) {
      xml_node *subnode = doc->allocate_node(node_element, "variables", doc->allocate_string(function->variablesId.c_str(), function->variablesId.size() + 1));
      parentNode->append_node(subnode);
      variables[function->variablesId] = function->variables;
    }

    if (!function->linkParametersId.empty()) {
      xml_node *subnode = doc->allocate_node(node_element, "linkParameters", doc->allocate_string(function->linkParametersId.c_str(), function->linkParametersId.size() + 1));
      parentNode->append_node(subnode);
      linkParameters[function->linkParametersId] = function->linkParameters;
    }

    for (std::vector<PFunction>::iterator i = function->alternativeFunctions.begin(); i != function->alternativeFunctions.end(); ++i) {
      xml_node *subnode = doc->allocate_node(node_element, "alternativeFunction");
      parentNode->append_node(subnode);
      saveFunction(doc, subnode, *i, configParameters, variables, linkParameters);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void HomegearDevice::parseXML(xml_node *node) {
  try {
    for (xml_attribute *attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
      std::string attributeName(attr->name());
      std::string attributeValue(attr->value());
      if (attributeName == "version") version = Math::getNumber(attributeValue);
      else if (attributeName == "xmlns") {}
      else _bl->out.printWarning("Warning: Unknown attribute for \"homegearDevice\": " + std::string(attr->name()));
    }
    std::map<std::string, PConfigParameters> configParameters;
    std::map<std::string, PVariables> variables;
    std::map<std::string, PLinkParameters> linkParameters;
    for (xml_node *subNode = node->first_node(); subNode; subNode = subNode->next_sibling()) {
      std::string nodeName(subNode->name());
      if (nodeName == "supportedDevices") {
        for (xml_node *typeNode = subNode->first_node("device"); typeNode; typeNode = typeNode->next_sibling("device")) {
          PSupportedDevice supportedDevice(new SupportedDevice(_bl, typeNode));
          supportedDevices.push_back(supportedDevice);
        }
      } else if (nodeName == "runProgram") {
        runProgram.reset(new RunProgram(_bl, subNode));
      } else if (nodeName == "properties") {
        bool receiveModeSet = false;
        for (xml_node *propertyNode = subNode->first_node(); propertyNode; propertyNode = propertyNode->next_sibling()) {
          std::string propertyName(propertyNode->name());
          std::string propertyValue(propertyNode->value());
          if (propertyName == "receiveMode") {
            receiveModeSet = true;
            if (propertyValue == "none") receiveModes = ReceiveModes::Enum::none;
            else if (propertyValue == "always") receiveModes = (ReceiveModes::Enum)(receiveModes | ReceiveModes::Enum::always);
            else if (propertyValue == "wakeOnRadio") receiveModes = (ReceiveModes::Enum)(receiveModes | ReceiveModes::Enum::wakeOnRadio);
            else if (propertyValue == "config") receiveModes = (ReceiveModes::Enum)(receiveModes | ReceiveModes::Enum::config);
            else if (propertyValue == "wakeUp") receiveModes = (ReceiveModes::Enum)(receiveModes | ReceiveModes::Enum::wakeUp);
            else if (propertyValue == "wakeUp2") receiveModes = (ReceiveModes::Enum)(receiveModes | ReceiveModes::Enum::wakeUp2);
            else if (propertyValue == "lazyConfig") receiveModes = (ReceiveModes::Enum)(receiveModes | ReceiveModes::Enum::lazyConfig);
            else {
              receiveModeSet = false;
              _bl->out.printWarning("Warning: Unknown receiveMode: " + propertyValue);
            }
          } else if (propertyName == "encryption") { if (propertyValue == "true") encryption = true; }
          else if (propertyName == "timeout") timeout = Math::getUnsignedNumber(propertyValue);
          else if (propertyName == "memorySize") memorySize = Math::getUnsignedNumber(propertyValue);
          else if (propertyName == "memorySize2") memorySize2 = Math::getUnsignedNumber(propertyValue);
          else if (propertyName == "visible") { if (propertyValue == "false") visible = false; }
          else if (propertyName == "deletable") { if (propertyValue == "false") deletable = false; }
          else if (propertyName == "internal") { if (propertyValue == "true") internal = true; }
          else if (propertyName == "needsTime") { if (propertyValue == "true") needsTime = true; }
          else if (propertyName == "hasBattery") { if (propertyValue == "true") hasBattery = true; }
          else if (propertyName == "addressSize") addressSize = Math::getUnsignedNumber(propertyValue);
          else if (propertyName == "pairingMethod") pairingMethod = propertyValue;
          else if (propertyName == "interface") interface = propertyValue;
          else _bl->out.printWarning("Warning: Unknown device property: " + propertyName);
        }
        if (!receiveModeSet) receiveModes = ReceiveModes::Enum::always;
      } else if (nodeName == "functions") {
        for (xml_node *functionNode = subNode->first_node("function"); functionNode; functionNode = functionNode->next_sibling("function")) {
          uint32_t channel = 0;
          PFunction function(new Function(_bl, functionNode, channel));
          for (uint32_t i = channel; i < channel + function->channelCount; i++) {
            if (functions.find(i) == functions.end()) functions[i] = function;
            else _bl->out.printError("Error: Tried to add function with the same channel twice. Channel: " + std::to_string(i));
          }
          if (function->dynamicChannelCountIndex > -1) {
            if (dynamicChannelCountIndex > -1) _bl->out.printError("Error: dynamicChannelCount is defined for two channels. That is not allowed.");
            dynamicChannelCountIndex = function->dynamicChannelCountIndex;
            dynamicChannelCountSize = function->dynamicChannelCountSize;
            if (dynamicChannelCountIndex < -1) dynamicChannelCountIndex = -1;
            if (dynamicChannelCountSize <= 0) dynamicChannelCountSize = 1;
          }
        }
      } else if (nodeName == "metadata") {
        bool isDataNode = false;
        metadata = HelperFunctions::xml2variable(subNode, isDataNode);
      } else if (nodeName == "parameterGroups") {
        for (xml_node *parameterGroupNode = subNode->first_node(); parameterGroupNode; parameterGroupNode = parameterGroupNode->next_sibling()) {
          std::string parameterGroupName(parameterGroupNode->name());
          if (parameterGroupName == "configParameters") {
            PConfigParameters config(new ConfigParameters(_bl));
            config->parseXml(parameterGroupNode);
            configParameters[config->id] = config;
          } else if (parameterGroupName == "variables") {
            PVariables config(new Variables(_bl));
            config->parseXml(parameterGroupNode);
            variables[config->id] = config;
          } else if (parameterGroupName == "linkParameters") {
            PLinkParameters config(new LinkParameters(_bl));
            config->parseXml(parameterGroupNode);
            linkParameters[config->id] = config;
          } else _bl->out.printWarning("Warning: Unknown parameter group: " + parameterGroupName);
        }
      } else if (nodeName == "packets") {
        for (xml_node *packetNode = subNode->first_node("packet"); packetNode; packetNode = packetNode->next_sibling("packet")) {
          PPacket packet(new Packet(_bl, packetNode));

          packetsByMessageType.insert(std::pair<uint32_t, PPacket>(packet->type, packet));
          packetsById[packet->id] = packet;
          if (!packet->function1.empty()) packetsByFunction1.insert(std::pair<std::string, PPacket>(packet->function1, packet));
          if (!packet->function2.empty()) packetsByFunction2.insert(std::pair<std::string, PPacket>(packet->function2, packet));
        }
      } else if (nodeName == "group") {
        group.reset(new HomegearDevice(_bl, subNode));
      } else _bl->out.printWarning("Warning: Unknown node name for \"homegearDevice\": " + nodeName);
    }
    for (Functions::iterator i = functions.begin(); i != functions.end(); ++i) {
      postProcessFunction(i->second, configParameters, variables, linkParameters);
      for (std::vector<PFunction>::iterator j = i->second->alternativeFunctions.begin(); j != i->second->alternativeFunctions.end(); ++j) {
        if (i->second->physicalChannelIndexOffset != (*j)->physicalChannelIndexOffset) {
          _bl->out.printWarning("Warning: physicalChannelIndexOffset of function and alternativeFunction differ.");
        }
        postProcessFunction(*j, configParameters, variables, linkParameters);
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void HomegearDevice::postProcessFunction(PFunction &function, std::map<std::string, PConfigParameters> &configParameters, std::map<std::string, PVariables> &variables, std::map<std::string, PLinkParameters> &linkParameters) {
  try {
    if (!function->configParametersId.empty()) {
      std::map<std::string, PConfigParameters>::iterator configIterator = configParameters.find(function->configParametersId);
      if (configIterator == configParameters.end()) _bl->out.printWarning("Warning: configParameters with id \"" + function->configParametersId + "\" does not exist.");
      else {
        function->configParameters = configIterator->second;
        if (configIterator->second->parameterGroupSelector) function->parameterGroupSelector = configIterator->second->parameterGroupSelector;
      }
    }
    if (!function->variablesId.empty()) {
      std::map<std::string, PVariables>::iterator configIterator = variables.find(function->variablesId);
      if (configIterator == variables.end()) _bl->out.printWarning("Warning: variables with id \"" + function->variablesId + "\" does not exist.");
      else {
        function->variables = configIterator->second;
        if (configIterator->second->parameterGroupSelector) function->parameterGroupSelector = configIterator->second->parameterGroupSelector;

        for (Parameters::iterator j = function->variables->parameters.begin(); j != function->variables->parameters.end(); ++j) {
          for (std::vector<std::shared_ptr<Parameter::Packet>>::iterator k = j->second->getPackets.begin(); k != j->second->getPackets.end(); ++k) {
            PacketsById::iterator packetIterator = packetsById.find((*k)->id);
            if (packetIterator != packetsById.end()) {
              packetIterator->second->associatedVariables.push_back(j->second);
              if (function->physicalChannelIndexOffset != 0) packetIterator->second->channelIndexOffset = function->physicalChannelIndexOffset;
              valueRequestPackets[function->channel][(*k)->id] = packetIterator->second;
            }
          }
          for (std::vector<std::shared_ptr<Parameter::Packet>>::iterator k = j->second->setPackets.begin(); k != j->second->setPackets.end(); ++k) {
            PacketsById::iterator packetIterator = packetsById.find((*k)->id);
            if (packetIterator != packetsById.end()) {
              packetIterator->second->associatedVariables.push_back(j->second);
              if (function->physicalChannelIndexOffset != 0) packetIterator->second->channelIndexOffset = function->physicalChannelIndexOffset;
            }
          }
          for (std::vector<std::shared_ptr<Parameter::Packet>>::iterator k = j->second->eventPackets.begin(); k != j->second->eventPackets.end(); ++k) {
            PacketsById::iterator packetIterator = packetsById.find((*k)->id);
            if (packetIterator != packetsById.end()) {
              packetIterator->second->associatedVariables.push_back(j->second);
              if (function->physicalChannelIndexOffset != 0) packetIterator->second->channelIndexOffset = function->physicalChannelIndexOffset;
            }
          }
        }
      }
    }
    if (!function->linkParametersId.empty()) {
      std::map<std::string, PLinkParameters>::iterator configIterator = linkParameters.find(function->linkParametersId);
      if (configIterator == linkParameters.end()) _bl->out.printWarning("Warning: linkParameters with id \"" + function->linkParametersId + "\" does not exist.");
      else {
        function->linkParameters = configIterator->second;
        if (configIterator->second->parameterGroupSelector) function->parameterGroupSelector = configIterator->second->parameterGroupSelector;
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

PSupportedDevice HomegearDevice::getType(uint64_t typeNumber) {
  try {
    for (SupportedDevices::iterator j = supportedDevices.begin(); j != supportedDevices.end(); ++j) {
      if ((*j)->matches(typeNumber, -1)) return *j;
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return PSupportedDevice();
}

PSupportedDevice HomegearDevice::getType(uint64_t typeNumber, int32_t firmwareVersion) {
  try {
    for (SupportedDevices::iterator j = supportedDevices.begin(); j != supportedDevices.end(); ++j) {
      if ((*j)->matches(typeNumber, firmwareVersion)) return *j;
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return PSupportedDevice();
}

}
}
