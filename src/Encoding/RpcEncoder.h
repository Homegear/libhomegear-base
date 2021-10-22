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

#ifndef RPCENCODER_H_
#define RPCENCODER_H_

#include "RpcHeader.h"
#include "../Variable.h"
#include "BinaryEncoder.h"

#include <memory>
#include <cstring>
#include <list>

namespace BaseLib {

class SharedObjects;

namespace Rpc {

class RpcEncoder {
 public:
  RpcEncoder();
  RpcEncoder(bool forceInteger64, bool encodeVoid);

  /**
   * Dummy constructor for backwards compatibility.
   */
  explicit RpcEncoder(BaseLib::SharedObjects *baseLib);

  /**
   * Dummy constructor for backwards compatibility.
   */
  RpcEncoder(BaseLib::SharedObjects *baseLib, bool forceInteger64, bool encodeVoid);

  ~RpcEncoder() = default;

  static void insertHeader(std::vector<char> &packet, const RpcHeader &header);
  static void insertHeader(std::vector<uint8_t> &packet, const RpcHeader &header);
  void encodeRequest(const std::string &methodName, const std::shared_ptr<std::list<std::shared_ptr<Variable>>> &parameters, std::vector<char> &encodedData, const std::shared_ptr<RpcHeader> &header = nullptr);
  void encodeRequest(const std::string &methodName, const std::shared_ptr<std::list<std::shared_ptr<Variable>>> &parameters, std::vector<uint8_t> &encodedData, const std::shared_ptr<RpcHeader> &header = nullptr);
  void encodeRequest(const std::string &methodName, const PArray &parameters, std::vector<char> &encodedData, const std::shared_ptr<RpcHeader> &header = nullptr);
  void encodeRequest(const std::string &methodName, const PArray &parameters, std::vector<uint8_t> &encodedData, const std::shared_ptr<RpcHeader> &header = nullptr);
  void encodeResponse(const std::shared_ptr<Variable> &variable, std::vector<char> &encodedData);
  void encodeResponse(const std::shared_ptr<Variable> &variable, std::vector<uint8_t> &encodedData);
 private:
  bool _forceInteger64 = false;
  bool _encodeVoid = false;
  char _packetStartRequest[4];
  char _packetStartResponse[5];
  char _packetStartError[5];

  static void expandPacket(std::vector<char> &packet, size_t sizeToInsert);
  static void expandPacket(std::vector<uint8_t> &packet, size_t sizeToInsert);
  static uint32_t encodeHeader(std::vector<char> &packet, const RpcHeader &header);
  static uint32_t encodeHeader(std::vector<uint8_t> &packet, const RpcHeader &header);
  void encodeVariable(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  void encodeVariable(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeInteger(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeInteger(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeInteger64(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeInteger64(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeFloat(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeFloat(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeBoolean(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeBoolean(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeType(std::vector<char> &packet, VariableType type);
  static void encodeType(std::vector<uint8_t> &packet, VariableType type);
  static void encodeString(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeString(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeBase64(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeBase64(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeBinary(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  static void encodeBinary(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
  void encodeVoid(std::vector<char> &packet);
  void encodeVoid(std::vector<uint8_t> &packet);
  void encodeStruct(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  void encodeStruct(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
  void encodeArray(std::vector<char> &packet, const std::shared_ptr<Variable> &variable);
  void encodeArray(std::vector<uint8_t> &packet, const std::shared_ptr<Variable> &variable);
};
}
}
#endif
