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

#include "SerialReaderWriter.h"

#include <memory>
#include <utility>
#include "../BaseLib.h"

namespace BaseLib {

SerialReaderWriter::SerialReaderWriter(BaseLib::SharedObjects *baseLib, std::string device, int32_t baudrate, int32_t flags, bool createLockFile, int32_t readThreadPriority, bool writeOnly) {
  _fileDescriptor = std::make_shared<FileDescriptor>();
  _bl = baseLib;
  _device = std::move(device);
  _baudrate = baudrate;
  _flags = flags;
  if (_flags == 0) _flags = O_RDWR | O_NOCTTY | O_NDELAY;
  else _flags |= O_NDELAY;
  _readThreadPriority = readThreadPriority;
  _stopReadThread = false;
  _writeOnly = writeOnly;
  memset(&_termios, 0, sizeof(termios));

  if (writeOnly) {
    _flags &= ~O_RDWR;
    _flags |= O_WRONLY;
  }
}

SerialReaderWriter::~SerialReaderWriter() {
  _handles = 0;
  closeDevice();
}

void SerialReaderWriter::setReadGpio(int32_t index) {
  read_gpio_index_ = index;
  if (!gpio_) gpio_ = std::make_unique<LowLevel::Gpio>(_bl, _bl->settings.gpioPath());
  gpio_->exportGpio(index);
  gpio_->setDirection(index, LowLevel::Gpio::GpioDirection::Enum::OUT);
  gpio_->openDevice(index, false);
  gpio_->set(index, false);
}

void SerialReaderWriter::setWriteGpio(int32_t index) {
  write_gpio_index_ = index;
  if (!gpio_) gpio_ = std::make_unique<LowLevel::Gpio>(_bl, _bl->settings.gpioPath());
  gpio_->exportGpio(index);
  gpio_->setDirection(index, LowLevel::Gpio::GpioDirection::Enum::OUT);
  gpio_->openDevice(index, false);
  gpio_->set(index, false);
}

void SerialReaderWriter::openDevice(bool parity, bool oddParity, bool events, CharacterSize characterSize, bool twoStopBits) {
  _handles++;
  if (_fileDescriptor->descriptor > -1) return;
  _fileDescriptor = _bl->fileDescriptorManager.add(open(_device.c_str(), _flags | O_CLOEXEC));
  if (_fileDescriptor->descriptor == -1) throw SerialReaderWriterException("Couldn't open device \"" + _device + "\": " + strerror(errno));

  if (!Io::writeLockFile(_fileDescriptor->descriptor, false)) {
    throw SerialReaderWriterException("Couldn't open device \"" + _device + "\": Device is locked.");
  }

  tcflag_t baudrate = 0;
  switch (_baudrate) {
    case 50: baudrate = B50;
      break;
    case 75: baudrate = B75;
      break;
    case 110: baudrate = B110;
      break;
    case 134: baudrate = B134;
      break;
    case 150: baudrate = B150;
      break;
    case 200: baudrate = B200;
      break;
    case 300: baudrate = B300;
      break;
    case 600: baudrate = B600;
      break;
    case 1200: baudrate = B1200;
      break;
    case 1800: baudrate = B1800;
      break;
    case 2400: baudrate = B2400;
      break;
    case 4800: baudrate = B4800;
      break;
    case 9600: baudrate = B9600;
      break;
    case 19200: baudrate = B19200;
      break;
    case 38400: baudrate = B38400;
      break;
    case 57600: baudrate = B57600;
      break;
    case 115200: baudrate = B115200;
      break;
    case 230400: baudrate = B230400;
      break;
    case 460800: baudrate = B460800;
      break;
    case 500000: baudrate = B500000;
      break;
    case 576000: baudrate = B576000;
      break;
    case 921600: baudrate = B921600;
      break;
    case 1000000: baudrate = B1000000;
      break;
    case 1152000: baudrate = B1152000;
      break;
    case 1500000: baudrate = B1500000;
      break;
    case 2000000: baudrate = B2000000;
      break;
    case 2500000: baudrate = B2500000;
      break;
    case 3000000: baudrate = B3000000;
      break;
    case 3500000: baudrate = B3500000;
      break;
    case 4000000: baudrate = B4000000;
      break;
    default: throw SerialReaderWriterException("Couldn't setup device \"" + _device + "\": Unsupported baudrate.");
  }
  memset(&_termios, 0, sizeof(termios));
  _termios.c_cflag = baudrate | (tcflag_t)characterSize | CREAD;
  if (parity) _termios.c_cflag |= PARENB;
  if (oddParity) _termios.c_cflag |= PARENB | PARODD;
  if (twoStopBits) _termios.c_cflag |= CSTOPB;
  _termios.c_iflag = 0;
  _termios.c_oflag = 0;
  _termios.c_lflag = 0;
  _termios.c_cc[VMIN] = 1;
  _termios.c_cc[VTIME] = 0;
  cfsetispeed(&_termios, baudrate);
  cfsetospeed(&_termios, baudrate);
  if (tcflush(_fileDescriptor->descriptor, TCIOFLUSH) == -1) throw SerialReaderWriterException("Couldn't flush device " + _device);
  if (tcsetattr(_fileDescriptor->descriptor, TCSANOW, &_termios) == -1) throw SerialReaderWriterException("Couldn't set device settings for device " + _device);

  int flags = fcntl(_fileDescriptor->descriptor, F_GETFL);
  if (!(flags & O_NONBLOCK)) {
    if (fcntl(_fileDescriptor->descriptor, F_SETFL, flags | O_NONBLOCK) == -1) throw SerialReaderWriterException("Couldn't set device to non blocking mode: " + _device);
  }

  _stopReadThread = false;
  if (events && !_writeOnly) {
    _readThreadMutex.lock();
    _bl->threadManager.join(_readThread);
    if (_readThreadPriority > -1) _bl->threadManager.start(_readThread, true, _readThreadPriority, SCHED_FIFO, &SerialReaderWriter::readThread, this, parity, oddParity, characterSize, twoStopBits);
    else _bl->threadManager.start(_readThread, true, &SerialReaderWriter::readThread, this, parity, oddParity, characterSize, twoStopBits);
    _readThreadMutex.unlock();
  }
}

void SerialReaderWriter::closeDevice() {
  _handles--;
  if (_handles > 0) return;
  _readThreadMutex.lock();
  _stopReadThread = true;
  _bl->threadManager.join(_readThread);
  _readThreadMutex.unlock();
  _openDeviceThreadMutex.lock();
  _bl->threadManager.join(_openDeviceThread);
  _openDeviceThreadMutex.unlock();
  _bl->fileDescriptorManager.close(_fileDescriptor);
}

int32_t SerialReaderWriter::readChar(char &data, uint32_t timeout) {
  if (_writeOnly) return -1;
  int32_t bytes_read = 0;
  while (!_stopReadThread) {
    if (_fileDescriptor->descriptor == -1) {
      _bl->out.printError("Error: File descriptor is invalid.");
      return -1;
    }

    pollfd poll_struct{
        (int)_fileDescriptor->descriptor,
        (short)(POLLIN),
        (short)(0)
    };

    int32_t poll_result = -1;
    do {
      poll_result = poll(&poll_struct, 1, (int)(timeout / 1000));
    } while (poll_result == -1 && errno == EINTR);
    if (poll_result == -1 || (poll_struct.revents & (POLLNVAL | POLLERR | POLLHUP)) || _fileDescriptor->descriptor == -1) {
      //Error
      _bl->fileDescriptorManager.close(_fileDescriptor);
      return -1;
    }

    if (poll_result == 0) {
      //Timeout
      return 1;
    }

    if (read_gpio_index_ != -1) gpio_->set(read_gpio_index_, true);
    bytes_read = read(_fileDescriptor->descriptor, &data, 1);
    if (read_gpio_index_ != -1) gpio_->set(read_gpio_index_, false);
    if (bytes_read == -1 || bytes_read == 0) {
      if (bytes_read == -1 && (errno == EAGAIN || errno == EINTR)) continue;
      _bl->fileDescriptorManager.close(_fileDescriptor);
      return -1;
    }
    return 0;
  }
  return -1;
}

int32_t SerialReaderWriter::readLine(std::string &data, uint32_t timeout, char splitChar) {
  if (_writeOnly) return -1;
  data.clear();
  int32_t bytes_read = 0;
  char localBuffer[1];
  while (!_stopReadThread) {
    if (_fileDescriptor->descriptor == -1) {
      _bl->out.printError("Error: File descriptor is invalid.");
      return -1;
    }

    pollfd poll_struct{
        (int)_fileDescriptor->descriptor,
        (short)(POLLIN),
        (short)(0)
    };

    int32_t poll_result = -1;
    do {
      poll_result = poll(&poll_struct, 1, (int)(timeout / 1000));
    } while (poll_result == -1 && errno == EINTR);
    if (poll_result == -1 || (poll_struct.revents & (POLLNVAL | POLLERR | POLLHUP)) || _fileDescriptor->descriptor == -1) {
      //Error
      _bl->fileDescriptorManager.close(_fileDescriptor);
      return -1;
    }

    if (poll_result == 0) {
      //Timeout
      return 1;
    }

    if (read_gpio_index_ != -1) gpio_->set(read_gpio_index_, true);
    bytes_read = read(_fileDescriptor->descriptor, localBuffer, 1);
    if (read_gpio_index_ != -1) gpio_->set(read_gpio_index_, false);
    if (bytes_read == -1 || bytes_read == 0) {
      if (errno == EAGAIN || errno == EINTR) continue;
      _bl->fileDescriptorManager.close(_fileDescriptor);
      continue;
    }
    data.push_back(localBuffer[0]);
    if (data.size() > 1024) {
      //Something is wrong
      _bl->fileDescriptorManager.close(_fileDescriptor);
    }
    if (localBuffer[0] == splitChar) return 0;
  }
  return -1;
}

void SerialReaderWriter::writeLine(std::string &data) {
  try {
    if (!_fileDescriptor || _fileDescriptor->descriptor == -1) throw SerialReaderWriterException("Couldn't write to device \"" + _device + "\", because the file descriptor is not valid.");
    if (data.empty()) return;
    if (data.back() != '\n') data.push_back('\n');
    int32_t bytesWritten = 0;
    int32_t i;
    std::lock_guard<std::mutex> sendGuard(_sendMutex);
    while (bytesWritten < (signed)data.length()) {
      if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Writing: " + data);
      if (write_gpio_index_ != -1) gpio_->set(write_gpio_index_, true);
      i = write(_fileDescriptor->descriptor, data.c_str() + bytesWritten, data.length() - bytesWritten);
      if (write_gpio_index_ != -1) gpio_->set(write_gpio_index_, false);
      if (i == -1) {
        if (errno == EAGAIN) continue;
        _bl->out.printError("Error writing to serial device \"" + _device + "\" (3, " + std::to_string(errno) + ").");
        return;
      }
      bytesWritten += i;
    }
    tcdrain(_fileDescriptor->descriptor);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void SerialReaderWriter::writeData(const std::vector<char> &data) {
  try {
    if (!_fileDescriptor || _fileDescriptor->descriptor == -1) throw SerialReaderWriterException("Couldn't write to device \"" + _device + "\", because the file descriptor is not valid.");
    if (data.empty()) return;
    int32_t bytesWritten = 0;
    int32_t i;
    std::lock_guard<std::mutex> sendGuard(_sendMutex);
    while (bytesWritten < (signed)data.size()) {
      if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Writing: " + HelperFunctions::getHexString(data));
      if (write_gpio_index_ != -1) gpio_->set(write_gpio_index_, true);
      i = write(_fileDescriptor->descriptor, data.data() + bytesWritten, data.size() - bytesWritten);
      if (write_gpio_index_ != -1) gpio_->set(write_gpio_index_, false);
      if (i == -1) {
        if (errno == EAGAIN) continue;
        _bl->out.printError("Error writing to serial device \"" + _device + "\" (3, " + std::to_string(errno) + ").");
        return;
      }
      bytesWritten += i;
    }
    tcdrain(_fileDescriptor->descriptor);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void SerialReaderWriter::writeData(const std::vector<uint8_t> &data) {
  try {
    if (!_fileDescriptor || _fileDescriptor->descriptor == -1) throw SerialReaderWriterException("Couldn't write to device \"" + _device + "\", because the file descriptor is not valid.");
    if (data.empty()) return;
    int32_t bytesWritten = 0;
    int32_t i;
    std::lock_guard<std::mutex> sendGuard(_sendMutex);
    while (bytesWritten < (signed)data.size()) {
      if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Writing: " + HelperFunctions::getHexString(data));
      if (write_gpio_index_ != -1) gpio_->set(write_gpio_index_, true);
      i = write(_fileDescriptor->descriptor, (char *)data.data() + bytesWritten, data.size() - bytesWritten);
      if (write_gpio_index_ != -1) gpio_->set(write_gpio_index_, false);
      if (i == -1) {
        if (errno == EAGAIN) continue;
        _bl->out.printError("Error writing to serial device \"" + _device + "\" (3, " + std::to_string(errno) + ").");
        return;
      }
      bytesWritten += i;
    }
    tcdrain(_fileDescriptor->descriptor);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void SerialReaderWriter::writeChar(char data) {
  try {
    if (!_fileDescriptor || _fileDescriptor->descriptor == -1) throw SerialReaderWriterException("Couldn't write to device \"" + _device + "\", because the file descriptor is not valid.");
    int32_t bytesWritten = 0;
    int32_t i;
    std::lock_guard<std::mutex> sendGuard(_sendMutex);
    while (bytesWritten < 1) {
      if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Writing: " + data);
      if (write_gpio_index_ != -1) gpio_->set(write_gpio_index_, true);
      i = write(_fileDescriptor->descriptor, &data, 1);
      if (write_gpio_index_ != -1) gpio_->set(write_gpio_index_, false);
      if (i == -1) {
        if (errno == EAGAIN) continue;
        _bl->out.printError("Error writing to serial device \"" + _device + "\" (3, " + std::to_string(errno) + ").");
        return;
      }
      bytesWritten += i;
    }
    tcdrain(_fileDescriptor->descriptor);
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void SerialReaderWriter::readThread(bool parity, bool oddParity, CharacterSize characterSize, bool twoStopBits) {
  std::string data;
  while (!_stopReadThread) {
    try {
      if (_fileDescriptor->descriptor == -1) {
        closeDevice();
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        _openDeviceThreadMutex.lock();
        _bl->threadManager.join(_openDeviceThread);
        _bl->threadManager.start(_openDeviceThread, true, &SerialReaderWriter::openDevice, this, parity, oddParity, true, characterSize, twoStopBits);
        _openDeviceThreadMutex.unlock();
        return;
      }
      if (readLine(data) == 0) {
        EventHandlers eventHandlers = getEventHandlers();
        for (const auto &eventHandler: eventHandlers) {
          eventHandler.second->lock();
          try {
            if (eventHandler.second->handler()) ((ISerialReaderWriterEventSink *)eventHandler.second->handler())->lineReceived(data);
          }
          catch (const std::exception &ex) {
            _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
          }
          eventHandler.second->unlock();
        }
      }
      continue;
    }
    catch (const std::exception &ex) {
      _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    _bl->fileDescriptorManager.close(_fileDescriptor);
  }
}
}
