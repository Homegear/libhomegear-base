/* Copyright 2013-2017 Sathya Laufer
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
#include "TcpSocket.h"

namespace BaseLib
{
TcpSocket::TcpSocket(BaseLib::SharedObjects* baseLib)
{
	_bl = baseLib;
	_autoConnect = false;
	_socketDescriptor.reset(new FileDescriptor);
}

TcpSocket::TcpSocket(BaseLib::SharedObjects* baseLib, std::shared_ptr<FileDescriptor> socketDescriptor)
{
	_bl = baseLib;
	_autoConnect = false;
	if(socketDescriptor) _socketDescriptor = socketDescriptor;
	else _socketDescriptor.reset(new FileDescriptor);
}

TcpSocket::TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port)
{
	_bl = baseLib;
	_socketDescriptor.reset(new FileDescriptor);
	_hostname = hostname;
	_port = port;
}

TcpSocket::TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port, bool useSSL, std::string caFile, bool verifyCertificate) : TcpSocket(baseLib, hostname, port)
{
	_useSSL = useSSL;
	_caFile = caFile;
	_verifyCertificate = verifyCertificate;

	if(_useSSL) initSSL();
}

TcpSocket::TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port, bool useSSL, bool verifyCertificate, std::string caData) : TcpSocket(baseLib, hostname, port)
{
	_useSSL = useSSL;
	_verifyCertificate = verifyCertificate;
	_caData = caData;

	if(_useSSL) initSSL();
}

TcpSocket::TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port, bool useSSL, std::string caFile, bool verifyCertificate, std::string clientCertFile, std::string clientKeyFile) : TcpSocket(baseLib, hostname, port)
{
	_useSSL = useSSL;
	_verifyCertificate = verifyCertificate;
	_caFile = caFile;
	_clientCertFile = clientCertFile;
	_clientKeyFile = clientKeyFile;

	if(_useSSL) initSSL();
}

TcpSocket::TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port, bool useSSL, bool verifyCertificate, std::string caData, std::string clientCertData, std::string clientKeyData) : TcpSocket(baseLib, hostname, port)
{
	_useSSL = useSSL;
	_verifyCertificate = verifyCertificate;
	_caData = caData;
	_clientCertData = clientCertData;
	_clientKeyData = clientKeyData;

	if(_useSSL) initSSL();
}

TcpSocket::~TcpSocket()
{
	_bl->fileDescriptorManager.close(_socketDescriptor);
	if(_x509Cred) gnutls_certificate_free_credentials(_x509Cred);
}

std::string TcpSocket::getIpAddress()
{
	if(!_ipAddress.empty()) return _ipAddress;
	_ipAddress = Net::resolveHostname(_hostname);
	return _ipAddress;
}

PFileDescriptor TcpSocket::bindSocket(std::string address, std::string port, std::string& listenAddress)
{
	PFileDescriptor socketDescriptor;
	addrinfo hostInfo;
	addrinfo *serverInfo = nullptr;

	int32_t yes = 1;

	memset(&hostInfo, 0, sizeof(hostInfo));

	hostInfo.ai_family = AF_UNSPEC;
	hostInfo.ai_socktype = SOCK_STREAM;
	hostInfo.ai_flags = AI_PASSIVE;
	char buffer[100];
	int32_t result;
	if((result = getaddrinfo(address.c_str(), port.c_str(), &hostInfo, &serverInfo)) != 0)
	{
		_bl->out.printError("Error: Could not get address information: " + std::string(gai_strerror(result)));
		return socketDescriptor;
	}

	bool bound = false;
	int32_t error = 0;
	for(struct addrinfo *info = serverInfo; info != 0; info = info->ai_next)
	{
		socketDescriptor = _bl->fileDescriptorManager.add(socket(info->ai_family, info->ai_socktype, info->ai_protocol));
		if(socketDescriptor->descriptor == -1) continue;
		if(!(fcntl(socketDescriptor->descriptor, F_GETFL) & O_NONBLOCK))
		{
			if(fcntl(socketDescriptor->descriptor, F_SETFL, fcntl(socketDescriptor->descriptor, F_GETFL) | O_NONBLOCK) < 0) throw BaseLib::Exception("Error: Could not set socket options.");
		}
		if(setsockopt(socketDescriptor->descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t)) == -1) throw BaseLib::Exception("Error: Could not set socket options.");
		if(bind(socketDescriptor->descriptor, info->ai_addr, info->ai_addrlen) == -1)
		{
			socketDescriptor.reset();
			error = errno;
			continue;
		}
		switch (info->ai_family)
		{
			case AF_INET:
				inet_ntop (info->ai_family, &((struct sockaddr_in *) info->ai_addr)->sin_addr, buffer, 100);
				listenAddress = std::string(buffer);
				break;
			case AF_INET6:
				inet_ntop (info->ai_family, &((struct sockaddr_in6 *) info->ai_addr)->sin6_addr, buffer, 100);
				listenAddress = std::string(buffer);
				break;
		}
		bound = true;
		break;
	}
	freeaddrinfo(serverInfo);
	if(!bound)
	{
		_bl->out.printError("Error: Could not start listening on port " + port + ": " + std::string(strerror(error)));
		_bl->fileDescriptorManager.shutdown(socketDescriptor);
		socketDescriptor.reset();
	}
	else if(socketDescriptor->descriptor == -1 || listen(socketDescriptor->descriptor, 100) == -1)
	{
		_bl->out.printError("Error: Server could not start listening on port " + port + ": " + std::string(strerror(errno)));
		_bl->fileDescriptorManager.shutdown(socketDescriptor);
		socketDescriptor.reset();
	}
	if(listenAddress == "0.0.0.0" || listenAddress == "::") listenAddress = Net::getMyIpAddress();
	return socketDescriptor;
}

void TcpSocket::initSSL()
{
	if(_x509Cred) gnutls_certificate_free_credentials(_x509Cred);

	int32_t result = 0;
	if((result = gnutls_certificate_allocate_credentials(&_x509Cred)) != GNUTLS_E_SUCCESS)
	{
		_x509Cred = nullptr;
		throw SocketSSLException("Could not allocate certificate credentials: " + std::string(gnutls_strerror(result)));
	}
	if(!_caData.empty())
	{
		gnutls_datum_t caData;
		caData.data = (unsigned char*)_caData.c_str();
		caData.size = _caData.size();
		if((result = gnutls_certificate_set_x509_trust_mem(_x509Cred, &caData, GNUTLS_X509_FMT_PEM)) < 0)
		{
			gnutls_certificate_free_credentials(_x509Cred);
			_x509Cred = nullptr;
			throw SocketSSLException("Could not load trusted certificates from \"" + _caFile + "\": " + std::string(gnutls_strerror(result)));
		}
	}
	else if(!_caFile.empty())
	{
		if((result = gnutls_certificate_set_x509_trust_file(_x509Cred, _caFile.c_str(), GNUTLS_X509_FMT_PEM)) < 0)
		{
			gnutls_certificate_free_credentials(_x509Cred);
			_x509Cred = nullptr;
			throw SocketSSLException("Could not load trusted certificates from \"" + _caFile + "\": " + std::string(gnutls_strerror(result)));
		}
	}
	else
	{
		if(_verifyCertificate) throw SocketSSLException("Certificate verification is enabled, but \"caFile\" and \"caData\" are not specified for the host \"" + _hostname + "\".");
		else _bl->out.printWarning("Warning: \"caFile\" is not specified for the host \"" + _hostname + "\" and certificate verification is disabled. It is highly recommended to enable certificate verification.");
	}
	if(result == 0 && _verifyCertificate)
	{
		gnutls_certificate_free_credentials(_x509Cred);
		_x509Cred = nullptr;
		throw SocketSSLException("No certificates found in \"" + _caFile + "\".");
	}
	if(!_clientCertData.empty() && !_clientKeyData.empty())
	{
		gnutls_datum_t clientCertData;
		clientCertData.data = (unsigned char*)_clientCertData.c_str();
		clientCertData.size = _clientCertData.size();

		gnutls_datum_t clientKeyData;
		clientKeyData.data = (unsigned char*)_clientKeyData.c_str();
		clientKeyData.size = _clientKeyData.size();

		if((result = gnutls_certificate_set_x509_key_mem(_x509Cred, &clientCertData, &clientKeyData, GNUTLS_X509_FMT_PEM)) < 0)
		{
			gnutls_certificate_free_credentials(_x509Cred);
			_x509Cred = nullptr;
			throw SocketSSLException("Could not load client certificate and key from \"" + _clientCertFile + "\" and \"" + _clientKeyFile + "\": " + std::string(gnutls_strerror(result)));
		}
	}
	else if(!_clientCertFile.empty() && !_clientKeyFile.empty())
	{
		if((result = gnutls_certificate_set_x509_key_file(_x509Cred, _clientCertFile.c_str(), _clientKeyFile.c_str(), GNUTLS_X509_FMT_PEM)) < 0)
		{
			gnutls_certificate_free_credentials(_x509Cred);
			_x509Cred = nullptr;
			throw SocketSSLException("Could not load client certificate and key from \"" + _clientCertFile + "\" and \"" + _clientKeyFile + "\": " + std::string(gnutls_strerror(result)));
		}
	}
}

void TcpSocket::open()
{
	if(!_socketDescriptor || _socketDescriptor->descriptor < 0) getSocketDescriptor();
	else if(!connected())
	{
		close();
		getSocketDescriptor();
	}
}

void TcpSocket::autoConnect()
{
	if(!_autoConnect) return;
	if(!_socketDescriptor || _socketDescriptor->descriptor < 0) getSocketDescriptor();
	else if(!connected())
	{
		close();
		getSocketDescriptor();
	}
}

void TcpSocket::close()
{
	_readMutex.lock();
	_writeMutex.lock();
	_bl->fileDescriptorManager.close(_socketDescriptor);
	_writeMutex.unlock();
	_readMutex.unlock();
}

int32_t TcpSocket::proofread(char* buffer, int32_t bufferSize)
{
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	_readMutex.lock();
	if(_autoConnect && !connected())
	{
		_readMutex.unlock();
		autoConnect();
		_readMutex.lock();
	}

	int32_t bytesRead = 0;
	if(_socketDescriptor->tlsSession && gnutls_record_check_pending(_socketDescriptor->tlsSession) > 0)
	{
		do
		{
			bytesRead = gnutls_record_recv(_socketDescriptor->tlsSession, buffer, bufferSize);
		} while(bytesRead == GNUTLS_E_INTERRUPTED || bytesRead == GNUTLS_E_AGAIN);
		if(bytesRead > 0)
		{
			_readMutex.unlock();
			return bytesRead;
		}
	}

	timeval timeout;
	int32_t seconds = _readTimeout / 1000000;
	timeout.tv_sec = seconds;
	timeout.tv_usec = _readTimeout - (1000000 * seconds);
	fd_set readFileDescriptor;
	FD_ZERO(&readFileDescriptor);
	auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
	fileDescriptorGuard.lock();
	int32_t nfds = _socketDescriptor->descriptor + 1;
	if(nfds <= 0)
	{
		fileDescriptorGuard.unlock();
		_readMutex.unlock();
		throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (1).");
	}
	FD_SET(_socketDescriptor->descriptor, &readFileDescriptor);
	fileDescriptorGuard.unlock();
	bytesRead = select(nfds, &readFileDescriptor, NULL, NULL, &timeout);
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
	if(_socketDescriptor->tlsSession)
	{
		do
		{
			bytesRead = gnutls_record_recv(_socketDescriptor->tlsSession, buffer, bufferSize);
		} while(bytesRead == GNUTLS_E_INTERRUPTED || bytesRead == GNUTLS_E_AGAIN);
	}
	else
	{
		do
		{
			bytesRead = read(_socketDescriptor->descriptor, buffer, bufferSize);
		} while(bytesRead < 0 && (errno == EAGAIN || errno == EINTR));
	}
	if(bytesRead <= 0)
	{
		_readMutex.unlock();
		if(bytesRead == -1) throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (3): " + strerror(errno));
		else throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (3).");
	}
	_readMutex.unlock();
	return bytesRead;
}

int32_t TcpSocket::proofwrite(const std::shared_ptr<std::vector<char>> data)
{
	_writeMutex.lock();
	if(!connected())
	{
		_writeMutex.unlock();
		autoConnect();
	}
	else _writeMutex.unlock();
	if(!data || data->empty()) return 0;
	return proofwrite(*data);
}

int32_t TcpSocket::proofwrite(const std::vector<char>& data)
{
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	_writeMutex.lock();
	if(!connected())
	{
		_writeMutex.unlock();
		autoConnect();
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
		timeval timeout;
		int32_t seconds = _writeTimeout / 1000000;
		timeout.tv_sec = seconds;
		timeout.tv_usec = _writeTimeout - (1000000 * seconds);
		fd_set writeFileDescriptor;
		FD_ZERO(&writeFileDescriptor);
		auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
		fileDescriptorGuard.lock();
		int32_t nfds = _socketDescriptor->descriptor + 1;
		if(nfds <= 0)
		{
			fileDescriptorGuard.unlock();
			_writeMutex.unlock();
			throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (4).");
		}
		FD_SET(_socketDescriptor->descriptor, &writeFileDescriptor);
		fileDescriptorGuard.unlock();
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
		}

		int32_t bytesWritten = _socketDescriptor->tlsSession ? gnutls_record_send(_socketDescriptor->tlsSession, &data.at(totalBytesWritten), data.size() - totalBytesWritten) : send(_socketDescriptor->descriptor, &data.at(totalBytesWritten), data.size() - totalBytesWritten, MSG_NOSIGNAL);
		if(bytesWritten <= 0)
		{
			if(bytesWritten == -1 && (errno == EINTR || errno == EAGAIN)) continue;
			_writeMutex.unlock();
			close();
			_writeMutex.lock();
			if(_socketDescriptor->tlsSession)
			{
				_writeMutex.unlock();
				throw SocketOperationException(gnutls_strerror(bytesWritten));
			}
			else
			{
				_writeMutex.unlock();
				throw SocketOperationException(strerror(errno));
			}
		}
		totalBytesWritten += bytesWritten;
	}
	_writeMutex.unlock();
	return totalBytesWritten;
}

int32_t TcpSocket::proofwrite(const char* buffer, int32_t bytesToWrite)
{
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	_writeMutex.lock();
	if(!connected())
	{
		_writeMutex.unlock();
		autoConnect();
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
		timeval timeout;
		int32_t seconds = _writeTimeout / 1000000;
		timeout.tv_sec = seconds;
		timeout.tv_usec = _writeTimeout - (1000000 * seconds);
		fd_set writeFileDescriptor;
		FD_ZERO(&writeFileDescriptor);
		auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
		fileDescriptorGuard.lock();
		int32_t nfds = _socketDescriptor->descriptor + 1;
		if(nfds <= 0)
		{
			fileDescriptorGuard.unlock();
			_writeMutex.unlock();
			throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (4).");
		}
		FD_SET(_socketDescriptor->descriptor, &writeFileDescriptor);
		fileDescriptorGuard.unlock();
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
		}

		int32_t bytesWritten = _socketDescriptor->tlsSession ? gnutls_record_send(_socketDescriptor->tlsSession, buffer + totalBytesWritten, bytesToWrite - totalBytesWritten) : send(_socketDescriptor->descriptor, buffer + totalBytesWritten, bytesToWrite - totalBytesWritten, MSG_NOSIGNAL);
		if(bytesWritten <= 0)
		{
			if(bytesWritten == -1 && (errno == EINTR || errno == EAGAIN)) continue;
			_writeMutex.unlock();
			close();
			_writeMutex.lock();
			if(_socketDescriptor->tlsSession)
			{
				_writeMutex.unlock();
				throw SocketOperationException(gnutls_strerror(bytesWritten));
			}
			else
			{
				_writeMutex.unlock();
				throw SocketOperationException(strerror(errno));
			}
		}
		totalBytesWritten += bytesWritten;
	}
	_writeMutex.unlock();
	return totalBytesWritten;
}

int32_t TcpSocket::proofwrite(const std::string& data)
{
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	_writeMutex.lock();
	if(!connected())
	{
		_writeMutex.unlock();
		autoConnect();
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
		timeval timeout;
		int32_t seconds = _writeTimeout / 1000000;
		timeout.tv_sec = seconds;
		timeout.tv_usec = _writeTimeout - (1000000 * seconds);
		fd_set writeFileDescriptor;
		FD_ZERO(&writeFileDescriptor);
		auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
		fileDescriptorGuard.lock();
		int32_t nfds = _socketDescriptor->descriptor + 1;
		if(nfds <= 0)
		{
			fileDescriptorGuard.unlock();
			_writeMutex.unlock();
			throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (6).");
		}
		FD_SET(_socketDescriptor->descriptor, &writeFileDescriptor);
		fileDescriptorGuard.unlock();
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
		}

		int32_t bytesToSend = data.size() - totalBytesWritten;
		int32_t bytesWritten = _socketDescriptor->tlsSession ? gnutls_record_send(_socketDescriptor->tlsSession, &data.at(totalBytesWritten), bytesToSend) : send(_socketDescriptor->descriptor, &data.at(totalBytesWritten), bytesToSend, MSG_NOSIGNAL);
		if(bytesWritten <= 0)
		{
			if(bytesWritten == -1 && (errno == EINTR || errno == EAGAIN)) continue;
			_writeMutex.unlock();
			close();
			_writeMutex.lock();
			if(_socketDescriptor->tlsSession)
			{
				_writeMutex.unlock();
				throw SocketOperationException(gnutls_strerror(bytesWritten));
			}
			else
			{
				_writeMutex.unlock();
				throw SocketOperationException(strerror(errno));
			}
		}
		totalBytesWritten += bytesWritten;
	}
	_writeMutex.unlock();
	return totalBytesWritten;
}

bool TcpSocket::connected()
{
	if(!_socketDescriptor || _socketDescriptor->descriptor < 0) return false;
	char buffer[1];
	if(recv(_socketDescriptor->descriptor, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT) == -1 && errno != EWOULDBLOCK && errno != EAGAIN && errno != EINTR) return false;
	return true;
}

void TcpSocket::getSocketDescriptor()
{
	_readMutex.lock();
	_writeMutex.lock();
	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Calling getFileDescriptor...");
	_bl->fileDescriptorManager.shutdown(_socketDescriptor);

	try
	{
		getConnection();
		if(!_socketDescriptor || _socketDescriptor->descriptor < 0)
		{
			_readMutex.unlock();
			_writeMutex.unlock();
			throw SocketOperationException("Could not connect to server.");
		}

		if(_useSSL) getSSL();
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

void TcpSocket::getSSL()
{
	if(!_socketDescriptor || _socketDescriptor->descriptor < 0) throw SocketSSLException("Could not connect to server using SSL. File descriptor is invalid.");
	if(!_x509Cred)
	{
		_bl->fileDescriptorManager.shutdown(_socketDescriptor);
		throw SocketSSLException("Could not connect to server using SSL. Certificate credentials are not initialized. Look for previous error messages.");
	}
	int32_t result = 0;
	//Disable TCP Nagle algorithm to improve performance
	const int32_t value = 1;
	if ((result = setsockopt(_socketDescriptor->descriptor, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value))) < 0) {
		_bl->fileDescriptorManager.shutdown(_socketDescriptor);
		throw SocketSSLException("Could not disable Nagle algorithm.");
	}
	if((result = gnutls_init(&_socketDescriptor->tlsSession, GNUTLS_CLIENT)) != GNUTLS_E_SUCCESS)
	{
		_bl->fileDescriptorManager.shutdown(_socketDescriptor);
		throw SocketSSLException("Could not initialize TLS session: " + std::string(gnutls_strerror(result)));
	}
	if(!_socketDescriptor->tlsSession) throw SocketSSLException("Could not initialize TLS session.");
	if((result = gnutls_priority_set_direct(_socketDescriptor->tlsSession, "NORMAL", NULL)) != GNUTLS_E_SUCCESS)
	{
		_bl->fileDescriptorManager.shutdown(_socketDescriptor);
		throw SocketSSLException("Could not set cipher priorities: " + std::string(gnutls_strerror(result)));
	}
	if((result = gnutls_credentials_set(_socketDescriptor->tlsSession, GNUTLS_CRD_CERTIFICATE, _x509Cred)) != GNUTLS_E_SUCCESS)
	{
		_bl->fileDescriptorManager.shutdown(_socketDescriptor);
		throw SocketSSLException("Could not set trusted certificates: " + std::string(gnutls_strerror(result)));
	}
	gnutls_transport_set_ptr(_socketDescriptor->tlsSession, (gnutls_transport_ptr_t)(uintptr_t)_socketDescriptor->descriptor);
	if((result = gnutls_server_name_set(_socketDescriptor->tlsSession, GNUTLS_NAME_DNS, _hostname.c_str(), _hostname.length())) != GNUTLS_E_SUCCESS)
	{
		_bl->fileDescriptorManager.shutdown(_socketDescriptor);
		throw SocketSSLException("Could not set server's hostname: " + std::string(gnutls_strerror(result)));
	}
	do
	{
		result = gnutls_handshake(_socketDescriptor->tlsSession);
	} while (result < 0 && gnutls_error_is_fatal(result) == 0);
	if(result != GNUTLS_E_SUCCESS)
	{
		_bl->fileDescriptorManager.shutdown(_socketDescriptor);
		throw SocketSSLException("Error during TLS handshake: " + std::string(gnutls_strerror(result)));
	}

	//Now verify the certificate
	uint32_t serverCertChainLength = 0;
	const gnutls_datum_t* const serverCertChain = gnutls_certificate_get_peers(_socketDescriptor->tlsSession, &serverCertChainLength);
	if(!serverCertChain || serverCertChainLength == 0)
	{
		_bl->fileDescriptorManager.shutdown(_socketDescriptor);
		throw SocketSSLException("Could not get server certificate.");
	}
	uint32_t status = (uint32_t)-1;
	if((result = gnutls_certificate_verify_peers2(_socketDescriptor->tlsSession, &status)) != GNUTLS_E_SUCCESS)
	{
		_bl->fileDescriptorManager.shutdown(_socketDescriptor);
		throw SocketSSLException("Could not verify server certificate: " + std::string(gnutls_strerror(result)));
	}
	if(status > 0)
	{
		if(_verifyCertificate)
		{
			_bl->fileDescriptorManager.shutdown(_socketDescriptor);
			throw SocketSSLException("Error verifying server certificate (Code: " + std::to_string(status) + "): " + HelperFunctions::getGNUTLSCertVerificationError(status));
		}
		else if((status & GNUTLS_CERT_REVOKED) || (status & GNUTLS_CERT_INSECURE_ALGORITHM) || (status & GNUTLS_CERT_NOT_ACTIVATED) || (status & GNUTLS_CERT_EXPIRED))
		{
			_bl->fileDescriptorManager.shutdown(_socketDescriptor);
			throw SocketSSLException("Error verifying server certificate (Code: " + std::to_string(status) + "): " + HelperFunctions::getGNUTLSCertVerificationError(status));
		}
		else _bl->out.printWarning("Warning: Certificate verification failed (Code: " + std::to_string(status) + "): " + HelperFunctions::getGNUTLSCertVerificationError(status));
	}
	else //Verify hostname
	{
		gnutls_x509_crt_t serverCert;
		if((result = gnutls_x509_crt_init(&serverCert)) != GNUTLS_E_SUCCESS)
		{
			_bl->fileDescriptorManager.shutdown(_socketDescriptor);
			throw SocketSSLException("Could not initialize server certificate structure: " + std::string(gnutls_strerror(result)));
		}
		//The peer certificate is the first certificate in the list
		if((result = gnutls_x509_crt_import(serverCert, serverCertChain, GNUTLS_X509_FMT_DER)) != GNUTLS_E_SUCCESS)
		{
			gnutls_x509_crt_deinit(serverCert);
			_bl->fileDescriptorManager.shutdown(_socketDescriptor);
			throw SocketSSLException("Could not import server certificate: " + std::string(gnutls_strerror(result)));
		}
		if((result = gnutls_x509_crt_check_hostname(serverCert, _hostname.c_str())) == 0)
		{
			gnutls_x509_crt_deinit(serverCert);
			_bl->fileDescriptorManager.shutdown(_socketDescriptor);
			throw SocketSSLException("Server's hostname does not match the server certificate.");
		}
		gnutls_x509_crt_deinit(serverCert);
	}
	_bl->out.printInfo("Info: SSL handshake with client " + std::to_string(_socketDescriptor->id) + " completed successfully.");
}

/*bool TcpSocket::waitForSocket()
{
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	fd_set readFileDescriptor;
	FD_ZERO(&readFileDescriptor);
	_bl->fileDescriptorManager.lock();
	int32_t nfds = _socketDescriptor->descriptor + 1;
	if(nfds <= 0)
	{
		_bl->fileDescriptorManager.unlock();
		throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (8).");
	}
	FD_SET(_socketDescriptor->descriptor, &readFileDescriptor);
	_bl->fileDescriptorManager.unlock();
	int32_t bytesRead = select(nfds, &readFileDescriptor, NULL, NULL, &timeout);
	if(bytesRead != 1) return false;
	return true;
}*/

