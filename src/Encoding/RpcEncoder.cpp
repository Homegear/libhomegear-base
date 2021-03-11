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

#include "RpcEncoder.h"
#include "../BaseLib.h"

namespace BaseLib {
namespace Rpc {

RpcEncoder::RpcEncoder() {
  strncpy(_packetStartRequest, "Bin", 4);
  strncpy(_packetStartResponse, "Bin", 4);
  _packetStartResponse[3] = 1;
  _packetStartResponse[4] = 0;
  strncpy(_packetStartError, "Bin", 4);
  _packetStartError[3] = (char)(uint8_t)0xFF;
  _packetStartError[4] = 0;
}

RpcEncoder::RpcEncoder(bool forceInteger64, bool encodeVoid) : RpcEncoder() {
  _forceInteger64 = forceInteger64;
  _encodeVoid = encodeVoid;
}

RpcEncoder::RpcEncoder(BaseLib::SharedObjects *baseLib) : RpcEncoder() {
}

RpcEncoder::RpcEncoder(BaseLib::SharedObjects *baseLib, bool forceInteger64, bool encodeVoid) : RpcEncoder(forceInteger64, encodeVoid) {
}

void RpcEncoder::encodeRequest(const std::string &methodName, const std::shared_ptr<std::list<std::shared_ptr<Variable>>> &parameters, std::vector<char> &encodedData, const std::shared_ptr<RpcHeader> &header) {
  //The "Bin", the type byte after that and the length itself are not part of the length
  encodedData.clear();
  encodedData.reserve(1024);
  encodedData.insert(encodedData.begin(), _packetStartRequest, _packetStartRequest + 4);
  uint32_t headerSize = 0;
  if (header) {
    headerSize = encodeHeader(encodedData, *header) + 4;
    if (headerSize > 0) encodedData.at(3) |= 0x40;
  }
  BinaryEncoder::encodeString(encodedData, methodName);
  if (!parameters) BinaryEncoder::encodeInteger(encodedData, 0);
  else BinaryEncoder::encodeInteger(encodedData, parameters->size());
  if (parameters) {
    for (auto &parameter : *parameters) {
      encodeVariable(encodedData, parameter);
    }
  }

  uint32_t dataSize = encodedData.size() - 4 - headerSize;
  char result[4];
  HelperFunctions::memcpyBigEndian(result, (char *)&dataSize, 4);
  encodedData.insert(encodedData.begin() + 4 + headerSize, result, result + 4);
}

void RpcEncoder::encodeRequest(const std::string &methodName, const std::shared_ptr<std::list<std::shared_ptr<Variable>>> &parameters, std::vector<uint8_t> &encodedData, const std::shared_ptr<RpcHeader> &header) {
  //The "Bin", the type byte after that and the length itself are not part of the length
  encodedData.clear();
  encodedData.reserve(1024);
  encodedData.insert(encodedData.begin(), _packetStartRequest, _packetStartRequest + 4);
  uint32_t headerSize = 0;
  if (header) {
    headerSize = encodeHeader(encodedData, *header) + 4;
    if (headerSize > 0) encodedData.at(3) |= 0x40;
  }
  BinaryEncoder::encodeString(encodedData, methodName);
  if (!parameters) BinaryEncoder::encodeInteger(encodedData, 0);
  else BinaryEncoder::encodeInteger(encodedData, parameters->size());
  if (parameters) {
    for (auto &parameter : *parameters) {
      encodeVariable(encodedData, parameter);
    }
  }

  uint32_t dataSize = encodedData.size() - 4 - headerSize;
  char result[4];
  HelperFunctions::memcpyBigEndian(result, (char *)&dataSize, 4);
  encodedData.insert(encodedData.begin() + 4 + headerSize, result, result + 4);
}

void RpcEncoder::encodeRequest(const std::string &methodName, const PArray &parameters, std::vector<char> &encodedData, const std::shared_ptr<RpcHeader> &header) {
  //The "Bin", the type byte after that and the length itself are not part of the length
  encodedData.clear();
  encodedData.reserve(1024);
  encodedData.insert(encodedData.begin(), _packetStartRequest, _packetStartRequest + 4);
  uint32_t headerSize = 0;
  if (header) {
    headerSize = encodeHeader(encodedData, *header) + 4;
    if (headerSize > 0) encodedData.at(3) |= 0x40;
  }
  BinaryEncoder::encodeString(encodedData, methodName);
  if (!parameters) BinaryEncoder::encodeInteger(encodedData, 0);
  else BinaryEncoder::encodeInteger(encodedData, parameters->size());
  if (parameters) {
    for (auto &parameter : *parameters) {
      encodeVariable(encodedData, parameter);
    }
  }

  uint32_t dataSize = encodedData.size() - 4 - headerSize;
  char result[4];
  HelperFunctions::memcpyBigEndian(result, (char *)&dataSize, 4);
  encodedData.insert(encodedData.begin() + 4 + headerSize, result, result + 4);
}

void RpcEncoder::encodeRequest(const std::string &methodName, const PArray &parameters, std::vector<uint8_t> &encodedData, const std::shared_ptr<RpcHeader> &header) {
  //The "Bin", the type byte after that and the length itself are not part of the length
  encodedData.clear();
  encodedData.reserve(1024);
  encodedData.insert(encodedData.begin(), _packetStartRequest, _packetStartRequest + 4);
  uint32_t headerSize = 0;
  if (header) {
    headerSize = encodeHeader(encodedData, *header) + 4;
    if (headerSize > 0) encodedData.at(3) |= 0x40;
  }
  BinaryEncoder::encodeString(encodedData, methodName);
  if (!parameters) BinaryEncoder::encodeInteger(encodedData, 0);
  else BinaryEncoder::encodeInteger(encodedData, parameters->size());
  if (parameters) {
    for (auto &parameter : *parameters) {
      encodeVariable(encodedData, parameter);
    }
  }

  uint32_t dataSize = encodedData.size() - 4 - headerSize;
  char result[4];
  HelperFunctions::memcpyBigEndian(result, (char *)&dataSize, 4);
  encodedData.insert(encodedData.begin() + 4 + headerSize, result, result + 4);
}

void RpcEncoder::encodeResponse(const std::shared_ptr<Variable> &variable, std::vector<char> &encodedData) {
  //The "Bin", the type byte after that and the length itself are not part of the length
  encodedData.clear();
  encodedData.reserve(1024);
  if (variable && variable->errorStruct) encodedData.insert(encodedData.begin(), _packetStartError, _packetStartError + 4);
  else encodedData.insert(encodedData.begin(), _packetStartResponse, _packetStartResponse + 4);

  encodeVariable(encodedData, variable ? variable : std::make_shared<Variable>());

  uint32_t dataSize = encodedData.size() - 4;
  char result[4];
  HelperFunctions::memcpyBigEndian(result, (char *)&dataSize, 4);
  encodedData.insert(encodedData.begin() + 4, result, result + 4);
}

void RpcEncoder::encodeResponse(const std::shared_ptr<Variable> &variable, std::vector<uint8_t> &encodedData) {
  //The "Bin", the type byte after that and the length itself are not part of the length
  encodedData.clear();
  encodedData.reserve(1024);
  if (variable && variable->errorStruct) encodedData.insert(encodedData.begin(), _packetStartError, _packetStartError + 4);
  else encodedData.insert(encodedData.begin(), _packetStartResponse, _packetStartResponse + 4);

  encodeVariable(encodedData, variable ? variable : std::make_shared<Variable>());

  uint32_t dataSize = encodedData.size() - 4;
  char result[4];
  HelperFunctions::memcpyBigEndian(result, (char *)&dataSize, 4);
  encodedData.insert(encodedData.begin() + 4, result, result + 4);
}

void RpcEncoder::insertHeader(std::vector<char> &packet, const RpcHeader &header) {
  std::vector<char> headerData;
  headerData.reserve(1024);
  uint32_t headerSize = encodeHeader(headerData, header);
  if (headerSize > 0) {
    packet.at(3) |= 0x40;
    packet.insert(packet.begin() + 4, headerData.begin(), headerData.end());
  }
}

void RpcEncoder::insertHeader(std::vector<uint8_t> &packet, const RpcHeader &header) {
  std::vector<uint8_t> headerData;
  headerData.reserve(1024);
  uint32_t headerSize = encodeHeader(headerData, header);
  if (headerSize > 0) {
    packet.at(3) |= 0x40;
    packet.insert(packet.begin() + 4, headerData.begin(), headerData.end());
  }
}

uint32_t RpcEncoder::encodeHeader(std::vector<char> &packet, const RpcHeader &header) {
  uint32_t oldPacketSize = packet.size();
  uint32_t parameterCount = 0;
  if (!header.authorization.empty()) {
    parameterCount++;
    std::string temp("Authorization");
    BinaryEncoder::encodeString(packet, temp);
    std::string authorization = header.authorization;
    BinaryEncoder::encodeString(packet, authorization);
  } else return 0; //No header
  char result[4];
  HelperFunctions::memcpyBigEndian(result, (char *)&parameterCount, 4);
  packet.insert(packet.begin() + oldPacketSize, result, result + 4);

  uint32_t headerSize = packet.size() - oldPacketSize;
  HelperFunctions::memcpyBigEndian(result, (char *)&headerSize, 4);
  packet.insert(packet.begin() + oldPacketSize, result, result + 4);
  return headerSize;
}

uint32_t RpcEncoder::encodeHeader(std::vector<uint8_t> &packet, const RpcHeader &header) {
  uint32_t oldPacketSize = packet.size();
  uint32_t parameterCount = 0;
  if (!header.authorization.empty()) {
    parameterCount++;
    std::string temp("Authorization");
    BinaryEncoder::encodeString(packet, temp);
    std::string authorization = header.authorization;
    BinaryEncoder::encodeString(packet, authorization);
  } else return 0; //No header
  char result[4];
  HelperFunctions::memcpyBigEndian(result, (char *)&parameterCount, 4);
  packet.insert(packet.begin() + oldPacketSize, result, result + 4);

  uint32_t headerSize = packet.size() - oldPacketSize;
  HelperFunctions::memcpyBigEndian(result, (char *)&headerSize, 4);
  packet.insert(packet.begin() + oldPacketSize, result, result + 4);
  return headerSize;
}

void RpcEncoder::expandPacket(std::vector<char> &packet, size_t sizeToInsert) {
  if (packet.size() + sizeToInsert > packet.capacity()) packet.reserve(packet.size() + sizeToInsert + 1024);
}

void RpcEncoder::expandPacket(std::vector<uint8_t> &packet, size_t sizeToInsert) {
  if (packet.size() + sizeToInsert > packet.capacity()) packet.reserve(packet.size() + sizeToInsert + 1024);
}

void RpcEncoder::encodeVariable(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  if (variable->type == VariableType::tVoid) {
    encodeVoid(packet);
  } else if (variable->type == VariableType::tInteger) {
    if (_forceInteger64) {
      variable->integerValue64 = variable->integerValue;
      encodeInteger64(packet, variable);
    } else encodeInteger(packet, variable);
  } else if (variable->type == VariableType::tInteger64) {
    encodeInteger64(packet, variable);
  } else if (variable->type == VariableType::tFloat) {
    encodeFloat(packet, variable);
  } else if (variable->type == VariableType::tBoolean) {
    encodeBoolean(packet, variable);
  } else if (variable->type == VariableType::tString) {
    encodeString(packet, variable);
  } else if (variable->type == VariableType::tBase64) {
    encodeBase64(packet, variable);
  } else if (variable->type == VariableType::tBinary) {
    encodeBinary(packet, variable);
  } else if (variable->type == VariableType::tStruct) {
    encodeStruct(packet, variable);
  } else if (variable->type == VariableType::tArray) {
    encodeArray(packet, variable);
  }
}

void RpcEncoder::encodeVariable(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  if (variable->type == VariableType::tVoid) {
    encodeVoid(packet);
  } else if (variable->type == VariableType::tInteger) {
    if (_forceInteger64) {
      variable->integerValue64 = variable->integerValue;
      encodeInteger64(packet, variable);
    } else encodeInteger(packet, variable);
  } else if (variable->type == VariableType::tInteger64) {
    encodeInteger64(packet, variable);
  } else if (variable->type == VariableType::tFloat) {
    encodeFloat(packet, variable);
  } else if (variable->type == VariableType::tBoolean) {
    encodeBoolean(packet, variable);
  } else if (variable->type == VariableType::tString) {
    encodeString(packet, variable);
  } else if (variable->type == VariableType::tBase64) {
    encodeBase64(packet, variable);
  } else if (variable->type == VariableType::tBinary) {
    encodeBinary(packet, variable);
  } else if (variable->type == VariableType::tStruct) {
    encodeStruct(packet, variable);
  } else if (variable->type == VariableType::tArray) {
    encodeArray(packet, variable);
  }
}

void RpcEncoder::encodeStruct(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4);
  encodeType(packet, VariableType::tStruct);
  BinaryEncoder::encodeInteger(packet, variable->structValue->size());
  for (auto &element : *variable->structValue) {
    std::string name = element.first.empty() ? "UNDEFINED" : element.first;
    expandPacket(packet, 4 + name.size());
    BinaryEncoder::encodeString(packet, name);
    encodeVariable(packet, element.second ? element.second : std::make_shared<Variable>());
  }
}

void RpcEncoder::encodeStruct(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4);
  encodeType(packet, VariableType::tStruct);
  BinaryEncoder::encodeInteger(packet, variable->structValue->size());
  for (auto &element : *variable->structValue) {
    std::string name = element.first.empty() ? "UNDEFINED" : element.first;
    expandPacket(packet, 4 + name.size());
    BinaryEncoder::encodeString(packet, name);
    encodeVariable(packet, element.second ? element.second : std::make_shared<Variable>());
  }
}

