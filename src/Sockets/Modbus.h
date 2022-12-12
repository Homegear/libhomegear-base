/* Copyright 2013-2019 Homegear GmbH
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

namespace BaseLib {

/**
 * Exception class for the Modbus server.
 *
 * @see Modbus
 */
class ModbusException : public Exception {
 public:
  ModbusException(const std::string &message) : Exception(message) {}
  ModbusException(const std::string &message, uint8_t code, std::vector<char> packet) : Exception(message), _code(code), _packet(std::move(packet)) {}

  uint8_t getCode() const { return _code; }
  std::vector<char> getPacket() const { return _packet; }
 private:
  uint8_t _code = 0;
  std::vector<char> _packet;
};

/**
 * Exception class thrown when the Modbus server is busy.
 *
 * @see Modbus
 */
class ModbusServerBusyException : public ModbusException {
 public:
  ModbusServerBusyException(std::string message) : ModbusException(message) {}
  ModbusServerBusyException(std::string message, uint8_t code, std::vector<char> packet) : ModbusException(message, code, packet) {}
};

/**
 * This class provides a Modbus client. The class is thread safe.
 *
 * Modbus Client Example Code
 * ==========================
 *
 * Save the example below in `main.cpp` and compile with:
 *
 *     g++ -o main -std=c++11 main.cpp -lhomegear-base -lgcrypt -lgnutls
 *
 * Start with:
 *
 *     ./main
 *
 * Example code:
 *
 *     // This program connects to a Modbus server, sets a register to "0xFFFF" and 2 seconds later to "0x0000".
 *     #include <homegear-base/BaseLib.h>
 *
 *     std::shared_ptr<BaseLib::SharedObjects> _bl;
 *
 *     int main()
 *     {
 *         _bl.reset(new BaseLib::SharedObjects(false));
 *
 *         BaseLib::Modbus::ModbusInfo modbusInfo;
 *         modbusInfo.hostname = "192.168.0.206"; //Replace with the IP address or hostname of your Modbus server.
 *
 *         BaseLib::Modbus modbus(_bl.get(), modbusInfo);
 *
 *         try
 *         {
 *             modbus.connect();
 *             modbus.writeSingleRegister(0x2001, 0xFFFF); //Replace with a working register address and value
 *             std::this_thread::sleep_for(std::chrono::milliseconds(2000));
 *             modbus.writeSingleRegister(0x2001, 0x0000);  //Replace with a working register address and value
 *             modbus.disconnect();
 *         }
 *         catch(BaseLib::ModbusException& ex)
 *         {
 *             std::cerr << "Code: " << (int32_t)ex.getCode()
 *                       << ", response packet: " << BaseLib::HelperFunctions::getHexString(ex.getPacket())
 *                       << ", message: " << ex.what() << std::endl;
 *         }
 *         catch(BaseLib::Exception& ex)
 *         {
 *             std::cerr << ex.what() << std::endl;
 *         }
 *     }
 *
 */
class Modbus {
 public:
  struct ModbusInfo {
    std::string hostname;
    int32_t port = 502;
    bool useSsl = false;
    bool keepAlive = true;
    std::string certFile;
    std::string certData;
    std::string keyFile;
    std::shared_ptr<Security::SecureVector<uint8_t>> keyData;
    bool verifyCertificate = true;
    std::string caFile; //For client certificate verification
    std::string caData; //For client certificate verification
    uint32_t timeout = 5000;
    std::function<void(const std::vector<char> &packet)> packetSentCallback;
    std::function<void(const std::vector<char> &packet)> packetReceivedCallback;
  };

  struct DeviceInfo {
    std::string vendorName;
    std::string productCode;
    std::string majorMinorRevision;
    std::string vendorUrl;
    std::string productName;
    std::string modelName;
    std::string userApplicationName;
    std::map<uint8_t, std::vector<uint8_t>> objects;
  };

  Modbus(BaseLib::SharedObjects *baseLib, ModbusInfo &serverInfo);

  /**
   * Destructor
   */
  virtual ~Modbus();

  /**
   * Sets the slave ID. For Modbus over TCP this is normally unnecessary. It is needed to reach devices on a serial network or for devices with broken firmware. The default value is "0xFF". Disconnecting doesn't change the ID. It can be set before "connect()" is executed.
   */
  void setSlaveId(uint8_t value) { _slaveId = value; }

  /**
   * Opens the connection to the Modbus server.
   * @throws SocketOperationException When the connection cannot be established.
   */
  void connect();

  /**
   * Closes the connection to the Modbus server.
   */
  void disconnect();

  /**
   * Checks if the socket is connected.
   * @return Returns true when the socket is connected and false otherwise.
   */
  bool isConnected() { return _socket && _socket->connected(); }

