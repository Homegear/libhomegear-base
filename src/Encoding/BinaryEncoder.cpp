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

#include "BinaryEncoder.h"
#include "../BaseLib.h"

namespace BaseLib {

BinaryEncoder::BinaryEncoder(BaseLib::SharedObjects *baseLib) {
}

void BinaryEncoder::encodeInteger(std::vector<char> &encodedData, int32_t integer) {
  char result[4];
  HelperFunctions::memcpyBigEndian(result, (char *)&integer, 4);
  encodedData.insert(encodedData.end(), result, result + 4);
}

void BinaryEncoder::encodeInteger(std::vector<uint8_t> &encodedData, int32_t integer) {
  uint8_t result[4];
  HelperFunctions::memcpyBigEndian(result, (uint8_t *)&integer, 4);
  encodedData.insert(encodedData.end(), result, result + 4);
}

void BinaryEncoder::encodeInteger64(std::vector<char> &encodedData, int64_t integer) {
  char result[8];
  HelperFunctions::memcpyBigEndian(result, (char *)&integer, 8);
  encodedData.insert(encodedData.end(), result, result + 8);
}

void BinaryEncoder::encodeInteger64(std::vector<uint8_t> &encodedData, int64_t integer) {
  char result[8];
  HelperFunctions::memcpyBigEndian(result, (char *)&integer, 8);
  encodedData.insert(encodedData.end(), result, result + 8);
}

void BinaryEncoder::encodeByte(std::vector<char> &encodedData, uint8_t byte) {
  encodedData.push_back(byte);
}

void BinaryEncoder::encodeByte(std::vector<uint8_t> &encodedData, uint8_t byte) {
  encodedData.push_back(byte);
}

void BinaryEncoder::encodeString(std::vector<char> &encodedData, const std::string &string) {
  encodeInteger(encodedData, string.size());
  if (!string.empty()) encodedData.insert(encodedData.end(), string.begin(), string.end());
}

void BinaryEncoder::encodeString(std::vector<uint8_t> &encodedData, const std::string &string) {
  encodeInteger(encodedData, string.size());
  if (!string.empty()) encodedData.insert(encodedData.end(), string.begin(), string.end());
}

void BinaryEncoder::encodeBinary(std::vector<char> &encodedData, const std::vector<uint8_t> &data) {
  encodeInteger(encodedData, data.size());
  if (!data.empty()) encodedData.insert(encodedData.end(), data.begin(), data.end());
}

void BinaryEncoder::encodeBinary(std::vector<uint8_t> &encodedData, const std::vector<uint8_t> &data) {
  encodeInteger(encodedData, data.size());
  if (!data.empty()) encodedData.insert(encodedData.end(), data.begin(), data.end());
}

void BinaryEncoder::encodeBoolean(std::vector<char> &encodedData, bool boolean) {
  encodedData.push_back((char)boolean);
}

void BinaryEncoder::encodeBoolean(std::vector<uint8_t> &encodedData, bool boolean) {
  encodedData.push_back((uint8_t)boolean);
}

void BinaryEncoder::encodeFloat(std::vector<char> &encodedData, double floatValue) {
  double temp = std::abs(floatValue);
  int32_t exponent = 0;
  if (temp != 0 && temp < 0.5) {
    while (temp < 0.5) {
      temp *= 2;
      exponent--;
    }
  } else
    while (temp >= 1) {
      temp /= 2;
      exponent++;
    }
  if (floatValue < 0) temp *= -1;
  int32_t mantissa = std::lround(temp * 0x40000000);
  char data[8];
  HelperFunctions::memcpyBigEndian(data, (char *)&mantissa, 4);
  HelperFunctions::memcpyBigEndian(data + 4, (char *)&exponent, 4);
  encodedData.insert(encodedData.end(), data, data + 8);
}

void BinaryEncoder::encodeFloat(std::vector<uint8_t> &encodedData, double floatValue) {
  double temp = std::abs(floatValue);
  int32_t exponent = 0;
  if (temp != 0 && temp < 0.5) {
    while (temp < 0.5) {
      temp *= 2;
      exponent--;
    }
  } else
    while (temp >= 1) {
      temp /= 2;
      exponent++;
    }
  if (floatValue < 0) temp *= -1;
  int32_t mantissa = std::lround(temp * 0x40000000);
  char data[8];
  HelperFunctions::memcpyBigEndian(data, (char *)&mantissa, 4);
  HelperFunctions::memcpyBigEndian(data + 4, (char *)&exponent, 4);
  encodedData.insert(encodedData.end(), data, data + 8);
}

}
