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

#define MODBUS_HEADER_SIZE 8

namespace BaseLib
{

const uint8_t Modbus::_reverseByteMask[256] =   {
                                                    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
                                                    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
                                                    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
                                                    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
                                                    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
                                                    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
                                                    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
                                                    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
                                                    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
                                                    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
                                                    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
                                                    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
                                                    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
                                                    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
                                                    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
                                                    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
                                                };

Modbus::Modbus(BaseLib::SharedObjects* baseLib, Modbus::ModbusInfo& serverInfo)
{
    _bl = baseLib;
    _hostname = serverInfo.hostname;
    if(_hostname.empty()) throw ModbusException("The provided hostname is empty.");
    if(serverInfo.port > 0 && serverInfo.port < 65536) _port = serverInfo.port;
    if(serverInfo.timeout < 1000) serverInfo.timeout = 1000;

    _readBuffer.reset(new std::vector<char>(1024));

    _socket = std::unique_ptr<BaseLib::TcpSocket>(new BaseLib::TcpSocket(_bl, _hostname, std::to_string(_port), serverInfo.useSsl, serverInfo.verifyCertificate, serverInfo.caFile, serverInfo.caData, serverInfo.certFile, serverInfo.certData, serverInfo.keyFile, serverInfo.keyData));
    _socket->setAutoConnect(false);
    _socket->setConnectionRetries(1);
    _socket->setReadTimeout(serverInfo.timeout * 1000);
    _socket->setWriteTimeout(serverInfo.timeout * 1000);
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

void Modbus::connect()
{
    if(_socket) _socket->open();
}

void Modbus::disconnect()
{
    if(_socket) _socket->close();
}

void Modbus::insertHeader(std::vector<char>& packet, uint8_t functionCode, uint16_t payloadSize)
{
    packet.push_back((char)(uint8_t)(_transactionId >> 8)); //Transaction identifier 1
    packet.push_back((char)(uint8_t)(_transactionId & 0xFF)); //Transaction identifier 2
    _transactionId++;
    packet.push_back(0); //Protocol identifier 1 (always 0)
    packet.push_back(0); //Protocol identifier 2 (always 0)
    payloadSize += 2;
    packet.push_back((char)(uint8_t)(payloadSize >> 8)); //Length 1
    packet.push_back((char)(uint8_t)(payloadSize & 0xFF)); //Length 2
    packet.push_back((char)(uint8_t)0xFF); //Unit identifier, always
    packet.push_back((char)functionCode);
}

std::vector<char> Modbus::getResponse(std::vector<char>& packet)
{
    if(packet.size() < 8) throw ModbusException("Could not send packet as it is invalid.");

    std::lock_guard<std::mutex> socketGuard(_socketMutex);
    _socket->proofwrite(packet);

    uint32_t bytesread = 0;
    uint32_t size = 0;
    while(true)
    {
        bytesread += _socket->proofread(_readBuffer->data() + bytesread, _readBuffer->size() - bytesread);
        if(bytesread == _readBuffer->size()) _readBuffer->resize(_readBuffer->size() + 1024);
        if(bytesread < 6) continue;
        if(size == 0) size = ((((uint16_t) _readBuffer->operator[](4)) << 8) | _readBuffer->operator[](5)) + 6;
        if(bytesread > size) bytesread = size;
        if(bytesread >= size) break;
    }

    if(bytesread < 9) throw ModbusException("Invalid Modbus packet received: " + BaseLib::HelperFunctions::getHexString(_readBuffer->data(), bytesread));
    else if((_readBuffer->at(7) & 0x7F) != packet.at(7)) throw ModbusException("Invalid response function code received: " + BaseLib::HelperFunctions::getHexString(_readBuffer->data(), bytesread));
    else if(_readBuffer->at(0) != packet.at(0) || _readBuffer->at(1) != packet.at(1)) throw ModbusException("Response has invalid transaction ID.");
    else if(_readBuffer->at(7) & 0x80) //Error response
    {
        uint8_t exceptionCode = _readBuffer->at(8);
        std::vector<char> packet(_readBuffer->begin(), _readBuffer->begin() + bytesread);
        switch(exceptionCode)
        {
            case 1:
                throw ModbusException("Exception code 1: The function code (" + std::to_string(packet.at(7)) + ") is unknown by the server.", exceptionCode, packet);
            case 2:
                throw ModbusException("Exception code 2: Illegal data address.", exceptionCode, packet);
            case 3:
                throw ModbusException("Exception code 3: Illegal data value.", exceptionCode, packet);
            case 4:
                throw ModbusException("Exception code 4: Server failure.", exceptionCode, packet);
            case 5:
                throw ModbusException("Exception code 5: Acknowledge: The server accepted the service invocation but the service requires a relatively long time to execute. The server therefore returns only an acknowledgement of the service invocation receipt.", exceptionCode, packet);
            case 6:
                throw ModbusServerBusyException("Exception code 6: Server busy", exceptionCode, packet);
            case 10:
                throw ModbusException("Exception code 10: Gateway problem: Gateway paths not available.", exceptionCode, packet);
            case 11:
                throw ModbusException("Exception code 11: Gateway problem: The targeted device failed to respond.", exceptionCode, packet);
            default:
                throw ModbusException("Unknown Modbus exception: " + std::to_string(exceptionCode) + ". Response was: " + BaseLib::HelperFunctions::getHexString(_readBuffer->data(), bytesread), exceptionCode, packet);
        }
    }

    return std::vector<char>(_readBuffer->begin(), _readBuffer->begin() + bytesread);
}

void Modbus::readCoils(uint16_t startingAddress, std::vector<uint8_t>& buffer, uint16_t coilCount)
{
    if(coilCount == 0) throw ModbusException("coilCount can't be 0.");

    std::vector<char> packet;
    packet.reserve(MODBUS_HEADER_SIZE + 4);
    insertHeader(packet, 1, 4);
    packet.push_back((char)(uint8_t)(startingAddress >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(startingAddress & 0xFF)); //Address 2
    packet.push_back((char)(uint8_t)(coilCount >> 8));
    packet.push_back((char)(uint8_t)(coilCount & 0xFF));

    uint32_t coilBytes = coilCount / 8 + (coilCount % 8 != 0 ? 1 : 0);
    if(buffer.size() < coilBytes) throw ModbusException("Buffer is too small.");

    std::vector<char> response;
    for(int32_t i = 0; i < 5; i++)
    {
        try
        {
            response = getResponse(packet);
            if((uint8_t)response.at(8) == coilBytes && response.size() == coilBytes + 9) break;
            else if(i == 4) throw ModbusException("Could not read Modbus coils from address 0x" + BaseLib::HelperFunctions::getHexString(startingAddress));
        }
        catch(ModbusServerBusyException& ex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(i == 4) throw ex;
        }
        catch(ModbusException& ex)
        {
            throw ex;
        }
        catch(Exception& ex)
        {
            if(i == 4) throw ex;
        }
    }

    if((uint8_t)response.at(8) == coilBytes && response.size() == coilBytes + 9)
    {
        for(uint32_t i = 9; i < response.size(); i++)
        {
            buffer.at(i - 9) = _reverseByteMask[(uint8_t)response.at(i)];
        }
    }
}

void Modbus::readDiscreteInputs(uint16_t startingAddress, std::vector<uint8_t>& buffer, uint16_t inputCount)
{
    if(inputCount == 0) throw ModbusException("inputCount can't be 0.");

    std::vector<char> packet;
    packet.reserve(MODBUS_HEADER_SIZE + 4);
    insertHeader(packet, 2, 4);
    packet.push_back((char)(uint8_t)(startingAddress >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(startingAddress & 0xFF)); //Address 2
    packet.push_back((char)(uint8_t)(inputCount >> 8));
    packet.push_back((char)(uint8_t)(inputCount & 0xFF));

    uint32_t inputBytes = inputCount / 8 + (inputCount % 8 != 0 ? 1 : 0);
    if(buffer.size() < inputBytes) throw ModbusException("Buffer is too small.");

    std::vector<char> response;
    for(int32_t i = 0; i < 5; i++)
    {
        try
        {
            response = getResponse(packet);
            if((uint8_t)response.at(8) == inputBytes && response.size() == inputBytes + 9) break;
            else if(i == 4) throw ModbusException("Could not read Modbus inputs from address 0x" + BaseLib::HelperFunctions::getHexString(startingAddress));
        }
        catch(ModbusServerBusyException& ex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(i == 4) throw ex;
        }
        catch(ModbusException& ex)
        {
            throw ex;
        }
        catch(Exception& ex)
        {
            if(i == 4) throw ex;
        }
    }

    if((uint8_t)response.at(8) == inputBytes && response.size() == inputBytes + 9)
    {
        for(uint32_t i = 9; i < response.size(); i++)
        {
            buffer.at(i - 9) = _reverseByteMask[(uint8_t)response.at(i)];
        }
    }
}

void Modbus::readHoldingRegisters(uint16_t startingAddress, std::vector<uint16_t>& buffer, uint16_t registerCount)
{
    if(registerCount == 0) throw ModbusException("registerCount can't be 0.");
    if(buffer.size() < registerCount) throw ModbusException("Buffer is too small.");

    std::vector<char> packet;
    packet.reserve(MODBUS_HEADER_SIZE + 4);
    insertHeader(packet, 3, 4);
    packet.push_back((char)(uint8_t)(startingAddress >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(startingAddress & 0xFF)); //Address 2
    packet.push_back((char)(uint8_t)(registerCount >> 8));
    packet.push_back((char)(uint8_t)(registerCount & 0xFF));

    uint32_t registerBytes = registerCount * 2;

    std::vector<char> response;
    for(int32_t i = 0; i < 5; i++)
    {
        try
        {
            response = getResponse(packet);
            if((uint8_t)response.at(8) == registerBytes && response.size() == registerBytes + 9) break;
            else if(i == 4) throw ModbusException("Could not read Modbus holding registers from address 0x" + BaseLib::HelperFunctions::getHexString(startingAddress));
        }
        catch(ModbusServerBusyException& ex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(i == 4) throw ex;
        }
        catch(ModbusException& ex)
        {
            throw ex;
        }
        catch(Exception& ex)
        {
            if(i == 4) throw ex;
        }
    }

    if((uint8_t)response.at(8) == registerBytes && response.size() == registerBytes + 9)
    {
        for(uint32_t i = 9; i < response.size(); i+=2)
        {
            buffer.at((i - 9) / 2) = (((uint16_t)response.at(i)) << 8) | response.at(i + 1);
        }
    }
}

void Modbus::readInputRegisters(uint16_t startingAddress, std::vector<uint16_t>& buffer, uint16_t registerCount)
{
    if(registerCount == 0) throw ModbusException("registerCount can't be 0.");
    if(buffer.size() < registerCount) throw ModbusException("Buffer is too small.");

    std::vector<char> packet;
    packet.reserve(MODBUS_HEADER_SIZE + 4);
    insertHeader(packet, 4, 4);
    packet.push_back((char)(uint8_t)(startingAddress >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(startingAddress & 0xFF)); //Address 2
    packet.push_back((char)(uint8_t)(registerCount >> 8));
    packet.push_back((char)(uint8_t)(registerCount & 0xFF));

    uint32_t registerBytes = registerCount * 2;

    std::vector<char> response;
    for(int32_t i = 0; i < 5; i++)
    {
        try
        {
            response = getResponse(packet);
            if((uint8_t)response.at(8) == registerBytes && response.size() == registerBytes + 9) break;
            else if(i == 4) throw ModbusException("Could not read Modbus input registers from address 0x" + BaseLib::HelperFunctions::getHexString(startingAddress));
        }
        catch(ModbusServerBusyException& ex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(i == 4) throw ex;
        }
        catch(ModbusException& ex)
        {
            throw ex;
        }
        catch(Exception& ex)
        {
            if(i == 4) throw ex;
        }
    }

    if((uint8_t)response.at(8) == registerBytes && response.size() == registerBytes + 9)
    {
        for(uint32_t i = 9; i < response.size(); i+=2)
        {
            buffer.at((i - 9) / 2) = (((uint16_t)response.at(i)) << 8) | response.at(i + 1);
        }
    }
}

void Modbus::writeSingleCoil(uint16_t address, bool value)
{
    std::vector<char> packet;
    packet.reserve(MODBUS_HEADER_SIZE + 4);
    insertHeader(packet, 5, 4);
    packet.push_back((char)(uint8_t)(address >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(address & 0xFF)); //Address 2
    packet.push_back(value ? (char)(uint8_t)0xFF : 0);
    packet.push_back(0);

    std::vector<char> response;

    for(int32_t i = 0; i < 5; i++)
    {
        try
        {
            response = getResponse(packet);
            if(response == packet) break;
            else if(i == 4) throw ModbusException("Could not write Modbus coil at address 0x" + BaseLib::HelperFunctions::getHexString(address));
        }
        catch(ModbusServerBusyException& ex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(i == 4) throw ex;
        }
        catch(ModbusException& ex)
        {
            throw ex;
        }
        catch(Exception& ex)
        {
            if(i == 4) throw ex;
        }
    }
}

void Modbus::writeSingleRegister(uint16_t address, uint16_t value)
{
    std::vector<char> packet;
    packet.reserve(MODBUS_HEADER_SIZE + 4);
    insertHeader(packet, 6, 4);
    packet.push_back((char)(uint8_t)(address >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(address & 0xFF)); //Address 2
    packet.push_back((char)(uint8_t)(value >> 8));
    packet.push_back((char)(uint8_t)(value & 0xFF));

    std::vector<char> response;

    for(int32_t i = 0; i < 5; i++)
    {
        try
        {
            response = getResponse(packet);
            if(response == packet) break;
            else if(i == 4) throw ModbusException("Could not write Modbus register at address 0x" + BaseLib::HelperFunctions::getHexString(address));
        }
        catch(ModbusServerBusyException& ex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(i == 4) throw ex;
        }
        catch(ModbusException& ex)
        {
            throw ex;
        }
        catch(Exception& ex)
        {
            if(i == 4) throw ex;
        }
    }
}

void Modbus::writeMultipleCoils(uint16_t startAddress, const std::vector<uint8_t>& values, uint16_t coilCount)
{
    uint8_t coilBytes = coilCount / 8 + (coilCount % 8 != 0 ? 1 : 0);
    if(coilCount == 0) throw ModbusException("coilCount can't be 0.");
    if(values.size() < coilBytes) throw ModbusException("Value array is too small.");

    std::vector<char> packet;
    packet.reserve(MODBUS_HEADER_SIZE + 5 + coilBytes);
    insertHeader(packet, 15, 5 + coilBytes);
    packet.push_back((char)(uint8_t)(startAddress >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(startAddress & 0xFF)); //Address 2
    packet.push_back((char)(uint8_t)(coilCount >> 8));
    packet.push_back((char)(uint8_t)(coilCount & 0xFF));
    packet.push_back((char)coilBytes);
    for(int32_t i = 0; i < coilBytes; i++)
    {
        packet.push_back((char)_reverseByteMask[values[i]]);
    }

    std::vector<char> response;
    for(int32_t i = 0; i < 5; i++)
    {
        try
        {
            response = getResponse(packet);
            if(response.size() == 12 && response.at(8) == (char)(uint8_t)(startAddress >> 8) && response.at(9) == (char)(uint8_t)(startAddress & 0xFF) && response.at(10) == (char)(uint8_t)(coilCount >> 8) && response.at(11) == (char)(uint8_t)(coilCount & 0xFF)) break;
            else if(i == 4) throw ModbusException("Could not write Modbus coils at address 0x" + BaseLib::HelperFunctions::getHexString(startAddress));
        }
        catch(ModbusServerBusyException& ex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(i == 4) throw ex;
        }
        catch(ModbusException& ex)
        {
            throw ex;
        }
        catch(Exception& ex)
        {
            if(i == 4) throw ex;
        }
    }
}

void Modbus::writeMultipleRegisters(uint16_t startAddress, const std::vector<uint16_t>& values, uint16_t registerCount)
{
    uint8_t registerBytes = registerCount * 2;
    if(registerCount == 0) throw ModbusException("registerCount can't be 0.");
    if(values.size() < registerCount) throw ModbusException("Value array is too small.");

    std::vector<char> packet;
    packet.reserve(MODBUS_HEADER_SIZE + 5 + registerBytes);
    insertHeader(packet, 16, 5 + registerBytes);
    packet.push_back((char)(uint8_t)(startAddress >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(startAddress & 0xFF)); //Address 2
    packet.push_back((char)(uint8_t)(registerCount >> 8));
    packet.push_back((char)(uint8_t)(registerCount & 0xFF));
    packet.push_back((char)registerBytes);
    for(int32_t i = 0; i < registerCount; i++)
    {
        packet.push_back(values[i] >> 8);
        packet.push_back(values[i] & 0xFF);
    }

    std::vector<char> response;
    for(int32_t i = 0; i < 5; i++)
    {
        try
        {
            response = getResponse(packet);
            if(response.size() == 12 && response.at(8) == (char)(uint8_t)(startAddress >> 8) && response.at(9) == (char)(uint8_t)(startAddress & 0xFF) && response.at(10) == (char)(uint8_t)(registerCount >> 8) && response.at(11) == (char)(uint8_t)(registerCount & 0xFF)) break;
            else if(i == 4) throw ModbusException("Could not write Modbus registers at address 0x" + BaseLib::HelperFunctions::getHexString(startAddress));
        }
        catch(ModbusServerBusyException& ex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(i == 4) throw ex;
        }
        catch(ModbusException& ex)
        {
            throw ex;
        }
        catch(Exception& ex)
        {
            if(i == 4) throw ex;
        }
    }
}

void Modbus::readWriteMultipleRegisters(uint16_t readStartAddress, std::vector<uint16_t>& readBuffer, uint16_t readRegisterCount, uint16_t writeStartAddress, const std::vector<uint16_t>& writeValues, uint16_t writeRegisterCount)
{
    uint8_t writeRegisterBytes = writeRegisterCount * 2;
    uint8_t readRegisterBytes = readRegisterCount * 2;
    if(writeRegisterCount == 0) throw ModbusException("writeRegisterCount can't be 0.");
    if(readRegisterCount == 0) throw ModbusException("readRegisterCount can't be 0.");
    if(writeValues.size() < writeRegisterCount) throw ModbusException("Value array is too small.");
    if(readBuffer.size() < readRegisterCount) throw ModbusException("Buffer is too small.");

    std::vector<char> packet;
    packet.reserve(MODBUS_HEADER_SIZE + 9 + writeRegisterBytes);
    insertHeader(packet, 23, 9 + writeRegisterBytes);
    packet.push_back((char)(uint8_t)(readStartAddress >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(readStartAddress & 0xFF)); //Address 2
    packet.push_back((char)(uint8_t)(readRegisterCount >> 8));
    packet.push_back((char)(uint8_t)(readRegisterCount & 0xFF));
    packet.push_back((char)(uint8_t)(writeStartAddress >> 8)); //Address 1
    packet.push_back((char)(uint8_t)(writeStartAddress & 0xFF)); //Address 2
    packet.push_back((char)(uint8_t)(writeRegisterCount >> 8));
    packet.push_back((char)(uint8_t)(writeRegisterCount & 0xFF));
    packet.push_back((char)writeRegisterBytes);
    for(int32_t i = 0; i < writeRegisterCount; i++)
    {
        packet.push_back(writeValues[i] >> 8);
        packet.push_back(writeValues[i] & 0xFF);
    }

    std::vector<char> response;
    for(int32_t i = 0; i < 5; i++)
    {
        try
        {
            response = getResponse(packet);
            if((uint8_t)response.at(8) == readRegisterBytes && response.size() == (uint32_t)readRegisterBytes + 9) break;
            else if(i == 4) throw ModbusException("Could not read/write Modbus registers at address 0x" + BaseLib::HelperFunctions::getHexString(readStartAddress) + " (write) and 0x" + BaseLib::HelperFunctions::getHexString(readStartAddress) + " (read)");
        }
        catch(ModbusServerBusyException& ex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(i == 4) throw ex;
        }
        catch(ModbusException& ex)
        {
            throw ex;
        }
        catch(Exception& ex)
        {
            if(i == 4) throw ex;
        }
    }

    if((uint8_t)response.at(8) == readRegisterBytes && response.size() == (uint32_t)readRegisterBytes + 9)
    {
        for(uint32_t i = 9; i < response.size(); i+=2)
        {
            readBuffer.at((i - 9) / 2) = (((uint16_t)response.at(i)) << 8) | response.at(i + 1);
        }
    }
}

Modbus::DeviceInfo Modbus::readDeviceIdentification()
{
    Modbus::DeviceInfo deviceInfo;

    bool exception = false;
    char currentDeviceId = 1;
    char currentObjectId = 0;
    char conformityLevel = 1;
    bool moreFollows = false;

    while(true)
    {
        std::vector<char> packet;
        packet.reserve(MODBUS_HEADER_SIZE + 4);
        insertHeader(packet, 43, 3);
        packet.push_back(0x0E); //Modbus Encapsulated Interface (MEI) type
        packet.push_back(currentDeviceId); //Device ID code
        packet.push_back(currentObjectId); //Object ID

        std::vector<char> response;
        for(int32_t i = 0; i < 10; i++)
        {
            try
            {
                response = getResponse(packet);
                if(response.size() >= 15 && response.at(8) == 0x2B && response.at(9) == 0x0E && response.at(10) == currentDeviceId) break;
                else if(i == 9) throw ModbusException("Could not read Modbus device identification.");
            }
            catch(ModbusServerBusyException& ex)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if(i == 9) throw ex;
            }
            catch(ModbusException& ex)
            {
                if(ex.getCode() != 2 && ex.getCode() != 3) throw ex;
                exception = true;
                response = ex.getPacket();
                break;
            }
            catch(Exception& ex)
            {
                if(i == 9) throw ex;
            }
        }

        if(exception || response.size() < 8 || response.at(7) & 0x80) //Exception
        {
            if((currentDeviceId == 1 && currentObjectId == 0) || currentDeviceId == 3) break;
            currentDeviceId++;
            if(currentDeviceId > conformityLevel) break;
            if(currentDeviceId == 2) currentObjectId = 3;
            else if(currentDeviceId == 3) currentObjectId = (char)(uint8_t)0x80;
            exception = false;
        }
        else
        {
            conformityLevel = response.at(11) & 0x7F;
            moreFollows = (uint8_t)response.at(12) == 0xFF;
            if(moreFollows) currentObjectId = response.at(13);
            uint8_t numberOfObjects = response.at(14);
            uint32_t pos = 15;

            for(int32_t i = 0; i < numberOfObjects; i++)
            {
                uint8_t objectId = response.at(pos++);
                if(pos >= response.size()) break;
                uint8_t length = response.at(pos++);
                if(pos + length > response.size()) break;
                std::vector<uint8_t> data(response.data() + pos, response.data() + pos + length);

                deviceInfo.objects[objectId] = data;
                switch(objectId)
                {
                    case 0:
                        deviceInfo.vendorName = std::string((char*)data.data(), data.size());
                        break;
                    case 1:
                        deviceInfo.productCode = std::string((char*)data.data(), data.size());
                        break;
                    case 2:
                        deviceInfo.majorMinorRevision = std::string((char*)data.data(), data.size());
                        break;
                    case 3:
                        deviceInfo.vendorUrl = std::string((char*)data.data(), data.size());
                        break;
                    case 4:
                        deviceInfo.productName = std::string((char*)data.data(), data.size());
                        break;
                    case 5:
                        deviceInfo.modelName = std::string((char*)data.data(), data.size());
                        break;
                    case 6:
                        deviceInfo.userApplicationName = std::string((char*)data.data(), data.size());
                        break;
                }

                pos += length;
                if(pos >= response.size()) break;
            }
        }

        if(currentDeviceId == 3 && !moreFollows) break;
    }

    return deviceInfo;
}

}