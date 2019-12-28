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

#ifndef RPCDECODER_H_
#define RPCDECODER_H_

#include <memory>
#include <vector>
#include <cstring>
#include <cmath>

#include "../Variable.h"
#include "BinaryDecoder.h"
#include "RpcHeader.h"

namespace BaseLib
{

class SharedObjects;

namespace Rpc
{
class RpcDecoder
{
public:
	RpcDecoder(BaseLib::SharedObjects* baseLib);
	RpcDecoder(BaseLib::SharedObjects* baseLib, bool ansi, bool setInteger32 = true);
	virtual ~RpcDecoder() {}

	virtual std::shared_ptr<RpcHeader> decodeHeader(const std::vector<char>& packet);
	virtual std::shared_ptr<RpcHeader> decodeHeader(const std::vector<uint8_t>& packet);
	virtual std::shared_ptr<std::vector<std::shared_ptr<Variable>>> decodeRequest(const std::vector<char>& packet, std::string& methodName);
	virtual std::shared_ptr<std::vector<std::shared_ptr<Variable>>> decodeRequest(const std::vector<uint8_t>& packet, std::string& methodName);
	virtual std::shared_ptr<Variable> decodeResponse(const std::vector<char>& packet, uint32_t offset = 0);
	virtual std::shared_ptr<Variable> decodeResponse(const std::vector<uint8_t>& packet, uint32_t offset = 0);
private:
	BaseLib::SharedObjects* _bl = nullptr;
	bool _ansi = false;
	std::unique_ptr<BinaryDecoder> _decoder;
	bool _setInteger32 = true;

	std::shared_ptr<Variable> decodeParameter(const std::vector<char>& packet, uint32_t& position);
	std::shared_ptr<Variable> decodeParameter(const std::vector<uint8_t>& packet, uint32_t& position);
	VariableType decodeType(const std::vector<char>& packet, uint32_t& position);
	VariableType decodeType(const std::vector<uint8_t>& packet, uint32_t& position);
	std::shared_ptr<Array> decodeArray(const std::vector<char>& packet, uint32_t& position);
	std::shared_ptr<Array> decodeArray(const std::vector<uint8_t>& packet, uint32_t& position);
	std::shared_ptr<Struct> decodeStruct(const std::vector<char>& packet, uint32_t& position);
	std::shared_ptr<Struct> decodeStruct(const std::vector<uint8_t>& packet, uint32_t& position);
};
}
}
#endif
