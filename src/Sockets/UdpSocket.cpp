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
#include "UdpSocket.h"

namespace BaseLib
{

UdpSocket::UdpSocket(BaseLib::SharedObjects* baseLib)
{
	_bl = baseLib;
	_autoConnect = false;
	_socketDescriptor.reset(new FileDescriptor);
}

UdpSocket::UdpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port)
{
	_bl = baseLib;
	signal(SIGPIPE, SIG_IGN);

	_socketDescriptor.reset(new FileDescriptor);
	_hostname = hostname;
	_port = port;
}

UdpSocket::~UdpSocket()
{
	_bl->fileDescriptorManager.close(_socketDescriptor);
}

void UdpSocket::open()
{
	if(!_serverInfo || !_socketDescriptor || _socketDescriptor->descriptor == -1) getSocketDescriptor();
}

void UdpSocket::autoConnect()
{
	if(!_autoConnect) return;
	if(!_serverInfo || !_socketDescriptor || _socketDescriptor->descriptor == -1) getSocketDescriptor();
}

void UdpSocket::close()
{
	_readMutex.lock();
	_writeMutex.lock();
	_bl->fileDescriptorManager.close(_socketDescriptor);
	if(_serverInfo)
	{
		freeaddrinfo(_serverInfo);
		_serverInfo = nullptr;
	}
	_writeMutex.unlock();
	_readMutex.unlock();
}

int32_t UdpSocket::proofread(char* buffer, int32_t bufferSize, std::string& senderIp)
{
	senderIp.clear();
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	_readMutex.lock();
	if(_autoConnect && !isOpen())
	{
		_readMutex.unlock();
		autoConnect();
		if(!isOpen()) throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (8).");
		_readMutex.lock();
	}
	timeval timeout;
	int32_t seconds = _readTimeout / 1000000;
	timeout.tv_sec = seconds;
	timeout.tv_usec = _readTimeout - (1000000 * seconds);
	fd_set readFileDescriptor;
	FD_ZERO(&readFileDescriptor);
	_bl->fileDescriptorManager.lock();
	int32_t nfds = _socketDescriptor->descriptor + 1;
	if(nfds <= 0)
	{
		_bl->fileDescriptorManager.unlock();
		_readMutex.unlock();
		throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (1).");
	}
	FD_SET(_socketDescriptor->descriptor, &readFileDescriptor);
	_bl->fileDescriptorManager.unlock();
	int32_t bytesRead = select(nfds, &readFileDescriptor, NULL, NULL, &timeout);
	if(bytesRead == 0)
	{
		_readMutex.unlock();
		throw SocketTimeOutException("Reading from socket timed out.");
	}
	if(bytesRead != 1)
	{
		_readMutex.unlock();
		throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (2).");
	}
	struct addrinfo clientInfo;
	memset(&clientInfo, 0, sizeof(addrinfo));
	uint32_t addressLength = sizeof(sockaddr);
	do
	{
		bytesRead = recvfrom(_socketDescriptor->descriptor, buffer, bufferSize, 0, clientInfo.ai_addr, &addressLength);
	} while(bytesRead < 0 && (errno == EAGAIN || errno == EINTR));
	if(bytesRead <= 0)
	{
		_readMutex.unlock();
		throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (3).");
	}
	_readMutex.unlock();
	char ipStringBuffer[INET6_ADDRSTRLEN];
	if (_serverInfo->ai_family == AF_INET)
	{
		struct sockaddr_in *s = (struct sockaddr_in*)_serverInfo->ai_addr;
		inet_ntop(AF_INET, &s->sin_addr, ipStringBuffer, sizeof(ipStringBuffer));
	}
	else
	{ // AF_INET6
		struct sockaddr_in6 *s = (struct sockaddr_in6*)_serverInfo->ai_addr;
		inet_ntop(AF_INET6, &s->sin6_addr, ipStringBuffer, sizeof(ipStringBuffer));
	}
	senderIp = std::string(&ipStringBuffer[0]);
	return bytesRead;
}

int32_t UdpSocket::proofwrite(const std::shared_ptr<std::vector<char>> data)
{
	if(!data || data->empty()) return 0;
	return proofwrite(*data);
}

