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

#include "HttpClient.h"

#include "../BaseLib.h"
#include "../Encoding/Http.h"

#include <memory>

namespace BaseLib {

HttpClient::HttpClient(BaseLib::SharedObjects *baseLib, std::string hostname, int32_t port, bool keepAlive, bool useSSL, std::string caFile, bool verifyCertificate, std::string certPath, std::string keyPath) {
  _bl = baseLib;
  _hostname = hostname;
  if (_hostname.empty()) throw HttpClientException("The provided hostname is empty.");
  if (port > 0 && port < 65536) _port = port;
  _keepAlive = keepAlive;

  C1Net::TcpSocketInfo tcp_socket_info;
  tcp_socket_info.read_timeout = 5000;
  tcp_socket_info.write_timeout = 5000;
  tcp_socket_info.log_callback = std::bind(&HttpClient::Log, this, std::placeholders::_1, std::placeholders::_2);;

  C1Net::TcpSocketHostInfo tcp_socket_host_info;
  tcp_socket_host_info.host = hostname;
  tcp_socket_host_info.port = _port;
  tcp_socket_host_info.auto_connect = false;
  tcp_socket_host_info.connection_retries = 2;
  tcp_socket_host_info.tls = useSSL;
  tcp_socket_host_info.ca_file = caFile;
  tcp_socket_host_info.verify_certificate = verifyCertificate;
  tcp_socket_host_info.client_cert_file = certPath;
  tcp_socket_host_info.client_key_file = keyPath;

  _socket = std::make_shared<C1Net::TcpSocket>(tcp_socket_info, tcp_socket_host_info);
}

HttpClient::HttpClient(BaseLib::SharedObjects *baseLib,
                       std::string hostname,
                       int32_t port,
                       bool keepAlive,
                       bool useSSL,
                       bool verifyCertificate,
                       std::string caFile,
                       std::string caData,
                       std::string certPath,
                       std::string certData,
                       std::string keyPath,
                       const std::shared_ptr<Security::SecureVector<uint8_t>> &keyData) {
  _bl = baseLib;
  _hostname = hostname;
  if (_hostname.empty()) throw HttpClientException("The provided hostname is empty.");
  if (port > 0 && port < 65536) _port = port;
  _keepAlive = keepAlive;

  C1Net::TcpSocketInfo tcp_socket_info;
  tcp_socket_info.read_timeout = 5000;
  tcp_socket_info.write_timeout = 5000;
  tcp_socket_info.log_callback = std::bind(&HttpClient::Log, this, std::placeholders::_1, std::placeholders::_2);;

  C1Net::TcpSocketHostInfo tcp_socket_host_info;
  tcp_socket_host_info.host = hostname;
  tcp_socket_host_info.port = _port;
  tcp_socket_host_info.auto_connect = false;
  tcp_socket_host_info.connection_retries = 2;
  tcp_socket_host_info.tls = useSSL;
  tcp_socket_host_info.ca_file = caFile;
  tcp_socket_host_info.ca_data = caData;
  tcp_socket_host_info.verify_certificate = verifyCertificate;
  tcp_socket_host_info.client_cert_file = certPath;
  tcp_socket_host_info.client_cert_data = certData;
  tcp_socket_host_info.client_key_file = keyPath;
  if (keyData) tcp_socket_host_info.client_key_data = std::string(keyData->begin(), keyData->end());

  _socket = std::make_shared<C1Net::TcpSocket>(tcp_socket_info, tcp_socket_host_info);
}

HttpClient::~HttpClient() {
  std::lock_guard<std::mutex> socketGuard(_socketMutex);
  if (_socket) {
    _socket->Shutdown();
    _socket.reset();
  }
}

void HttpClient::Log(uint32_t log_level, const std::string &message) {
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + message);
}

void HttpClient::setTimeout(uint32_t value) {
  if (value == 0) value = 1000;
  _socket->SetReadTimeout((int64_t)value);
  _socket->SetWriteTimeout((int64_t)value);
}

