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
#include "../Encoding/Http.h"
#include "HttpClient.h"

namespace BaseLib
{

HttpClient::HttpClient(BaseLib::SharedObjects* baseLib, std::string hostname, int32_t port, bool keepAlive, bool useSSL, std::string caFile, bool verifyCertificate, std::string certPath, std::string keyPath)
{
	_bl = baseLib;
	_hostname = hostname;
	if(_hostname.empty()) throw HttpClientException("The provided hostname is empty.");
	if(port > 0 && port < 65536) _port = port;
	_keepAlive = keepAlive;
	_socket = std::unique_ptr<BaseLib::TcpSocket>(new BaseLib::TcpSocket(_bl, hostname, std::to_string(port), useSSL, caFile, verifyCertificate, certPath, keyPath));
	_socket->setConnectionRetries(1);
}

HttpClient::HttpClient(BaseLib::SharedObjects* baseLib, std::string hostname, int32_t port, bool keepAlive, bool useSSL, bool verifyCertificate, std::string caFile, std::string caData, std::string certPath, std::string certData, std::string keyPath, const std::shared_ptr<Security::SecureVector<uint8_t>>& keyData)
{
	_bl = baseLib;
	_hostname = hostname;
	if(_hostname.empty()) throw HttpClientException("The provided hostname is empty.");
	if(port > 0 && port < 65536) _port = port;
	_keepAlive = keepAlive;
	_socket = std::unique_ptr<BaseLib::TcpSocket>(new BaseLib::TcpSocket(_bl, hostname, std::to_string(port), useSSL, verifyCertificate, caFile, caData, certPath, certData, keyPath, keyData));
	_socket->setConnectionRetries(1);
}

HttpClient::~HttpClient()
{
	std::lock_guard<std::mutex> socketGuard(_socketMutex);
	if(_socket)
	{
		_socket->close();
		_socket.reset();
	}
}

void HttpClient::setTimeout(uint32_t value)
{
	if(value == 0) value = 1000;
	_socket->setReadTimeout((int64_t)value * 1000);
	_socket->setWriteTimeout((int64_t)value * 1000);
}

void HttpClient::get(const std::string& path, std::string& data)
{
	std::string fixedPath = path;
	if(fixedPath.empty()) fixedPath = "/";
	std::string getRequest = "GET " + fixedPath + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\n\r\n";
	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + getRequest);
	sendRequest(getRequest, data);
}

void HttpClient::get(const std::string& path, Http& http)
{
	std::string fixedPath = path;
	if(fixedPath.empty()) fixedPath = "/";
	std::string getRequest = "GET " + fixedPath + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\n\r\n";
	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + getRequest);
	sendRequest(getRequest, http);
}

void HttpClient::post(const std::string& path, std::string& dataIn, std::string& dataOut)
{
	std::string fixedPath = path;
	if(fixedPath.empty()) fixedPath = "/";
	std::string postRequest = "POST " + fixedPath + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\nContent-Length: " + std::to_string(dataIn.size() + 2) + "\r\n\r\n" + dataIn + "\r\n";
	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + postRequest);
	sendRequest(postRequest, dataOut);
}