int32_t UdpSocket::proofwrite(const std::vector<char>& data)
{
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	_writeMutex.lock();
	if(!isOpen())
	{
		_writeMutex.unlock();
		autoConnect();
		if(!isOpen()) throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (8).");
		_writeMutex.lock();
	}
	if(data.empty())
	{
		_writeMutex.unlock();
		return 0;
	}
	if(data.size() > 104857600)
	{
		_writeMutex.unlock();
		throw SocketDataLimitException("Data size is larger than 100 MiB.");
	}

	int32_t totalBytesWritten = 0;
	while (totalBytesWritten < (signed)data.size())
	{
		/*timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		fd_set writeFileDescriptor;
		FD_ZERO(&writeFileDescriptor);
		_bl->fileDescriptorManager.lock();
		int32_t nfds = _socketDescriptor->descriptor + 1;
		if(nfds <= 0)
		{
			_bl->fileDescriptorManager.unlock();
			_writeMutex.unlock();
			throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (4).");
		}
		FD_SET(_socketDescriptor->descriptor, &writeFileDescriptor);
		_bl->fileDescriptorManager.unlock();
		int32_t readyFds = select(nfds, NULL, &writeFileDescriptor, NULL, &timeout);
		if(readyFds == 0)
		{
			_writeMutex.unlock();
			throw SocketTimeOutException("Writing to socket timed out.");
		}
		if(readyFds != 1)
		{
			_writeMutex.unlock();
			throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (5).");
		}*/

		int32_t bytesWritten = sendto(_socketDescriptor->descriptor, &data.at(totalBytesWritten), data.size() - totalBytesWritten, 0, _serverInfo->ai_addr, sizeof(sockaddr));
		if(bytesWritten <= 0)
		{
			if(bytesWritten == -1 && errno == EINTR) continue;
			_writeMutex.unlock();
			close();
			throw SocketOperationException(strerror(errno));
		}
		totalBytesWritten += bytesWritten;
	}
	_writeMutex.unlock();
	return totalBytesWritten;
}

int32_t UdpSocket::proofwrite(const char* buffer, int32_t bytesToWrite)
{
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	_writeMutex.lock();
	if(!isOpen())
	{
		_writeMutex.unlock();
		autoConnect();
		if(!isOpen()) throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (8).");
		_writeMutex.lock();
	}
	if(bytesToWrite <= 0)
	{
		_writeMutex.unlock();
		return 0;
	}
	if(bytesToWrite > 104857600)
	{
		_writeMutex.unlock();
		throw SocketDataLimitException("Data size is larger than 100 MiB.");
	}

	int32_t totalBytesWritten = 0;
	while (totalBytesWritten < bytesToWrite)
	{
		/*timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		fd_set writeFileDescriptor;
		FD_ZERO(&writeFileDescriptor);
		_bl->fileDescriptorManager.lock();
		int32_t nfds = _socketDescriptor->descriptor + 1;
		if(nfds <= 0)
		{
			_bl->fileDescriptorManager.unlock();
			_writeMutex.unlock();
			throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (4).");
		}
		FD_SET(_socketDescriptor->descriptor, &writeFileDescriptor);
		_bl->fileDescriptorManager.unlock();
		int32_t readyFds = select(nfds, NULL, &writeFileDescriptor, NULL, &timeout);
		if(readyFds == 0)
		{
			_writeMutex.unlock();
			throw SocketTimeOutException("Writing to socket timed out.");
		}
		if(readyFds != 1)
		{
			_writeMutex.unlock();
			throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (5).");
		}*/

		int32_t bytesWritten = sendto(_socketDescriptor->descriptor, buffer + totalBytesWritten, bytesToWrite - totalBytesWritten, 0, _serverInfo->ai_addr, sizeof(sockaddr));
		if(bytesWritten <= 0)
		{
			if(bytesWritten == -1 && errno == EINTR) continue;
			_writeMutex.unlock();
			close();
			throw SocketOperationException(strerror(errno));
		}
		totalBytesWritten += bytesWritten;
	}
	_writeMutex.unlock();
	return totalBytesWritten;
}

