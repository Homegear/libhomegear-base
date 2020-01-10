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

#ifndef BINARYDECODER_H_
#define BINARYDECODER_H_

#include "Ansi.h"
#include "../Exception.h"

#include <memory>
#include <cstring>
#include <vector>
#include <string>

namespace BaseLib
{

class SharedObjects;

class BinaryDecoderException : public BaseLib::Exception
{
public:
    explicit BinaryDecoderException(std::string message) : BaseLib::Exception(message) {}
};

class BinaryDecoder
{
public:
    BinaryDecoder();
    explicit BinaryDecoder(bool ansi);

    /**
     * Dummy constructor for backwards compatability.
     */
	explicit BinaryDecoder(BaseLib::SharedObjects* baseLib);

    /**
     * Dummy constructor for backwards compatability.
     */
	BinaryDecoder(BaseLib::SharedObjects* baseLib, bool ansi);
	~BinaryDecoder() = default;

	int32_t decodeInteger(const std::vector<char>& encodedData, uint32_t& position);
	int32_t decodeInteger(const std::vector<uint8_t>& encodedData, uint32_t& position);
	int64_t decodeInteger64(const std::vector<char>& encodedData, uint32_t& position);
	int64_t decodeInteger64(const std::vector<uint8_t>& encodedData, uint32_t& position);
	uint8_t decodeByte(const std::vector<char>& encodedData, uint32_t& position);
	uint8_t decodeByte(const std::vector<uint8_t>& encodedData, uint32_t& position);
	std::string decodeString(const std::vector<char>& encodedData, uint32_t& position);
	std::string decodeString(const std::vector<uint8_t>& encodedData, uint32_t& position);
	std::vector<uint8_t> decodeBinary(const std::vector<char>& encodedData, uint32_t& position);
	std::vector<uint8_t> decodeBinary(const std::vector<uint8_t>& encodedData, uint32_t& position);
	bool decodeBoolean(const std::vector<char>& encodedData, uint32_t& position);
	bool decodeBoolean(const std::vector<uint8_t>& encodedData, uint32_t& position);
	double decodeFloat(const std::vector<char>& encodedData, uint32_t& position);
	double decodeFloat(const std::vector<uint8_t>& encodedData, uint32_t& position);
protected:
	bool _ansi = false;
	std::shared_ptr<Ansi> _ansiConverter;
};
}
#endif
