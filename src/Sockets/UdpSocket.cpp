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

#include "../BaseLib.h"
#include "UdpSocket.h"

#include <netdb.h>
#include <sys/poll.h>
#include <arpa/inet.h>

namespace BaseLib {

UdpSocket::UdpSocket(BaseLib::SharedObjects *baseLib) : UdpSocket(baseLib, "") {
}

UdpSocket::UdpSocket(BaseLib::SharedObjects *baseLib, std::string listenPort) {
  _bl = baseLib;
  _autoConnect = false;
  _socketDescriptor.reset(new FileDescriptor);
  _listenPort = Math::getUnsignedNumber(listenPort);
}

UdpSocket::UdpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port) : UdpSocket(baseLib, hostname, port, "") {
}

UdpSocket::UdpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port, std::string listenPort) {
  _bl = baseLib;
  _socketDescriptor.reset(new FileDescriptor);
  _hostname = hostname;
  _port = port;
  _listenPort = Math::getUnsignedNumber(listenPort);
}

UdpSocket::~UdpSocket() {
  close();
}

void UdpSocket::open() {
  close();
  getSocketDescriptor();
}

void UdpSocket::autoConnect() {
  if (!_autoConnect) return;
  close();
  getSocketDescriptor();
}

void UdpSocket::close() {
  _readMutex.lock();
  _writeMutex.lock();
  _bl->fileDescriptorManager.close(_socketDescriptor);
  if (_serverInfo) {
    freeaddrinfo(_serverInfo);
    _serverInfo = nullptr;
  }
  _writeMutex.unlock();
  _readMutex.unlock();
}

int32_t UdpSocket::proofread(char *buffer, int32_t bufferSize, std::string &senderIp) {
  senderIp.clear();
  if (!_socketDescriptor) throw C1Net::Exception("Socket descriptor is nullptr.");
  std::unique_lock<std::mutex> readGuard(_readMutex);
  if (_autoConnect && !isOpen()) {
    readGuard.unlock();
    autoConnect();
    if (!isOpen()) throw C1Net::ClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (8).");
    readGuard.lock();
  }

  { //Check if we can read from socket
    pollfd poll_struct{
        (int)_socketDescriptor->descriptor,
        (short)(POLLIN),
        (short)(0)
    };

    int32_t poll_result = -1;
    do {
      poll_result = poll(&poll_struct, 1, (int)(_readTimeout / 1000));
    } while (poll_result == -1 && errno == EINTR);
    if (poll_result == -1 || (poll_struct.revents & (POLLNVAL | POLLERR | POLLHUP)) || _socketDescriptor->descriptor == -1) {
      readGuard.unlock();
      close();
      throw C1Net::ClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (2).");
    } else if (poll_result == 0) {
      throw C1Net::TimeoutException("Reading from socket timed out (1).");
    }
  }

  ssize_t bytesRead = -1;
  struct sockaddr clientInfo{};
  uint32_t addressLength = sizeof(sockaddr);
  do {
    bytesRead = recvfrom(_socketDescriptor->descriptor, buffer, bufferSize, 0, &clientInfo, &addressLength);
  } while (bytesRead < 0 && (errno == EAGAIN || errno == EINTR));
  if (bytesRead <= 0) {
    throw C1Net::ClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (3).");
  }
  readGuard.unlock();
  std::array<char, INET6_ADDRSTRLEN + 1> ipStringBuffer{};
  if (clientInfo.sa_family == AF_INET) {
    auto *s = (struct sockaddr_in *)&clientInfo;
    inet_ntop(AF_INET, &s->sin_addr, ipStringBuffer.data(), ipStringBuffer.size());
  } else { // AF_INET6
    auto *s = (struct sockaddr_in6 *)&clientInfo;
    inet_ntop(AF_INET6, &s->sin6_addr, ipStringBuffer.data(), ipStringBuffer.size());
  }
  ipStringBuffer.back() = 0;
  senderIp = std::string(ipStringBuffer.data());
  return bytesRead;
}

int32_t UdpSocket::proofwrite(const std::shared_ptr<std::vector<char>> &data) {
  if (!data || data->empty()) return 0;
  return proofwrite(*data);
}