int32_t UdpSocket::proofwrite(const std::string& data)
{
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	_writeMutex.lock();
	if(!isOpen())
	{
		_writeMutex.unlock();
		autoConnect();
		if(!isOpen()) throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (8).");
		_writeMutex.lock();
	}
	if(data.empty())
	{
		_writeMutex.unlock();
		return 0;
	}
	if(data.size() > 104857600)
	{
		_writeMutex.unlock();
		throw SocketDataLimitException("Data size is larger than 100 MiB.");
	}

	int32_t totalBytesWritten = 0;
	while (totalBytesWritten < (signed)data.size())
	{
		/*timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		fd_set writeFileDescriptor;
		FD_ZERO(&writeFileDescriptor);
		_bl->fileDescriptorManager.lock();
		int32_t nfds = _socketDescriptor->descriptor + 1;
		if(nfds <= 0)
		{
			_bl->fileDescriptorManager.unlock();
			_writeMutex.unlock();
			throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (6).");
		}
		FD_SET(_socketDescriptor->descriptor, &writeFileDescriptor);
		_bl->fileDescriptorManager.unlock();
		int32_t readyFds = select(nfds, NULL, &writeFileDescriptor, NULL, &timeout);
		if(readyFds == 0)
		{
			_writeMutex.unlock();
			throw SocketTimeOutException("Writing to socket timed out.");
		}
		if(readyFds != 1)
		{
			_writeMutex.unlock();
			throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (7).");
		}*/

		int32_t bytesToSend = data.size() - totalBytesWritten;
		int32_t bytesWritten = sendto(_socketDescriptor->descriptor, &data.at(totalBytesWritten), bytesToSend, 0, _serverInfo->ai_addr, sizeof(sockaddr));
		if(bytesWritten <= 0)
		{
			if(bytesWritten == -1 && errno == EINTR) continue;
			_writeMutex.unlock();
			close();
			throw SocketOperationException(strerror(errno));
		}
		totalBytesWritten += bytesWritten;
	}
	_writeMutex.unlock();
	return totalBytesWritten;
}

bool UdpSocket::isOpen()
{
	if(!_serverInfo || !_socketDescriptor || _socketDescriptor->descriptor == -1) return false;
	return true;
}

void UdpSocket::getSocketDescriptor()
{
	_readMutex.lock();
	_writeMutex.lock();
	_bl->out.printDebug("Debug: Calling getFileDescriptor...");
	_bl->fileDescriptorManager.shutdown(_socketDescriptor);

	try
	{
		getConnection();
		if(!_serverInfo || !_socketDescriptor || _socketDescriptor->descriptor == -1)
		{
			_readMutex.unlock();
			_writeMutex.unlock();
			throw SocketOperationException("Could not connect to server.");
		}
		_writeMutex.unlock();
		_readMutex.unlock();
	}
	catch(const std::exception& ex)
    {
		_writeMutex.unlock();
    	_readMutex.unlock();
		throw(ex);
    }
	catch(Exception& ex)
	{
		_writeMutex.unlock();
		_readMutex.unlock();
		throw(ex);
	}

}

