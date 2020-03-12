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

namespace BaseLib
{

HttpServer::HttpServer(BaseLib::SharedObjects* baseLib, HttpServerInfo& serverInfo)
{
	_bl = baseLib;

	TcpSocket::TcpServerInfo tcpServerInfo;
	tcpServerInfo.useSsl = serverInfo.useSsl;
	tcpServerInfo.maxConnections = serverInfo.maxConnections;
    tcpServerInfo.serverThreads = serverInfo.serverThreads;
	tcpServerInfo.certificates = serverInfo.certificates;
	tcpServerInfo.dhParamFile = serverInfo.dhParamFile;
	tcpServerInfo.dhParamData = serverInfo.dhParamData;
	tcpServerInfo.requireClientCert = serverInfo.requireClientCert;
    tcpServerInfo.newConnectionCallback = std::bind(&HttpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    tcpServerInfo.connectionClosedCallback = std::bind(&HttpServer::connectionClosed, this, std::placeholders::_1);
	tcpServerInfo.packetReceivedCallback = std::bind(&HttpServer::packetReceived, this, std::placeholders::_1, std::placeholders::_2);

    _newConnectionCallback.swap(serverInfo.newConnectionCallback);
    _connectionClosedCallback.swap(serverInfo.connectionClosedCallback);
	_packetReceivedCallback.swap(serverInfo.packetReceivedCallback);

	_socket = std::make_shared<TcpSocket>(baseLib, tcpServerInfo);
}

HttpServer::~HttpServer()
{
	stop();
}

void HttpServer::bind(std::string address, std::string port, std::string& listenAddress)
{
	_socket->bindServerSocket(address, port, listenAddress);
}

void HttpServer::startPrebound(std::string& listenAddress, size_t processingThreads)
{
	_socket->startPreboundServer(listenAddress, processingThreads);
}

void HttpServer::start(std::string address, std::string port, std::string& listenAddress, size_t processingThreads)
{
	_socket->startServer(address, port, listenAddress, processingThreads);
}

void HttpServer::stop()
{
	_socket->stopServer();
}

void HttpServer::waitForStop()
{
	_socket->waitForServerStopped();
}

void HttpServer::newConnection(int32_t clientId, std::string address, uint16_t port)
{
	try
	{
		HttpClientInfo clientInfo;
		clientInfo.http = std::make_shared<BaseLib::Http>();

        {
            std::lock_guard<std::mutex> httpClientInfoGuard(_httpClientInfoMutex);
            _httpClientInfo[clientId] = std::move(clientInfo);
        }

        if(_newConnectionCallback) _newConnectionCallback(clientId, address, port);
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
}

void HttpServer::connectionClosed(int32_t clientId)
{
	try
	{
        if(_connectionClosedCallback) _connectionClosedCallback(clientId);

		std::lock_guard<std::mutex> httpClientInfoGuard(_httpClientInfoMutex);
		_httpClientInfo.erase(clientId);
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
}

void HttpServer::packetReceived(int32_t clientId, TcpSocket::TcpPacket& packet)
{
	std::shared_ptr<BaseLib::Http> http;
	try
	{
		{
			std::lock_guard<std::mutex> httpClientInfoGuard(_httpClientInfoMutex);
			auto clientIterator = _httpClientInfo.find(clientId);
			if(clientIterator == _httpClientInfo.end()) return;
			http = clientIterator->second.http;
		}

		uint32_t processedBytes = 0;
		while(processedBytes < packet.size())
		{
			processedBytes = http->process((char*)(packet.data() + processedBytes), packet.size() - processedBytes);
			if(http->isFinished())
			{
				if(_packetReceivedCallback) _packetReceivedCallback(clientId, *http);
				http->reset();
			}
		}
		return;
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	http->reset();
}

void HttpServer::send(int32_t clientId, const TcpSocket::TcpPacket& packet, bool closeConnection)
{
	_socket->sendToClient(clientId, packet, closeConnection);
}

void HttpServer::send(int32_t clientId, const std::vector<char>& packet, bool closeConnection)
{
    _socket->sendToClient(clientId, packet, closeConnection);
}

std::string HttpServer::getClientCertDn(int32_t clientId)
{
    return _socket ? _socket->getClientCertDn(clientId) : "";
}

}