void HttpClient::setVerifyHostname(bool value) {
  _socket->SetVerifyHostname(value);
}

void HttpClient::setUserAgent(const std::string &value) {
  if (!value.empty()) _userAgent = value;
}

void HttpClient::delete_(const std::string &path, std::string &data, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string
      deleteRequest = "DELETE " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\n" + additionalHeaders + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + deleteRequest);
  sendRequest(deleteRequest, data);
}

void HttpClient::delete_(const std::string &path, Http &http, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string
      deleteRequest = "DELETE " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\n" + additionalHeaders + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + deleteRequest);
  sendRequest(deleteRequest, http);
}

void HttpClient::get(const std::string &path, std::string &data, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string getRequest = "GET " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\n" + additionalHeaders + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + getRequest);
  sendRequest(getRequest, data);
}

void HttpClient::get(const std::string &path, Http &http, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string getRequest = "GET " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\n" + additionalHeaders + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + getRequest);
  sendRequest(getRequest, http);
}

void HttpClient::patch(const std::string &path, std::string &dataIn, std::string &dataOut, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string patchRequest =
      "PATCH " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\nContent-Length: " + std::to_string(dataIn.size() + 2)
          + "\r\n" + additionalHeaders + "\r\n" + dataIn + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + patchRequest);
  sendRequest(patchRequest, dataOut);
}

void HttpClient::patch(const std::string &path, std::string &dataIn, Http &dataOut, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string patchRequest =
      "PATCH " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\nContent-Length: " + std::to_string(dataIn.size() + 2)
          + "\r\n" + additionalHeaders + "\r\n" + dataIn + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + patchRequest);
  sendRequest(patchRequest, dataOut);
}

void HttpClient::post(const std::string &path, std::string &dataIn, std::string &dataOut, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string postRequest =
      "POST " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\nContent-Length: " + std::to_string(dataIn.size() + 2)
          + "\r\n" + additionalHeaders + "\r\n" + dataIn + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + postRequest);
  sendRequest(postRequest, dataOut);
}

void HttpClient::post(const std::string &path, std::string &dataIn, Http &dataOut, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string postRequest =
      "POST " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\nContent-Length: " + std::to_string(dataIn.size() + 2)
          + "\r\n" + additionalHeaders + "\r\n" + dataIn + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + postRequest);
  sendRequest(postRequest, dataOut);
}

void HttpClient::put(const std::string &path, std::string &dataIn, std::string &dataOut, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string putRequest =
      "PUT " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\nContent-Length: " + std::to_string(dataIn.size() + 2)
          + "\r\n" + additionalHeaders + "\r\n" + dataIn + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + putRequest);
  sendRequest(putRequest, dataOut);
}

void HttpClient::put(const std::string &path, std::string &dataIn, Http &dataOut, const std::string &additionalHeaders) {
  std::string fixedPath = path;
  if (fixedPath.empty()) fixedPath = "/";
  std::string putRequest =
      "PUT " + fixedPath + " HTTP/1.1\r\nUser-Agent: " + _userAgent + "\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\nContent-Length: " + std::to_string(dataIn.size() + 2)
          + "\r\n" + additionalHeaders + "\r\n" + dataIn + "\r\n";
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + putRequest);
  sendRequest(putRequest, dataOut);
}

void HttpClient::sendRequest(const std::string &request, std::string &response, bool responseIsHeaderOnly) {
  response.clear();
  Http http;
  sendRequest(request, http, responseIsHeaderOnly);
  if (http.isFinished() && http.getContentSize() > 0) {
    const std::vector<char> &content = http.getContent();
    response.insert(response.end(), content.begin(), content.begin() + http.getContentSize());
  }
}

