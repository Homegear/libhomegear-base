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

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include "../Exception.h"
#include "../Managers/FileDescriptorManager.h"
#include "../Encoding/Http.h"

#include <c1-net/TcpServer.h>

#include <atomic>

namespace BaseLib {

/**
 * Exception class for the HTTP server.
 *
 * @see HttpServer
 */
class HttpServerException : public Exception {
 public:
  HttpServerException(std::string message) : Exception(message) {}
};

/**
 * This class provides a basic HTTP server. The class is thread safe.
 *
 * HTTP Server Example Code
 * ========================
 *
 * Save the example below in `main.cpp` and compile with:
 *
 *     g++ -o main -std=c++11 main.cpp -lhomegear-base -lgcrypt -lgnutls
 *
 * Start and connect using a browser:
 *
 *     ./main
 *
 * Example code:
 *
 *     #include <homegear-base/BaseLib.h>
 *
 *     std::shared_ptr<BaseLib::SharedObjects> _bl;
 *     std::shared_ptr<BaseLib::HttpServer> _httpServer;
 *
 *     void newConnection(int32_t clientId, std::string address, uint16_t port)
 *     {
 *         std::cout << "New connection from " << address << " on port " << port << std::endl;
 *     }
 *
 *     void packetReceived(int32_t clientId, BaseLib::Http http)
 *     {
 *         if(http.getHeader().method != "GET") return;
 *
 *         std::cout << "Client requested " << http.getHeader().path << std::endl;
 *
 *         std::string json = "{\"result\":\"It worked!\",\"time\":" + std::to_string(BaseLib::HelperFunctions::getTime()) + ",\"path\":\"" + http.getHeader().path + "\"}";
 *
 *         std::string header;
 *         header.append("HTTP/1.1 200 OK\r\n");
 *         header.append("Connection: close\r\n");
 *         header.append("Content-Type: application/json\r\n");
 *         header.append("Content-Length: ").append(std::to_string(json.size())).append("\r\n\r\n");
 *
 *         BaseLib::TcpSocket::TcpPacket response;
 *         response.insert(response.end(), header.begin(), header.end());
 *         response.insert(response.end(), json.begin(), json.end());
 *
 *         _httpServer->send(clientId, response);
 *     }
 *
 *     int main()
 *     {
 *         _bl.reset(new BaseLib::SharedObjects(false));
 *
 *         BaseLib::HttpServer::HttpServerInfo serverInfo;
 *         serverInfo.packetReceivedCallback = std::bind(&packetReceived, std::placeholders::_1, std::placeholders::_2);
 *
 *         _httpServer = std::make_shared<BaseLib::HttpServer>(_bl.get(), serverInfo);
 *
 *         std::string listenAddress;
 *         _httpServer->start("::", "8082", listenAddress);
 *         std::cout << "Started listening on " + listenAddress << std::endl;
 *
 *         for(int32_t i = 0; i < 300; i++) //Run for 300 seconds
 *         {
 *             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
 *         }
 *
 *         _httpServer->stop();
 *         _httpServer->waitForStop();
 *     }
 *
 */
class HttpServer {
 public:
  struct HttpServerInfo {
    std::string listenAddress = "::";
    uint16_t port = 80;
    bool useSsl = false;
    uint32_t connectionBacklogSize = 100;
    uint32_t maxConnections = 10;
    uint32_t serverThreads = 1;
    uint32_t processingThreads = 1;
    std::unordered_map<std::string, C1Net::PCertificateInfo> certificates;
    std::string dhParamFile;
    std::string dhParamData;
    bool requireClientCert = false;

    /**
     * Use proxy protocol to read original sender information. Please note that non-proxy protocol packets are
     * discarded when this is enabled.
     *
     * See https://www.haproxy.org/download/1.8/doc/proxy-protocol.txt
     */
    bool useProxyProtocol = false;

    /**
     * Only relevant when `useProxyProtocol` is set to `true`. List of allowed IPv4 and IPv6 source addresses. All
     * other source addresses are rejected.
     */
    std::vector<std::string> validProxyProtocolSources;

    std::function<void(int32_t clientId, std::string address, uint16_t port)> newConnectionCallback;
    std::function<void(int32_t clientId)> connectionClosedCallback;
    std::function<void(int32_t clientId, Http &http)> packetReceivedCallback;
  };

  HttpServer(BaseLib::SharedObjects *baseLib, HttpServerInfo &serverInfo);
  virtual ~HttpServer();

  /**
   * Binds a socket to an IP address and a port. This splits up the start process to be able to listen on ports lower
   * than 1024 and do a privilege drop. Call startPrebound() to start listening. Don't call start() when using
   * pre-binding as this recreates the socket.
   *
   * @param address The address to bind the server to (e. g. `::` or `0.0.0.0`).
   * @param port The port number to bind the server to.
   * @param[out] listenAddress The IP address the server was bound to (e. g. `192.168.0.152`).
   */
  void bind();

  void start();
  void stop();
  void waitForStop();
  void reload();

  double GetPacketsPerMinuteReceived();
  double GetPacketsPerMinuteSent();
  double GetServerThreadLoad();
  double GetProcessingThreadLoad();
  double GetProcessingThreadLoadMax();

  void send(int32_t clientId, const C1Net::TcpPacket &packet, bool closeConnection = true);
  void send(int32_t clientId, const std::vector<char> &packet, bool closeConnection = true);

  std::string getClientCertDn(int32_t clientId);
  std::string getClientCertSerial(int32_t clientId);
  int64_t getClientCertExpiration(int32_t clientId);
 protected:
  struct HttpClientInfo {
    std::shared_ptr<Http> http;
  };

  BaseLib::SharedObjects *_bl = nullptr;
  std::shared_ptr<C1Net::TcpServer> _socket;

  std::mutex _httpClientInfoMutex;
  std::unordered_map<int32_t, HttpClientInfo> _httpClientInfo;

  std::function<void(int32_t clientId, std::string address, uint16_t port)> _newConnectionCallback;
  std::function<void(int32_t clientId)> _connectionClosedCallback;
  std::function<void(int32_t clientId, Http &http)> _packetReceivedCallback;

  void Log(uint32_t log_level, const std::string &message);
  void newConnection(const C1Net::TcpServer::PTcpClientData &client_data);
  void connectionClosed(const C1Net::TcpServer::PTcpClientData &client_data, int32_t errorCode, const std::string &error_string);
  void packetReceived(const C1Net::TcpServer::PTcpClientData &client_data, const C1Net::TcpPacket &packet);
};

}
#endif
