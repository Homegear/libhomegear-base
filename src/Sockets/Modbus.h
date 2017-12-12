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

#ifndef LIBHOMEGEAR_BASE_MODBUS_H
#define LIBHOMEGEAR_BASE_MODBUS_H

#include "../Exception.h"
#include "TcpSocket.h"

namespace BaseLib
{

/**
 * Exception class for the HTTP server.
 *
 * @see Modbus
 */
class ModbusException : public Exception
{
public:
    ModbusException(std::string message) : Exception(message) {}
};

/**
 * This class provides a Modbus client. The class is thread safe.
 */
class Modbus
{
public:
    struct ModbusInfo
    {
        std::string hostname;
        int32_t port = 502;
        bool useSsl = false;
        std::string certFile;
        std::string certData;
        std::string keyFile;
        std::string keyData;
        bool verifyCertificate = true;
        std::string caFile; //For client certificate verification
        std::string caData; //For client certificate verification
    };

    Modbus(BaseLib::SharedObjects* baseLib, ModbusInfo& serverInfo);

    /**
	 * Destructor
	 */
    virtual ~Modbus();

    void writeSingleCoil();
private:
    /**
	 * The common base library object.
	 */
    BaseLib::SharedObjects* _bl = nullptr;

    /**
     * Protects _socket to only allow one operation at a time.
     *
     * @see _socket
     */
    std::mutex _socketMutex;

    /**
     * The socket object.
     */
    std::unique_ptr<TcpSocket> _socket;

    /**
     * The hostname of the Modbus server.
     */
    std::string _hostname = "";

    /**
     * The port the Modbus server listens on.
     */
    int32_t _port = 502;
};

}

#endif
