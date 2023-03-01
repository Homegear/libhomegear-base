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

#include "../BaseLib.h"
#include "HttpServer.h"

namespace BaseLib {

HttpServer::HttpServer(BaseLib::SharedObjects *baseLib, HttpServerInfo &serverInfo) {
  _bl = baseLib;

  C1Net::TcpServer::TcpServerInfo tcpServerInfo;
  tcpServerInfo.listen_address = serverInfo.listenAddress;
  tcpServerInfo.port = serverInfo.port;
  tcpServerInfo.tls = serverInfo.useSsl;
  tcpServerInfo.connection_backlog_size = serverInfo.connectionBacklogSize;
  tcpServerInfo.max_connections = serverInfo.maxConnections;
  tcpServerInfo.listen_threads = serverInfo.serverThreads;
  tcpServerInfo.processing_threads = serverInfo.processingThreads;
  tcpServerInfo.certificates = serverInfo.certificates;
  tcpServerInfo.require_client_cert = serverInfo.requireClientCert;
  tcpServerInfo.log_callback = std::bind(&HttpServer::Log, this, std::placeholders::_1, std::placeholders::_2);
  tcpServerInfo.new_connection_callback = std::bind(&HttpServer::newConnection, this, std::placeholders::_1);
  tcpServerInfo.connection_closed_callback = std::bind(&HttpServer::connectionClosed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  tcpServerInfo.packet_received_callback = std::bind(&HttpServer::packetReceived, this, std::placeholders::_1, std::placeholders::_2);

  _newConnectionCallback.swap(serverInfo.newConnectionCallback);
  _connectionClosedCallback.swap(serverInfo.connectionClosedCallback);
  _packetReceivedCallback.swap(serverInfo.packetReceivedCallback);

  _socket = std::make_shared<C1Net::TcpServer>(baseLib, tcpServerInfo);
}

HttpServer::~HttpServer() {
  stop();
}

void HttpServer::bind() {
  _socket->Bind();
}

void HttpServer::start() {
  _socket->Start();
}

void HttpServer::stop() {
  _socket->Stop();
}

void HttpServer::waitForStop() {
  _socket->WaitForServerStopped();
}

void HttpServer::reload() {
  _socket->ReloadCertificates();
}

double HttpServer::GetPacketsPerMinuteReceived() {
  return _socket->GetPacketsPerMinuteReceived();
}

double HttpServer::GetPacketsPerMinuteSent() {
  return _socket->GetPacketsPerMinuteSent();
}

double HttpServer::GetServerThreadLoad() {
  return _socket->GetServerThreadLoad();
}

double HttpServer::GetProcessingThreadLoad() {
  return _socket->GetProcessingThreadLoad();
}

double HttpServer::GetProcessingThreadLoadMax() {
  return _socket->GetProcessingThreadLoadMax();
}

void HttpServer::Log(uint32_t log_level, const std::string &message) {
  _bl->out.printMessage(message, log_level, log_level < 3);
}

void HttpServer::newConnection(const C1Net::TcpServer::PTcpClientData &client_data) {
  try {
    HttpClientInfo clientInfo;
    clientInfo.http = std::make_shared<BaseLib::Http>();

    {
      std::lock_guard<std::mutex> httpClientInfoGuard(_httpClientInfoMutex);
      _httpClientInfo[client_data->GetId()] = std::move(clientInfo);
    }

    if (_newConnectionCallback) _newConnectionCallback(client_data->GetId(), client_data->GetIpAddress(), client_data->GetPort());
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void HttpServer::connectionClosed(const C1Net::TcpServer::PTcpClientData &client_data, int32_t errorCode, const std::string &error_string) {
  try {
    if (_connectionClosedCallback) _connectionClosedCallback(client_data->GetId());

    std::lock_guard<std::mutex> httpClientInfoGuard(_httpClientInfoMutex);
    _httpClientInfo.erase(client_data->GetId());
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void HttpServer::packetReceived(const C1Net::TcpServer::PTcpClientData &client_data, const C1Net::TcpPacket &packet) {
  std::shared_ptr<BaseLib::Http> http;
  try {
    {
      std::lock_guard<std::mutex> httpClientInfoGuard(_httpClientInfoMutex);
      auto clientIterator = _httpClientInfo.find(client_data->GetId());
      if (clientIterator == _httpClientInfo.end()) return;
      http = clientIterator->second.http;
    }

    uint32_t processedBytes = 0;
    while (processedBytes < packet.size()) {
      processedBytes = http->process((char *)(packet.data() + processedBytes), packet.size() - processedBytes);
      if (http->isFinished()) {
        if (_packetReceivedCallback) _packetReceivedCallback(client_data->GetId(), *http);
        http->reset();
      }
    }
    return;
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  http->reset();
}

void HttpServer::send(int32_t clientId, const C1Net::TcpPacket &packet, bool closeConnection) {
  _socket->Send(clientId, packet, closeConnection);
}

void HttpServer::send(int32_t clientId, const std::vector<char> &packet, bool closeConnection) {
  _socket->Send(clientId, packet, closeConnection);
}

std::string HttpServer::getClientCertDn(int32_t clientId) {
  if (!_socket) return "";
  auto client_data = _socket->GetClientData(clientId);
  if (!client_data) return "";
  return client_data->GetClientCertDn();
}

std::string HttpServer::getClientCertSerial(int32_t clientId) {
  if (!_socket) return "";
  auto client_data = _socket->GetClientData(clientId);
  if (!client_data) return "";
  return client_data->GetClientCertSerial();
}

int64_t HttpServer::getClientCertExpiration(int32_t clientId) {
  if (!_socket) return 0;
  auto client_data = _socket->GetClientData(clientId);
  if (!client_data) return 0;
  return client_data->GetClientCertExpiration();
}

}
