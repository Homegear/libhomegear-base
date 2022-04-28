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

#include "Parameter.h"
#include "../BaseLib.h"

namespace BaseLib {
namespace DeviceDescription {

bool Parameter::Packet::checkCondition(int32_t lhs) {
  switch (conditionOperator) {
    case ConditionOperator::Enum::e: return lhs == conditionValue;
    case ConditionOperator::Enum::g: return lhs > conditionValue;
    case ConditionOperator::Enum::l: return lhs < conditionValue;
    case ConditionOperator::Enum::ge: return lhs >= conditionValue;
    case ConditionOperator::Enum::le: return lhs <= conditionValue;
    default: return false;
  }
}

Parameter::Parameter(BaseLib::SharedObjects *baseLib, const PParameterGroup &parent) : _parent(parent) {
  _bl = baseLib;
  logical.reset(new LogicalInteger(baseLib));
  physical.reset(new PhysicalInteger(baseLib));
}

Parameter::~Parameter() = default;

void Parameter::parseXml(xml_node *node) {
  for (xml_attribute *attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
    std::string attributeName(attr->name());
    std::string attributeValue(attr->value());
    if (attributeName == "id") {
      id = attributeValue;
    } else _bl->out.printWarning("Warning: Unknown attribute for \"parameter\": " + attributeName);
  }
  for (xml_node *subNode = node->first_node(); subNode; subNode = subNode->next_sibling()) {
    std::string nodeName(subNode->name());
    if (nodeName == "properties") {
      for (xml_node *propertyNode = subNode->first_node(); propertyNode; propertyNode = propertyNode->next_sibling()) {
        std::string propertyName(propertyNode->name());
        std::string propertyValue(propertyNode->value());
        if (propertyName == "readable") readable = propertyValue == "true";
        else if (propertyName == "writeable") writeable = propertyValue == "true";
        else if (propertyName == "readOnInit") readOnInit = propertyValue == "true";
        else if (propertyName == "transmitted") transmitted = propertyValue == "true";
        else if (propertyName == "addonWriteable") { addonWriteable = (propertyValue == "true"); }
        else if (propertyName == "password") { password = (propertyValue == "true"); }
        else if (propertyName == "visible") { visible = (propertyValue == "true"); }
        else if (propertyName == "internal") { internal = (propertyValue == "true"); }
        else if (propertyName == "parameterGroupSelector") { parameterGroupSelector = (propertyValue == "true"); }
        else if (propertyName == "service") { service = (propertyValue == "true"); }
        else if (propertyName == "serviceInverted") { serviceInverted = (propertyValue == "true"); }
        else if (propertyName == "sticky") { sticky = (propertyValue == "true"); }
        else if (propertyName == "transform") { transform = (propertyValue == "true"); }
        else if (propertyName == "signed") {
          isSignedSet = true;
          isSigned = (propertyValue == "true");
        } else if (propertyName == "control") control = propertyValue;
        else if (propertyName == "unit") unit = propertyValue;
        else if (propertyName == "mandatory") mandatory = (propertyValue == "true");
        else if (propertyName == "formFieldType") formFieldType = propertyValue;
        else if (propertyName == "formPosition") formPosition = Math::getNumber(propertyValue);
        else if (propertyName == "metadata") metadata = propertyValue;
        else if (propertyName == "resetAfterRestart") { resetAfterRestart = (propertyValue == "true"); }
        else if (propertyName == "ccu2Visible") { ccu2Visible = (propertyValue == "true"); }
        else if (propertyName == "linkedParameter") { linkedParameter = propertyValue; }
        else if (propertyName == "priority") { priority = Math::getNumber(propertyValue); }
        else if (propertyName == "casts") {
          for (xml_attribute *attr = propertyNode->first_attribute(); attr; attr = attr->next_attribute()) {
            _bl->out.printWarning("Warning: Unknown attribute for \"casts\": " + std::string(attr->name()));
          }
          for (xml_node *castNode = propertyNode->first_node(); castNode; castNode = castNode->next_sibling()) {
            std::string castName(castNode->name());
            if (castName == "decimalIntegerScale") casts.push_back(std::make_shared<DecimalIntegerScale>(_bl, castNode, shared_from_this()));
            else if (castName == "decimalIntegerInverseScale") casts.push_back(std::make_shared<DecimalIntegerInverseScale>(_bl, castNode, shared_from_this()));
            else if (castName == "integerIntegerScale") casts.push_back(std::make_shared<IntegerIntegerScale>(_bl, castNode, shared_from_this()));
            else if (castName == "integerOffset") casts.push_back(std::make_shared<IntegerOffset>(_bl, castNode, shared_from_this()));
            else if (castName == "decimalOffset") casts.push_back(std::make_shared<DecimalOffset>(_bl, castNode, shared_from_this()));
            else if (castName == "integerIntegerMap") casts.push_back(std::make_shared<IntegerIntegerMap>(_bl, castNode, shared_from_this()));
            else if (castName == "booleanInteger") casts.push_back(std::make_shared<BooleanInteger>(_bl, castNode, shared_from_this()));
            else if (castName == "booleanDecimal") casts.push_back(std::make_shared<BooleanDecimal>(_bl, castNode, shared_from_this()));
            else if (castName == "booleanString") casts.push_back(std::make_shared<BooleanString>(_bl, castNode, shared_from_this()));
            else if (castName == "decimalConfigTime") casts.push_back(std::make_shared<DecimalConfigTime>(_bl, castNode, shared_from_this()));
            else if (castName == "integerTinyFloat") casts.push_back(std::make_shared<IntegerTinyFloat>(_bl, castNode, shared_from_this()));
            else if (castName == "stringUnsignedInteger") casts.push_back(std::make_shared<StringUnsignedInteger>(_bl, castNode, shared_from_this()));
            else if (castName == "blindTest") casts.push_back(std::make_shared<BlindTest>(_bl, castNode, shared_from_this()));
            else if (castName == "optionString") casts.push_back(std::make_shared<OptionString>(_bl, castNode, shared_from_this()));
            else if (castName == "optionInteger") casts.push_back(std::make_shared<OptionInteger>(_bl, castNode, shared_from_this()));
            else if (castName == "stringJsonArrayDecimal") casts.push_back(std::make_shared<StringJsonArrayDecimal>(_bl, castNode, shared_from_this()));
            else if (castName == "rpcBinary") casts.push_back(std::make_shared<RpcBinary>(_bl, castNode, shared_from_this()));
            else if (castName == "toggle") casts.push_back(std::make_shared<Toggle>(_bl, castNode, shared_from_this()));
            else if (castName == "cfm") casts.push_back(std::make_shared<Cfm>(_bl, castNode, shared_from_this()));
            else if (castName == "ccrtdnParty") casts.push_back(std::make_shared<CcrtdnParty>(_bl, castNode, shared_from_this()));
            else if (castName == "stringReplace") casts.push_back(std::make_shared<StringReplace>(_bl, castNode, shared_from_this()));
            else if (castName == "hexStringByteArray") casts.push_back(std::make_shared<HexStringByteArray>(_bl, castNode, shared_from_this()));
            else if (castName == "timeStringSeconds") casts.push_back(std::make_shared<TimeStringSeconds>(_bl, castNode, shared_from_this()));
            else if (castName == "invert") casts.push_back(std::make_shared<Invert>(_bl, castNode, shared_from_this()));
            else if (castName == "round") casts.push_back(std::make_shared<Round>(_bl, castNode, shared_from_this()));
            else if (castName == "generic") casts.push_back(std::make_shared<Generic>(_bl, castNode, shared_from_this()));
            else _bl->out.printWarning("Warning: Unknown cast: " + castName);
          }
        } else if (propertyName == "roles") {
          for (xml_attribute *attr = propertyNode->first_attribute(); attr; attr = attr->next_attribute()) {
            _bl->out.printWarning("Warning: Unknown attribute for \"roles\": " + std::string(attr->name()));
          }
          for (xml_node *roleNode = propertyNode->first_node(); roleNode; roleNode = roleNode->next_sibling()) {
            std::string roleName(roleNode->name());
            std::string roleValue(roleNode->value());
            if (roleName == "role") {
              Role role;
              for (xml_attribute *attr = roleNode->first_attribute(); attr; attr = attr->next_attribute()) {
                std::string attributeName(attr->name());
                if (attributeName == "direction") role.direction = (RoleDirection)Math::getNumber(std::string(attr->value()));
                else if (attributeName == "invert") role.invert = std::string(attr->value()) == "true";
                else if (attributeName == "valueMin" || attributeName == "valueMax" || attributeName == "scaleMin" || attributeName == "scaleMax") {
                  role.scale = true;
                  if (attributeName == "valueMin" || attributeName == "valueMax") role.scaleInfo.valueSet = true;
                  if (attributeName == "valueMin") role.scaleInfo.valueMin = BaseLib::Math::getDouble(std::string(attr->value()));
                  if (attributeName == "valueMax") role.scaleInfo.valueMax = BaseLib::Math::getDouble(std::string(attr->value()));
                  if (attributeName == "scaleMin") role.scaleInfo.scaleMin = BaseLib::Math::getDouble(std::string(attr->value()));
                  if (attributeName == "scaleMax") role.scaleInfo.scaleMax = BaseLib::Math::getDouble(std::string(attr->value()));
                } else _bl->out.printWarning("Warning: Unknown attribute for \"role\": " + std::string(attr->name()));
              }

              role.id = Math::getUnsignedNumber64(roleValue);
              if (role.id != 0) {
                uint64_t middleGroupRoleId = 0;
                uint64_t mainGroupRoleId = 0;

                //{{{ Get parent roles
                {
                  uint64_t hexRoleId = BaseLib::Math::getNumber64(std::to_string(role.id), true);
                  middleGroupRoleId = BaseLib::Math::getNumber64(BaseLib::HelperFunctions::getHexString(hexRoleId & 0x00FFFF00, 6));
                  mainGroupRoleId = BaseLib::Math::getNumber64(BaseLib::HelperFunctions::getHexString(hexRoleId & 0x00FF0000, 6));
                  if (middleGroupRoleId == mainGroupRoleId || middleGroupRoleId == role.id) middleGroupRoleId = 0;
                  if (mainGroupRoleId == role.id) mainGroupRoleId = 0;
                }
                //}}}

                if (mainGroupRoleId != 0) roles.emplace(role.id, BaseLib::Role(mainGroupRoleId, role.direction, false, false, BaseLib::RoleScaleInfo()));
                if (middleGroupRoleId != 0) roles.emplace(role.id, BaseLib::Role(middleGroupRoleId, role.direction, false, false, BaseLib::RoleScaleInfo()));
                roles.emplace(role.id, role);
              }
            } else _bl->out.printWarning("Warning: Unknown parameter role: " + roleName);
          }
        } else _bl->out.printWarning("Warning: Unknown parameter property: " + propertyName);
      }
    } else if (nodeName == "logicalBoolean") logical.reset(new LogicalBoolean(_bl, subNode));
    else if (nodeName == "logicalAction") logical.reset(new LogicalAction(_bl, subNode));
    else if (nodeName == "logicalArray") logical.reset(new LogicalArray(_bl, subNode));
    else if (nodeName == "logicalInteger") logical.reset(new LogicalInteger(_bl, subNode));
    else if (nodeName == "logicalInteger64") logical.reset(new LogicalInteger64(_bl, subNode));
    else if (nodeName == "logicalDecimal" || nodeName == "logicalFloat") logical.reset(new LogicalDecimal(_bl, subNode));
    else if (nodeName == "logicalString") logical.reset(new LogicalString(_bl, subNode));
    else if (nodeName == "logicalStruct") logical.reset(new LogicalStruct(_bl, subNode));
    else if (nodeName == "logicalEnumeration") logical.reset(new LogicalEnumeration(_bl, subNode));
    else if (nodeName == "physical" || nodeName == "physicalNone") physical.reset(new Physical(_bl, subNode));
    else if (nodeName == "physicalInteger") physical.reset(new PhysicalInteger(_bl, subNode));
    else if (nodeName == "physicalBoolean") physical.reset(new PhysicalBoolean(_bl, subNode));
    else if (nodeName == "physicalString") physical.reset(new PhysicalString(_bl, subNode));
    else if (nodeName == "packets") {
      for (xml_node *packetsNode = subNode->first_node(); packetsNode; packetsNode = packetsNode->next_sibling()) {
        std::string packetsNodeName(packetsNode->name());
        if (packetsNodeName == "packet") {
          std::shared_ptr<Packet> packet(new Packet());
          for (xml_attribute *attr = packetsNode->first_attribute(); attr; attr = attr->next_attribute()) {
            std::string attributeName(attr->name());
            if (attributeName == "id") packet->id = std::string(attr->value());
            else _bl->out.printWarning(R"(Warning: Unknown attribute for "parameter\packets\packet": )" + std::string(attr->name()));
          }
          for (xml_node *packetNode = packetsNode->first_node(); packetNode; packetNode = packetNode->next_sibling()) {
            std::string packetNodeName(packetNode->name());
            if (packetNodeName == "type") {
              for (xml_attribute *attr = packetNode->first_attribute(); attr; attr = attr->next_attribute()) {
                _bl->out.printWarning(R"(Warning: Unknown attribute for "parameter\packets\packet\type": )" + std::string(attr->name()));
              }
              std::string value(packetNode->value());
              if (value == "get") {
                packet->type = Packet::Type::Enum::get;
                getPackets.push_back(packet);
              } else if (value == "set") {
                packet->type = Packet::Type::Enum::set;
                setPackets.push_back(packet);
              } else if (value == "event") {
                packet->type = Packet::Type::Enum::event;
                eventPackets.push_back(packet);
              } else _bl->out.printWarning(R"(Warning: Unknown value for "parameter\packets\packet\type": )" + value);
            } else if (packetNodeName == "responseId") {
              for (xml_attribute *attr = packetNode->first_attribute(); attr; attr = attr->next_attribute()) {
                _bl->out.printWarning(R"(Warning: Unknown attribute for "parameter\packets\packet\response": )" + std::string(attr->name()));
              }
              packet->responseId = std::string(packetNode->value());
            } else if (packetNodeName == "autoReset") {
              for (xml_attribute *attr = packetNode->first_attribute(); attr; attr = attr->next_attribute()) {
                _bl->out.printWarning(R"(Warning: Unknown attribute for "parameter\packets\packet\autoReset": )" + std::string(attr->name()));
              }
              for (xml_node *autoResetNode = packetNode->first_node(); autoResetNode; autoResetNode = autoResetNode->next_sibling()) {
                std::string autoResetNodeName(autoResetNode->name());
                std::string autoResetNodeValue(autoResetNode->value());
                if (autoResetNodeValue.empty()) continue;
                if (autoResetNodeName == "parameterId") packet->autoReset.push_back(autoResetNodeValue);
                else _bl->out.printWarning(R"(Warning: Unknown subnode for "parameter\packets\packet\autoReset": )" + packetsNodeName);
              }
            } else if (packetNodeName == "delayedAutoReset") {
              for (xml_attribute *attr = packetNode->first_attribute(); attr; attr = attr->next_attribute()) {
                _bl->out.printWarning(R"(Warning: Unknown attribute for "parameter\packets\packet\delayedAutoReset": )" + std::string(attr->name()));
              }
              for (xml_node *autoResetNode = packetNode->first_node(); autoResetNode; autoResetNode = autoResetNode->next_sibling()) {
                std::string autoResetNodeName(autoResetNode->name());
                std::string autoResetNodeValue(autoResetNode->value());
                if (autoResetNodeValue.empty()) continue;
                if (autoResetNodeName == "resetDelayParameterId") packet->delayedAutoReset.first = autoResetNodeValue;
                else if (autoResetNodeName == "resetTo") packet->delayedAutoReset.second = Math::getNumber(autoResetNodeValue);
                else _bl->out.printWarning(R"(Warning: Unknown subnode for "parameter\packets\packet\delayedAutoReset": )" + packetsNodeName);
                hasDelayedAutoResetParameters = true;
              }
            } else if (packetNodeName == "conditionOperator") {
              std::string value(packetNode->value());
              HelperFunctions::toLower(HelperFunctions::trim(value));
              if (value == "e" || value == "eq") packet->conditionOperator = Packet::ConditionOperator::Enum::e;
              else if (value == "g") packet->conditionOperator = Packet::ConditionOperator::Enum::g;
              else if (value == "l") packet->conditionOperator = Packet::ConditionOperator::Enum::l;
              else if (value == "ge") packet->conditionOperator = Packet::ConditionOperator::Enum::ge;
              else if (value == "le") packet->conditionOperator = Packet::ConditionOperator::Enum::le;
              else _bl->out.printWarning(R"(Warning: Unknown attribute value for "cond" in node "setEx": )" + value);
            } else if (packetNodeName == "conditionValue") {
              std::string value(packetNode->value());
              packet->conditionValue = Math::getNumber(value);
            } else if (packetNodeName == "delay") {
              std::string value(packetNode->value());
              packet->delay = Math::getNumber(value);
            } else _bl->out.printWarning(R"(Warning: Unknown subnode for "parameter\packets\packet": )" + packetsNodeName);
          }
        } else _bl->out.printWarning(R"(Warning: Unknown subnode for "parameter\packets": )" + packetsNodeName);
      }
    } else _bl->out.printWarning("Warning: Unknown node in \"parameter\": " + nodeName);
  }
  if (logical->type == ILogical::Type::Enum::tFloat) {
    auto parameter = std::dynamic_pointer_cast<LogicalDecimal>(logical);
    if (parameter && parameter->minimumValue < 0 && parameter->minimumValue != std::numeric_limits<double>::min() && (!isSignedSet || isSigned)) isSigned = true;
  } else if (logical->type == ILogical::Type::Enum::tInteger) {
    auto parameter = std::dynamic_pointer_cast<LogicalInteger>(logical);
    if (parameter && parameter->minimumValue < 0 && parameter->minimumValue != std::numeric_limits<int32_t>::min() && (!isSignedSet || isSigned)) isSigned = true;
  } else if (logical->type == ILogical::Type::Enum::tInteger64) {
    auto parameter = std::dynamic_pointer_cast<LogicalInteger64>(logical);
    if (parameter && parameter->minimumValue < 0 && parameter->minimumValue != std::numeric_limits<int64_t>::min() && (!isSignedSet || isSigned)) isSigned = true;
  }

  // {{{ Set role.scaleInfo.valueMin and role.scaleInfo.valueMax if not specified in XML
  for (auto &role : roles) {
    if (!role.second.scale || role.second.scaleInfo.valueSet) continue;
    if (logical->type == ILogical::Type::Enum::tFloat) {
      auto parameter = std::dynamic_pointer_cast<LogicalDecimal>(logical);
      role.second.scaleInfo.valueMin = parameter->minimumValue;
      role.second.scaleInfo.valueMin = parameter->maximumValue;
    } else if (logical->type == ILogical::Type::Enum::tInteger) {
      auto parameter = std::dynamic_pointer_cast<LogicalInteger>(logical);
      role.second.scaleInfo.valueMin = parameter->minimumValue;
      role.second.scaleInfo.valueMin = parameter->maximumValue;
    } else if (logical->type == ILogical::Type::Enum::tInteger64) {
      auto parameter = std::dynamic_pointer_cast<LogicalInteger64>(logical);
      role.second.scaleInfo.valueMin = parameter->minimumValue;
      role.second.scaleInfo.valueMin = parameter->maximumValue;
    }
  }
  // }}}
}

PVariable Parameter::convertFromPacket(const std::vector<uint8_t> &data, const Role &role, bool isEvent) {
  try {
    std::vector<uint8_t> reversedData;
    const std::vector<uint8_t> *value = nullptr;
    if (physical->endianess == IPhysical::Endianess::Enum::little) {
      reverseData(data, reversedData);
      value = &reversedData;
    } else value = &data;
    if (logical->type == ILogical::Type::Enum::tEnum && casts.empty()) {
      int32_t integerValue = 0;
      BaseLib::HelperFunctions::memcpyBigEndian(integerValue, *value);
      if (role.invert) {
        auto *parameter = (LogicalEnumeration *)logical.get();
        integerValue = parameter->minimumValue + ((parameter->maximumValue - parameter->minimumValue) - (integerValue - parameter->minimumValue));
      }
      if (role.scale) integerValue = std::lround(Math::scale((double)integerValue, role.scaleInfo.valueMin, role.scaleInfo.valueMax, role.scaleInfo.scaleMin, role.scaleInfo.scaleMax));
      return std::make_shared<Variable>(integerValue);
    } else if (logical->type == ILogical::Type::Enum::tBoolean && casts.empty()) {
      int32_t integerValue = 0;
      BaseLib::HelperFunctions::memcpyBigEndian(integerValue, *value);
      return std::make_shared<Variable>(role.invert == !(bool)integerValue);
    } else if (logical->type == ILogical::Type::Enum::tString && casts.empty()) {
      if (!value->empty() && value->at(0) != 0) {
        int32_t size = value->back() == 0 ? value->size() - 1 : value->size();
        std::string string(value->begin(), value->begin() + size);
        return std::make_shared<Variable>(string);
      }
      return std::make_shared<Variable>(VariableType::tString);
    } else if (logical->type == ILogical::Type::Enum::tAction) {
      return std::make_shared<Variable>(isEvent);
    } else if (id == "RSSI_DEVICE") {
      int32_t integerValue = 0;
      BaseLib::HelperFunctions::memcpyBigEndian(integerValue, *value);
      std::shared_ptr<Variable> variable(new Variable(integerValue * -1));
      return variable;
    } else {
      std::shared_ptr<Variable> variable;
      if (physical->type == IPhysical::Type::tString) {
        variable.reset(new Variable(VariableType::tString));
        variable->stringValue.insert(variable->stringValue.end(), value->begin(), value->end());
      } else if (value->size() <= 4) {
        int32_t integerValue = 0;
        BaseLib::HelperFunctions::memcpyBigEndian(integerValue, *value);
        variable.reset(new Variable(integerValue));
        if (isSigned && !value->empty() && value->size() <= 4) {
          int32_t byteSize = std::lround(std::ceil(physical->size));
          if (byteSize > 0 && (signed)value->size() == byteSize) {
            int32_t bitSize = std::lround(physical->size * 10) % 10;
            int32_t signPosition = 0;
            if (bitSize == 0) signPosition = 7;
            else signPosition = bitSize - 1;
            if (value->at(0) & (1 << signPosition)) {
              int32_t bits = (std::lround(std::floor(physical->size)) * 8) + bitSize;
              variable->integerValue -= (1 << bits);
            }
          }
        }
      }
      if (!variable) variable.reset(new Variable(VariableType::tBinary));

      for (auto i = casts.rbegin(); i != casts.rend(); ++i) {
        if ((*i)->needsBinaryPacketData() && variable->binaryValue.empty()) variable->binaryValue = *value;
        (*i)->fromPacket(variable);
      }

      //{{{ Control boundaries and invert value
      if (logical->type == ILogical::Type::Enum::tEnum) {
        auto *parameter = (LogicalEnumeration *)logical.get();
        if (variable->integerValue > parameter->maximumValue) variable->integerValue = parameter->maximumValue;
        if (variable->integerValue < parameter->minimumValue) variable->integerValue = parameter->minimumValue;
        if (role.invert) variable->integerValue = parameter->minimumValue + ((parameter->maximumValue - parameter->minimumValue) - (variable->integerValue - parameter->minimumValue));
        if (role.scale) variable->integerValue = std::lround(Math::scale((double)variable->integerValue, role.scaleInfo.valueMin, role.scaleInfo.valueMax, role.scaleInfo.scaleMin, role.scaleInfo.scaleMax));
      } else if (logical->type == ILogical::Type::Enum::tFloat) {
        auto *parameter = (LogicalDecimal *)logical.get();
        bool specialValue = (variable->floatValue == parameter->defaultValue);
        if (!specialValue) specialValue = parameter->specialValuesFloatMap.find(variable->floatValue) != parameter->specialValuesFloatMap.end();
        if (!specialValue) {
          if (variable->floatValue > parameter->maximumValue) variable->floatValue = parameter->maximumValue;
          else if (variable->floatValue < parameter->minimumValue) variable->floatValue = parameter->minimumValue;
          if (role.invert) variable->floatValue = parameter->minimumValue + ((parameter->maximumValue - parameter->minimumValue) - (variable->floatValue - parameter->minimumValue));
          if (role.scale) variable->floatValue = Math::scale(variable->floatValue, role.scaleInfo.valueMin, role.scaleInfo.valueMax, role.scaleInfo.scaleMin, role.scaleInfo.scaleMax);
        }
      } else if (logical->type == ILogical::Type::Enum::tInteger) {
        auto *parameter = (LogicalInteger *)logical.get();
        bool specialValue = (variable->integerValue == parameter->defaultValue);
        if (!specialValue) specialValue = parameter->specialValuesIntegerMap.find(variable->floatValue) != parameter->specialValuesIntegerMap.end();
        if (!specialValue) {
          if (variable->integerValue > parameter->maximumValue) variable->integerValue = parameter->maximumValue;
          else if (variable->integerValue < parameter->minimumValue) variable->integerValue = parameter->minimumValue;
          if (role.invert) {
            if (parameter->minimumValue == 0 && parameter->maximumValue <= 2) variable->integerValue = !((bool)variable->integerValue);
            else variable->integerValue = parameter->minimumValue + ((parameter->maximumValue - parameter->minimumValue) - (variable->integerValue - parameter->minimumValue));
          }
          if (role.scale) variable->integerValue = std::lround(Math::scale((double)variable->integerValue, role.scaleInfo.valueMin, role.scaleInfo.valueMax, role.scaleInfo.scaleMin, role.scaleInfo.scaleMax));
        }
      } else if (logical->type == ILogical::Type::Enum::tInteger64) {
        auto *parameter = (LogicalInteger64 *)logical.get();
        bool specialValue = (variable->integerValue64 == parameter->defaultValue);
        if (!specialValue) specialValue = parameter->specialValuesIntegerMap.find(variable->floatValue) != parameter->specialValuesIntegerMap.end();
        if (!specialValue) {
          if (variable->integerValue64 > parameter->maximumValue) variable->integerValue64 = parameter->maximumValue;
          else if (variable->integerValue64 < parameter->minimumValue) variable->integerValue64 = parameter->minimumValue;
          if (role.invert) {
            if (parameter->minimumValue == 0 && parameter->maximumValue <= 2) variable->integerValue64 = !((bool)variable->integerValue64);
            else variable->integerValue64 = parameter->minimumValue + ((parameter->maximumValue - parameter->minimumValue) - (variable->integerValue64 - parameter->minimumValue));
          }
          if (role.scale) variable->integerValue64 = std::llround(Math::scale((double)variable->integerValue64, role.scaleInfo.valueMin, role.scaleInfo.valueMax, role.scaleInfo.scaleMin, role.scaleInfo.scaleMax));
        }
      } else if (logical->type == ILogical::Type::Enum::tBoolean) {
        if (role.invert) variable->booleanValue = !variable->booleanValue;
      }
      //}}}
      return variable;
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::make_shared<BaseLib::Variable>();
}

void Parameter::convertToPacket(const std::string &value, const Role &role, std::vector<uint8_t> &convertedValue) {
  try {
    PVariable rpcValue;
    if (logical->type == ILogical::Type::Enum::tInteger) rpcValue.reset(new Variable(Math::getNumber(value)));
    else if (logical->type == ILogical::Type::Enum::tInteger64) rpcValue.reset(new Variable(Math::getNumber64(value)));
    else if (logical->type == ILogical::Type::Enum::tEnum) {
      if (Math::isNumber(value)) rpcValue.reset(new Variable(Math::getNumber(value)));
      else //value is id of enum element
      {
        auto *parameter = (LogicalEnumeration *)logical.get();
        for (auto &element : parameter->values) {
          if (element.id == value) {
            rpcValue.reset(new Variable(element.index));
            break;
          }
        }
        if (!rpcValue) rpcValue = std::make_shared<Variable>(0);
      }
    } else if (logical->type == ILogical::Type::Enum::tBoolean || logical->type == ILogical::Type::Enum::tAction) {
      rpcValue.reset(new Variable(false));
      std::string valueCopy = value;
      if (HelperFunctions::toLower(valueCopy) == "true") rpcValue->booleanValue = true;
    } else if (logical->type == ILogical::Type::Enum::tFloat) rpcValue.reset(new Variable(Math::getDouble(value)));
    else if (logical->type == ILogical::Type::Enum::tString) rpcValue.reset(new Variable(value));
    if (!rpcValue) {
      _bl->out.printWarning("Warning: Could not convert parameter " + id + " from String.");
      return;
    }
    return convertToPacket(rpcValue, role, convertedValue);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Parameter::convertToPacket(const PVariable &value, const Role &role, std::vector<uint8_t> &convertedValue) {
  try {
    convertedValue.clear();
    if (!value) return;
    PVariable variable(new Variable());
    *variable = *value;
    if (logical->type == ILogical::Type::Enum::tAction && casts.empty()) {
      variable->integerValue = (int32_t)variable->booleanValue;
    } else if (logical->type == ILogical::Type::Enum::tString && casts.empty()) {
      if (!variable->stringValue.empty()) {
        convertedValue.insert(convertedValue.end(), variable->stringValue.begin(), variable->stringValue.end());
      }
      if ((signed)convertedValue.size() < std::lround(physical->size)) convertedValue.push_back(0); //0 termination. Otherwise parts of old string will still be visible
    } else {
      if (logical->type == ILogical::Type::Enum::tEnum) {
        auto *parameter = (LogicalEnumeration *)logical.get();
        if (variable->integerValue > parameter->maximumValue) variable->integerValue = parameter->maximumValue;
        if (variable->integerValue < parameter->minimumValue) variable->integerValue = parameter->minimumValue;
        if (role.scale) variable->integerValue = std::lround(Math::scale((double)variable->integerValue, role.scaleInfo.scaleMin, role.scaleInfo.scaleMax, role.scaleInfo.valueMin, role.scaleInfo.valueMax));
        if (role.invert) variable->integerValue = parameter->minimumValue + ((parameter->maximumValue - parameter->minimumValue) - (variable->integerValue - parameter->minimumValue));
      } else if (logical->type == ILogical::Type::Enum::tFloat) {
        if (variable->floatValue == 0) {
          if (variable->integerValue != 0) variable->floatValue = variable->integerValue;
          else if (variable->integerValue64 != 0) variable->floatValue = variable->integerValue64;
          else if (!variable->stringValue.empty()) variable->floatValue = Math::getDouble(variable->stringValue);
        }
        auto *parameter = (LogicalDecimal *)logical.get();
        bool specialValue = (variable->floatValue == parameter->defaultValue);
        if (!specialValue) specialValue = parameter->specialValuesFloatMap.find(variable->floatValue) != parameter->specialValuesFloatMap.end();
        if (!specialValue) {
          if (variable->floatValue > parameter->maximumValue) variable->floatValue = parameter->maximumValue;
          else if (variable->floatValue < parameter->minimumValue) variable->floatValue = parameter->minimumValue;
          if (role.scale) variable->floatValue = Math::scale(variable->floatValue, role.scaleInfo.scaleMin, role.scaleInfo.scaleMax, role.scaleInfo.valueMin, role.scaleInfo.valueMax);
          if (role.invert) variable->floatValue = parameter->minimumValue + ((parameter->maximumValue - parameter->minimumValue) - (variable->floatValue - parameter->minimumValue));
        }
      } else if (logical->type == ILogical::Type::Enum::tInteger) {
        if (variable->integerValue == 0) {
          if (variable->floatValue != 0) variable->integerValue = variable->floatValue;
            //else if(variable->integerValue64 != 0) variable->integerValue = (int32_t)variable->integerValue64; //Dangerous
          else if (!variable->stringValue.empty()) variable->integerValue = Math::getNumber(variable->stringValue);
        }
        auto *parameter = (LogicalInteger *)logical.get();
        bool specialValue = (variable->integerValue == parameter->defaultValue);
        if (!specialValue) specialValue = parameter->specialValuesIntegerMap.find(variable->floatValue) != parameter->specialValuesIntegerMap.end();
        if (!specialValue) {
          if (variable->integerValue > parameter->maximumValue) variable->integerValue = parameter->maximumValue;
          else if (variable->integerValue < parameter->minimumValue) variable->integerValue = parameter->minimumValue;
          if (role.scale) variable->integerValue = std::lround(Math::scale((double)variable->integerValue, role.scaleInfo.scaleMin, role.scaleInfo.scaleMax, role.scaleInfo.valueMin, role.scaleInfo.valueMax));
          if (role.invert) variable->integerValue = parameter->minimumValue + ((parameter->maximumValue - parameter->minimumValue) - (variable->integerValue - parameter->minimumValue));
        }
      } else if (logical->type == ILogical::Type::Enum::tInteger64) {
        if (variable->integerValue64 == 0) {
          if (variable->floatValue != 0) variable->integerValue64 = variable->floatValue;
            //if(variable->integerValue != 0) variable->integerValue64 = variable->integerValue;
          else if (!variable->stringValue.empty()) variable->integerValue64 = Math::getNumber(variable->stringValue);
        }
        auto *parameter = (LogicalInteger64 *)logical.get();
        bool specialValue = (variable->integerValue64 == parameter->defaultValue);
        if (!specialValue) specialValue = parameter->specialValuesIntegerMap.find(variable->floatValue) != parameter->specialValuesIntegerMap.end();
        if (!specialValue) {
          if (variable->integerValue64 > parameter->maximumValue) variable->integerValue64 = parameter->maximumValue;
          else if (variable->integerValue64 < parameter->minimumValue) variable->integerValue64 = parameter->minimumValue;
          if (role.scale) variable->integerValue64 = std::llround(Math::scale((double)variable->integerValue64, role.scaleInfo.scaleMin, role.scaleInfo.scaleMax, role.scaleInfo.valueMin, role.scaleInfo.valueMax));
          if (role.invert) variable->integerValue64 = parameter->minimumValue + ((parameter->maximumValue - parameter->minimumValue) - (variable->integerValue64 - parameter->minimumValue));
        }
      } else if (logical->type == ILogical::Type::Enum::tBoolean) {
        if (!variable->booleanValue &&
            (variable->integerValue != 0 || variable->floatValue != 0 || variable->stringValue == "true")) {
          variable->booleanValue = true;
        }
        if (role.invert) variable->booleanValue = !variable->booleanValue;
      }
      if (casts.empty()) {
        if (logical->type == ILogical::Type::Enum::tBoolean) {
          if (!variable->stringValue.empty() && variable->stringValue == "true") variable->integerValue = 1;
          else variable->integerValue = (int32_t)variable->booleanValue;
        }
      } else {
        for (auto &cast : casts) {
          cast->toPacket(variable);
        }
      }
    }
    if (variable->type == VariableType::tBinary) {
      convertedValue = variable->binaryValue;
    } else if (physical->type != IPhysical::Type::Enum::tString) {
      if (physical->sizeDefined) //Values have no size defined. That is a problem if the value is larger than 1 byte.
      {
        //Crop to size. Most importantly for negative numbers
        uint32_t byteSize = std::lround(std::floor(physical->size));
        int32_t bitSize = std::lround(physical->size * 10) % 10;
        if (byteSize >= 4) {
          byteSize = 4;
          bitSize = 0;
        }
        int32_t valueMask = 0xFFFFFFFF >> (((4 - byteSize) * 8) - bitSize);
        variable->integerValue &= valueMask;
      }
      HelperFunctions::memcpyBigEndian(convertedValue, variable->integerValue);

      if (physical->endianess == IPhysical::Endianess::Enum::little) {
        std::vector<uint8_t> reversedData;
        reverseData(convertedValue, reversedData);
        convertedValue = reversedData;
      }
    } else if (convertedValue.empty() && !variable->stringValue.empty()) {
      convertedValue.insert(convertedValue.end(), variable->stringValue.begin(), variable->stringValue.end());
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Parameter::reverseData(const std::vector<uint8_t> &data, std::vector<uint8_t> &reversedData) {
  try {
    reversedData.clear();
    int32_t size = std::lround(std::ceil(physical->size));
    if (size == 0) size = 1;
    int32_t j = data.size() - 1;
    for (int32_t i = 0; i < size; i++) {
      if (j < 0) reversedData.push_back(0);
      else reversedData.push_back(data.at(j));
      j--;
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Parameter::adjustBitPosition(std::vector<uint8_t> &data) {
  try {
    if (data.size() > 4 || data.empty() || logical->type == ILogical::Type::Enum::tString) return;
    int32_t value = 0;
    BaseLib::HelperFunctions::memcpyBigEndian(value, data);
    if (physical->size < 0) {
      _bl->out.printError("Error: Negative size not allowed.");
      return;
    }
    double i = physical->index;
    i -= std::floor(i);
    double byteIndex = std::floor(i);
    if (byteIndex != i || physical->size < 0.8) //0.8 == 8 Bits
    {
      if (physical->size > 1) {
        _bl->out.printError("Error: Can't set partial byte index > 1.");
        return;
      }
      data.clear();
      data.push_back(value << (std::lround(i * 10) % 10));
    }
    //Adjust data size. See for example ENDTIME_SATURDAY_1 and TEMPERATURE_SATURDAY_1 of HM-CC-RT-DN
    if ((int32_t)physical->size > (signed)data.size()) {
      uint32_t bytesMissing = (int32_t)physical->size - data.size();
      std::vector<uint8_t> oldData = data;
      data.clear();
      for (uint32_t j = 0; j < bytesMissing; j++) data.push_back(0);
      for (auto element : oldData) data.push_back(element);
    }
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

PParameterGroup Parameter::parent() {
  return _parent.lock();
}

}
}
