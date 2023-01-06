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

#ifndef SERIALREADERWRITER_H_
#define SERIALREADERWRITER_H_

#include "../Exception.h"
#include "../Managers/FileDescriptorManager.h"
#include "../IEvents.h"
#include "../LowLevel/Gpio.h"

#include <thread>
#include <atomic>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

namespace BaseLib {
class SerialReaderWriterException : public Exception {
 public:
  SerialReaderWriterException(std::string message) : Exception(message) {}
};

class SerialReaderWriter : public IEventsEx {
 public:
  enum class CharacterSize : tcflag_t {
    Five = CS5,
    Six = CS6,
    Seven = CS7,
    Eight = CS8
  };

  // {{{ Event handling
  class ISerialReaderWriterEventSink : public IEventSinkBase {
   public:
    virtual void lineReceived(const std::string &data) = 0;
  };
  // }}}

  /**
   * Constructor.
   *
   * @param baseLib The base library object.
   * @param device The device to use (e. g. "/dev/ttyUSB0")
   * @param baudrate The baudrate (e. g. 115200)
   * @param flags Flags passed to the C function "open". 0 should be fine for most cases. "O_NDELAY" is always added by the constructor. By default "O_RDWR | O_NOCTTY | O_NDELAY" is used.
   * @param createLockFile Dummy parameter for compatibility.
   * @param readThreadPriority The priority of the read thread between 0 and 99. Set to -1 to not prioritize the thread. Only relevent when "events" are enabled in "openDevice()".
   * @param writeOnly Open the device for writing only.
   */
  SerialReaderWriter(BaseLib::SharedObjects *baseLib, std::string device, int32_t baudrate, int32_t flags, bool createLockFile, int32_t readThreadPriority, bool writeOnly = false);

  /**
   * Destructor.
   */
  virtual ~SerialReaderWriter();

  bool isOpen() { return _fileDescriptor && _fileDescriptor->descriptor != -1; }
  std::shared_ptr<FileDescriptor> fileDescriptor() { return _fileDescriptor; }

  /**
   * This GPIO is set to high when reading the UART buffer starts and to low when it has finished.
   *
   * @param gpio
   */
  void setReadGpio(int32_t index);

  /**
   * This GPIO is set to high when writing to the UART buffer starts and to low when it has finished.
   *
   * @param gpio
   */
  void setWriteGpio(int32_t index);

  /**
   * Opens the serial device.
   *
   * @param evenParity Enable parity checking using an even parity bit.
   * @param oddParity Enable parity checking using an odd parity bit. "evenParity" and "oddParity" are mutually exclusive.
   * @param events Enable events. This starts a thread which calls "lineReceived()" in a derived class for each received packet.
   * @param characterSize Set the character Size.
   * @param twoStopBits Enable two stop bits instead of one.
   */
  void openDevice(bool parity, bool oddParity, bool events = true, CharacterSize characterSize = CharacterSize::Eight, bool twoStopBits = false);

  /**
   * Closes the serial device.
   */
  void closeDevice();

  /**
   * SerialReaderWriter can either be used through events (by implementing ISerialReaderWriterEventSink and usage of addEventHandler) or by polling using this method.
   * @param data The variable to write the returned line into.
   * @param timeout The maximum amount of time to wait in microseconds before the function returns (default: 500000).
   * @param splitChar The character to split at (default: '\n')
   * @return Returns "0" on success, "1" on timeout or "-1" on error.
   */
  int32_t readLine(std::string &data, uint32_t timeout = 500000, char splitChar = '\n');

  /**
   * SerialReaderWriter can either be used through events (by implementing ISerialReaderWriterEventSink and usage of addEventHandler) or by polling using this method.
   * @param data The variable to write the returned character into.
   * @param timeout The maximum amount of time to wait in microseconds before the function returns (default: 500000).
   * @return Returns "0" on success, "1" on timeout or "-1" on error.
   */
  int32_t readChar(char &data, uint32_t timeout = 500000);

  /**
   * Writes one line of data.
   * @param data The data to write. If data is not terminated by a new line character, it is appended.
   */
  void writeLine(std::string &data);

  /**
   * Writes binary data to the serial device.
   * @param data The data to write. It is written as is without any modification.
   */
  void writeData(const std::vector<char> &data);

  /**
   * Writes binary data to the serial device.
   * @param data The data to write. It is written as is without any modification.
   */
  void writeData(const std::vector<uint8_t> &data);

  /**
   * Writes one character to the serial device.
   * @param data The (binary) character to write.
   */
  void writeChar(char data);
 protected:
  BaseLib::SharedObjects *_bl = nullptr;
  std::shared_ptr<FileDescriptor> _fileDescriptor;
  std::string _device;
  bool _writeOnly = false;
  struct termios _termios;
  int32_t _baudrate = 0;
  int32_t _flags = 0;
  int32_t _readThreadPriority = 0;
  int32_t _handles = 0;

  int32_t read_gpio_index_ = -1;
  int32_t write_gpio_index_ = -1;
  std::unique_ptr<LowLevel::Gpio> gpio_;

  std::atomic_bool _stopReadThread;
  std::mutex _readThreadMutex;
  std::thread _readThread;
  std::mutex _sendMutex;
  std::mutex _openDeviceThreadMutex;
  std::thread _openDeviceThread;

  void readThread(bool parity, bool oddParity, CharacterSize characterSize, bool twoStopBits);
};

}
#endif
