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

#ifndef BINARYENCODER_H_
#define BINARYENCODER_H_

#include <memory>
#include <cstring>
#include <vector>
#include <string>

namespace BaseLib
{

class SharedObjects;

class BinaryEncoder
{
public:
    BinaryEncoder() = default;

    /**
     * Dummy constructor for backwards compatability.
     */
    explicit BinaryEncoder(BaseLib::SharedObjects* baseLib);

	static void encodeInteger(std::vector<char>& encodedData, int32_t integer);
    static void encodeInteger(std::vector<uint8_t>& encodedData, int32_t integer);
    static void encodeInteger64(std::vector<char>& encodedData, int64_t integer);
    static void encodeInteger64(std::vector<uint8_t>& encodedData, int64_t integer);
    static void encodeByte(std::vector<char>& encodedData, uint8_t byte);
    static void encodeByte(std::vector<uint8_t>& encodedData, uint8_t byte);
    static void encodeString(std::vector<char>& packet, const std::string& string);
    static void encodeString(std::vector<uint8_t>& encodedData, const std::string& string);
    static void encodeBinary(std::vector<char>& packet, const std::vector<uint8_t>& data);
    static void encodeBinary(std::vector<uint8_t>& encodedData, const std::vector<uint8_t>& data);
    static void encodeBoolean(std::vector<char>& encodedData, bool boolean);
    static void encodeBoolean(std::vector<uint8_t>& encodedData, bool boolean);
    static void encodeFloat(std::vector<char>& encodedData, double floatValue);
    static void encodeFloat(std::vector<uint8_t>& encodedData, double floatValue);
};
}
#endif