int32_t UdpSocket::proofwrite(const std::vector<char> &data) {
  if (!_socketDescriptor) throw C1Net::Exception("Socket descriptor is nullptr.");
  std::unique_lock<std::mutex> writeGuard(_writeMutex);
  if (!isOpen()) {
    writeGuard.unlock();
    autoConnect();
    if (!isOpen()) throw C1Net::ClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (8).");
    writeGuard.lock();
  }
  if (data.empty()) {
    return 0;
  }
  if (data.size() > 104857600) {
    throw C1Net::Exception("Data size is larger than 100 MiB.");
  }

  int32_t totalBytesWritten = 0;
  while (totalBytesWritten < (signed)data.size()) {
    int32_t bytesWritten = sendto(_socketDescriptor->descriptor, data.data() + totalBytesWritten, data.size() - totalBytesWritten, 0, _serverInfo->ai_addr, sizeof(sockaddr));
    if (bytesWritten <= 0) {
      if (bytesWritten == -1 && (errno == EINTR || errno == EAGAIN)) continue;
      writeGuard.unlock();
      close();
      throw C1Net::Exception(strerror(errno));
    }
    totalBytesWritten += bytesWritten;
  }
  return totalBytesWritten;
}

int32_t UdpSocket::proofwrite(const char *buffer, int32_t bytesToWrite) {
  if (!_socketDescriptor) throw C1Net::Exception("Socket descriptor is nullptr.");
  std::unique_lock<std::mutex> writeGuard(_writeMutex);
  if (!isOpen()) {
    writeGuard.unlock();
    autoConnect();
    if (!isOpen()) throw C1Net::ClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (8).");
    writeGuard.lock();
  }
  if (bytesToWrite <= 0) {
    return 0;
  }
  if (bytesToWrite > 104857600) {
    throw C1Net::Exception("Data size is larger than 100 MiB.");
  }

  int32_t totalBytesWritten = 0;
  while (totalBytesWritten < bytesToWrite) {
    int32_t bytesWritten = sendto(_socketDescriptor->descriptor, buffer + totalBytesWritten, bytesToWrite - totalBytesWritten, 0, _serverInfo->ai_addr, sizeof(sockaddr));
    if (bytesWritten <= 0) {
      if (bytesWritten == -1 && (errno == EINTR || errno == EAGAIN)) continue;
      writeGuard.unlock();
      close();
      throw C1Net::Exception(strerror(errno));
    }
    totalBytesWritten += bytesWritten;
  }
  return totalBytesWritten;
}

int32_t UdpSocket::proofwrite(const std::string &data) {
  if (!_socketDescriptor) throw C1Net::Exception("Socket descriptor is nullptr.");
  std::unique_lock<std::mutex> writeGuard(_writeMutex);
  if (!isOpen()) {
    writeGuard.unlock();
    autoConnect();
    if (!isOpen()) throw C1Net::ClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (8).");
    writeGuard.lock();
  }
  if (data.empty()) {
    return 0;
  }
  if (data.size() > 104857600) {
    throw C1Net::Exception("Data size is larger than 100 MiB.");
  }

  int32_t totalBytesWritten = 0;
  while (totalBytesWritten < (signed)data.size()) {
    int32_t bytesWritten = sendto(_socketDescriptor->descriptor, data.data() + totalBytesWritten, data.size() - totalBytesWritten, 0, _serverInfo->ai_addr, sizeof(sockaddr));
    if (bytesWritten <= 0) {
      if (bytesWritten == -1 && (errno == EINTR || errno == EAGAIN)) continue;
      writeGuard.unlock();
      close();
      throw C1Net::Exception(strerror(errno));
    }
    totalBytesWritten += bytesWritten;
  }
  return totalBytesWritten;
}

bool UdpSocket::isOpen() {
  if (!_serverInfo || !_socketDescriptor || _socketDescriptor->descriptor == -1) return false;
  return true;
}

void UdpSocket::getSocketDescriptor() {
  _readMutex.lock();
  _writeMutex.lock();
  _bl->out.printDebug("Debug: Calling getFileDescriptor...");
  _bl->fileDescriptorManager.shutdown(_socketDescriptor);

  try {
    getConnection();
    if (!_serverInfo || !_socketDescriptor || _socketDescriptor->descriptor == -1) {
      _readMutex.unlock();
      _writeMutex.unlock();
      throw C1Net::Exception("Could not connect to server.");
    }
    _writeMutex.unlock();
    _readMutex.unlock();
  }
  catch (const std::exception &ex) {
    _writeMutex.unlock();
    _readMutex.unlock();
    throw (ex);
  }

}