  /**
   * Executes modbus function 01 (0x01) "Read Coils".
   *
   * @param startingAddress Valid values range from 0x0000 to 0xFFFF.
   * @param[out] buffer The buffer to fill. Make sure the size is at least number of coils bits.
   * @param coilCount The number of coils to read (from 1 to 2000 [= 0x7D0]).
   * @returns Returns the coil states as a byte array. The least significant bit is to the left (in contrast to the Modbus packet). So the bits and bytes can be read from left to right.
   * @throws SocketOperationException On socket errors.
   * @throws SocketTimeOutException On socket timeout.
   * @throws SocketClosedException When the socket is closed during the request.
   * @throws ModbusException Thrown on all Modbus errors.
   * @throws ModbusServerBusyException Thrown when the server is currently busy.
   */
  void readCoils(uint16_t startingAddress, std::vector<uint8_t> &buffer, uint16_t coilCount);

  /**
   * Executes modbus function 02 (0x02) "Read Discrete Inputs".
   *
   * @param startingAddress Valid values range from 0x0000 to 0xFFFF.
   * @param[out] buffer The buffer to fill. Make sure the size is at least number of inputs bits.
   * @param inputCount The number of inputs to read (from 1 to 2000 [= 0x7D0]).
   * @returns Returns the input states as a byte array. The least significant bit is to the left (in contrast to the Modbus packet). So the bits and bytes can be read from left to right.
   * @throws SocketOperationException On socket errors.
   * @throws SocketTimeOutException On socket timeout.
   * @throws SocketClosedException When the socket is closed during the request.
   * @throws ModbusException Thrown on all Modbus errors.
   * @throws ModbusServerBusyException Thrown when the server is currently busy.
   */
  void readDiscreteInputs(uint16_t startingAddress, std::vector<uint8_t> &buffer, uint16_t inputCount);

  /**
   * Executes modbus function 03 (0x03) "Read Holding Registers".
   *
   * @param startingAddress Valid values range from 0x0000 to 0xFFFF.
   * @param[out] buffer The buffer to fill. Make sure the size is at least number of number of registers.
   * @param registerCount The number of registers to read (from 1 to 125 [= 0x7D]).
   * @returns Returns the register values.
   * @throws SocketOperationException On socket errors.
   * @throws SocketTimeOutException On socket timeout.
   * @throws SocketClosedException When the socket is closed during the request.
   * @throws ModbusException Thrown on all Modbus errors.
   * @throws ModbusServerBusyException Thrown when the server is currently busy.
   */
  void readHoldingRegisters(uint16_t startingAddress, std::vector<uint16_t> &buffer, uint16_t registerCount);

  /**
   * Executes modbus function 04 (0x04) "Read Input Registers".
   *
   * @param startingAddress Valid values range from 0x0000 to 0xFFFF.
   * @param[out] buffer The buffer to fill. Make sure the size is at least number of number of registers.
   * @param registerCount The number of registers to read (from 1 to 125 [= 0x7D]).
   * @returns Returns the register values.
   * @throws SocketOperationException On socket errors.
   * @throws SocketTimeOutException On socket timeout.
   * @throws SocketClosedException When the socket is closed during the request.
   * @throws ModbusException Thrown on all Modbus errors.
   * @throws ModbusServerBusyException Thrown when the server is currently busy.
   */
  void readInputRegisters(uint16_t startingAddress, std::vector<uint16_t> &buffer, uint16_t registerCount);

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
  void writeMultipleCoils(uint16_t startAddress, const std::vector<uint8_t> &values, uint16_t coilCount);

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
  void writeMultipleRegisters(uint16_t startAddress, const std::vector<uint16_t> &values, uint16_t registerCount);

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
  void readWriteMultipleRegisters(uint16_t readStartAddress, std::vector<uint16_t> &readBuffer, uint16_t readRegisterCount, uint16_t writeStartAddress, const std::vector<uint16_t> &writeValues, uint16_t writeRegisterCount);

  /**
   * Reads all device identification and returns it as a DeviceInfo struct.
   *
   * @return A DeviceInfo struct containing all identification objects (basic, regular and extended).
   * @throws SocketOperationException On socket errors.
   * @throws SocketTimeOutException On socket timeout.
   * @throws SocketClosedException When the socket is closed during the request.
   * @throws ModbusServerBusyException Thrown when the server is currently busy.
   */
  DeviceInfo readDeviceIdentification();
 private:
  static const uint8_t _reverseByteMask[256];

  /**
   * The Modbus slave ID
   */
  uint8_t _slaveId = 0xFF;

  /**
   * Keep alive
   */
  bool _keepAlive = true;

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
  std::string _hostname;

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

  std::function<void(const std::vector<char> &packet)> _packetSentCallback;
  std::function<void(const std::vector<char> &packet)> _packetReceivedCallback;

  /**
   * Inserts the Modbus header into a packet.
   * @param packet The empty buffer to insert the packet to.
   * @param functionCode The Modbus function code to insert.
   * @param payloadSize The number of bytes of the payload.
   */
  void insertHeader(std::vector<char> &packet, uint8_t functionCode, uint16_t payloadSize);

  std::vector<char> getResponse(std::vector<char> &packet);
};

}

#endif
