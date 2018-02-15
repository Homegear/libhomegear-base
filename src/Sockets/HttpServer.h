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

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include "../Exception.h"
#include "../Managers/FileDescriptorManager.h"
#include "../Encoding/Http.h"
#include "TcpSocket.h"

#include <atomic>

namespace BaseLib
{

/**
 * Exception class for the HTTP server.
 *
 * @see HttpServer
 */
class HttpServerException : public Exception
{
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
	struct HttpServerInfo
	{
		bool useSsl = false;
		uint32_t maxConnections = 10;
		std::unordered_map<std::string, std::string> certFiles;
		std::unordered_map<std::string, std::string> certData;
		std::unordered_map<std::string, std::string> keyFiles;
		std::unordered_map<std::string, std::string> keyData;
		std::string dhParamFile;
		std::string dhParamData;
		bool requireClientCert = false;
		std::unordered_map<std::string, std::string> caFiles; //For client certificate verification
		std::unordered_map<std::string, std::string> caData; //For client certificate verification

		std::function<void(int32_t clientId, Http& http)> packetReceivedCallback;
	};

	HttpServer(BaseLib::SharedObjects* baseLib, HttpServerInfo& serverInfo);
	virtual ~HttpServer();

	void start(std::string address, std::string port, std::string& listenAddress);
	void stop();
	void waitForStop();

	void send(int32_t clientId, TcpSocket::TcpPacket packet, bool closeConnection = true);
protected:
	BaseLib::SharedObjects* _bl = nullptr;
	std::shared_ptr<TcpSocket> _socket;
	Http _http;

	std::function<void(int32_t clientId, Http& http)> _packetReceivedCallback;

	void packetReceived(int32_t clientId, TcpSocket::TcpPacket packet);
};

}
#endif
