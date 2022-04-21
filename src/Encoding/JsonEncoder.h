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

#ifndef JSONENCODER_H_
#define JSONENCODER_H_

#include "../Exception.h"
#include "../Variable.h"

#include <list>
#if __GNUC__ > 4
#include <codecvt>
#endif

namespace BaseLib {

class SharedObjects;

namespace Rpc {

class JsonEncoder {
 public:
  JsonEncoder() = default;
  explicit JsonEncoder(BaseLib::SharedObjects *dummy) {}
  virtual ~JsonEncoder() = default;

  static void encode(const std::shared_ptr<Variable> &variable, std::string &json);
  static void encode(const std::shared_ptr<Variable> &variable, std::vector<char> &json);
  static std::string encode(const std::shared_ptr<Variable> &variable);
  static std::vector<char> encodeBinary(const std::shared_ptr<Variable> &variable);
  void encodeRequest(std::string &methodName, std::shared_ptr<std::list<std::shared_ptr<Variable>>> &parameters, std::vector<char> &encodedData);
  void encodeRequest(std::string &methodName, std::shared_ptr<Variable> &parameters, std::vector<char> &encodedData);
  static void encodeResponse(const std::shared_ptr<Variable> &variable, int32_t id, std::vector<char> &json);
  static void encodeMQTTResponse(const std::string &methodName, const std::shared_ptr<Variable> &variable, int32_t id, std::vector<char> &json);

  static std::string encodeString(const std::string &s);
 private:
  int32_t _requestId = 1;

  static void encodeValue(const std::shared_ptr<Variable> &variable, std::ostringstream &s);
  static void encodeValue(const std::shared_ptr<Variable> &variable, std::vector<char> &s);
  static void encodeArray(const std::shared_ptr<Variable> &variable, std::ostringstream &s);
  static void encodeArray(const std::shared_ptr<Variable> &variable, std::vector<char> &s);
  static void encodeStruct(const std::shared_ptr<Variable> &variable, std::ostringstream &s);
  static void encodeStruct(const std::shared_ptr<Variable> &variable, std::vector<char> &s);
  static void encodeBoolean(const std::shared_ptr<Variable> &variable, std::ostringstream &s);
  static void encodeBoolean(const std::shared_ptr<Variable> &variable, std::vector<char> &s);
  static void encodeInteger(const std::shared_ptr<Variable> &variable, std::ostringstream &s);
  static void encodeInteger(const std::shared_ptr<Variable> &variable, std::vector<char> &s);
  static void encodeInteger64(const std::shared_ptr<Variable> &variable, std::ostringstream &s);
  static void encodeInteger64(const std::shared_ptr<Variable> &variable, std::vector<char> &s);
  static void encodeFloat(const std::shared_ptr<Variable> &variable, std::ostringstream &s);
  static void encodeFloat(const std::shared_ptr<Variable> &variable, std::vector<char> &s);
  static void encodeString(const std::shared_ptr<Variable> &variable, std::ostringstream &s);
  static void encodeString(const std::shared_ptr<Variable> &variable, std::vector<char> &s);
  static void encodeVoid(const std::shared_ptr<Variable> &variable, std::ostringstream &s);
  static void encodeVoid(const std::shared_ptr<Variable> &variable, std::vector<char> &s);
};
}
}

#endif