void RpcEncoder::encodeArray(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4);
  encodeType(packet, VariableType::tArray);
  BinaryEncoder::encodeInteger(packet, variable->arrayValue->size());
  for (auto &element : *variable->arrayValue) {
    encodeVariable(packet, element ? element : std::make_shared<Variable>());
  }
}

void RpcEncoder::encodeArray(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4);
  encodeType(packet, VariableType::tArray);
  BinaryEncoder::encodeInteger(packet, variable->arrayValue->size());
  for (auto &element : *variable->arrayValue) {
    encodeVariable(packet, element ? element : std::make_shared<Variable>());
  }
}

void RpcEncoder::encodeType(std::vector<char> &packet, VariableType type) {
  BinaryEncoder::encodeInteger(packet, (int32_t)type);
}

void RpcEncoder::encodeType(std::vector<uint8_t> &packet, VariableType type) {
  BinaryEncoder::encodeInteger(packet, (int32_t)type);
}

void RpcEncoder::encodeInteger(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4);
  encodeType(packet, VariableType::tInteger);
  BinaryEncoder::encodeInteger(packet, variable->integerValue);
}

void RpcEncoder::encodeInteger(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4);
  encodeType(packet, VariableType::tInteger);
  BinaryEncoder::encodeInteger(packet, variable->integerValue);
}