void UdpSocket::getConnection() {
  _socketDescriptor.reset();
  if (_hostname.empty()) throw C1Net::Exception("Hostname is empty");
  if (_port.empty()) throw C1Net::Exception("Port is empty");

  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Opening socket to UDP host " + _hostname + " on port " + _port + "...");

  if (_serverInfo) {
    freeaddrinfo(_serverInfo);
    _serverInfo = nullptr;
  }

  struct addrinfo hostInfo{};
  hostInfo.ai_family = AF_UNSPEC;
  hostInfo.ai_socktype = SOCK_DGRAM;

  if (getaddrinfo(_hostname.c_str(), _port.c_str(), &hostInfo, &_serverInfo) != 0) {
    freeaddrinfo(_serverInfo);
    _serverInfo = nullptr;
    throw C1Net::Exception("Could not get address information: " + std::string(strerror(errno)));
  }

  char ipStringBuffer[INET6_ADDRSTRLEN + 1];
  if (_serverInfo->ai_family == AF_INET) {
    auto s = (struct sockaddr_in *)_serverInfo->ai_addr;
    inet_ntop(AF_INET, &s->sin_addr, ipStringBuffer, sizeof(ipStringBuffer));
  } else { // AF_INET6
    auto s = (struct sockaddr_in6 *)_serverInfo->ai_addr;
    inet_ntop(AF_INET6, &s->sin6_addr, ipStringBuffer, sizeof(ipStringBuffer));
  }
  ipStringBuffer[INET6_ADDRSTRLEN] = '\0';
  _clientIp = std::string(ipStringBuffer);

  _socketDescriptor = _bl->fileDescriptorManager.add(socket(_serverInfo->ai_family, _serverInfo->ai_socktype | SOCK_CLOEXEC | SOCK_NONBLOCK, _serverInfo->ai_protocol));
  if (!_socketDescriptor || _socketDescriptor->descriptor == -1) {
    freeaddrinfo(_serverInfo);
    _serverInfo = nullptr;
    throw C1Net::Exception("Could not create UDP socket for server " + _clientIp + " on port " + _port + ": " + strerror(errno));
  }

  if (_serverInfo->ai_family == AF_INET) {
    struct sockaddr_in clientInfo{};
    clientInfo.sin_family = _serverInfo->ai_family;
    clientInfo.sin_addr.s_addr = INADDR_ANY;
    clientInfo.sin_port = htons(_listenPort);
    if (bind(_socketDescriptor->descriptor.load(), (struct sockaddr *)&clientInfo, sizeof(clientInfo)) == -1) {
      freeaddrinfo(_serverInfo);
      _serverInfo = nullptr;
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw C1Net::Exception("Could not bind server: " + std::string(strerror(errno)));
    }

    uint32_t clientInfoSize = sizeof(clientInfo);
    if (getsockname(_socketDescriptor->descriptor, (struct sockaddr *)&clientInfo, &clientInfoSize) == -1) {
      freeaddrinfo(_serverInfo);
      _serverInfo = nullptr;
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw C1Net::Exception("Could not get listen ip and port: " + std::string(strerror(errno)));
    }

    auto s = (struct sockaddr_in *)&(clientInfo.sin_addr);
    inet_ntop(AF_INET, s, ipStringBuffer, sizeof(ipStringBuffer));
    ipStringBuffer[INET6_ADDRSTRLEN] = '\0';
    _listenIp = std::string(ipStringBuffer);
  } else //AF_INET6
  {
    struct sockaddr_in6 clientInfo{};
    clientInfo.sin6_family = _serverInfo->ai_family;
    clientInfo.sin6_addr = in6addr_any;
    clientInfo.sin6_port = htons(_listenPort);
    if (bind(_socketDescriptor->descriptor.load(), (struct sockaddr *)&clientInfo, sizeof(clientInfo)) == -1) {
      freeaddrinfo(_serverInfo);
      _serverInfo = nullptr;
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw C1Net::Exception("Could not bind server: " + std::string(strerror(errno)));
    }

    uint32_t clientInfoSize = sizeof(clientInfo);
    if (getsockname(_socketDescriptor->descriptor, (struct sockaddr *)&clientInfo, &clientInfoSize) == -1) {
      freeaddrinfo(_serverInfo);
      _serverInfo = nullptr;
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw C1Net::Exception("Could not get listen ip and port: " + std::string(strerror(errno)));
    }

    auto s = (struct sockaddr_in6 *)&(clientInfo.sin6_addr);
    inet_ntop(AF_INET6, s, ipStringBuffer, sizeof(ipStringBuffer));
    ipStringBuffer[INET6_ADDRSTRLEN] = '\0';
    _listenIp = std::string(ipStringBuffer);
  }
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Opened UDP socket to host " + _hostname + " on port " + _port + ". Client number is: " + std::to_string(_socketDescriptor->id));
}

}
