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

#ifndef TCPSOCKETOPERATIONS_H_
#define TCPSOCKETOPERATIONS_H_

#include "SocketExceptions.h"
#include "../Managers/FileDescriptorManager.h"

#include <thread>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <sstream>
#include <mutex>
#include <memory>
#include <map>
#include <unordered_map>
#include <utility>
#include <cstring>
#include <atomic>
#include <functional>

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h> //Needed for BSD
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

#include <gnutls/x509.h>
#include <gnutls/gnutls.h>

namespace BaseLib
{

class SharedObjects;

/**
 * Class to easily create a TCP server or client.
 *
 * TCP Server Example Code
 * =======================
 *
 * Save the example below in `main.cpp` and compile with:
 *
 *     g++ -o main -std=c++11 main.cpp -lhomegear-base -lgcrypt -lgnutls
 *
 * Start and connect using e. g. telnet:
 *
 *     ./main
 *     telnet localhost 8082
 *
 * Example code:
 *
 *     #include <homegear-base/BaseLib.h>
 *
 *     std::shared_ptr<BaseLib::SharedObjects> _bl;
 *     std::shared_ptr<BaseLib::TcpSocket> _tcpServer;
 *
 *     void newConnection(int32_t clientId, std::string address, uint16_t port)
 *     {
 *     	std::cout << "New connection from " << address << " on port " << port << std::endl;
 *     }
 *
 *     void packetReceived(int32_t clientId, BaseLib::TcpSocket::TcpPacket packet)
 *     {
 *     	std::string data((char*)packet.data(), packet.size());
 *     	BaseLib::HelperFunctions::trim(data);
 *     	std::cout << "Packet received: " << data << std::endl;
 *     	std::vector<uint8_t> response;
 *     	response.push_back('R');
 *     	response.push_back(':');
 *     	response.push_back(' ');
 *     	response.insert(response.end(), packet.begin(), packet.end());
 *     	_tcpServer->sendToClient(clientId, response);
 *     }
 *
 *     int main()
 *     {
 *     	_bl.reset(new BaseLib::SharedObjects(false));
 *
 *     	BaseLib::TcpSocket::TcpServerInfo serverInfo;
 *     	serverInfo.newConnectionCallback = std::bind(&newConnection, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
 *     	serverInfo.packetReceivedCallback = std::bind(&packetReceived, std::placeholders::_1, std::placeholders::_2);
 *
 *     	_tcpServer = std::make_shared<BaseLib::TcpSocket>(_bl.get(), serverInfo);
 *
 *     	std::string listenAddress;
 *     	_tcpServer->startServer("::", "8082", listenAddress);
 *     	std::cout << "Started listening on " + listenAddress << std::endl;
 *
 *     	for(int32_t i = 0; i < 300; i++) //Run for 300 seconds
 *     	{
 *     		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
 *     	}
 *
 *     	_tcpServer->stopServer();
 *     	_tcpServer->waitForServerStopped();
 *     }
 *
 */
class TcpSocket
{
public:
	typedef std::vector<uint8_t> TcpPacket;

    struct CertificateInfo
    {
        std::string certFile;
        std::string certData;
        std::string keyFile;
        std::string keyData;
        std::string caFile; //For client certificate verification
        std::string caData; //For client certificate verification
    };
    typedef std::shared_ptr<CertificateInfo> PCertificateInfo;

	struct TcpServerInfo
	{
		bool useSsl = false;
		uint32_t maxConnections = 10;
		std::unordered_map<std::string, PCertificateInfo> certificates;
		std::string dhParamFile;
		std::string dhParamData;
		bool requireClientCert = false;
		std::function<void(int32_t clientId, std::string address, uint16_t port)> newConnectionCallback;
		std::function<void(int32_t clientId, TcpPacket& packet)> packetReceivedCallback;
	};

	// {{{ TCP server or client
		/**
		 * Constructor to create an empty socket object.
		 *
		 * @param baseLib The base library object.
		 */
		TcpSocket(BaseLib::SharedObjects* baseLib);

		/**
		 * Constructor to just wrap an existing socket descriptor.
		 *
		 * @param baseLib The base library object.
		 * @param socketDescriptor The socket descriptor to wrap.
		 */
		TcpSocket(BaseLib::SharedObjects* baseLib, std::shared_ptr<FileDescriptor> socketDescriptor);
	// }}}

	// {{{ TCP client
		/**
		 * Constructor to create a TCP client socket.
		 *
		 * @param baseLib The base library object.
		 * @param hostname The host name or IP address to connect to.
		 * @param port The port to connect to.
		 */
		TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port);

