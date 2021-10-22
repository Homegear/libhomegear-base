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

#include "BinaryDecoder.h"
#include "../BaseLib.h"

namespace BaseLib {

BinaryDecoder::BinaryDecoder(bool ansi) {
  _ansi = ansi;
  if (_ansi) _ansiConverter = std::make_shared<Ansi>(true, false);
}

BinaryDecoder::BinaryDecoder(BaseLib::SharedObjects *baseLib) {
}

BinaryDecoder::BinaryDecoder(BaseLib::SharedObjects *baseLib, bool ansi) : BinaryDecoder(ansi) {
}

int32_t BinaryDecoder::decodeInteger(const std::vector<char> &encodedData, uint32_t &position) {
  int32_t integer = 0;
  if (position + 4 > encodedData.size()) {
    if (position + 1 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
    //IP-Symcon encodes integers as string => Difficult to interpret. This works for numbers up to 3 digits:
    std::string string(&encodedData.at(position), encodedData.size());
    position = encodedData.size();
    integer = Math::getNumber(string);
    return integer;
  }
  HelperFunctions::memcpyBigEndian((char *)&integer, &encodedData.at(position), 4);
  position += 4;
  return integer;
}

int32_t BinaryDecoder::decodeInteger(const std::vector<uint8_t> &encodedData, uint32_t &position) {
  int32_t integer = 0;
  if (position + 4 > encodedData.size()) {
    if (position + 1 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
    //IP-Symcon encodes integers as string => Difficult to interpret. This works for numbers up to 3 digits:
    std::string string((char *)&encodedData.at(position), encodedData.size());
    position = encodedData.size();
    integer = Math::getNumber(string);
    return integer;
  }
  HelperFunctions::memcpyBigEndian((char *)&integer, (char *)&encodedData.at(position), 4);
  position += 4;
  return integer;
}

int64_t BinaryDecoder::decodeInteger64(const std::vector<char> &encodedData, uint32_t &position) {
  int64_t integer = 0;
  if (position + 8 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  HelperFunctions::memcpyBigEndian((char *)&integer, &encodedData.at(position), 8);
  position += 8;
  return integer;
}

int64_t BinaryDecoder::decodeInteger64(const std::vector<uint8_t> &encodedData, uint32_t &position) {
  int64_t integer = 0;
  if (position + 8 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  HelperFunctions::memcpyBigEndian((char *)&integer, (char *)&encodedData.at(position), 8);
  position += 8;
  return integer;
}

uint8_t BinaryDecoder::decodeByte(const std::vector<char> &encodedData, uint32_t &position) {
  uint8_t byte = 0;
  if (position + 1 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  byte = encodedData.at(position);
  position += 1;
  return byte;
}

uint8_t BinaryDecoder::decodeByte(const std::vector<uint8_t> &encodedData, uint32_t &position) {
  uint8_t byte = 0;
  if (position + 1 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  byte = encodedData.at(position);
  position += 1;
  return byte;
}

std::string BinaryDecoder::decodeString(const std::vector<char> &encodedData, uint32_t &position) {
  int32_t stringLength = decodeInteger(encodedData, position);
  if (stringLength == 0) return "";
  if (position + stringLength > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  if (_ansi && _ansiConverter) {
    std::string string = std::move(_ansiConverter->toUtf8((char *)&encodedData.at(position), stringLength));
    position += stringLength;
    return string;
  } else {
    std::string string(&encodedData.at(position), stringLength);
    position += stringLength;
    return string;
  }
}

std::string BinaryDecoder::decodeString(const std::vector<uint8_t> &encodedData, uint32_t &position) {
  int32_t stringLength = decodeInteger(encodedData, position);
  if (stringLength == 0) return "";
  if (position + stringLength > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  if (_ansi && _ansiConverter) {
    std::string string = std::move(_ansiConverter->toUtf8((char *)&encodedData.at(position), stringLength));
    position += stringLength;
    return string;
  } else {
    std::string string((char *)&encodedData.at(position), stringLength);
    position += stringLength;
    return string;
  }
}

std::vector<uint8_t> BinaryDecoder::decodeBinary(const std::vector<char> &encodedData, uint32_t &position) {
  int32_t length = decodeInteger(encodedData, position);
  if (length == 0) return std::vector<uint8_t>();
  if (position + length > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  std::vector<uint8_t> data(encodedData.begin() + position, encodedData.begin() + position + length);
  position += length;
  return data;
}

std::vector<uint8_t> BinaryDecoder::decodeBinary(const std::vector<uint8_t> &encodedData, uint32_t &position) {
  int32_t length = decodeInteger(encodedData, position);
  if (length == 0) return std::vector<uint8_t>();
  if (position + length > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  std::vector<uint8_t> data(encodedData.begin() + position, encodedData.begin() + position + length);
  position += length;
  return data;
}

double BinaryDecoder::decodeFloat(const std::vector<char> &encodedData, uint32_t &position) {
  if (position + 8 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  int32_t mantissa = 0;
  int32_t exponent = 0;
  HelperFunctions::memcpyBigEndian((char *)&mantissa, &encodedData.at(position), 4);
  position += 4;
  HelperFunctions::memcpyBigEndian((char *)&exponent, &encodedData.at(position), 4);
  position += 4;
  double floatValue = (double)mantissa / 0x40000000;
  floatValue *= std::pow(2, exponent);
  if (floatValue != 0) {
    int32_t digits = std::lround(std::floor(std::log10(floatValue) + 1));
    double factor = std::pow(10, 9 - digits);
    //Round to 9 digits
    floatValue = std::floor(floatValue * factor + 0.5) / factor;
  }
  return floatValue;
}

double BinaryDecoder::decodeFloat(const std::vector<uint8_t> &encodedData, uint32_t &position) {
  if (position + 8 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  int32_t mantissa = 0;
  int32_t exponent = 0;
  HelperFunctions::memcpyBigEndian((char *)&mantissa, (char *)&encodedData.at(position), 4);
  position += 4;
  HelperFunctions::memcpyBigEndian((char *)&exponent, (char *)&encodedData.at(position), 4);
  position += 4;
  double floatValue = (double)mantissa / 0x40000000;
  if (exponent >= 0) floatValue *= (1 << exponent);
  else floatValue /= (1 << (exponent * -1));
  if (floatValue != 0) {
    int32_t digits = std::lround(std::floor(std::log10(floatValue) + 1));
    double factor = std::pow(10, 9 - digits);
    //Round to 9 digits
    floatValue = std::floor(floatValue * factor + 0.5) / factor;
  }
  return floatValue;
}

bool BinaryDecoder::decodeBoolean(const std::vector<char> &encodedData, uint32_t &position) {
  if (position + 1 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  bool boolean = (bool)encodedData.at(position);
  position += 1;
  return boolean;
}

bool BinaryDecoder::decodeBoolean(const std::vector<uint8_t> &encodedData, uint32_t &position) {
  if (position + 1 > encodedData.size()) throw BinaryDecoderException("Unexpected end of data.");
  bool boolean = (bool)encodedData.at(position);
  position += 1;
  return boolean;
}

}
