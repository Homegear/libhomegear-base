/* Copyright 2013-2016 Sathya Laufer
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
#include "../Encoding/Http.h"
#include "HttpClient.h"

namespace BaseLib
{

HttpClient::HttpClient(BaseLib::Obj* baseLib, std::string hostname, int32_t port, bool keepAlive, bool useSSL, std::string caFile, bool verifyCertificate)
{
	_bl = baseLib;
	_hostname = hostname;
	if(_hostname.empty()) throw HttpClientException("The provided hostname is empty.");
	if(port > 0 && port < 65536) _port = port;
	_keepAlive = keepAlive;
	_socket = std::unique_ptr<BaseLib::TcpSocket>(new BaseLib::TcpSocket(_bl, hostname, std::to_string(port), useSSL, caFile, verifyCertificate));
}

HttpClient::~HttpClient()
{
	_socketMutex.lock();
	if(_socket)
	{
		_socket->close();
		_socket.reset();
	}
	_socketMutex.unlock();
}

void HttpClient::get(const std::string& path, std::string& data)
{
	std::string fixedPath = path;
	if(fixedPath.empty()) fixedPath = "/";
	std::string getRequest = "GET " + fixedPath + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\n\r\n";
	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + getRequest);
	sendRequest(getRequest, data);
}

void HttpClient::sendRequest(const std::string& request, std::string& response, bool responseIsHeaderOnly)
{
	response.clear();
	Http http;
	sendRequest(request, http, responseIsHeaderOnly);
	if(http.isFinished() && http.getContentSize() > 0)
	{
		std::vector<char>& content = http.getContent();
		response.insert(response.end(), content.begin(), content.begin() + http.getContentSize());
	}
}

void HttpClient::get(const std::string& path, Http& http)
{
	std::string fixedPath = path;
	if(fixedPath.empty()) fixedPath = "/";
	std::string getRequest = "GET " + fixedPath + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\n\r\n";
	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + getRequest);
	sendRequest(getRequest, http);
}

void HttpClient::sendRequest(const std::string& request, Http& http, bool responseIsHeaderOnly)
{
	if(request.empty()) throw HttpClientException("Request is empty.");

	_socketMutex.lock();
	try
	{
		try
		{
			if(!_socket->connected()) _socket->open();
		}
		catch(const BaseLib::SocketOperationException& ex)
		{
			_socketMutex.unlock();
			throw HttpClientException("Unable to connect to HTTP server \"" + _hostname + "\": " + ex.what());
		}

		try
		{
			if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Sending packet to HTTP server \"" + _hostname + "\": " + request);
			_socket->proofwrite(request);
		}
		catch(BaseLib::SocketDataLimitException& ex)
		{
			if(!_keepAlive) _socket->close();
			_socketMutex.unlock();
			throw HttpClientException("Unable to write to HTTP server \"" + _hostname + "\": " + ex.what());
		}
		catch(const BaseLib::SocketOperationException& ex)
		{
			if(!_keepAlive) _socket->close();
			_socketMutex.unlock();
			throw HttpClientException("Unable to write to HTTP server \"" + _hostname + "\": " + ex.what());
		}

		ssize_t receivedBytes;
		int32_t temp = 0;

		int32_t bufferPos = 0;
		int32_t bufferMax = 4096;
		char buffer[bufferMax + 1];

		std::this_thread::sleep_for(std::chrono::milliseconds(5)); //Some servers need a little, before the socket can be read.

		bool firstLoop = true;
		while(true)
		{
			if(!firstLoop && !_socket->connected())
			{
				if(http.getContentSize() == 0)
				{
					_socketMutex.unlock();
					throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": Connection closed.");
				}
				else
				{
					http.setFinished();
					break;
				}
			}
			firstLoop = false;

			try
			{
				if(bufferPos > bufferMax - 1) bufferPos = 0;
				receivedBytes = _socket->proofread(buffer + bufferPos, bufferMax - bufferPos);

				//Some clients send only one byte in the first packet
				if(receivedBytes == 1 && bufferPos == 0 && !http.headerIsFinished()) receivedBytes += _socket->proofread(buffer + bufferPos + 1, bufferMax - bufferPos - 1);
				temp += receivedBytes;
			}
			catch(const BaseLib::SocketTimeOutException& ex)
			{
				if(!_keepAlive) _socket->close();
				_socketMutex.unlock();
				throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": " + ex.what());
			}
			catch(const BaseLib::SocketClosedException& ex)
			{
				if(http.getContentSize() == 0)
				{
					_socketMutex.unlock();
					throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": " + ex.what());
				}
				else
				{
					http.setFinished();
					break;
				}
			}
			catch(const BaseLib::SocketOperationException& ex)
			{
				if(!_keepAlive) _socket->close();
				_socketMutex.unlock();
				throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": " + ex.what());
			}
			if(bufferPos + receivedBytes > bufferMax)
			{
				bufferPos = 0;
				continue;
			}
			//We are using string functions to process the buffer. So just to make sure,
			//they don't do something in the memory after buffer, we add '\0'
			buffer[bufferPos + receivedBytes] = '\0';

			if(!http.headerIsFinished() && (!strncmp(buffer, "401", 3) || !strncmp(&buffer[9], "401", 3))) //"401 Unauthorized" or "HTTP/1.X 401 Unauthorized"
			{
				_socketMutex.unlock();
				throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": Server requires authentication.");
			}
			if(!http.headerIsFinished() && !strncmp(&buffer[9], "200", 3) && !strstr(buffer, "\r\n\r\n") && !strstr(buffer, "\n\n"))
			{
				bufferPos = receivedBytes;
				continue;
			}
			receivedBytes = bufferPos + receivedBytes;
			bufferPos = 0;

			try
			{
				if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Received packet from HTTP server \"" + _hostname + "\": " + std::string(buffer, receivedBytes));
				http.process(buffer, receivedBytes);
				if(http.headerIsFinished() && responseIsHeaderOnly)
				{
					http.setFinished();
					break;
				}
			}
			catch(HttpException& ex)
			{
				if(!_keepAlive) _socket->close();
				_socketMutex.unlock();
				throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": " + ex.what());
			}
			if(http.getContentSize() > 104857600 || http.getHeader().contentLength > 104857600)
			{
				if(!_keepAlive) _socket->close();
				_socketMutex.unlock();
				throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": Packet with data larger than 100 MiB received.");
			}

			if(http.isFinished()) break;
		}
		if(!_keepAlive) _socket->close();
		_socketMutex.unlock();
	}
    catch(const std::exception& ex)
    {
    	_socketMutex.unlock();
    	throw ex;
    }
    catch(BaseLib::Exception& ex)
    {
    	_socketMutex.unlock();
    	throw ex;
    }
    catch(...)
    {
    	_socketMutex.unlock();
    	throw Exception("Unknown exception.");
    }
}
}