void RpcEncoder::encodeInteger64(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 8);
  encodeType(packet, VariableType::tInteger64);
  BinaryEncoder::encodeInteger64(packet, variable->integerValue64);
}

void RpcEncoder::encodeInteger64(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 8);
  encodeType(packet, VariableType::tInteger64);
  BinaryEncoder::encodeInteger64(packet, variable->integerValue64);
}

void RpcEncoder::encodeFloat(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 8);
  encodeType(packet, VariableType::tFloat);
  BinaryEncoder::encodeFloat(packet, variable->floatValue);
}

void RpcEncoder::encodeFloat(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 8);
  encodeType(packet, VariableType::tFloat);
  BinaryEncoder::encodeFloat(packet, variable->floatValue);
}

void RpcEncoder::encodeBoolean(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 1);
  encodeType(packet, VariableType::tBoolean);
  BinaryEncoder::encodeBoolean(packet, variable->booleanValue);
}

void RpcEncoder::encodeBoolean(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 1);
  encodeType(packet, VariableType::tBoolean);
  BinaryEncoder::encodeBoolean(packet, variable->booleanValue);
}

void RpcEncoder::encodeString(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4 + variable->stringValue.size());
  encodeType(packet, VariableType::tString);
  //We could call encodeRawString here, but then the string would have to be copied and that would cost time.
  BinaryEncoder::encodeInteger(packet, variable->stringValue.size());
  if (!variable->stringValue.empty()) {
    packet.insert(packet.end(), variable->stringValue.begin(), variable->stringValue.end());
  }
}