void HttpClient::post(const std::string& path, std::string& dataIn, Http& dataOut)
{
	std::string fixedPath = path;
	if(fixedPath.empty()) fixedPath = "/";
	std::string postRequest = "POST " + fixedPath + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\nConnection: " + (_keepAlive ? "Keep-Alive" : "Close") + "\r\nContent-Length: " + std::to_string(dataIn.size() + 2) + "\r\n\r\n" + dataIn + "\r\n";
	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: HTTP request: " + postRequest);
	sendRequest(postRequest, dataOut);
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

void HttpClient::sendRequest(const std::string& request, Http& http, bool responseIsHeaderOnly)
{
	_rawContent.clear();
	if(request.empty()) throw HttpClientException("Request is empty.");

	std::lock_guard<std::mutex> socketGuard(_socketMutex);
    try
    {
        if(!_socket->connected()) _socket->open();
    }
    catch(const BaseLib::SocketTimeOutException& ex)
    {
        throw HttpClientTimeOutException(std::string(ex.what()));
    }
    catch(const BaseLib::SocketOperationException& ex)
    {
        throw HttpClientException("Unable to connect to HTTP server \"" + _hostname + "\": " + ex.what());
    }

    try
    {
        if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Sending packet to HTTP server \"" + _hostname + "\": " + request);
        _socket->proofwrite(request);
    }
    catch(const BaseLib::SocketDataLimitException& ex)
    {
        if(!_keepAlive) _socket->close();
        throw HttpClientException("Unable to write to HTTP server \"" + _hostname + "\": " + ex.what());
    }
    catch(const BaseLib::SocketOperationException& ex)
    {
        if(!_keepAlive) _socket->close();
        throw HttpClientException("Unable to write to HTTP server \"" + _hostname + "\": " + ex.what());
    }

    ssize_t receivedBytes;

    int32_t bufferPos = 0;
    const int32_t bufferMax = 4096;
    std::array<char, bufferMax + 1> buffer{};

    std::this_thread::sleep_for(std::chrono::milliseconds(5)); //Some servers need a little, before the socket can be read.

    bool firstLoop = true;
    while(true)
    {
        if(!firstLoop && !_socket->connected())
        {
            if(http.getContentSize() == 0 || http.getHeader().responseCode == -1)
            {
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
            if(bufferPos > bufferMax - 1)
            {
                bufferPos = 0;
                throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\" (1): Buffer overflow.");
            }
            receivedBytes = _socket->proofread(buffer.data() + bufferPos, bufferMax - bufferPos);

            //Some clients send only one byte in the first packet
            if(receivedBytes < 13 && bufferPos == 0 && !http.headerIsFinished()) receivedBytes += _socket->proofread(buffer.data() + bufferPos + 1, bufferMax - bufferPos - 1);
        }
        catch(const BaseLib::SocketTimeOutException& ex)
        {
            if(!_keepAlive) _socket->close();
            throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\" (1): " + ex.what());
        }
        catch(const BaseLib::SocketClosedException& ex)
        {
            http.setFinished();
            throw HttpClientSocketClosedException("Connection to client was closed.");
        }
        catch(const BaseLib::SocketOperationException& ex)
        {
            if(!_keepAlive) _socket->close();
            throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\" (3): " + ex.what());
        }

        if(bufferPos + receivedBytes > bufferMax)
        {
            if(!_keepAlive) _socket->close();
            throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\" (2): Buffer overflow.");
        }

        if(_keepRawContent)
        {
            if(_rawContent.size() + receivedBytes > _rawContent.capacity()) _rawContent.reserve(_rawContent.capacity() + 4096);
            _rawContent.insert(_rawContent.end(), buffer.begin(), buffer.begin() + receivedBytes);
        }

        //We are using string functions to process the buffer. So just to make sure,
        //they don't do something in the memory after buffer, we add '\0'
        buffer.at(bufferPos + receivedBytes) = '\0';

        if(!http.headerIsFinished() && (!strncmp(buffer.data(), "401", 3) || !strncmp(buffer.data() + 9, "401", 3))) //"401 Unauthorized" or "HTTP/1.X 401 Unauthorized"
        {
            throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": Server requires authentication.", 401);
        }
        receivedBytes = bufferPos + receivedBytes;
        bufferPos = 0;

        try
        {
            if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Received packet from HTTP server \"" + _hostname + "\": " + std::string(buffer.begin(), buffer.begin() + receivedBytes));
            http.process(buffer.data(), receivedBytes);
            if(http.headerIsFinished() && responseIsHeaderOnly)
            {
                http.setFinished();
                break;
            }
        }
        catch(HttpException& ex)
        {
            if(!_keepAlive) _socket->close();
            throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": " + ex.what(), ex.responseCode());
        }
        if(http.getContentSize() > 104857600 || http.getHeader().contentLength > 104857600)
        {
            if(!_keepAlive) _socket->close();
            throw HttpClientException("Unable to read from HTTP server \"" + _hostname + "\": Packet with data larger than 100 MiB received.");
        }

        if(http.isFinished()) break;
    }
    if(!_keepAlive) _socket->close();
}

}