void HttpClient::sendRequest(const std::string &request, Http &http, bool responseIsHeaderOnly) {
  if (request.empty()) throw HttpClientException("Request is empty.");

  std::lock_guard<std::mutex> socketGuard(_socketMutex);
  //The loop is implemented to resend a request in case we get an EOF on first read.
  for (uint32_t i = 0; i < 2; i++) {
    _rawContent.clear();
    http.reset();
    try {
      if (!_socket->Connected()) _socket->Open();
    }
    catch (const C1Net::TimeoutException &ex) {
      throw HttpClientTimeOutException(std::string(ex.what()));
    }
    catch (const C1Net::Exception &ex) {
      throw HttpClientException("Unable to connect to HTTP server \"" + _hostname + "\": " + ex.what());
    }

    try {
      if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Sending packet to HTTP server \"" + _hostname + "\": " + request);
      _socket->Send((uint8_t *)request.data(), request.size());
    }
    catch (const C1Net::ClosedException &ex) {
      _socket->Shutdown();
      if (i == 1) http.setFinished();
      else continue;
    }
    catch (const C1Net::TimeoutException &ex) {
      if (i == 1) throw HttpClientTimeOutException(std::string(ex.what()));
      continue;
    }
    catch (const C1Net::Exception &ex) {
      _socket->Shutdown();
      if (i == 1) throw HttpClientException("Unable to write to HTTP server \"" + _hostname + "\": " + ex.what());
      continue;
    }

    std::size_t receivedBytes;

    int32_t bufferPos = 0;
    const int32_t bufferMax = 4096;
    std::array<uint8_t, bufferMax + 1> buffer{};
    bool more_data = false;

    std::this_thread::sleep_for(std::chrono::milliseconds(5)); //Some servers need a little, before the socket can be read.

    while (true) {
      try {
        if (bufferPos > bufferMax - 1) {
          bufferPos = 0;
          throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\" (1): Buffer overflow.");
        }
        receivedBytes = _socket->Read(buffer.data() + bufferPos, bufferMax - bufferPos, more_data);
      }
      catch (const C1Net::TimeoutException &ex) {
        if (!_keepAlive) _socket->Shutdown();
        throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\" (1): " + ex.what());
      }
      catch (const C1Net::ClosedException &ex) {
        _socket->Shutdown();
        if (i == 1) http.setFinished();
        break;
      }
      catch (const C1Net::Exception &ex) {
        if (!_keepAlive) _socket->Shutdown();
        throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\" (3): " + ex.what());
      }

      if (bufferPos + receivedBytes > bufferMax) {
        if (!_keepAlive) _socket->Shutdown();
        throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\" (2): Buffer overflow.");
      }

      if (_keepRawContent) {
        if (_rawContent.size() + receivedBytes > _rawContent.capacity()) _rawContent.reserve(_rawContent.capacity() * 2);
        _rawContent.insert(_rawContent.end(), buffer.begin(), buffer.begin() + receivedBytes);
      }

      //We are using string functions to process the buffer. So just to make sure,
      //they don't do something in the memory after buffer, we add '\0'
      buffer.at(bufferPos + receivedBytes) = '\0';

      if (!http.headerIsFinished() && (!strncmp((char *)buffer.data(), "401", 3) || !strncmp((char *)buffer.data() + 9, "401", 3))) //"401 Unauthorized" or "HTTP/1.X 401 Unauthorized"
      {
        throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": Server requires authentication.", 401);
      }
      receivedBytes = bufferPos + receivedBytes;
      bufferPos = 0;

      try {
        if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Received packet from HTTP server \"" + _hostname + "\": " + std::string(buffer.begin(), buffer.begin() + receivedBytes));
        http.process((char *)buffer.data(), receivedBytes);
        if (http.headerIsFinished() && responseIsHeaderOnly) {
          http.setFinished();
          break;
        }
      }
      catch (const HttpException &ex) {
        if (!_keepAlive) _socket->Shutdown();
        throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": " + ex.what(), ex.responseCode());
      }
      if (http.getContentSize() > 104857600 || http.getHeader().contentLength > 104857600) {
        if (!_keepAlive) _socket->Shutdown();
        throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": Packet with data larger than 100 MiB received.");
      }

      if (http.isFinished()) break;
    }
    if (!_keepAlive) _socket->Shutdown();
    if (http.isFinished()) break;
  }
}

}
