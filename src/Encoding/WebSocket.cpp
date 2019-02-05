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

#include "WebSocket.h"
#include "../HelperFunctions/HelperFunctions.h"
#include <iostream>

namespace BaseLib
{
WebSocket::WebSocket()
{
}

void WebSocket::setFinished()
{
    _finished = true;
}

void WebSocket::reset()
{
    _header = Header();
    _content.clear();
    _rawHeader.clear();
    _finished = false;
    _dataProcessingStarted = false;
    _oldContentSize = 0;
}

uint32_t WebSocket::process(char* buffer, int32_t bufferLength)
{
    if(bufferLength <= 0) return 0;
    if(_finished) reset();
    uint32_t processedBytes = 0;
    if(!_header.parsed)
    {
        processedBytes += processHeader(&buffer, bufferLength);
        if(!_header.parsed) return processedBytes;
    }
    if(_header.length == 0 || _header.rsv1 || _header.rsv2 || _header.rsv3 || (_header.opcode != Header::Opcode::continuation && _header.opcode != Header::Opcode::text  && _header.opcode != Header::Opcode::binary && _header.opcode != Header::Opcode::ping && _header.opcode != Header::Opcode::pong))
    {
        _header.close = true;
        _dataProcessingStarted = true;
        setFinished();
        return processedBytes;
    }
    _dataProcessingStarted = true;
    processedBytes += processContent(buffer, bufferLength);
    return processedBytes;
}

uint32_t WebSocket::processHeader(char** buffer, int32_t& bufferLength)
{
    int32_t headBytesInserted = 0;
    int32_t bytesInserted = 0;
    if(_rawHeader.empty()) _rawHeader.reserve(14); //14 is maximum header size (2 + max number of length bytes + mask)
    if(_rawHeader.size() + bufferLength < 2)
    {
        _rawHeader.insert(_rawHeader.end(), *buffer, (*buffer) + bufferLength);
        return bufferLength;
    }
    else if(_rawHeader.size() < 2)
    {
        headBytesInserted = 2 - _rawHeader.size();
        _rawHeader.insert(_rawHeader.end(), *buffer, (*buffer) + headBytesInserted);
        if(bufferLength == headBytesInserted) return headBytesInserted;
        *buffer += headBytesInserted;
        bufferLength -= headBytesInserted;
    }

    uint32_t lengthBytes = 0;
    _header.fin = _rawHeader.at(0) & 0x80;
    _header.rsv1 = _rawHeader.at(0) & 0x40;
    _header.rsv2 = _rawHeader.at(0) & 0x20;
    _header.rsv3 = _rawHeader.at(0) & 0x10;
    _header.opcode = (Header::Opcode::Enum)(_rawHeader.at(0) & 0x0F);
    _header.hasMask = _rawHeader.at(1) & 0x80;
    uint8_t sizeByte = _rawHeader.at(1) & 0x7F;
    if(sizeByte == 126) lengthBytes = 2;
    else if(sizeByte == 127) lengthBytes = 8;
    else
    {
        lengthBytes = 0;
        _header.length = sizeByte;
    }
    uint32_t headerSize = 2 + lengthBytes + (_header.hasMask ? 4 : 0);
    if(_rawHeader.size() + bufferLength < headerSize)
    {
        _rawHeader.insert(_rawHeader.end(), *buffer, (*buffer) + bufferLength);
        bytesInserted += bufferLength;
        return headBytesInserted + bytesInserted;
    }
    else
    {
        bytesInserted += headerSize - _rawHeader.size();
        _rawHeader.insert(_rawHeader.end(), *buffer, (*buffer) + (headerSize - _rawHeader.size()));
    }

    if(lengthBytes == 2)
    {
        _header.length = (((uint32_t)(uint8_t)_rawHeader.at(2)) << 8) + (uint8_t)_rawHeader.at(3);
    }
    else if(lengthBytes == 8)
    {
        _header.length = (((uint64_t)(uint8_t)_rawHeader.at(2)) << 56) + (((uint64_t)(uint8_t)_rawHeader.at(3)) << 48) + (((uint64_t)(uint8_t)_rawHeader.at(4)) << 40) + (((uint64_t)(uint8_t)_rawHeader.at(5)) << 32) + (((uint64_t)(uint8_t)_rawHeader.at(6)) << 24) + (((uint64_t)(uint8_t)_rawHeader.at(7)) << 16) + (((uint64_t)(uint8_t)_rawHeader.at(8)) << 8) + (uint8_t)_rawHeader.at(9);
    }
    if(_header.hasMask)
    {
        _header.maskingKey.reserve(4);
        _header.maskingKey.push_back(_rawHeader.at(2 + lengthBytes));
        _header.maskingKey.push_back(_rawHeader.at(2 + lengthBytes + 1));
        _header.maskingKey.push_back(_rawHeader.at(2 + lengthBytes + 2));
        _header.maskingKey.push_back(_rawHeader.at(2 + lengthBytes + 3));
    }
    _header.parsed = true;
    _rawHeader.clear();

    if(bufferLength == bytesInserted)
    {
        bufferLength = 0;
        return headBytesInserted + bytesInserted;
    }

    *buffer += bytesInserted;
    bufferLength -= bytesInserted;
    return headBytesInserted + bytesInserted;
}

uint32_t WebSocket::processContent(char* buffer, int32_t bufferLength)
{
    uint32_t currentContentSize = _content.size() - _oldContentSize;
    if(currentContentSize + bufferLength > 10485760) throw WebSocketException("Data is larger than 10MiB.");
    if(currentContentSize + bufferLength > _header.length) bufferLength -= (currentContentSize + bufferLength) - _header.length;
    _content.insert(_content.end(), buffer, buffer + bufferLength);
    if(_content.size() - _oldContentSize == _header.length)
    {
        applyMask();
        if(_header.fin) _finished = true;
        else
        {
            _header.parsed = false;
            _oldContentSize = _content.size();
        }
    }
    return bufferLength;
}

void WebSocket::applyMask()
{
    if(!_header.hasMask) return;
    for(uint32_t i = _oldContentSize; i < _content.size(); i++)
    {
        _content.operator [](i) ^= _header.maskingKey[i % 4];
    }
}

void WebSocket::encode(const std::vector<char>& data, Header::Opcode::Enum messageType, std::vector<char>& output)
{
    output.clear();
    int32_t lengthBytes = 0;
    if(data.size() < 126) lengthBytes = 0;
    else if(data.size() <= 0xFFFF) lengthBytes = 3;
    else lengthBytes = 9;
    output.reserve(2 + lengthBytes + data.size());
    if(messageType == Header::Opcode::continuation) output.push_back(0);
    else if(messageType == Header::Opcode::text) output.push_back(1);
    else if(messageType == Header::Opcode::binary) output.push_back(2);
    else if(messageType == Header::Opcode::close) output.push_back(8);
    else if(messageType == Header::Opcode::ping) output.push_back(9);
    else if(messageType == Header::Opcode::pong) output.push_back(10);
    else throw WebSocketException("Unknown message type.");

    if(messageType != Header::Opcode::continuation) output[0] |= 0x80;

    if(lengthBytes == 0) output.push_back(data.size());
    else if(lengthBytes == 3)
    {
        output.push_back(126);
        output.push_back(data.size() >> 8);
        output.push_back(data.size() & 0xFF);
    }
    else
    {
        output.push_back(127);
        output.push_back(((uint64_t)data.size()) >> 56);
        output.push_back((((uint64_t)data.size()) >> 48) & 0xFF);
        output.push_back((((uint64_t)data.size()) >> 40) & 0xFF);
        output.push_back((((uint64_t)data.size()) >> 32) & 0xFF);
        output.push_back((data.size() >> 24) & 0xFF);
        output.push_back((data.size() >> 16) & 0xFF);
        output.push_back((data.size() >> 8) & 0xFF);
        output.push_back(data.size() & 0xFF);
    }

    if(!data.empty()) output.insert(output.end(), data.begin(), data.end());
}

void WebSocket::encodeClose(std::vector<char>& output)
{
    output.clear();
    output.reserve(2);
    output.push_back(0x80 | (char)Header::Opcode::close);
    output.push_back(0);
}
}