void UdpSocket::getConnection()
{
	_socketDescriptor.reset();
	if(_hostname.empty()) throw SocketInvalidParametersException("Hostname is empty");
	if(_port.empty()) throw SocketInvalidParametersException("Port is empty");

	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Opening socket to UDP host " + _hostname + " on port " + _port + "...");

	//Retry for two minutes
	for(uint32_t i = 0; i < 6; ++i)
	{
		if(_serverInfo)
		{
			freeaddrinfo(_serverInfo);
			_serverInfo = nullptr;
		}
		struct addrinfo hostInfo;
		memset(&hostInfo, 0, sizeof(hostInfo));

		hostInfo.ai_family = AF_UNSPEC;
		hostInfo.ai_socktype = SOCK_DGRAM;

		if(getaddrinfo(_hostname.c_str(), _port.c_str(), &hostInfo, &_serverInfo) != 0)
		{
			freeaddrinfo(_serverInfo);
			_serverInfo = nullptr;
			throw SocketOperationException("Could not get address information: " + std::string(strerror(errno)));
		}

		char ipStringBuffer[INET6_ADDRSTRLEN];
		if (_serverInfo->ai_family == AF_INET) {
			struct sockaddr_in *s = (struct sockaddr_in*)_serverInfo->ai_addr;
			inet_ntop(AF_INET, &s->sin_addr, ipStringBuffer, sizeof(ipStringBuffer));
		} else { // AF_INET6
			struct sockaddr_in6 *s = (struct sockaddr_in6*)_serverInfo->ai_addr;
			inet_ntop(AF_INET6, &s->sin6_addr, ipStringBuffer, sizeof(ipStringBuffer));
		}
		_clientIp = std::string(&ipStringBuffer[0]);

		_socketDescriptor = _bl->fileDescriptorManager.add(socket(_serverInfo->ai_family, _serverInfo->ai_socktype, _serverInfo->ai_protocol));
		if(!_socketDescriptor || _socketDescriptor->descriptor == -1)
		{
			freeaddrinfo(_serverInfo);
			_serverInfo = nullptr;
			throw SocketOperationException("Could not create UDP socket for server " + _clientIp + " on port " + _port + ": " + strerror(errno));
		}

		if(!(fcntl(_socketDescriptor->descriptor, F_GETFL) & O_NONBLOCK))
		{
			if(fcntl(_socketDescriptor->descriptor, F_SETFL, fcntl(_socketDescriptor->descriptor, F_GETFL) | O_NONBLOCK) < 0)
			{
				freeaddrinfo(_serverInfo);
				_serverInfo = nullptr;
				_bl->fileDescriptorManager.shutdown(_socketDescriptor);
				throw SocketOperationException("Could not set socket options for server " + _clientIp + " on port " + _port + ": " + strerror(errno));
			}
		}

		if(_serverInfo->ai_family == AF_INET)
		{
			struct sockaddr_in clientInfo;
			memset(&clientInfo, 0, sizeof(clientInfo));
			clientInfo.sin_family = _serverInfo->ai_family;
			clientInfo.sin_addr.s_addr = INADDR_ANY;
			clientInfo.sin_port = htons(0);
			if(bind(_socketDescriptor->descriptor, (struct sockaddr*)&clientInfo, sizeof(clientInfo)) == -1)
			{
				freeaddrinfo(_serverInfo);
				_serverInfo = nullptr;
				_bl->fileDescriptorManager.shutdown(_socketDescriptor);
				throw SocketOperationException("Could not bind server: " + std::string(strerror(errno)));
			}

			uint32_t clientInfoSize = sizeof(clientInfo);
			if (getsockname(_socketDescriptor->descriptor, (struct sockaddr*)&clientInfo, &clientInfoSize) == -1)
			{
				freeaddrinfo(_serverInfo);
				_serverInfo = nullptr;
				_bl->fileDescriptorManager.shutdown(_socketDescriptor);
				throw SocketOperationException("Could not get listen ip and port: " + std::string(strerror(errno)));
			}

			struct sockaddr_in* s = (struct sockaddr_in*)&(clientInfo.sin_addr);
			inet_ntop(AF_INET, s, ipStringBuffer, sizeof(ipStringBuffer));
			_listenIp = std::string(ipStringBuffer);
			_listenPort = ntohs(clientInfo.sin_port);
		}
		else //AF_INET6
		{
			struct sockaddr_in6 clientInfo;
			memset(&clientInfo, 0, sizeof(clientInfo));
			clientInfo.sin6_family = _serverInfo->ai_family;
			clientInfo.sin6_addr = in6addr_any;
			clientInfo.sin6_port = htons(0);
			if(bind(_socketDescriptor->descriptor, (struct sockaddr*)&clientInfo, sizeof(clientInfo)) == -1)
			{
				freeaddrinfo(_serverInfo);
				_serverInfo = nullptr;
				_bl->fileDescriptorManager.shutdown(_socketDescriptor);
				throw SocketOperationException("Could not bind server: " + std::string(strerror(errno)));
			}

			uint32_t clientInfoSize = sizeof(clientInfo);
			if (getsockname(_socketDescriptor->descriptor, (struct sockaddr*)&clientInfo, &clientInfoSize) == -1)
			{
				freeaddrinfo(_serverInfo);
				_serverInfo = nullptr;
				_bl->fileDescriptorManager.shutdown(_socketDescriptor);
				throw SocketOperationException("Could not get listen ip and port: " + std::string(strerror(errno)));
			}

			struct sockaddr_in6* s = (struct sockaddr_in6*)&(clientInfo.sin6_addr);
			inet_ntop(AF_INET6, s, ipStringBuffer, sizeof(ipStringBuffer));
			_listenIp = std::string(ipStringBuffer);
			_listenPort = ntohs(clientInfo.sin6_port);
		}
	}
	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Opened UDP socket to host " + _hostname + " on port " + _port + ". Client number is: " + std::to_string(_socketDescriptor->id));
}

}
