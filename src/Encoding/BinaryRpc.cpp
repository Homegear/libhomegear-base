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

#include "BinaryRpc.h"
#include "../BaseLib.h"

namespace BaseLib::Rpc {

BinaryRpc::BinaryRpc() {
  _data.reserve(4096);
}

BinaryRpc::BinaryRpc(BaseLib::SharedObjects *bl) : BinaryRpc() {
}

int32_t BinaryRpc::process(char *buffer, int32_t bufferLength) {
  int32_t initialBufferLength = bufferLength;
  if (bufferLength <= 0) return 0;
  if (_finished) reset();
  if (!_dataProcessingStarted) {
    _headerProcessingStarted = true;
    if (_data.size() + bufferLength < 8) {
      _data.insert(_data.end(), buffer, buffer + bufferLength);
      return initialBufferLength;
    } else if (_data.size() < 8) {
      int32_t sizeToInsert = 8 - _data.size();
      _data.insert(_data.end(), buffer, buffer + sizeToInsert);
      buffer += sizeToInsert;
      bufferLength -= sizeToInsert;
    }
    if (strncmp(_data.data(), "Bin", 3) != 0) {
      _finished = true;
      throw BinaryRpcException("Packet does not start with \"Bin\".");
    }
    _type = (_data[3] & 1) ? Type::response : Type::request;
    if (_data[3] == 0x40 || _data[3] == 0x41) {
      _hasHeader = true;
      BaseLib::HelperFunctions::memcpyBigEndian((char *)&_headerSize, _data.data() + 4, 4);
      if (_headerSize > _maxHeaderSize) {
        _finished = true;
        throw BinaryRpcException("Header is larger than " + std::to_string(_maxHeaderSize) + " bytes.");
      }
    } else {
      BaseLib::HelperFunctions::memcpyBigEndian((char *)&_dataSize, _data.data() + 4, 4);
      if (_dataSize > _maxContentSize) {
        _finished = true;
        throw BinaryRpcException("Data is larger than " + std::to_string(_maxContentSize) + " bytes.");
      }
    }
    if (_dataSize == 0 && _headerSize == 0) {
      _finished = true;
      throw BinaryRpcException("Invalid packet format.");
    }
    if (_dataSize == 0) //Has header
    {
      if (_data.size() + bufferLength < 8 + _headerSize + 4) {
        if (_headerSize + 8 + 100 > _data.capacity()) _data.reserve(_headerSize + 8 + 1024);
        _data.insert(_data.end(), buffer, buffer + bufferLength);
        return initialBufferLength;
      }
      int32_t sizeToInsert = (8 + _headerSize + 4) - _data.size();
      if (sizeToInsert <= 0) {
        _finished = true;
        throw BinaryRpcException("Invalid data (calculated size to insert is wrong).");
      }
      _data.insert(_data.end(), buffer, buffer + sizeToInsert);
      buffer += sizeToInsert;
      bufferLength -= sizeToInsert;
      BaseLib::HelperFunctions::memcpyBigEndian((char *)&_dataSize, _data.data() + 8 + _headerSize, 4);
      _dataSize += _headerSize + 4;
      if (_dataSize > _maxContentSize) {
        _finished = true;
        throw BinaryRpcException("Data is larger than " + std::to_string(_maxContentSize) + " bytes.");
      }
    }

    _dataProcessingStarted = true;
    _data.reserve(8 + _dataSize);
  }

  if (_data.size() + bufferLength < _dataSize + 8) {
    _data.insert(_data.end(), buffer, buffer + bufferLength);
    return initialBufferLength;
  }
  int32_t sizeToInsert = (8 + _dataSize) - _data.size();
  if (sizeToInsert <= 0) {
    _finished = true;
    throw BinaryRpcException("Invalid data (calculated size to insert is wrong).");
  }
  _data.insert(_data.end(), buffer, buffer + sizeToInsert);
  bufferLength -= sizeToInsert;
  _finished = true;
  return initialBufferLength - bufferLength;
}

void BinaryRpc::reset() {
  if (_data.capacity() > 4096) {
    _data.resize(4096);
    _data.shrink_to_fit();
  }
  _data.clear();
  _type = Type::unknown;
  _headerProcessingStarted = false;
  _dataProcessingStarted = false;
  _finished = false;
  _hasHeader = false;
  _headerSize = 0;
  _dataSize = 0;
}

}
