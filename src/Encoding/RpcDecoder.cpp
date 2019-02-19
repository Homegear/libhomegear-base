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

#include "RpcDecoder.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Rpc
{

RpcDecoder::RpcDecoder(BaseLib::SharedObjects* baseLib) : RpcDecoder(baseLib, false)
{

}

RpcDecoder::RpcDecoder(BaseLib::SharedObjects* baseLib, bool ansi, bool setInteger32) : _bl(baseLib), _setInteger32(setInteger32)
{
    _decoder = std::unique_ptr<BinaryDecoder>(new BinaryDecoder(baseLib, ansi));
}


std::shared_ptr<RpcHeader> RpcDecoder::decodeHeader(std::vector<char>& packet)
{
    std::shared_ptr<RpcHeader> header = std::make_shared<RpcHeader>();
    try
    {
        if(!(packet.size() < 12 || packet.at(3) == 0x40 || packet.at(3) == 0x41)) return header;
        uint32_t position = 4;
        uint32_t headerSize = 0;
        headerSize = _decoder->decodeInteger(packet, position);
        if(headerSize < 4) return header;
        uint32_t parameterCount = _decoder->decodeInteger(packet, position);
        for(uint32_t i = 0; i < parameterCount; i++)
        {
            std::string field = _decoder->decodeString(packet, position);
            HelperFunctions::toLower(field);
            std::string value = _decoder->decodeString(packet, position);
            if(field == "authorization") header->authorization = value;
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
    return header;
}

std::shared_ptr<RpcHeader> RpcDecoder::decodeHeader(std::vector<uint8_t>& packet)
{
    std::shared_ptr<RpcHeader> header = std::make_shared<RpcHeader>();
    try
    {
        if(!(packet.size() < 12 || packet.at(3) == 0x40 || packet.at(3) == 0x41)) return header;
        uint32_t position = 4;
        uint32_t headerSize = 0;
        headerSize = _decoder->decodeInteger(packet, position);
        if(headerSize < 4) return header;
        uint32_t parameterCount = _decoder->decodeInteger(packet, position);
        for(uint32_t i = 0; i < parameterCount; i++)
        {
            std::string field = _decoder->decodeString(packet, position);
            HelperFunctions::toLower(field);
            std::string value = _decoder->decodeString(packet, position);
            if(field == "authorization") header->authorization = value;
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
    return header;
}

std::shared_ptr<std::vector<std::shared_ptr<Variable>>> RpcDecoder::decodeRequest(std::vector<char>& packet, std::string& methodName)
{
    try
    {
        uint32_t position = 4;
        uint32_t headerSize = 0;
        if(packet.at(3) == 0x40 || packet.at(3) == 0x41) headerSize = _decoder->decodeInteger(packet, position) + 4;
        position = 8 + headerSize;
        methodName = _decoder->decodeString(packet, position);
        uint32_t parameterCount = _decoder->decodeInteger(packet, position);
        std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters = std::make_shared<std::vector<std::shared_ptr<Variable>>>();
        if(parameterCount > 100)
        {
            _bl->out.printError("Parameter count of RPC request is larger than 100.");
            return parameters;
        }
        for(uint32_t i = 0; i < parameterCount; i++)
        {
            parameters->push_back(decodeParameter(packet, position));
        }
        return parameters;
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
    return std::shared_ptr<std::vector<std::shared_ptr<Variable>>>();
}

std::shared_ptr<std::vector<std::shared_ptr<Variable>>> RpcDecoder::decodeRequest(std::vector<uint8_t>& packet, std::string& methodName)
{
    try
    {
        uint32_t position = 4;
        uint32_t headerSize = 0;
        if(packet.at(3) == 0x40 || packet.at(3) == 0x41) headerSize = _decoder->decodeInteger(packet, position) + 4;
        position = 8 + headerSize;
        methodName = _decoder->decodeString(packet, position);
        uint32_t parameterCount = _decoder->decodeInteger(packet, position);
        std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters = std::make_shared<std::vector<std::shared_ptr<Variable>>>();
        if(parameterCount > 100)
        {
            _bl->out.printError("Parameter count of RPC request is larger than 100.");
            return parameters;
        }
        for(uint32_t i = 0; i < parameterCount; i++)
        {
            parameters->push_back(decodeParameter(packet, position));
        }
        return parameters;
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
    return std::shared_ptr<std::vector<std::shared_ptr<Variable>>>();
}

std::shared_ptr<Variable> RpcDecoder::decodeResponse(std::vector<char>& packet, uint32_t offset)
{
    uint32_t position = offset + 8;
    std::shared_ptr<Variable> response = decodeParameter(packet, position);
    if(packet.size() < 4) return response; //response is Void when packet is empty.
    if(packet.at(3) == 0xFF)
    {
        response->errorStruct = true;
        if(response->structValue->find("faultCode") == response->structValue->end()) response->structValue->insert(StructElement("faultCode", std::make_shared<Variable>(-1)));
        if(response->structValue->find("faultString") == response->structValue->end()) response->structValue->insert(StructElement("faultString", std::make_shared<Variable>(std::string("undefined"))));
    }
    return response;
}

std::shared_ptr<Variable> RpcDecoder::decodeResponse(std::vector<uint8_t>& packet, uint32_t offset)
{
    uint32_t position = offset + 8;
    std::shared_ptr<Variable> response = decodeParameter(packet, position);
    if(packet.size() < 4) return response; //response is Void when packet is empty.
    if(packet.at(3) == 0xFF)
    {
        response->errorStruct = true;
        if(response->structValue->find("faultCode") == response->structValue->end()) response->structValue->insert(StructElement("faultCode", std::make_shared<Variable>(-1)));
        if(response->structValue->find("faultString") == response->structValue->end()) response->structValue->insert(StructElement("faultString", std::make_shared<Variable>(std::string("undefined"))));
    }
    return response;
}

void RpcDecoder::decodeResponse(PVariable& variable, uint32_t offset)
{
    uint32_t position = offset + 8;
    decodeParameter(variable, position);
    if(variable->binaryValue.size() < 4) return; //response is Void when packet is empty.
    if(variable->binaryValue.at(3) == 0xFF)
    {
        variable->errorStruct = true;
        if(variable->structValue->find("faultCode") == variable->structValue->end()) variable->structValue->insert(StructElement("faultCode", std::make_shared<Variable>(-1)));
        if(variable->structValue->find("faultString") == variable->structValue->end()) variable->structValue->insert(StructElement("faultString", std::make_shared<Variable>(std::string("undefined"))));
    }
}

VariableType RpcDecoder::decodeType(std::vector<char>& packet, uint32_t& position)
{
    return (VariableType)_decoder->decodeInteger(packet, position);
}

VariableType RpcDecoder::decodeType(std::vector<uint8_t>& packet, uint32_t& position)
{
    return (VariableType)_decoder->decodeInteger(packet, position);
}

std::shared_ptr<Variable> RpcDecoder::decodeParameter(std::vector<char>& packet, uint32_t& position)
{
    try
    {
        VariableType type = decodeType(packet, position);
        std::shared_ptr<Variable> variable = std::make_shared<Variable>(type);
        if(type == VariableType::tVoid)
        {
            //Nothing
        }
        else if(type == VariableType::tString || type == VariableType::tBase64)
        {
            variable->stringValue = _decoder->decodeString(packet, position);
            variable->integerValue64 = Math::getNumber64(variable->stringValue);
            variable->integerValue = (int32_t)variable->integerValue64;
            variable->booleanValue = !variable->stringValue.empty() && variable->stringValue != "0" && variable->stringValue != "false" && variable->stringValue != "f";

        }
        else if(type == VariableType::tInteger)
        {
            variable->integerValue = _decoder->decodeInteger(packet, position);
            variable->integerValue64 = variable->integerValue;
            variable->booleanValue = (bool)variable->integerValue;
            variable->floatValue = variable->integerValue;
        }
        else if(type == VariableType::tInteger64)
        {
            variable->integerValue64 = _decoder->decodeInteger64(packet, position);
            variable->integerValue = (int32_t)variable->integerValue64;
            variable->booleanValue = (bool)variable->integerValue64;
            variable->floatValue = variable->integerValue64;
            if(_setInteger32 && (int64_t)variable->integerValue == variable->integerValue64) variable->type = VariableType::tInteger;
        }
        else if(type == VariableType::tFloat)
        {
            variable->floatValue = _decoder->decodeFloat(packet, position);
            variable->integerValue = (int32_t)std::lround(variable->floatValue);
            variable->integerValue64 = std::llround(variable->floatValue);
            variable->booleanValue = (bool)variable->floatValue;
        }
        else if(type == VariableType::tBoolean)
        {
            variable->booleanValue = _decoder->decodeBoolean(packet, position);
            variable->integerValue = (int32_t)variable->booleanValue;
            variable->integerValue64 = (int64_t)variable->booleanValue;
        }
        else if(type == VariableType::tBinary)
        {
            variable->binaryValue = _decoder->decodeBinary(packet, position);
        }
        else if(type == VariableType::tArray)
        {
            variable->arrayValue = decodeArray(packet, position);
        }
        else if(type == VariableType::tStruct)
        {
            variable->structValue = decodeStruct(packet, position);
            if(variable->structValue->size() == 2 && variable->structValue->find("faultCode") != variable->structValue->end() && variable->structValue->find("faultString") != variable->structValue->end())
            {
                variable->errorStruct = true;
            }
        }
        return variable;
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
    return std::shared_ptr<Variable>();
}

std::shared_ptr<Variable> RpcDecoder::decodeParameter(std::vector<uint8_t>& packet, uint32_t& position)
{
    try
    {
        VariableType type = decodeType(packet, position);
        std::shared_ptr<Variable> variable = std::make_shared<Variable>(type);
        if(type == VariableType::tVoid)
        {
            //Nothing
        }
        else if(type == VariableType::tString || type == VariableType::tBase64)
        {
            variable->stringValue = _decoder->decodeString(packet, position);
            variable->integerValue64 = Math::getNumber64(variable->stringValue);
            variable->integerValue = (int32_t)variable->integerValue64;
            variable->booleanValue = !variable->stringValue.empty() && variable->stringValue != "0" && variable->stringValue != "false" && variable->stringValue != "f";

        }
        else if(type == VariableType::tInteger)
        {
            variable->integerValue = _decoder->decodeInteger(packet, position);
            variable->integerValue64 = variable->integerValue;
            variable->booleanValue = (bool)variable->integerValue;
            variable->floatValue = variable->integerValue;
        }
        else if(type == VariableType::tInteger64)
        {
            variable->integerValue64 = _decoder->decodeInteger64(packet, position);
            variable->integerValue = (int32_t)variable->integerValue64;
            variable->booleanValue = (bool)variable->integerValue64;
            variable->floatValue = variable->integerValue64;
            if(_setInteger32 && (int64_t)variable->integerValue == variable->integerValue64) variable->type = VariableType::tInteger;
        }
        else if(type == VariableType::tFloat)
        {
            variable->floatValue = _decoder->decodeFloat(packet, position);
            variable->integerValue = (int32_t)std::lround(variable->floatValue);
            variable->integerValue64 = std::llround(variable->floatValue);
            variable->booleanValue = (bool)variable->floatValue;
        }
        else if(type == VariableType::tBoolean)
        {
            variable->booleanValue = _decoder->decodeBoolean(packet, position);
            variable->integerValue = (int32_t)variable->booleanValue;
            variable->integerValue64 = (int64_t)variable->booleanValue;
        }
        else if(type == VariableType::tBinary)
        {
            variable->binaryValue = _decoder->decodeBinary(packet, position);
        }
        else if(type == VariableType::tArray)
        {
            variable->arrayValue = decodeArray(packet, position);
        }
        else if(type == VariableType::tStruct)
        {
            variable->structValue = decodeStruct(packet, position);
            if(variable->structValue->size() == 2 && variable->structValue->find("faultCode") != variable->structValue->end() && variable->structValue->find("faultString") != variable->structValue->end())
            {
                variable->errorStruct = true;
            }
        }
        return variable;
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
    return std::shared_ptr<Variable>();
}

void RpcDecoder::decodeParameter(PVariable& variable, uint32_t& position)
{
    try
    {
        variable->type = decodeType(variable->binaryValue, position);
        if(variable->type == VariableType::tVoid)
        {
            //Nothing
        }
        else if(variable->type == VariableType::tString || variable->type == VariableType::tBase64)
        {
            variable->stringValue = _decoder->decodeString(variable->binaryValue, position);
            variable->integerValue64 = Math::getNumber64(variable->stringValue);
            variable->integerValue = (int32_t)variable->integerValue64;
            variable->booleanValue = !variable->stringValue.empty() && variable->stringValue != "0" && variable->stringValue != "false" && variable->stringValue != "f";

        }
        else if(variable->type == VariableType::tInteger)
        {
            variable->integerValue = _decoder->decodeInteger(variable->binaryValue, position);
            variable->integerValue64 = variable->integerValue;
            variable->booleanValue = (bool)variable->integerValue;
            variable->floatValue = variable->integerValue;
        }
        else if(variable->type == VariableType::tInteger64)
        {
            variable->integerValue64 = _decoder->decodeInteger64(variable->binaryValue, position);
            variable->integerValue = (int32_t)variable->integerValue64;
            variable->booleanValue = (bool)variable->integerValue64;
            variable->floatValue = variable->integerValue64;
            if(_setInteger32 && (int64_t)variable->integerValue == variable->integerValue64) variable->type = VariableType::tInteger;
        }
        else if(variable->type == VariableType::tFloat)
        {
            variable->floatValue = _decoder->decodeFloat(variable->binaryValue, position);
            variable->integerValue = (int32_t)std::lround(variable->floatValue);
            variable->integerValue64 = std::llround(variable->floatValue);
            variable->booleanValue = (bool)variable->floatValue;
        }
        else if(variable->type == VariableType::tBoolean)
        {
            variable->booleanValue = _decoder->decodeBoolean(variable->binaryValue, position);
            variable->integerValue = (int32_t)variable->booleanValue;
            variable->integerValue64 = (int64_t)variable->booleanValue;
        }
        else if(variable->type == VariableType::tBinary)
        {
            variable->binaryValue = _decoder->decodeBinary(variable->binaryValue, position);
        }
        else if(variable->type == VariableType::tArray)
        {
            variable->arrayValue = decodeArray(variable->binaryValue, position);
        }
        else if(variable->type == VariableType::tStruct)
        {
            variable->structValue = decodeStruct(variable->binaryValue, position);
            if(variable->structValue->size() == 2 && variable->structValue->find("faultCode") != variable->structValue->end() && variable->structValue->find("faultString") != variable->structValue->end())
            {
                variable->errorStruct = true;
            }
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

PArray RpcDecoder::decodeArray(std::vector<char>& packet, uint32_t& position)
{
    try
    {
        uint32_t arrayLength = _decoder->decodeInteger(packet, position);
        PArray array = std::make_shared<Array>();
        for(uint32_t i = 0; i < arrayLength; i++)
        {
            array->push_back(decodeParameter(packet, position));
        }
        return array;
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
    return std::shared_ptr<std::vector<std::shared_ptr<Variable>>>();
}

PArray RpcDecoder::decodeArray(std::vector<uint8_t>& packet, uint32_t& position)
{
    try
    {
        uint32_t arrayLength = _decoder->decodeInteger(packet, position);
        PArray array = std::make_shared<Array>();
        for(uint32_t i = 0; i < arrayLength; i++)
        {
            array->push_back(decodeParameter(packet, position));
        }
        return array;
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
    return std::shared_ptr<std::vector<std::shared_ptr<Variable>>>();
}

PStruct RpcDecoder::decodeStruct(std::vector<char>& packet, uint32_t& position)
{
    try
    {
        uint32_t structLength = _decoder->decodeInteger(packet, position);
        PStruct rpcStruct = std::make_shared<Struct>();
        for(uint32_t i = 0; i < structLength; i++)
        {
            std::string name = _decoder->decodeString(packet, position);
            rpcStruct->insert(StructElement(name, decodeParameter(packet, position)));
        }
        return rpcStruct;
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
    return PStruct();
}

PStruct RpcDecoder::decodeStruct(std::vector<uint8_t>& packet, uint32_t& position)
{
    try
    {
        uint32_t structLength = _decoder->decodeInteger(packet, position);
        PStruct rpcStruct = std::make_shared<Struct>();
        for(uint32_t i = 0; i < structLength; i++)
        {
            std::string name = _decoder->decodeString(packet, position);
            rpcStruct->insert(StructElement(name, decodeParameter(packet, position)));
        }
        return rpcStruct;
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
    return PStruct();
}

}
}
