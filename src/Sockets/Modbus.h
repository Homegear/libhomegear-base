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
 * Exception class for the Modbus server.
 *
 * @see Modbus
 */
class ModbusException : public Exception
{
public:
    ModbusException(std::string message) : Exception(message) {}
};

/**
 * Exception class thrown when the Modbus server is busy.
 *
 * @see Modbus
 */
class ModbusServerBusyException : public ModbusException
{
public:
    ModbusServerBusyException(std::string message) : ModbusException(message) {}
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
        uint32_t timeout = 5000;
    };

    Modbus(BaseLib::SharedObjects* baseLib, ModbusInfo& serverInfo);

    /**
	 * Destructor
	 */
    virtual ~Modbus();

    /**
     * Executes modbus function 01 (0x01) "Read Coils".
     *
     * @param startingAddress Valid values range from 0x0000 to 0xFFFF.
     * @param coilCount The number of coils to read (from 1 to 2000 [= 0x7D0]).
     * @param[out] buffer The buffer to fill. Make sure the size is at least number of coils bits.
     * @returns Returns the coil states as a byte array. The least significant bit is to the left (in contrast to the Modbus packet). So the bits and bytes can be read from left to right.
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    void readCoils(uint16_t startingAddress, uint16_t coilCount, std::vector<uint8_t>& buffer);

    /**
     * Executes modbus function 02 (0x02) "Read Discrete Inputs".
     *
     * @param startingAddress Valid values range from 0x0000 to 0xFFFF.
     * @param inputCount The number of inputs to read (from 1 to 2000 [= 0x7D0]).
     * @param[out] buffer The buffer to fill. Make sure the size is at least number of inputs bits.
     * @returns Returns the input states as a byte array. The least significant bit is to the left (in contrast to the Modbus packet). So the bits and bytes can be read from left to right.
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    void readDiscreteInputs(uint16_t startingAddress, uint16_t inputCount, std::vector<uint8_t>& buffer);

    /**
     * Executes modbus function 02 (0x02) "Read Discrete Inputs".
     *
     * @param startingAddress Valid values range from 0x0000 to 0xFFFF.
     * @param inputCount The number of inputs to read (from 1 to 2000 [= 0x7D0]).
     * @returns Returns the input states as a byte array. The least significant bit is to the left (in contrast to the Modbus packet). So the bits and bytes can be read from left to right.
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    std::vector<uint8_t> readDiscreteInputs(uint16_t startingAddress, uint16_t inputCount);

    /**
     * Executes modbus function 03 (0x03) "Read Holding Registers".
     *
     * @param startingAddress Valid values range from 0x0000 to 0xFFFF.
     * @param registerCount The number of registers to read (from 1 to 125 [= 0x7D]).
     * @param[out] buffer The buffer to fill. Make sure the size is at least number of number of registers.
     * @returns Returns the register values.
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    void readHoldingRegisters(uint16_t startingAddress, uint16_t registerCount, std::vector<uint16_t>& buffer);

    /**
     * Executes modbus function 04 (0x04) "Read Input Registers".
     *
     * @param startingAddress Valid values range from 0x0000 to 0xFFFF.
     * @param registerCount The number of registers to read (from 1 to 125 [= 0x7D]).
     * @param[out] buffer The buffer to fill. Make sure the size is at least number of number of registers.
     * @returns Returns the register values.
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    void readInputRegisters(uint16_t startingAddress, uint16_t registerCount, std::vector<uint16_t>& buffer);

    /**
     * Executes modbus function 05 (0x05) "Write Single Coil".
     *
     * @param address Valid values range from 0x0000 to 0xFFFF.
     * @param value The value to set the coil to.
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    void writeSingleCoil(uint16_t address, bool value);

    /**
     * Executes modbus function 06 (0x06) "Write Single Register".
     *
     * @param address Valid values range from 0x0000 to 0xFFFF.
     * @param value The value to set the register to (between 0x0000 and 0xFFFF).
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    void writeSingleRegister(uint16_t address, uint16_t value);

    /**
     * Executes modbus function 15 (0x0F) "Write Multiple Coils".
     *
     * @param startAddress Valid values range from 0x0000 to 0xFFFF.
     * @param values The values to write. The least significant bit is to the left (in contrast to the Modbus packet). So the bits and bytes can be read from left to right.
     * @param coilCount The number of coils to write from 0x0001 to 0x07B0.
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    void writeMultipleCoils(uint16_t startAddress, const std::vector<uint8_t>& values, uint16_t coilCount);

    /**
     * Executes modbus function 16 (0x10) "Write Multiple Registers".
     *
     * @param startAddress Valid values range from 0x0000 to 0xFFFF.
     * @param values The values to write.
     * @param registerCount The number of registers to write from 0x0001 to 0x007B.
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    void writeMultipleRegisters(uint16_t startAddress, const std::vector<uint16_t>& values, uint16_t registerCount);

    /**
     * Executes modbus function 23 (0x17) "Read/Write Multiple Registers".
     *
     * @param readStartAddress Valid values range from 0x0000 to 0xFFFF.
     * @param[out] readBuffer The buffer to read values to.
     * @param readRegisterCount The number of registers to read from 0x0001 to 0x007D.
     * @param writeStartAddress Valid values range from 0x0000 to 0xFFFF.
     * @param writeValues The values to write.
     * @param writeRegisterCount The number of registers to write from 0x0001 to 0x0079.
     * @throws SocketOperationException On socket errors.
     * @throws SocketTimeOutException On socket timeout.
     * @throws SocketClosedException When the socket is closed during the request.
     * @throws ModbusException Thrown on all Modbus errors.
     * @throws ModbusServerBusyException Thrown when the server is currently busy.
     */
    void readWriteMultipleRegisters(uint16_t readStartAddress, std::vector<uint16_t>& readBuffer, uint16_t readRegisterCount, uint16_t writeStartAddress, const std::vector<uint16_t>& writeValues, uint16_t writeRegisterCount);
private:
    static const uint8_t _reverseByteMask[256];

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

    /**
     * The buffer to read responses to.
     */
    std::unique_ptr<std::vector<char>> _readBuffer;

    /**
     * The transaction ID used for Modbus packet numbering.
     */
    uint16_t _transactionId = 0;

    /**
     * Inserts the Modbus header into a packet.
     * @param packet The empty buffer to insert the packet to.
     * @param functionCode The Modbus function code to insert.
     * @param payloadSize The number of bytes of the payload.
     */
    void insertHeader(std::vector<char>& packet, uint8_t functionCode, uint16_t payloadSize);

    std::vector<char> getResponse(std::vector<char>& packet);
};

}

#endif
