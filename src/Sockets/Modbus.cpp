/* Copyright 2013-2017 Sathya Laufer
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Homegear.  If not, see
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

#include "Modbus.h"
#include "../BaseLib.h"

namespace BaseLib
{

Modbus::Modbus(BaseLib::SharedObjects* baseLib, Modbus::ModbusInfo& serverInfo)
{
    _bl = baseLib;
    _hostname = serverInfo.hostname;
    if(_hostname.empty()) throw ModbusException("The provided hostname is empty.");
    if(serverInfo.port > 0 && serverInfo.port < 65536) _port = serverInfo.port;

    _socket = std::unique_ptr<BaseLib::TcpSocket>(new BaseLib::TcpSocket(_bl, _hostname, std::to_string(_port), serverInfo.useSsl, serverInfo.verifyCertificate, serverInfo.caFile, serverInfo.caData, serverInfo.certFile, serverInfo.certData, serverInfo.keyFile, serverInfo.keyData));
    _socket->setConnectionRetries(1);
}

Modbus::~Modbus()
{
    std::lock_guard<std::mutex> socketGuard(_socketMutex);
    if(_socket)
    {
        _socket->close();
        _socket.reset();
    }
}

void Modbus::writeSingleCoil()
{
    std::vector<char> data;
    data.push_back(0); //Transaction identifier 1
    data.push_back(0); //Transaction identifier 2
    data.push_back(0); //Protocol identifier 1 (always 0)
    data.push_back(0); //Protocol identifier 2 (always 0)
    data.push_back(0); //Length 1
    data.push_back(6); //Length 2
    data.push_back((char)(uint8_t)0xFF); //Unit identifier
    data.push_back(5); //Function code 5
    data.push_back(0x20); //Coil address 1 (= coil number - 1)
    data.push_back(1); //Coil address 2
    data.push_back((char)(uint8_t)0xFF);
    data.push_back(0);

    _socket->proofwrite(data);

    data.resize(50);
    int32_t bytesread = _socket->proofread(data.data(), data.size());
    data.resize(bytesread);

    _bl->out.printError(BaseLib::HelperFunctions::getHexString(data));

    sleep(2);

    data.clear();
    data.push_back(0); //Transaction identifier 1
    data.push_back(1); //Transaction identifier 2
    data.push_back(0); //Protocol identifier 1 (always 0)
    data.push_back(0); //Protocol identifier 2 (always 0)
    data.push_back(0); //Length 1
    data.push_back(6); //Length 2
    data.push_back((char)(uint8_t)0xFF); //Unit identifier
    data.push_back(5); //Function code 5
    data.push_back(0x20); //Coil address 1 (= coil number - 1)
    data.push_back(1); //Coil address 2
    data.push_back(0);
    data.push_back(0);

    _socket->proofwrite(data);

    data.resize(50);
    bytesread = _socket->proofread(data.data(), data.size());
    data.resize(bytesread);

    _bl->out.printError(BaseLib::HelperFunctions::getHexString(data));
}

}