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

RpcDecoder::RpcDecoder() : RpcDecoder(false)
{
}

RpcDecoder::RpcDecoder(bool ansi, bool setInteger32) : _setInteger32(setInteger32)
{
    _decoder = std::unique_ptr<BinaryDecoder>(new BinaryDecoder(ansi));
}

RpcDecoder::RpcDecoder(BaseLib::SharedObjects* baseLib) : RpcDecoder(false)
{
}

RpcDecoder::RpcDecoder(BaseLib::SharedObjects* baseLib, bool ansi, bool setInteger32) : RpcDecoder(ansi, setInteger32)
{
}


std::shared_ptr<RpcHeader> RpcDecoder::decodeHeader(const std::vector<char>& packet)
{
    std::shared_ptr<RpcHeader> header = std::make_shared<RpcHeader>();
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
    return header;
}

std::shared_ptr<RpcHeader> RpcDecoder::decodeHeader(const std::vector<uint8_t>& packet)
{
    std::shared_ptr<RpcHeader> header = std::make_shared<RpcHeader>();
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
    return header;
}

std::shared_ptr<std::vector<std::shared_ptr<Variable>>> RpcDecoder::decodeRequest(const std::vector<char>& packet, std::string& methodName)
{
    uint32_t position = 4;
    uint32_t headerSize = 0;
    if(packet.at(3) == 0x40 || packet.at(3) == 0x41) headerSize = _decoder->decodeInteger(packet, position) + 4;
    position = 8 + headerSize;
    methodName = _decoder->decodeString(packet, position);
    uint32_t parameterCount = _decoder->decodeInteger(packet, position);
    std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters = std::make_shared<std::vector<std::shared_ptr<Variable>>>();
    if(parameterCount > 100) throw RpcDecoderException("Parameter count of RPC request is larger than 100.");
    for(uint32_t i = 0; i < parameterCount; i++)
    {
        parameters->push_back(decodeParameter(packet, position));
    }
    return parameters;
}

std::shared_ptr<std::vector<std::shared_ptr<Variable>>> RpcDecoder::decodeRequest(const std::vector<uint8_t>& packet, std::string& methodName)
{
    uint32_t position = 4;
    uint32_t headerSize = 0;
    if(packet.at(3) == 0x40 || packet.at(3) == 0x41) headerSize = _decoder->decodeInteger(packet, position) + 4;
    position = 8 + headerSize;
    methodName = _decoder->decodeString(packet, position);
    uint32_t parameterCount = _decoder->decodeInteger(packet, position);
    std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters = std::make_shared<std::vector<std::shared_ptr<Variable>>>();
    if(parameterCount > 100) throw RpcDecoderException("Parameter count of RPC request is larger than 100.");
    for(uint32_t i = 0; i < parameterCount; i++)
    {
        parameters->push_back(decodeParameter(packet, position));
    }
    return parameters;
}

std::shared_ptr<Variable> RpcDecoder::decodeResponse(const std::vector<char>& packet, uint32_t offset)
{
    uint32_t position = offset + 8;
    std::shared_ptr<Variable> response = decodeParameter(packet, position);
    if(packet.size() < 4) throw RpcDecoderException("Invalid packet."); //response is Void when packet is empty.
    if(packet.at(3) == 0xFF)
    {
        response->errorStruct = true;
        if(response->structValue->find("faultCode") == response->structValue->end()) response->structValue->insert(StructElement("faultCode", std::make_shared<Variable>(-1)));
        if(response->structValue->find("faultString") == response->structValue->end()) response->structValue->insert(StructElement("faultString", std::make_shared<Variable>(std::string("undefined"))));
    }
    return response;
}

std::shared_ptr<Variable> RpcDecoder::decodeResponse(const std::vector<uint8_t>& packet, uint32_t offset)
{
    uint32_t position = offset + 8;
    std::shared_ptr<Variable> response = decodeParameter(packet, position);
    if(packet.size() < 4) throw RpcDecoderException("Invalid packet."); //response is Void when packet is empty.
    if(packet.at(3) == 0xFF)
    {
        response->errorStruct = true;
        if(response->structValue->find("faultCode") == response->structValue->end()) response->structValue->insert(StructElement("faultCode", std::make_shared<Variable>(-1)));
        if(response->structValue->find("faultString") == response->structValue->end()) response->structValue->insert(StructElement("faultString", std::make_shared<Variable>(std::string("undefined"))));
    }
    return response;
}

VariableType RpcDecoder::decodeType(const std::vector<char>& packet, uint32_t& position)
{
    return (VariableType)_decoder->decodeInteger(packet, position);
}

VariableType RpcDecoder::decodeType(const std::vector<uint8_t>& packet, uint32_t& position)
{
    return (VariableType)_decoder->decodeInteger(packet, position);
}

std::shared_ptr<Variable> RpcDecoder::decodeParameter(const std::vector<char>& packet, uint32_t& position)
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

std::shared_ptr<Variable> RpcDecoder::decodeParameter(const std::vector<uint8_t>& packet, uint32_t& position)
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

PArray RpcDecoder::decodeArray(const std::vector<char>& packet, uint32_t& position)
{
    uint32_t arrayLength = _decoder->decodeInteger(packet, position);
    PArray array = std::make_shared<Array>();
    for(uint32_t i = 0; i < arrayLength; i++)
    {
        array->push_back(decodeParameter(packet, position));
    }
    return array;
}

PArray RpcDecoder::decodeArray(const std::vector<uint8_t>& packet, uint32_t& position)
{
    uint32_t arrayLength = _decoder->decodeInteger(packet, position);
    PArray array = std::make_shared<Array>();
    for(uint32_t i = 0; i < arrayLength; i++)
    {
        array->push_back(decodeParameter(packet, position));
    }
    return array;
}

PStruct RpcDecoder::decodeStruct(const std::vector<char>& packet, uint32_t& position)
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

PStruct RpcDecoder::decodeStruct(const std::vector<uint8_t>& packet, uint32_t& position)
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

}
}