void TcpSocket::getConnection()
{
	_socketDescriptor.reset();
	if(_hostname.empty()) throw SocketInvalidParametersException("Hostname is empty");
	if(_port.empty()) throw SocketInvalidParametersException("Port is empty");
	if(_connectionRetries < 1) _connectionRetries = 1;
	else if(_connectionRetries > 10) _connectionRetries = 10;

	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Connecting to host " + _hostname + " on port " + _port + (_useSSL ? " using SSL" : "") + "...");

	//Retry for two minutes
	for(int32_t i = 0; i < _connectionRetries; ++i)
	{
		struct addrinfo *serverInfo = nullptr;
		struct addrinfo hostInfo;
		memset(&hostInfo, 0, sizeof hostInfo);

		hostInfo.ai_family = AF_UNSPEC;
		hostInfo.ai_socktype = SOCK_STREAM;

		if(getaddrinfo(_hostname.c_str(), _port.c_str(), &hostInfo, &serverInfo) != 0)
		{
			freeaddrinfo(serverInfo);
			throw SocketOperationException("Could not get address information: " + std::string(strerror(errno)));
		}

		char ipStringBuffer[INET6_ADDRSTRLEN];
		if (serverInfo->ai_family == AF_INET) {
			struct sockaddr_in *s = (struct sockaddr_in *)serverInfo->ai_addr;
			inet_ntop(AF_INET, &s->sin_addr, ipStringBuffer, sizeof(ipStringBuffer));
		} else { // AF_INET6
			struct sockaddr_in6 *s = (struct sockaddr_in6 *)serverInfo->ai_addr;
			inet_ntop(AF_INET6, &s->sin6_addr, ipStringBuffer, sizeof(ipStringBuffer));
		}
		_ipAddress = std::string(&ipStringBuffer[0]);

		_socketDescriptor = _bl->fileDescriptorManager.add(socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol));
		if(!_socketDescriptor || _socketDescriptor->descriptor == -1)
		{
			freeaddrinfo(serverInfo);
			throw SocketOperationException("Could not create socket for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
		}
		int32_t optValue = 1;
		if(setsockopt(_socketDescriptor->descriptor, SOL_SOCKET, SO_KEEPALIVE, (void*)&optValue, sizeof(int32_t)) == -1)
		{
			freeaddrinfo(serverInfo);
			_bl->fileDescriptorManager.shutdown(_socketDescriptor);
			throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
		}
		optValue = 30;
		//Don't use SOL_TCP, as this constant doesn't exists in BSD
		if(setsockopt(_socketDescriptor->descriptor, getprotobyname("TCP")->p_proto, TCP_KEEPIDLE, (void*)&optValue, sizeof(int32_t)) == -1)
		{
			freeaddrinfo(serverInfo);
			_bl->fileDescriptorManager.shutdown(_socketDescriptor);
			throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
		}
		optValue = 4;
		if(setsockopt(_socketDescriptor->descriptor, getprotobyname("TCP")->p_proto, TCP_KEEPCNT, (void*)&optValue, sizeof(int32_t)) == -1)
		{
			freeaddrinfo(serverInfo);
			_bl->fileDescriptorManager.shutdown(_socketDescriptor);
			throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
		}
		optValue = 15;
		if(setsockopt(_socketDescriptor->descriptor, getprotobyname("TCP")->p_proto, TCP_KEEPINTVL, (void*)&optValue, sizeof(int32_t)) == -1)
		{
			freeaddrinfo(serverInfo);
			_bl->fileDescriptorManager.shutdown(_socketDescriptor);
			throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
		}

		if(!(fcntl(_socketDescriptor->descriptor, F_GETFL) & O_NONBLOCK))
		{
			if(fcntl(_socketDescriptor->descriptor, F_SETFL, fcntl(_socketDescriptor->descriptor, F_GETFL) | O_NONBLOCK) < 0)
			{
				freeaddrinfo(serverInfo);
				_bl->fileDescriptorManager.shutdown(_socketDescriptor);
				throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
			}
		}

		int32_t connectResult;
		if((connectResult = connect(_socketDescriptor->descriptor, serverInfo->ai_addr, serverInfo->ai_addrlen)) == -1 && errno != EINPROGRESS)
		{
			if(i < _connectionRetries - 1)
			{
				freeaddrinfo(serverInfo);
				_bl->fileDescriptorManager.shutdown(_socketDescriptor);
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				continue;
			}
			else
			{
				freeaddrinfo(serverInfo);
				_bl->fileDescriptorManager.shutdown(_socketDescriptor);
				throw SocketTimeOutException("Connecting to server " + _ipAddress + " on port " + _port + " timed out: " + strerror(errno));
			}
		}
		freeaddrinfo(serverInfo);

		if(connectResult != 0) //We have to wait for the connection
		{
			pollfd pollstruct
			{
				(int)_socketDescriptor->descriptor,
				(short)(POLLIN | POLLOUT | POLLERR),
				(short)0
			};

			int32_t pollResult = poll(&pollstruct, 1, _readTimeout / 1000);
			if(pollResult < 0 || (pollstruct.revents & POLLERR))
			{
				if(i < _connectionRetries - 1)
				{
					_bl->fileDescriptorManager.shutdown(_socketDescriptor);
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					continue;
				}
				else
				{
					_bl->fileDescriptorManager.shutdown(_socketDescriptor);
					throw SocketTimeOutException("Could not connect to server " + _ipAddress + " on port " + _port + ". Poll failed with error code: " + std::to_string(pollResult) + ".");
				}
			}
			else if(pollResult > 0)
			{
				socklen_t resultLength = sizeof(connectResult);
				if(getsockopt(_socketDescriptor->descriptor, SOL_SOCKET, SO_ERROR, &connectResult, &resultLength) < 0)
				{
					_bl->fileDescriptorManager.shutdown(_socketDescriptor);
					throw SocketOperationException("Could not connect to server " + _ipAddress + " on port " + _port + ": " + strerror(errno) + ".");
				}
				break;
			}
			else if(pollResult == 0)
			{
				if(i < _connectionRetries - 1)
				{
					_bl->fileDescriptorManager.shutdown(_socketDescriptor);
					continue;
				}
				else
				{
					_bl->fileDescriptorManager.shutdown(_socketDescriptor);
					throw SocketTimeOutException("Connecting to server " + _ipAddress + " on port " + _port + " timed out.");
				}
			}
		}
	}
	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Connected to host " + _hostname + " on port " + _port + ". Client number is: " + std::to_string(_socketDescriptor->id));
}
}