void RpcEncoder::encodeString(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4 + variable->stringValue.size());
  encodeType(packet, VariableType::tString);
  //We could call encodeRawString here, but then the string would have to be copied and that would cost time.
  BinaryEncoder::encodeInteger(packet, variable->stringValue.size());
  if (!variable->stringValue.empty()) {
    packet.insert(packet.end(), variable->stringValue.begin(), variable->stringValue.end());
  }
}

void RpcEncoder::encodeBase64(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4 + variable->stringValue.size());
  encodeType(packet, VariableType::tBase64);
  //We could call encodeRawString here, but then the string would have to be copied and that would cost time.
  BinaryEncoder::encodeInteger(packet, variable->stringValue.size());
  if (!variable->stringValue.empty()) {
    packet.insert(packet.end(), variable->stringValue.begin(), variable->stringValue.end());
  }
}

void RpcEncoder::encodeBase64(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4 + variable->stringValue.size());
  encodeType(packet, VariableType::tBase64);
  //We could call encodeRawString here, but then the string would have to be copied and that would cost time.
  BinaryEncoder::encodeInteger(packet, variable->stringValue.size());
  if (!variable->stringValue.empty()) {
    packet.insert(packet.end(), variable->stringValue.begin(), variable->stringValue.end());
  }
}