		/**
		 * Constructor to create a TCP client socket with SSL enabled.
		 *
		 * @param baseLib The base library object.
		 * @param hostname The host name or IP address to connect to.
		 * @param port The port to connect to.
		 * @param useSsl Set to `true` to enable SSL.
		 * @param caFile Path to a file containing the PEM-encoded CA certificate used to sign the server certificate.
		 * @param verifyCertificate Enable certificate verification. Only set to `false` for testing.
		 */
		TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port, bool useSsl, std::string caFile, bool verifyCertificate);

		/**
		 * Constructor to create a TCP client socket with SSL enabled.
		 *
		 * @param baseLib The base library object.
		 * @param hostname The host name or IP address to connect to.
		 * @param port The port to connect to.
		 * @param useSsl Set to `true` to enable SSL.
		 * @param verifyCertificate Enable certificate verification. Only set to `false` for testing.
		 * @param caData The PEM-encoded CA certificate (not the path) used to sign the server certificate.
		 */
		TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port, bool useSsl, bool verifyCertificate, std::string caData);

		/**
		 * Constructor to create a TCP client socket with SSL enabled using certificate login.
		 *
		 * @param baseLib The base library object.
		 * @param hostname The host name or IP address to connect to.
		 * @param port The port to connect to.
		 * @param useSsl Set to `true` to enable SSL.
		 * @param caFile Path to a file containing the PEM-encoded CA certificate used to sign the server certificate.
		 * @param verifyCertificate Enable certificate verification. Only set to `false` for testing.
		 * @param clientCertFile Path to a file containing the PEM-encoded client certificate.
		 * @param clientKeyFile Path to a file containing the PEM-encoded client key file.
		 */
		TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port, bool useSsl, std::string caFile, bool verifyCertificate, std::string clientCertFile, std::string clientKeyFile);

		/**
		 * Constructor to create a TCP client socket with SSL enabled using certificate login.
		 *
		 * @param baseLib The base library object.
		 * @param hostname The host name or IP address to connect to.
		 * @param port The port to connect to.
		 * @param useSsl Set to `true` to enable SSL.
		 * @param verifyCertificate Enable certificate verification. Only set to `false` for testing.
		 * @param caData The PEM-encoded CA certificate (not the path) used to sign the server certificate.
		 * @param clientCertData The PEM-encoded client certificate (not the path).
		 * @param clientKeyData The PEM-encoded client key (not the path).
		 */
		TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port, bool useSsl, bool verifyCertificate, std::string caData, std::string clientCertData, std::string clientKeyData);

        /**
         * Constructor to create a TCP client socket with SSL enabled using certificate login.
         *
         * @param baseLib The base library object.
         * @param hostname The host name or IP address to connect to.
         * @param port The port to connect to.
         * @param useSsl Set to `true` to enable SSL.
         * @param verifyCertificate Enable certificate verification. Only set to `false` for testing.
         * @param caFile Path to a file containing the PEM-encoded CA certificate used to sign the server certificate.
         * @param caData The PEM-encoded CA certificate (not the path) used to sign the server certificate.
         * @param clientCertFile Path to a file containing the PEM-encoded client certificate.
         * @param clientCertData The PEM-encoded client certificate (not the path).
         * @param clientKeyFile Path to a file containing the PEM-encoded client key file.
         * @param clientKeyData The PEM-encoded client key (not the path).
         */
        TcpSocket(BaseLib::SharedObjects* baseLib, std::string hostname, std::string port, bool useSsl, bool verifyCertificate, std::string caFile, std::string caData, std::string clientCertFile, std::string clientCertData, std::string clientKeyFile, std::string clientKeyData);
	// }}}

	// {{{ TCP server
		/**
		 * Constructor to create a TCP server.
		 */
		TcpSocket(BaseLib::SharedObjects* baseLib, TcpServerInfo& serverInfo);
	// }}}

	virtual ~TcpSocket();

	static PFileDescriptor bindAndReturnSocket(FileDescriptorManager& fileDescriptorManager, std::string address, std::string port, std::string& listenAddress, int32_t& listenPort);

	std::string getIpAddress();
	int32_t getPort() { return _boundListenPort; }
	void setConnectionRetries(int32_t retries) { _connectionRetries = retries; }
	void setReadTimeout(int64_t timeout) { _readTimeout = timeout; }
	void setWriteTimeout(int64_t timeout) { _writeTimeout = timeout; }
	void setAutoConnect(bool autoConnect) { _autoConnect = autoConnect; }
	void setHostname(std::string hostname) { close(); _hostname = hostname; }
	void setPort(std::string port) { close(); _port = port; }
	void setUseSSL(bool useSsl) { close(); _useSsl = useSsl; if(_useSsl) initSsl(); }
	void setCertificates(std::unordered_map<std::string, PCertificateInfo>& certificates) { close(); _certificates = certificates; }
	void setVerifyCertificate(bool verifyCertificate) { close(); _verifyCertificate = verifyCertificate; }
	std::unordered_map<std::string, gnutls_certificate_credentials_t>& getCredentials() { return _x509Credentials; }

	bool connected();

	/**
	 * Use this overload when there are no socket operations outside of this class.
	 *
	 * @param buffer The buffer to fill.
	 * @param bufferSize The size of the buffer.
	 * @returns The number of bytes read. The returned value is always greater than 0.
	 * @throws SocketOperationException Thrown when socket is nullptr.
	 * @throws SocketTimeOutException Thrown when reading times out.
	 * @throws SocketClosedException Thrown when socket is closed.
	 */
	int32_t proofread(char* buffer, int32_t bufferSize);

	/**
	 * Use this overload of proofread when select is called outside of this class.
	 *
	 * @param buffer The buffer to fill.
	 * @param bufferSize The size of the buffer.
	 * @param[out] moreData If true, call proofread immediately again (without calling e. g. select first).
	 * @returns The number of bytes read. The returned value is always greater than 0.
	 * @throws SocketOperationException Thrown when socket is nullptr.
	 * @throws SocketTimeOutException Thrown when reading times out.
	 * @throws SocketClosedException Thrown when socket is closed.
	 */
	int32_t proofread(char* buffer, int32_t bufferSize, bool& moreData);
	int32_t proofwrite(const std::shared_ptr<std::vector<char>> data);
	int32_t proofwrite(const std::vector<char>& data);
	int32_t proofwrite(const std::string& data);
	int32_t proofwrite(const char* buffer, int32_t bytesToWrite);
	void open();
	void close();

	// {{{ Servers only
		/**
		 * Starts listening.
		 *
		 * @param address The address to bind the server to (e. g. `::` or `0.0.0.0`).
		 * @param port The port number to bind the server to.
		 * @param[out] listenAddress The IP address the server was bound to (e. g. `192.168.0.152`).
		 */
		void startServer(std::string address, std::string port, std::string& listenAddress);

		/**
		 * Starts listening on a dynamically assigned port.
		 *
		 * @param address The address to bind the server to (e. g. `::` or `0.0.0.0`).
		 * @param[out] listenAddress The IP address the server was bound to (e. g. `192.168.0.152`).
		 * @param[out] listenPort The port the server was bound to (e. g. `45735`).
		 */
		void startServer(std::string address, std::string& listenAddress, int32_t& listenPort);

		/**
		 * Starts stopping the server and returns immediately.
		 */
		void stopServer();

		/**
		 * Waits until the server is stopped.
		 */
		void waitForServerStopped();

		/**
		 * Sends a response to a TCP client connected to the server.
		 *
		 * @param clientId The ID of the client as passed to TcpSocket::TcpServerServer::packetReceivedCallback.
		 * @param packet The data to send.
		 * @param closeConnection Close the connection after sending the packet.
		 */
		void sendToClient(int32_t clientId, TcpPacket packet, bool closeConnection = false);

        /**
         * Returns the number of clients connected to the TCP server
         * @return The number of connected clients.
         */
        int32_t clientCount();
	// }}}
