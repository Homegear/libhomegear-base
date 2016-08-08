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

#include "RpcEncoder.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Rpc
{

RpcEncoder::RpcEncoder(BaseLib::Obj* baseLib)
{
	_bl = baseLib;
	_encoder = std::unique_ptr<BinaryEncoder>(new BinaryEncoder(baseLib));

	strncpy(&_packetStartRequest[0], "Bin", 4);
	strncpy(&_packetStartResponse[0], "Bin", 4);
	_packetStartResponse[3] = 1;
	_packetStartResponse[4] = 0;
	strncpy(&_packetStartError[0], "Bin", 4);
	_packetStartError[3] = 0xFF;
	_packetStartError[4] = 0;
}

RpcEncoder::RpcEncoder(BaseLib::Obj* baseLib, bool forceInteger64) : RpcEncoder(baseLib)
{
	_forceInteger64 = forceInteger64;
}

void RpcEncoder::encodeRequest(std::string methodName, std::shared_ptr<std::list<std::shared_ptr<Variable>>> parameters, std::vector<char>& encodedData, std::shared_ptr<RpcHeader> header)
{
	//The "Bin", the type byte after that and the length itself are not part of the length
	try
	{
		encodedData.clear();
		encodedData.insert(encodedData.begin(), _packetStartRequest, _packetStartRequest + 4);
		uint32_t headerSize = 0;
		if(header)
		{
			headerSize = encodeHeader(encodedData, *header) + 4;
			if(headerSize > 0) encodedData.at(3) |= 0x40;
		}
		_encoder->encodeString(encodedData, methodName);
		if(!parameters) _encoder->encodeInteger(encodedData, 0);
		else _encoder->encodeInteger(encodedData, parameters->size());
		if(parameters)
		{
			for(std::list<std::shared_ptr<Variable>>::iterator i = parameters->begin(); i != parameters->end(); ++i)
			{
				encodeVariable(encodedData, (*i));
			}
		}

		uint32_t dataSize = encodedData.size() - 4 - headerSize;
		char result[4];
		_bl->hf.memcpyBigEndian(result, (char*)&dataSize, 4);
		encodedData.insert(encodedData.begin() + 4 + headerSize, result, result + 4);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeRequest(std::string methodName, std::shared_ptr<std::list<std::shared_ptr<Variable>>> parameters, std::vector<uint8_t>& encodedData, std::shared_ptr<RpcHeader> header)
{
	//The "Bin", the type byte after that and the length itself are not part of the length
	try
	{
		encodedData.clear();
		encodedData.insert(encodedData.begin(), _packetStartRequest, _packetStartRequest + 4);
		uint32_t headerSize = 0;
		if(header)
		{
			headerSize = encodeHeader(encodedData, *header) + 4;
			if(headerSize > 0) encodedData.at(3) |= 0x40;
		}
		_encoder->encodeString(encodedData, methodName);
		if(!parameters) _encoder->encodeInteger(encodedData, 0);
		else _encoder->encodeInteger(encodedData, parameters->size());
		if(parameters)
		{
			for(std::list<std::shared_ptr<Variable>>::iterator i = parameters->begin(); i != parameters->end(); ++i)
			{
				encodeVariable(encodedData, (*i));
			}
		}

		uint32_t dataSize = encodedData.size() - 4 - headerSize;
		char result[4];
		_bl->hf.memcpyBigEndian(result, (char*)&dataSize, 4);
		encodedData.insert(encodedData.begin() + 4 + headerSize, result, result + 4);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeRequest(std::string methodName, PArray parameters, std::vector<char>& encodedData, std::shared_ptr<RpcHeader> header)
{
	//The "Bin", the type byte after that and the length itself are not part of the length
	try
	{
		encodedData.clear();
		encodedData.insert(encodedData.begin(), _packetStartRequest, _packetStartRequest + 4);
		uint32_t headerSize = 0;
		if(header)
		{
			headerSize = encodeHeader(encodedData, *header) + 4;
			if(headerSize > 0) encodedData.at(3) |= 0x40;
		}
		_encoder->encodeString(encodedData, methodName);
		if(!parameters) _encoder->encodeInteger(encodedData, 0);
		else _encoder->encodeInteger(encodedData, parameters->size());
		if(parameters)
		{
			for(Array::iterator i = parameters->begin(); i != parameters->end(); ++i)
			{
				encodeVariable(encodedData, (*i));
			}
		}

		uint32_t dataSize = encodedData.size() - 4 - headerSize;
		char result[4];
		_bl->hf.memcpyBigEndian(result, (char*)&dataSize, 4);
		encodedData.insert(encodedData.begin() + 4 + headerSize, result, result + 4);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeRequest(std::string methodName, PArray parameters, std::vector<uint8_t>& encodedData, std::shared_ptr<RpcHeader> header)
{
	//The "Bin", the type byte after that and the length itself are not part of the length
	try
	{
		encodedData.clear();
		encodedData.insert(encodedData.begin(), _packetStartRequest, _packetStartRequest + 4);
		uint32_t headerSize = 0;
		if(header)
		{
			headerSize = encodeHeader(encodedData, *header) + 4;
			if(headerSize > 0) encodedData.at(3) |= 0x40;
		}
		_encoder->encodeString(encodedData, methodName);
		if(!parameters) _encoder->encodeInteger(encodedData, 0);
		else _encoder->encodeInteger(encodedData, parameters->size());
		if(parameters)
		{
			for(Array::iterator i = parameters->begin(); i != parameters->end(); ++i)
			{
				encodeVariable(encodedData, (*i));
			}
		}

		uint32_t dataSize = encodedData.size() - 4 - headerSize;
		char result[4];
		_bl->hf.memcpyBigEndian(result, (char*)&dataSize, 4);
		encodedData.insert(encodedData.begin() + 4 + headerSize, result, result + 4);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeResponse(std::shared_ptr<Variable> variable, std::vector<char>& encodedData)
{
	//The "Bin", the type byte after that and the length itself are not part of the length
	try
	{
		encodedData.clear();
		if(!variable) variable.reset(new Variable(VariableType::tVoid));
		if(variable->errorStruct) encodedData.insert(encodedData.begin(), _packetStartError, _packetStartError + 4);
		else encodedData.insert(encodedData.begin(), _packetStartResponse, _packetStartResponse + 4);

		encodeVariable(encodedData, variable);

		uint32_t dataSize = encodedData.size() - 4;
		char result[4];
		_bl->hf.memcpyBigEndian(result, (char*)&dataSize, 4);
		encodedData.insert(encodedData.begin() + 4, result, result + 4);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeResponse(std::shared_ptr<Variable> variable, std::vector<uint8_t>& encodedData)
{
	//The "Bin", the type byte after that and the length itself are not part of the length
	try
	{
		encodedData.clear();
		if(!variable) variable.reset(new Variable(VariableType::tVoid));
		if(variable->errorStruct) encodedData.insert(encodedData.begin(), _packetStartError, _packetStartError + 4);
		else encodedData.insert(encodedData.begin(), _packetStartResponse, _packetStartResponse + 4);

		encodeVariable(encodedData, variable);

		uint32_t dataSize = encodedData.size() - 4;
		char result[4];
		_bl->hf.memcpyBigEndian(result, (char*)&dataSize, 4);
		encodedData.insert(encodedData.begin() + 4, result, result + 4);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::insertHeader(std::vector<char>& packet, const RpcHeader& header)
{
	std::vector<char> headerData;
	uint32_t headerSize = encodeHeader(headerData, header);
	if(headerSize > 0)
	{
		packet.at(3) |= 0x40;
		packet.insert(packet.begin() + 4, headerData.begin(), headerData.end());
	}
}

void RpcEncoder::insertHeader(std::vector<uint8_t>& packet, const RpcHeader& header)
{
	std::vector<uint8_t> headerData;
	uint32_t headerSize = encodeHeader(headerData, header);
	if(headerSize > 0)
	{
		packet.at(3) |= 0x40;
		packet.insert(packet.begin() + 4, headerData.begin(), headerData.end());
	}
}

uint32_t RpcEncoder::encodeHeader(std::vector<char>& packet, const RpcHeader& header)
{
	uint32_t oldPacketSize = packet.size();
	uint32_t parameterCount = 0;
	if(!header.authorization.empty())
	{
		parameterCount++;
		std::string temp("Authorization");
		_encoder->encodeString(packet, temp);
		std::string authorization = header.authorization;
		_encoder->encodeString(packet, authorization);
	}
	else return 0; //No header
	char result[4];
	_bl->hf.memcpyBigEndian(result, (char*)&parameterCount, 4);
	packet.insert(packet.begin() + oldPacketSize, result, result + 4);

	uint32_t headerSize = packet.size() - oldPacketSize;
	_bl->hf.memcpyBigEndian(result, (char*)&headerSize, 4);
	packet.insert(packet.begin() + oldPacketSize, result, result + 4);
	return headerSize;
}

uint32_t RpcEncoder::encodeHeader(std::vector<uint8_t>& packet, const RpcHeader& header)
{
	uint32_t oldPacketSize = packet.size();
	uint32_t parameterCount = 0;
	if(!header.authorization.empty())
	{
		parameterCount++;
		std::string temp("Authorization");
		_encoder->encodeString(packet, temp);
		std::string authorization = header.authorization;
		_encoder->encodeString(packet, authorization);
	}
	else return 0; //No header
	char result[4];
	_bl->hf.memcpyBigEndian(result, (char*)&parameterCount, 4);
	packet.insert(packet.begin() + oldPacketSize, result, result + 4);

	uint32_t headerSize = packet.size() - oldPacketSize;
	_bl->hf.memcpyBigEndian(result, (char*)&headerSize, 4);
	packet.insert(packet.begin() + oldPacketSize, result, result + 4);
	return headerSize;
}

void RpcEncoder::encodeVariable(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		if(!variable) variable.reset(new Variable(VariableType::tVoid));
		if(variable->type == VariableType::tVoid)
		{
			encodeVoid(packet);
		}
		else if(variable->type == VariableType::tInteger)
		{
			if(_forceInteger64)
			{
				variable->integerValue64 = variable->integerValue;
				encodeInteger64(packet, variable);
			}
			else encodeInteger(packet, variable);
		}
		else if(variable->type == VariableType::tInteger64)
		{
			encodeInteger64(packet, variable);
		}
		else if(variable->type == VariableType::tFloat)
		{
			encodeFloat(packet, variable);
		}
		else if(variable->type == VariableType::tBoolean)
		{
			encodeBoolean(packet, variable);
		}
		else if(variable->type == VariableType::tString)
		{
			encodeString(packet, variable);
		}
		else if(variable->type == VariableType::tBase64)
		{
			encodeBase64(packet, variable);
		}
		else if(variable->type == VariableType::tBinary)
		{
			encodeBinary(packet, variable);
		}
		else if(variable->type == VariableType::tStruct)
		{
			encodeStruct(packet, variable);
		}
		else if(variable->type == VariableType::tArray)
		{
			encodeArray(packet, variable);
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeVariable(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		if(!variable) variable.reset(new Variable(VariableType::tVoid));
		if(variable->type == VariableType::tVoid)
		{
			encodeVoid(packet);
		}
		else if(variable->type == VariableType::tInteger)
		{
			if(_forceInteger64)
			{
				variable->integerValue64 = variable->integerValue;
				encodeInteger64(packet, variable);
			}
			else encodeInteger(packet, variable);
		}
		else if(variable->type == VariableType::tInteger64)
		{
			encodeInteger64(packet, variable);
		}
		else if(variable->type == VariableType::tFloat)
		{
			encodeFloat(packet, variable);
		}
		else if(variable->type == VariableType::tBoolean)
		{
			encodeBoolean(packet, variable);
		}
		else if(variable->type == VariableType::tString)
		{
			encodeString(packet, variable);
		}
		else if(variable->type == VariableType::tBase64)
		{
			encodeBase64(packet, variable);
		}
		else if(variable->type == VariableType::tBinary)
		{
			encodeBinary(packet, variable);
		}
		else if(variable->type == VariableType::tStruct)
		{
			encodeStruct(packet, variable);
		}
		else if(variable->type == VariableType::tArray)
		{
			encodeArray(packet, variable);
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeStruct(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tStruct);
		_encoder->encodeInteger(packet, variable->structValue->size());
		for(Struct::iterator i = variable->structValue->begin(); i != variable->structValue->end(); ++i)
		{
			std::string name = i->first.empty() ? "UNDEFINED" : i->first;
			_encoder->encodeString(packet, name);
			if(!i->second) i->second.reset(new Variable(VariableType::tVoid));
			encodeVariable(packet, i->second);
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeStruct(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tStruct);
		_encoder->encodeInteger(packet, variable->structValue->size());
		for(Struct::iterator i = variable->structValue->begin(); i != variable->structValue->end(); ++i)
		{
			std::string name = i->first.empty() ? "UNDEFINED" : i->first;
			_encoder->encodeString(packet, name);
			if(!i->second) i->second.reset(new Variable(VariableType::tVoid));
			encodeVariable(packet, i->second);
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeArray(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tArray);
		_encoder->encodeInteger(packet, variable->arrayValue->size());
		for(std::vector<std::shared_ptr<Variable>>::iterator i = variable->arrayValue->begin(); i != variable->arrayValue->end(); ++i)
		{
			encodeVariable(packet, *i);
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeArray(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tArray);
		_encoder->encodeInteger(packet, variable->arrayValue->size());
		for(std::vector<std::shared_ptr<Variable>>::iterator i = variable->arrayValue->begin(); i != variable->arrayValue->end(); ++i)
		{
			encodeVariable(packet, *i);
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeType(std::vector<char>& packet, VariableType type)
{
	_encoder->encodeInteger(packet, (int32_t)type);
}

void RpcEncoder::encodeType(std::vector<uint8_t>& packet, VariableType type)
{
	_encoder->encodeInteger(packet, (int32_t)type);
}

void RpcEncoder::encodeInteger(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	encodeType(packet, VariableType::tInteger);
	_encoder->encodeInteger(packet, variable->integerValue);
}

void RpcEncoder::encodeInteger(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	encodeType(packet, VariableType::tInteger);
	_encoder->encodeInteger(packet, variable->integerValue);
}

void RpcEncoder::encodeInteger64(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	encodeType(packet, VariableType::tInteger64);
	_encoder->encodeInteger64(packet, variable->integerValue64);
}

void RpcEncoder::encodeInteger64(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	encodeType(packet, VariableType::tInteger64);
	_encoder->encodeInteger64(packet, variable->integerValue64);
}

void RpcEncoder::encodeFloat(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tFloat);
		_encoder->encodeFloat(packet, variable->floatValue);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeFloat(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tFloat);
		_encoder->encodeFloat(packet, variable->floatValue);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeBoolean(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	encodeType(packet, VariableType::tBoolean);
	_encoder->encodeBoolean(packet, variable->booleanValue);
}

void RpcEncoder::encodeBoolean(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	encodeType(packet, VariableType::tBoolean);
	_encoder->encodeBoolean(packet, variable->booleanValue);
}

void RpcEncoder::encodeString(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tString);
		//We could call encodeRawString here, but then the string would have to be copied and that would cost time.
		_encoder->encodeInteger(packet, variable->stringValue.size());
		if(variable->stringValue.size() > 0)
		{
			packet.insert(packet.end(), variable->stringValue.begin(), variable->stringValue.end());
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeString(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tString);
		//We could call encodeRawString here, but then the string would have to be copied and that would cost time.
		_encoder->encodeInteger(packet, variable->stringValue.size());
		if(variable->stringValue.size() > 0)
		{
			packet.insert(packet.end(), variable->stringValue.begin(), variable->stringValue.end());
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeBase64(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tBase64);
		//We could call encodeRawString here, but then the string would have to be copied and that would cost time.
		_encoder->encodeInteger(packet, variable->stringValue.size());
		if(variable->stringValue.size() > 0)
		{
			packet.insert(packet.end(), variable->stringValue.begin(), variable->stringValue.end());
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeBase64(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tBase64);
		//We could call encodeRawString here, but then the string would have to be copied and that would cost time.
		_encoder->encodeInteger(packet, variable->stringValue.size());
		if(variable->stringValue.size() > 0)
		{
			packet.insert(packet.end(), variable->stringValue.begin(), variable->stringValue.end());
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeBinary(std::vector<char>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tBinary);
		_encoder->encodeInteger(packet, variable->binaryValue.size());
		if(variable->binaryValue.size() > 0)
		{
			packet.insert(packet.end(), variable->binaryValue.begin(), variable->binaryValue.end());
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeBinary(std::vector<uint8_t>& packet, std::shared_ptr<Variable>& variable)
{
	try
	{
		encodeType(packet, VariableType::tBinary);
		_encoder->encodeInteger(packet, variable->binaryValue.size());
		if(variable->binaryValue.size() > 0)
		{
			packet.insert(packet.end(), variable->binaryValue.begin(), variable->binaryValue.end());
		}
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RpcEncoder::encodeVoid(std::vector<char>& packet)
{
	std::shared_ptr<Variable> string(new Variable(VariableType::tString));
	encodeString(packet, string);
}

void RpcEncoder::encodeVoid(std::vector<uint8_t>& packet)
{
	std::shared_ptr<Variable> string(new Variable(VariableType::tString));
	encodeString(packet, string);
}

}
}
