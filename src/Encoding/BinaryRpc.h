/* Copyright 2013-2016 Sathya Laufer
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

#ifndef BINARYRPC_H_
#define BINARYRPC_H_

#include "../Variable.h"
#include "../Exception.h"

namespace BaseLib
{

class Obj;

namespace Rpc
{

class BinaryRpcException : public BaseLib::Exception
{
public:
	BinaryRpcException(std::string message) : BaseLib::Exception(message) {}
};

class BinaryRpc
{
public:
	enum class Type
	{
		unknown,
		request,
		response
	};

	BinaryRpc(BaseLib::Obj* bl);
	virtual ~BinaryRpc();

	Type getType() { return _type; }
	bool hasHeader() { return _hasHeader; }
	bool processingStarted() { return _processingStarted; }
	bool isFinished() { return _finished; }
	std::vector<char>& getData() { return _data; }

	void reset();

	/**
	 * Parses binary RPC data from a buffer.
	 *
	 * @param buffer The buffer to parse
	 * @param bufferLength The maximum number of bytes to process.
	 * @return The number of processed bytes.
	 */
	int32_t process(char* buffer, int32_t bufferLength);
private:
	Obj* _bl = nullptr;
	bool _hasHeader = false;
	bool _processingStarted = false;
	bool _finished = false;
	Type _type = Type::unknown;
	uint32_t _headerSize = 0;
	uint32_t _dataSize = 0;
	std::vector<char> _data;
};
}
}
#endif
