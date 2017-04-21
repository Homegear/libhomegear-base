/* Copyright 2013-2017 Sathya Laufer
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

namespace BaseLib
{

class SharedObjects;

namespace Rpc
{

class RpcEncoder
{
public:
	RpcEncoder(BaseLib::SharedObjects* baseLib);
	RpcEncoder(BaseLib::SharedObjects* baseLib, bool forceInteger64);
	virtual ~RpcEncoder() {}

	virtual void insertHeader(std::vector<char>& packet, const RpcHeader& header);
	virtual void insertHeader(std::vector<uint8_t>& packet, const RpcHeader& header);
	virtual void encodeRequest(std::string methodName, std::shared_ptr<std::list<std::shared_ptr<Variable>>> parameters, std::vector<char>& encodedData, std::shared_ptr<RpcHeader> header = nullptr);
	virtual void encodeRequest(std::string methodName, std::shared_ptr<std::list<std::shared_ptr<Variable>>> parameters, std::vector<uint8_t>& encodedData, std::shared_ptr<RpcHeader> header = nullptr);
	virtual void encodeRequest(std::string methodName, PArray parameters, std::vector<char>& encodedData, std::shared_ptr<RpcHeader> header = nullptr);
	virtual void encodeRequest(std::string methodName, PArray parameters, std::vector<uint8_t>& encodedData, std::shared_ptr<RpcHeader> header = nullptr);
	virtual void encodeResponse(std::shared_ptr<Variable> variable, std::vector<char>& encodedData);
	virtual void encodeResponse(std::shared_ptr<Variable> variable, std::vector<uint8_t>& encodedData);
private:
	BaseLib::SharedObjects* _bl = nullptr;
	bool _forceInteger64 = false;
	std::unique_ptr<BinaryEncoder> _encoder;
	char _packetStartRequest[4];
	char _packetStartResponse[5];
	char _packetStartError[5];

	uint32_t encodeHeader(std::vector<char>& packet, const RpcHeader& header);
	uint32_t encodeHeader(std::vector<uint8_t>& packet, const RpcHeader& header);
	void encodeVariable(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeVariable(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
	void encodeInteger(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeInteger(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
	void encodeInteger64(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeInteger64(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
	void encodeFloat(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeFloat(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
	void encodeBoolean(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeBoolean(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
	void encodeType(std::vector<char>& packet, VariableType type);
	void encodeType(std::vector<uint8_t>& packet, VariableType type);
	void encodeString(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeString(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
	void encodeBase64(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeBase64(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
	void encodeBinary(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeBinary(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
	void encodeVoid(std::vector<char>& packet);
	void encodeVoid(std::vector<uint8_t>& packet);
	void encodeStruct(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeStruct(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
	void encodeArray(std::vector<char>& packet, std::shared_ptr<Variable>& variable);
	void encodeArray(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable);
};
}
}
#endif
