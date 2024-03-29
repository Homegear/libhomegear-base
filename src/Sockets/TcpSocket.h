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

#ifndef TCPSOCKETOPERATIONS_H_
#define TCPSOCKETOPERATIONS_H_

#include "SocketExceptions.h"
#include "../Managers/FileDescriptorManager.h"
#include "../IQueue.h"

#include <thread>
#include <string>
#include <vector>
#include <list>
#include <iterator>
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
#include <queue>

namespace BaseLib {

namespace Security {
template<typename T>
class SecureVector;
}

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
 *     	std::cout << "New connection from " << address << " on port " << port << ". Client ID: " << clientId << std::endl;
 *     }
 *
 *     void packetReceived(int32_t clientId, BaseLib::TcpSocket::TcpPacket& packet)
 *     {
 *     	std::string data((char*)packet.data(), packet.size());
 *     	BaseLib::HelperFunctions::trim(data);
 *     	std::cout << "Packet received from client " << clientId << ": " << data << std::endl;
 *     	std::vector<uint8_t> response;
 *     	response.push_back('R');
 *     	response.push_back(':');
 *     	response.push_back(' ');
 *     	response.insert(response.end(), packet.begin(), packet.end());
 *     	_tcpServer->sendToClient(clientId, response);
 *     }
 *
 *     void connectionClosed(int32_t clientId)
 *     {
 *      std::cout << "Connection to client " << clientId << " closed." << std::endl;
 *     }
 *
 *     int main()
 *     {
 *     	_bl.reset(new BaseLib::SharedObjects(false));
 *
 *     	BaseLib::TcpSocket::TcpServerInfo serverInfo;
 *     	serverInfo.newConnectionCallback = std::bind(&newConnection, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
 *     	//For class methods add `this` as first parameter:
 *     	//serverInfo.newConnectionCallback = std::bind(&MyClass::newConnection, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
 *     	serverInfo.packetReceivedCallback = std::bind(&packetReceived, std::placeholders::_1, std::placeholders::_2);
 *     	serverInfo.connectionClosedCallback = std::bind(&connectionClosed, std::placeholders::_1);
 *
 *     	_tcpServer = std::make_shared<BaseLib::TcpSocket>(_bl.get(), serverInfo);
 *
 *     	std::string listenAddress;
 *     	_tcpServer->startServer("::", "8082", listenAddress);
 *     	std::cout << "Started listening on " + listenAddress << std::endl;
 *
 *     	//Alternatively you can call `bindServerSocket()` as root first, then drop privileges and start the server:
 *     	//_tcpServer->bindServerSocket("::", "8082", listenAddress);
 *     	//...
 *     	//Drop privileges
 *     	//...
 *     	//_tcpServer->startPreboundServer(listenAddress);
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
class TcpSocket : public IQueue {
 public:
  class CertificateCredentials {
   private:
    gnutls_certificate_credentials_t _credentials = nullptr;
    //"Note that only a pointer to the parameters are stored in the certificate handle, so you must not deallocate the parameters before the certificate is deallocated."
    gnutls_dh_params_t _dhParams = nullptr;
   public:
    CertificateCredentials(gnutls_certificate_credentials_t credentials, gnutls_datum_t dhParams);
    ~CertificateCredentials();
    gnutls_certificate_credentials_t get();
  };

  typedef std::vector<uint8_t> TcpPacket;

  struct TcpClientData {
    int32_t id = 0;
    PFileDescriptor fileDescriptor;
    std::vector<uint8_t> buffer;
    std::shared_ptr<TcpSocket> socket;
    std::unordered_map<std::string, std::shared_ptr<CertificateCredentials>> certificateCredentials;
    std::string clientCertDn;
    std::string clientCertSerial;
    int64_t clientCertExpiration = -1;
    /**
     * Mutex for `busy` and `backlog`
     */
    std::mutex backlogMutex;
    bool busy = false;
    std::queue<std::shared_ptr<TcpPacket>> backlog;

    TcpClientData() {
      buffer.resize(4096);
    }
  };
  typedef std::shared_ptr<TcpClientData> PTcpClientData;

  class QueueEntry : public BaseLib::IQueueEntry {
   public:
    QueueEntry() = default;

    explicit QueueEntry(const PTcpClientData &clientData) {
      this->clientData = clientData;
    }

    ~QueueEntry() override = default;

    PTcpClientData clientData;
  };

  struct CertificateInfo {
    std::string certFile;
    std::string certData;
    std::string keyFile;
    std::shared_ptr<Security::SecureVector<uint8_t>> keyData;
    std::string caFile; //For client certificate verification
    std::string caData; //For client certificate verification
  };
  typedef std::shared_ptr<CertificateInfo> PCertificateInfo;

  struct TcpServerInfo {
    bool useSsl = false;
    uint32_t connectionBacklogSize = 100;
    uint32_t maxConnections = 10;
    uint32_t serverThreads = 1;
    std::unordered_map<std::string, PCertificateInfo> certificates;
    std::string dhParamFile;
    std::string dhParamData;
    bool requireClientCert = false;
    std::function<void(int32_t clientId, std::string address, uint16_t port)> newConnectionCallback;
    std::function<void(int32_t clientId)> connectionClosedCallback;
    std::function<void(int32_t clientId, int32_t errorCode, const std::string &errorString)> connectionClosedCallbackEx;
    std::function<void(int32_t clientId, TcpPacket &packet)> packetReceivedCallback;
  };

  // {{{ TCP server or client
  /**
   * Constructor to create an empty socket object.
   *
   * @param baseLib The base library object.
   */
  TcpSocket(BaseLib::SharedObjects *baseLib);

  /**
   * Constructor to just wrap an existing socket descriptor.
   *
   * @param baseLib The base library object.
   * @param socketDescriptor The socket descriptor to wrap.
   */
  TcpSocket(BaseLib::SharedObjects *baseLib, std::shared_ptr<FileDescriptor> socketDescriptor);
  // }}}

  // {{{ TCP client
  /**
   * Constructor to create a TCP client socket.
   *
   * @param baseLib The base library object.
   * @param hostname The host name or IP address to connect to.
   * @param port The port to connect to.
   */
  TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port);

  /**
   * Constructor to create a TCP client socket with SSL enabled.
   *
   * @param baseLib The base library object.
   * @param hostname The host name or IP address to connect to.
   * @param port The port to connect to.
   * @param useSsl Set to `true` to enable SSL.
   * @param caFile Path to a file containing the PEM-encoded CA certificate used to sign the server certificate. If empty, the system's certificate store is used.
   * @param verifyCertificate Enable certificate verification. Only set to `false` for testing.
   */
  TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port, bool useSsl, std::string caFile, bool verifyCertificate);

  /**
   * Constructor to create a TCP client socket with SSL enabled.
   *
   * @param baseLib The base library object.
   * @param hostname The host name or IP address to connect to.
   * @param port The port to connect to.
   * @param useSsl Set to `true` to enable SSL.
   * @param verifyCertificate Enable certificate verification. Only set to `false` for testing.
   * @param caData The PEM-encoded CA certificate (not the path) used to sign the server certificate. If empty, the system's certificate store is used.
   */
  TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port, bool useSsl, bool verifyCertificate, std::string caData);

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
  TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port, bool useSsl, std::string caFile, bool verifyCertificate, std::string clientCertFile, std::string clientKeyFile);

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
  TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port, bool useSsl, bool verifyCertificate, std::string caData, std::string clientCertData, const std::shared_ptr<Security::SecureVector<uint8_t>> &clientKeyData);

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
  TcpSocket(BaseLib::SharedObjects *baseLib,
            std::string hostname,
            std::string port,
            bool useSsl,
            bool verifyCertificate,
            std::string caFile,
            std::string caData,
            std::string clientCertFile,
            std::string clientCertData,
            std::string clientKeyFile,
            const std::shared_ptr<Security::SecureVector<uint8_t>> &);
  // }}}

  // {{{ TCP server
  /**
   * Constructor to create a TCP server.
   */
  TcpSocket(BaseLib::SharedObjects *baseLib, TcpServerInfo &serverInfo);
  // }}}

  virtual ~TcpSocket();

  static PFileDescriptor bindAndReturnSocket(FileDescriptorManager &fileDescriptorManager, const std::string &address, const std::string &port, uint32_t connectionBacklogSize, std::string &listenAddress, int32_t &listenPort);

  PFileDescriptor getFileDescriptor();
  std::string getIpAddress();
  int32_t getPort() { return _boundListenPort; }
  bool getRequireClientCert() { return _requireClientCert; }
  void setConnectionRetries(int32_t retries) { _connectionRetries = retries; }
  void setReadTimeout(int64_t timeout) { _readTimeout = timeout; }
  void setWriteTimeout(int64_t timeout) { _writeTimeout = timeout; }
  void setAutoConnect(bool autoConnect) { _autoConnect = autoConnect; }
  void setHostname(std::string hostname) {
    close();
    _hostname = hostname;
  }
  void setPort(std::string port) {
    close();
    _port = port;
  }
  void setUseSSL(bool useSsl) {
    if (!_isServer) {
      close();
      _useSsl = useSsl;
      if (_useSsl) initSsl();
    }
  }
  void setCertificates(std::unordered_map<std::string, PCertificateInfo> &certificates);
  void reloadCertificates();
  void setVerifyCertificate(bool verifyCertificate) {
    close();
    _verifyCertificate = verifyCertificate;
  }
  void setVerifyHostname(bool verifyHostname) {
    close();
    _verifyHostname = verifyHostname;
  }

  /**
   * Only relevant for TLS connections. Sets the hostname the certificate's common name is verified against.
   * @param hostname The compare the certificate's common name to.
   */
  void setVerificationHostname(const std::string &hostname) {
    close();
    _verificationHostname = hostname;
  }
  std::unordered_map<std::string, std::shared_ptr<CertificateCredentials>> getCredentials() { return _certificateCredentials; }

  /**
   * Tests in this order and returns false if one of the tests fails:
   *
   * 1. Tests if there is a valid socket (!= -1)
   * 2. If we are currently connecting
   * 3. Calls recv with the MSG_PEEK flag set and checks for an error return.
   *
   * @return Returns true if the socket is connected.
   */
  bool connected();

  /**
   * Use this overload when there are no socket operations outside of this class.
   *
   * @param buffer The buffer to fill.
   * @param bufferSize The size of the buffer.
   * @returns The number of bytes read. The returned value is always greater than 0 (in all other cases exceptions are thrown).
   * @throws SocketOperationException Thrown when socket is nullptr.
   * @throws SocketTimeOutException Thrown when reading times out.
   * @throws SocketClosedException Thrown when socket is closed.
   */
  int32_t proofread(char *buffer, int32_t bufferSize);

  /**
   * Use this overload of proofread when select is called outside of this class.
   *
   * @param buffer The buffer to fill.
   * @param bufferSize The size of the buffer.
   * @param[out] moreData If true, call proofread immediately again (without calling e. g. select first).
   * @returns The number of bytes read. The returned value is always greater than 0 (in all other cases exceptions are thrown).
   * @throws SocketOperationException Thrown when socket is nullptr.
   * @throws SocketTimeOutException Thrown when reading times out.
   * @throws SocketClosedException Thrown when socket is closed.
   */
  int32_t proofread(char *buffer, int32_t bufferSize, bool &moreData);

  /**
   * Writes data to a socket.
   *
   * @returns Returns the number of bytes written. Never returns a negative number.
   * @throws SocketOperationException Thrown when socket is nullptr.
   * @throws SocketDataLimitException Thrown when trying to send more than 104857600 bytes of data.
   * @throws SocketTimeOutException Thrown when writing times out.
   * @throws SocketClosedException Thrown when socket is closed.
   */
  int32_t proofwrite(const std::shared_ptr<std::vector<char>> &data);

  /**
   * Writes data to a socket.
   *
   * @returns Returns the number of bytes written. Never returns a negative number.
   * @throws SocketOperationException Thrown when socket is nullptr.
   * @throws SocketDataLimitException Thrown when trying to send more than 104857600 bytes of data.
   * @throws SocketTimeOutException Thrown when writing times out.
   * @throws SocketClosedException Thrown when socket is closed.
   */
  int32_t proofwrite(const std::vector<char> &data);

  /**
   * Writes data to a socket.
   *
   * @returns Returns the number of bytes written. Never returns a negative number.
   * @throws SocketOperationException Thrown when socket is nullptr.
   * @throws SocketDataLimitException Thrown when trying to send more than 104857600 bytes of data.
   * @throws SocketTimeOutException Thrown when writing times out.
   * @throws SocketClosedException Thrown when socket is closed.
   */
  int32_t proofwrite(const std::string &data);

  /**
   * Writes data to a socket.
   *
   * @returns Returns the number of bytes written. Never returns a negative number.
   * @throws SocketOperationException Thrown when socket is nullptr.
   * @throws SocketDataLimitException Thrown when trying to send more than 104857600 bytes of data.
   * @throws SocketTimeOutException Thrown when writing times out.
   * @throws SocketClosedException Thrown when socket is closed.
   */
  int32_t proofwrite(const char *buffer, int32_t bytesToWrite);

  void open();
  void close();

  // {{{ Servers only
  /**
   * Binds a socket to an IP address and a port. This splits up the start process to be able to listen on ports
   * lower than 1024 and do a privilege drop. Call startPreboundServer() to start listening. Don't call
   * startServer() when using pre-binding as this recreates the socket.
   *
   * @see startPreboundServer
   *
   * @param address The address to bind the server to (e. g. `::` or `0.0.0.0`).
   * @param port The port number to bind the server to.
   * @param[out] listenAddress The IP address the server was bound to (e. g. `192.168.0.152`).
   */
  void bindServerSocket(std::string address, std::string port, std::string &listenAddress);

  /**
   * Starts listening on the already bound socket (created with bindServerSocket()). This splits up the start process to
   * be able to listen on ports lower than 1024 and do a privilege drop. Don't call startServer() when using
   * pre-binding as this recreates the socket.
   *
   * @see bindServerSocket
   *
   * @param[out] listenAddress The IP address the server was bound to (e. g. `192.168.0.152`).
   * @param processingThreads The number of processing threads to start. By default no threads are started.
   */
  void startPreboundServer(std::string &listenAddress, size_t processingThreads = 0);

  /**
   * Starts listening.
   *
   * @param address The address to bind the server to (e. g. `::` or `0.0.0.0`).
   * @param port The port number to bind the server to.
   * @param[out] listenAddress The IP address the server was bound to (e. g. `192.168.0.152`).
   * @param processingThreads The number of processing threads to start. By default no threads are started.
   */
  void startServer(std::string address, std::string port, std::string &listenAddress, size_t processingThreads = 0);

  /**
   * Starts listening on a dynamically assigned port.
   *
   * @param address The address to bind the server to (e. g. `::` or `0.0.0.0`).
   * @param[out] listenAddress The IP address the server was bound to (e. g. `192.168.0.152`).
   * @param[out] listenPort The port the server was bound to (e. g. `45735`).
   * @param processingThreads The number of processing threads to start. By default no threads are started.
   */
  void startServer(std::string address, std::string &listenAddress, int32_t &listenPort, size_t processingThreads = 0);

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
  bool sendToClient(int32_t clientId, const TcpPacket &packet, bool closeConnection = false);

  /**
   * Sends a response to a TCP client connected to the server.
   *
   * @param clientId The ID of the client as passed to TcpSocket::TcpServerServer::packetReceivedCallback.
   * @param packet The data to send.
   * @param closeConnection Close the connection after sending the packet.
   */
  bool sendToClient(int32_t clientId, const std::vector<char> &packet, bool closeConnection = false);

  /**
   * Closes the connection to a connected client.
   *
   * @param clientId The ID of the client to close as passed to TcpSocket::TcpServerServer::packetReceivedCallback.
   */
  void closeClientConnection(int32_t clientId);

  /**
   * Returns the number of clients connected to the TCP server
   * @return The number of connected clients.
   */
  int32_t clientCount();

  /**
   * Returns the size of the packet processing queue.
   * @return Returns the size of the packet processing queue.
   */
  uint32_t processingQueueSize();

  /**
   * Returns the distinguished name of the client certificate. This method only returns a non empty string if
   * the client certificate is valid.
   *
   * @param clientId The ID of the client to get the distinguished name for.
   * @return Returns the DN when the client certificate verification was successful.
   */
  std::string getClientCertDn(int32_t clientId);

  std::string getClientCertSerial(int32_t clientId);

  int64_t getClientCertExpiration(int32_t clientId);
  // }}}
 protected:
  BaseLib::SharedObjects *_bl = nullptr;
  int32_t _connectionRetries = 3;
  int64_t _readTimeout = 15000000;
  int64_t _writeTimeout = 15000000;
  std::atomic_bool _connecting;
  bool _autoConnect = true;
  std::string _ipAddress;
  std::string _hostname;
  std::string _verificationHostname;
  std::string _port;
  std::mutex _readMutex;
  std::mutex _writeMutex;
  std::unordered_map<std::string, PCertificateInfo> _certificates;
  bool _verifyCertificate = true;
  bool _verifyHostname = true;

  // {{{ For server only
  bool _isServer = false;
  uint32_t _backlogSize = 100;
  uint32_t _maxConnections = 10;
  std::string _dhParamFile;
  std::string _dhParamData;
  bool _requireClientCert = false;
  std::function<void(int32_t clientId, std::string address, uint16_t port)> _newConnectionCallback;
  std::function<void(int32_t clientId)> _connectionClosedCallback;
  std::function<void(int32_t clientId, int32_t errorCode, const std::string &errorString)> _connectionClosedCallbackEx;
  std::function<void(int32_t clientId, TcpPacket &packet)> _packetReceivedCallback;

  std::string _listenAddress;
  std::string _listenPort;
  int32_t _boundListenPort = -1;

  gnutls_priority_t _tlsPriorityCache = nullptr;

  std::atomic_bool _stopServer;
  std::vector<std::thread> _serverThreads;

  int64_t _lastGarbageCollection = 0;

  /**
   * Stores the current client ID. The client ID is incremented by one for every client, so it is unique for a long time.
   */
  int32_t _currentClientId = 0;
  std::mutex _clientsMutex;
  std::map<int32_t, PTcpClientData> _clients;
  // }}}

  std::mutex _socketDescriptorMutex;
  PFileDescriptor _socketDescriptor;
  bool _useSsl = false;
  std::mutex _certificateCredentialsMutex;
  /**
   * Contains a copy of the current credentials in case the credentials are replaced while the connection is open.
   */
  std::shared_ptr<CertificateCredentials> _currentClientCertificateCredentials;
  /**
   * Stores the certificate credentials so that they can be replaced at any time.
   */
  std::unordered_map<std::string, std::shared_ptr<CertificateCredentials>> _certificateCredentials;

  void getSocketDescriptor();
  void getConnection();
  void getSsl();
  void initSsl();
  void initTlsPriorityCache();
  void autoConnect();

  // {{{ For server only
  void bindSocket();

  void serverThread();
  void processQueueEntry(int32_t index, std::shared_ptr<BaseLib::IQueueEntry> &entry) override;
  void collectGarbage();
  void collectGarbage(std::map<int32_t, PTcpClientData> &clients);
  void initClientSsl(PTcpClientData &clientData);
  void readClient(const PTcpClientData &clientData);
  // }}}
};

typedef std::shared_ptr<BaseLib::TcpSocket> PTcpSocket;

}
#endif