void RpcEncoder::encodeBinary(std::vector<char> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4 + variable->binaryValue.size());
  encodeType(packet, VariableType::tBinary);
  BinaryEncoder::encodeInteger(packet, variable->binaryValue.size());
  if (!variable->binaryValue.empty()) {
    packet.insert(packet.end(), variable->binaryValue.begin(), variable->binaryValue.end());
  }
}

void RpcEncoder::encodeBinary(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable) {
  expandPacket(packet, 4 + 4 + variable->binaryValue.size());
  encodeType(packet, VariableType::tBinary);
  BinaryEncoder::encodeInteger(packet, variable->binaryValue.size());
  if (!variable->binaryValue.empty()) {
    packet.insert(packet.end(), variable->binaryValue.begin(), variable->binaryValue.end());
  }
}

void RpcEncoder::encodeVoid(std::vector<char> &packet) {
  expandPacket(packet, 4 + 4);
  if (_encodeVoid) encodeType(packet, VariableType::tVoid);
  else {
    std::shared_ptr<Variable> string = std::make_shared<Variable>(VariableType::tString);
    encodeString(packet, string);
  }
}

void RpcEncoder::encodeVoid(std::vector<uint8_t> &packet) {
  expandPacket(packet, 4 + 4);
  if (_encodeVoid) encodeType(packet, VariableType::tVoid);
  else {
    std::shared_ptr<Variable> string = std::make_shared<Variable>(VariableType::tString);
    encodeString(packet, string);
  }
}

}
}