protected:
	struct TcpClientData
	{
		int32_t id = 0;
		PFileDescriptor fileDescriptor;
		std::vector<uint8_t> buffer;
		std::shared_ptr<TcpSocket> socket;

		TcpClientData()
		{
			buffer.resize(1024);
		}
	};
	typedef std::shared_ptr<TcpClientData> PTcpClientData;

	BaseLib::SharedObjects* _bl = nullptr;
	int32_t _connectionRetries = 3;
	int64_t _readTimeout = 15000000;
	int64_t _writeTimeout = 15000000;
	bool _autoConnect = true;
	std::string _ipAddress;
	std::string _hostname;
	std::string _port;
	std::mutex _readMutex;
	std::mutex _writeMutex;
    std::unordered_map<std::string, PCertificateInfo> _certificates;
	bool _verifyCertificate = true;

	// {{{ For server only
		bool _isServer = false;
		uint32_t _maxConnections = 10;
		std::string _dhParamFile;
		std::string _dhParamData;
		bool _requireClientCert = false;
		std::function<void(int32_t clientId, std::string address, uint16_t port)> _newConnectionCallback;
		std::function<void(int32_t clientId, TcpPacket& packet)> _packetReceivedCallback;

		std::string _listenAddress;
		std::string _listenPort;
		int32_t _boundListenPort = -1;

		gnutls_dh_params_t _dhParams = nullptr;
		gnutls_priority_t _tlsPriorityCache = nullptr;

		std::atomic_bool _stopServer;
		std::thread _serverThread;

		int64_t _lastGarbageCollection = 0;

		int32_t _currentClientId = 0;
		std::mutex _clientsMutex;
		std::map<int32_t, PTcpClientData> _clients;
	// }}}

	PFileDescriptor _socketDescriptor;
	bool _useSsl = false;
	std::unordered_map<std::string, gnutls_certificate_credentials_t> _x509Credentials;

	void getSocketDescriptor();
	void getConnection();
	void getSsl();
	void initSsl();
	void autoConnect();
    void freeCredentials();

	// {{{ For server only
		void bindSocket();

		void serverThread();
		void collectGarbage();
		void initClientSsl(PFileDescriptor fileDescriptor);
		void readClient(PTcpClientData clientData);
	// }}}
};

typedef std::shared_ptr<BaseLib::TcpSocket> PTcpSocket;

}
#endif
