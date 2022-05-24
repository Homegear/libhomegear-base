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

#include "TcpSocket.h"
#include "../BaseLib.h"
#include "../Security/SecureVector.h"

namespace BaseLib {

TcpSocket::CertificateCredentials::CertificateCredentials(gnutls_certificate_credentials_t credentials, gnutls_datum_t dhParams) : _credentials(credentials) {
  if (dhParams.size > 0) {
    int result = 0;
    if ((result = gnutls_dh_params_init(&_dhParams)) != GNUTLS_E_SUCCESS) {
      _dhParams = nullptr;
      throw SocketSslException("Error: Could not initialize DH parameters: " + std::string(gnutls_strerror(result)));
    }

    if ((result = gnutls_dh_params_import_pkcs3(_dhParams, &dhParams, GNUTLS_X509_FMT_PEM)) != GNUTLS_E_SUCCESS) {
      gnutls_dh_params_deinit(_dhParams);
      _dhParams = nullptr;
      throw SocketSslException("Error: Could not import DH parameters: " + std::string(gnutls_strerror(result)));
    }

    gnutls_certificate_set_dh_params(credentials, _dhParams);
  }
}

TcpSocket::CertificateCredentials::~CertificateCredentials() {
  gnutls_certificate_free_credentials(_credentials);
  if (_dhParams) gnutls_dh_params_deinit(_dhParams);
}

gnutls_certificate_credentials_t TcpSocket::CertificateCredentials::get() {
  return _credentials;
}

TcpSocket::TcpSocket(BaseLib::SharedObjects *baseLib) : IQueue(baseLib, 1, 10000) {
  _bl = baseLib;
  _stopServer = false;
  _autoConnect = false;
  _connecting = false;
  _socketDescriptor.reset(new FileDescriptor);
}

TcpSocket::TcpSocket(BaseLib::SharedObjects *baseLib, std::shared_ptr<FileDescriptor> socketDescriptor) : IQueue(baseLib, 1, 10000) {
  _bl = baseLib;
  _stopServer = false;
  _autoConnect = false;
  _connecting = false;
  if (socketDescriptor) _socketDescriptor = socketDescriptor;
  else _socketDescriptor.reset(new FileDescriptor);
}

TcpSocket::TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port) : IQueue(baseLib, 1, 10000) {
  _bl = baseLib;
  _stopServer = false;
  _connecting = false;
  _socketDescriptor.reset(new FileDescriptor);
  _hostname = hostname;
  _verificationHostname = hostname;
  _port = port;
}

TcpSocket::TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port, bool useSsl, std::string caFile, bool verifyCertificate) : TcpSocket(baseLib, hostname, port) {
  _useSsl = useSsl;
  if (!caFile.empty()) {
    PCertificateInfo certificateInfo = std::make_shared<CertificateInfo>();
    certificateInfo->caFile = caFile;
    _certificates.emplace("*", certificateInfo);
  }
  _verifyCertificate = verifyCertificate;

  if (_useSsl) initSsl();
}

TcpSocket::TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port, bool useSsl, bool verifyCertificate, std::string caData) : TcpSocket(baseLib, hostname, port) {
  _useSsl = useSsl;
  _verifyCertificate = verifyCertificate;
  if (!caData.empty()) {
    PCertificateInfo certificateInfo = std::make_shared<CertificateInfo>();
    certificateInfo->caData = caData;
    _certificates.emplace("*", certificateInfo);
  }

  if (_useSsl) initSsl();
}

TcpSocket::TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port, bool useSsl, std::string caFile, bool verifyCertificate, std::string clientCertFile, std::string clientKeyFile) : TcpSocket(baseLib, hostname, port) {
  _useSsl = useSsl;
  _verifyCertificate = verifyCertificate;
  if (!caFile.empty() || !clientCertFile.empty() || !clientKeyFile.empty()) {
    PCertificateInfo certificateInfo = std::make_shared<CertificateInfo>();
    certificateInfo->caFile = caFile;
    certificateInfo->certFile = clientCertFile;
    certificateInfo->keyFile = clientKeyFile;
    _certificates.emplace("*", certificateInfo);
  }

  if (_useSsl) initSsl();
}

TcpSocket::TcpSocket(BaseLib::SharedObjects *baseLib, std::string hostname, std::string port, bool useSsl, bool verifyCertificate, std::string caData, std::string clientCertData, const std::shared_ptr<Security::SecureVector<uint8_t>> &clientKeyData)
    : TcpSocket(baseLib, hostname, port) {
  _useSsl = useSsl;
  _verifyCertificate = verifyCertificate;
  if (!caData.empty() || !clientCertData.empty() || (clientKeyData && !clientKeyData->empty())) {
    PCertificateInfo certificateInfo = std::make_shared<CertificateInfo>();
    certificateInfo->caData = caData;
    certificateInfo->certData = clientCertData;
    certificateInfo->keyData = clientKeyData;
    _certificates.emplace("*", certificateInfo);
  }

  if (_useSsl) initSsl();
}

TcpSocket::TcpSocket(BaseLib::SharedObjects *baseLib,
                     std::string hostname,
                     std::string port,
                     bool useSsl,
                     bool verifyCertificate,
                     std::string caFile,
                     std::string caData,
                     std::string clientCertFile,
                     std::string clientCertData,
                     std::string clientKeyFile,
                     const std::shared_ptr<Security::SecureVector<uint8_t>> &clientKeyData) : TcpSocket(baseLib, hostname, port) {
  _useSsl = useSsl;
  _verifyCertificate = verifyCertificate;
  if (!caFile.empty() || !caData.empty() || !clientCertFile.empty() || !clientCertData.empty() || !clientKeyFile.empty() || (clientKeyData && !clientKeyData->empty())) {
    PCertificateInfo certificateInfo = std::make_shared<CertificateInfo>();
    certificateInfo->caFile = caFile;
    certificateInfo->caData = caData;
    certificateInfo->certFile = clientCertFile;
    certificateInfo->certData = clientCertData;
    certificateInfo->keyFile = clientKeyFile;
    certificateInfo->keyData = clientKeyData;
    _certificates.emplace("*", certificateInfo);
  }

  if (_useSsl) initSsl();
}

TcpSocket::TcpSocket(BaseLib::SharedObjects *baseLib, TcpServerInfo &serverInfo) : IQueue(baseLib, 1, 10000) {
  _bl = baseLib;
  _connecting = false;

  _stopServer = false;
  _isServer = true;
  _useSsl = serverInfo.useSsl;
  if (serverInfo.connectionBacklogSize > 0) _backlogSize = serverInfo.connectionBacklogSize;
  _maxConnections = serverInfo.maxConnections;
  _certificates = serverInfo.certificates;
  _dhParamFile = serverInfo.dhParamFile;
  _dhParamData = serverInfo.dhParamData;
  _requireClientCert = serverInfo.requireClientCert;
  _newConnectionCallback.swap(serverInfo.newConnectionCallback);
  _connectionClosedCallback.swap(serverInfo.connectionClosedCallback);
  _connectionClosedCallbackEx.swap(serverInfo.connectionClosedCallbackEx);
  _packetReceivedCallback.swap(serverInfo.packetReceivedCallback);

  server_threads_.resize(serverInfo.serverThreads);
  average_packets_per_minute_received_.resize(serverInfo.serverThreads);
  average_packets_per_minute_sent_.resize(serverInfo.serverThreads);
}

TcpSocket::~TcpSocket() {
  _stopServer = true;
  for (auto &serverThread: server_threads_) {
    _bl->threadManager.join(serverThread);
  }

  std::unique_lock<std::mutex> readGuard(_readMutex, std::defer_lock);
  std::unique_lock<std::mutex> writeGuard(_writeMutex, std::defer_lock);
  std::lock(readGuard, writeGuard);
  _bl->fileDescriptorManager.close(_socketDescriptor);
  _certificateCredentials.clear();
  if (_tlsPriorityCache) gnutls_priority_deinit(_tlsPriorityCache);
}

PFileDescriptor TcpSocket::getFileDescriptor() {
  std::lock_guard<std::mutex> socketDescriptorGuard(_socketDescriptorMutex);
  return _socketDescriptor;
}

std::string TcpSocket::getIpAddress() {
  if (!_ipAddress.empty()) return _ipAddress;
  _ipAddress = Net::resolveHostname(_hostname);
  return _ipAddress;
}

void TcpSocket::setCertificates(std::unordered_map<std::string, PCertificateInfo> &certificates) {
  _certificates = certificates;
  initSsl();
}

void TcpSocket::reloadCertificates() {
  initSsl();
}

// {{{ Server
void TcpSocket::bindServerSocket(std::string address, std::string port, std::string &listenAddress) {
  waitForServerStopped();

  if (_useSsl) {
    initSsl();
    initTlsPriorityCache();
  }

  _listenAddress = std::move(address);
  _listenPort = std::move(port);
  bindSocket();
  listenAddress = _ipAddress;
}

void TcpSocket::bindSocket() {
  _socketDescriptor = bindAndReturnSocket(_bl->fileDescriptorManager, _listenAddress, _listenPort, _backlogSize, _ipAddress, _boundListenPort);
}

void TcpSocket::startPreboundServer(std::string &listenAddress, size_t processingThreads) {
  _stopServer = false;
  listenAddress = _ipAddress;

  if (processingThreads > 0) startQueue(0, false, processingThreads, 0, SCHED_OTHER);

  for (uint32_t i = 0; i < server_threads_.size(); i++) {
    _bl->threadManager.start(server_threads_.at(i), true, &TcpSocket::serverThread, this, i);
  }
}

void TcpSocket::startServer(std::string address, std::string port, std::string &listenAddress, size_t processingThreads) {
  waitForServerStopped();

  if (_useSsl) {
    initSsl();
    initTlsPriorityCache();
  }

  _stopServer = false;
  _listenAddress = std::move(address);
  _listenPort = std::move(port);
  bindSocket();
  listenAddress = _ipAddress;

  if (processingThreads > 0) startQueue(0, false, processingThreads, 0, SCHED_OTHER);

  for (uint32_t i = 0; i < server_threads_.size(); i++) {
    _bl->threadManager.start(server_threads_.at(i), true, &TcpSocket::serverThread, this, i);
  }
}

void TcpSocket::startServer(std::string address, std::string &listenAddress, int32_t &listenPort, size_t processingThreads) {
  waitForServerStopped();

  if (_useSsl) {
    initSsl();
    initTlsPriorityCache();
  }

  _stopServer = false;
  _listenAddress = address;
  _listenPort = "0";
  bindSocket();
  listenAddress = _ipAddress;
  listenPort = _boundListenPort;

  if (processingThreads > 0) startQueue(0, false, processingThreads, 0, SCHED_OTHER);

  for (uint32_t i = 0; i < server_threads_.size(); i++) {
    _bl->threadManager.start(server_threads_.at(i), true, &TcpSocket::serverThread, this, i);
  }
}

void TcpSocket::stopServer() {
  _stopServer = true;
}

void TcpSocket::waitForServerStopped() {
  stopQueue(0);

  _stopServer = true;
  for (auto &serverThread: server_threads_) {
    _bl->threadManager.join(serverThread);
  }

  _bl->fileDescriptorManager.close(_socketDescriptor);
  std::lock_guard<std::mutex> certificateCredentialsGuard(_certificateCredentialsMutex);
  _certificateCredentials.clear();
  if (_tlsPriorityCache) gnutls_priority_deinit(_tlsPriorityCache);
  _tlsPriorityCache = nullptr;
}

int postClientHello(gnutls_session_t tlsSession) {
  auto *clientData = (TcpSocket::TcpClientData *)gnutls_session_get_ptr(tlsSession);
  if (!clientData) return GNUTLS_E_INTERNAL_ERROR;

  int32_t result = 0;
  if (clientData->certificateCredentials.size() > 1) {
    std::array<char, 300> nameBuffer{};
    size_t dataSize = nameBuffer.size() - 1;
    unsigned int type = GNUTLS_NAME_DNS;
    if (gnutls_server_name_get(tlsSession, nameBuffer.data(), &dataSize, &type, 0) != GNUTLS_E_SUCCESS) {
      if ((result = gnutls_credentials_set(tlsSession, GNUTLS_CRD_CERTIFICATE, clientData->certificateCredentials.begin()->second->get())) != GNUTLS_E_SUCCESS) {
        return result;
      }
    } else {
      nameBuffer.at(nameBuffer.size() - 1) = 0;
      std::string serverName(nameBuffer.data());
      auto credentialsIterator = clientData->certificateCredentials.find(serverName);
      if (credentialsIterator != clientData->certificateCredentials.end()) {
        if ((result = gnutls_credentials_set(tlsSession, GNUTLS_CRD_CERTIFICATE, credentialsIterator->second->get())) != GNUTLS_E_SUCCESS) {
          return result;
        }
      } else {
        if ((result = gnutls_credentials_set(tlsSession, GNUTLS_CRD_CERTIFICATE, clientData->certificateCredentials.begin()->second->get())) != GNUTLS_E_SUCCESS) {
          return result;
        }
      }
    }
  } else if (clientData->certificateCredentials.size() == 1) {
    if ((result = gnutls_credentials_set(tlsSession, GNUTLS_CRD_CERTIFICATE, clientData->certificateCredentials.begin()->second->get())) != GNUTLS_E_SUCCESS) {
      return GNUTLS_E_CERTIFICATE_ERROR;
    }
  } else return GNUTLS_E_CERTIFICATE_ERROR;

  return GNUTLS_E_SUCCESS;
}

void TcpSocket::initClientSsl(PTcpClientData &clientData) {
  if (!_tlsPriorityCache) {
    _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
    throw SocketSslException("Error: Could not initiate TLS connection. _tlsPriorityCache is nullptr.");
  }

  {
    std::lock_guard<std::mutex> certificateCredentialsGuard(_certificateCredentialsMutex);
    if (_certificateCredentials.empty()) {
      _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
      throw SocketSslException("Error: Could not initiate TLS connection. _certificateCredentials is empty.");
    }

    //We need to copy the credentials in case they are replaced while a connection is still open.
    clientData->certificateCredentials = _certificateCredentials;
  }

  int32_t result = 0;
  if ((result = gnutls_init(&clientData->fileDescriptor->tlsSession, GNUTLS_SERVER)) != GNUTLS_E_SUCCESS) {
    clientData->fileDescriptor->tlsSession = nullptr;
    _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
    throw SocketSslException("Error: Could not initialize TLS session: " + std::string(gnutls_strerror(result)));
  }
  if (!clientData->fileDescriptor->tlsSession) {
    _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
    throw SocketSslException("Error: Client TLS session is nullptr.");
  }

  gnutls_session_set_ptr(clientData->fileDescriptor->tlsSession, clientData.get());

  if ((result = gnutls_priority_set(clientData->fileDescriptor->tlsSession, _tlsPriorityCache)) != GNUTLS_E_SUCCESS) {
    _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
    throw SocketSslException("Error: Could not set cipher priority on TLS session: " + std::string(gnutls_strerror(result)));
  }

  gnutls_handshake_set_post_client_hello_function(clientData->fileDescriptor->tlsSession, &postClientHello);

  gnutls_certificate_server_set_request(clientData->fileDescriptor->tlsSession, _requireClientCert ? GNUTLS_CERT_REQUIRE : GNUTLS_CERT_REQUEST);
  if (!clientData->fileDescriptor || clientData->fileDescriptor->descriptor == -1) {
    _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
    throw SocketSslException("Error setting TLS socket descriptor: Provided socket descriptor is invalid.");
  }
  gnutls_transport_set_ptr(clientData->fileDescriptor->tlsSession, (gnutls_transport_ptr_t)(uintptr_t)clientData->fileDescriptor->descriptor);
  result = gnutls_handshake(clientData->fileDescriptor->tlsSession);
  if (result < 0) {
    _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
    throw SocketSslHandshakeFailedException("TLS handshake has failed: " + std::string(gnutls_strerror(result)));
  }

  { //Check client certificate
    const gnutls_datum_t *derClientCertificates = gnutls_certificate_get_peers(clientData->fileDescriptor->tlsSession, nullptr);
    if (!derClientCertificates) {
      if (_requireClientCert) {
        _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
        throw SocketSslException("Client certificate verification has failed: Error retrieving client certificate.");
      }
    } else {
      gnutls_x509_crt_t clientCertificates = nullptr;
      unsigned int certMax = 1;
      if (gnutls_x509_crt_list_import(&clientCertificates, &certMax, derClientCertificates, GNUTLS_X509_FMT_DER, 0) < 1) {
        if (_requireClientCert) {
          _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
          gnutls_x509_crt_deinit(clientCertificates);
          throw SocketSslException("Client certificate verification has failed: Error importing client certificate.");
        }
      } else {
        gnutls_datum_t distinguishedName{};
        if (gnutls_x509_crt_get_dn2(clientCertificates, &distinguishedName) != GNUTLS_E_SUCCESS) {
          if (_requireClientCert) {
            _bl->fileDescriptorManager.shutdown(clientData->fileDescriptor);
            gnutls_free(distinguishedName.data);
            gnutls_x509_crt_deinit(clientCertificates);
            throw SocketSslException("Client certificate verification has failed: Error getting client certificate's distinguished name.");
          }
        } else clientData->clientCertDn = std::string((char *)distinguishedName.data, distinguishedName.size);
        gnutls_free(distinguishedName.data);

        std::array<char, 40> certificateSerial{};
        size_t certificateSerialSize = certificateSerial.size();
        gnutls_x509_crt_get_serial(clientCertificates, certificateSerial.data(), &certificateSerialSize);
        if (certificateSerialSize > certificateSerial.size()) certificateSerialSize = certificateSerial.size();
        clientData->clientCertSerial = HelperFunctions::getHexString(certificateSerial.data(), certificateSerialSize);
        clientData->clientCertExpiration = gnutls_x509_crt_get_expiration_time(clientCertificates);
      }
      gnutls_x509_crt_deinit(clientCertificates);
    }
  }
}

void TcpSocket::readClient(const PTcpClientData &clientData) {
  try {
    int32_t bytesRead = 0;
    bool moreData = true;

    while (moreData) {
      bytesRead = clientData->socket->proofread((char *)clientData->buffer.data(), clientData->buffer.size(), moreData);

      if (bytesRead > (signed)clientData->buffer.size()) bytesRead = clientData->buffer.size();

      {
        auto time = BaseLib::HelperFunctions::getTimeMicroseconds();
        auto &average_mean_data = average_packets_per_minute_received_.at(clientData->thread_index);
        auto interval = (double)(time - average_mean_data.last_measurement);
        if (interval == 0) interval = 1;
        double current_packets_per_minute = 60000000 / interval;
        average_mean_data.last_output = Math::metricExponentialMovingAverage(interval, 60000000, current_packets_per_minute, average_mean_data.last_output);
        average_mean_data.last_measurement = time;
      }

      if (_packetReceivedCallback) {
        if (queueIsStarted(0)) {
          auto bytesReceived = std::make_shared<std::vector<uint8_t>>(clientData->buffer.data(), clientData->buffer.data() + bytesRead);
          std::lock_guard<std::mutex> backlogGuard(clientData->backlogMutex);
          clientData->backlog.push(std::move(bytesReceived));
          if (clientData->backlog.size() > 10000) {
            while (!clientData->backlog.empty()) {
              clientData->backlog.pop();
            }
            _bl->fileDescriptorManager.close(clientData->fileDescriptor);
            if (_connectionClosedCallbackEx) _connectionClosedCallbackEx(clientData->id, -200, "Error reading from client: Backlog is full.");
            else if (_connectionClosedCallback) _connectionClosedCallback(clientData->id);
            return;
          }
          if (!clientData->busy) {
            clientData->busy = true;
            std::shared_ptr<BaseLib::IQueueEntry> queueEntry = std::make_shared<QueueEntry>(clientData);
            enqueue(0, queueEntry);
          }
        } else {
          std::vector<uint8_t> bytesReceived(clientData->buffer.data(), clientData->buffer.data() + bytesRead);
          _packetReceivedCallback(clientData->id, bytesReceived);
        }
      }
    }
  }
  catch (const std::exception &ex) {
    _bl->fileDescriptorManager.close(clientData->fileDescriptor);
    if (_connectionClosedCallbackEx) _connectionClosedCallbackEx(clientData->id, -201, std::string("Error reading from client: ") + ex.what());
    else if (_connectionClosedCallback) _connectionClosedCallback(clientData->id);
  }
}

bool TcpSocket::sendToClient(int32_t clientId, const TcpPacket &packet, bool closeConnection) {
  PTcpClientData clientData;
  try {
    {
      std::lock_guard<std::mutex> clientsGuard(_clientsMutex);
      auto clientIterator = _clients.find(clientId);
      if (clientIterator == _clients.end()) return false;
      clientData = clientIterator->second;
    }

    clientData->socket->proofwrite((char *)packet.data(), packet.size());
    if (closeConnection) {
      _bl->fileDescriptorManager.close(clientData->fileDescriptor);
      if (_connectionClosedCallbackEx) _connectionClosedCallbackEx(clientData->id, 0, "");
      else if (_connectionClosedCallback) _connectionClosedCallback(clientData->id);
    }

    {
      auto time = BaseLib::HelperFunctions::getTimeMicroseconds();
      auto &average_mean_data = average_packets_per_minute_sent_.at(clientData->thread_index);
      auto interval = (double)(time - average_mean_data.last_measurement);
      if (interval == 0) interval = 1;
      double current_packets_per_minute = 60000000 / interval;
      average_mean_data.last_output = Math::metricExponentialMovingAverage(interval, 60000000, current_packets_per_minute, average_mean_data.last_output);
      average_mean_data.last_measurement = time;
    }

    return true;
  }
  catch (const std::exception &ex) {
    _bl->fileDescriptorManager.close(clientData->fileDescriptor);
    if (_connectionClosedCallbackEx) _connectionClosedCallbackEx(clientData->id, -300, std::string("Error sending data to client: ") + ex.what());
    else if (_connectionClosedCallback) _connectionClosedCallback(clientData->id);
  }
  return false;
}

bool TcpSocket::sendToClient(int32_t clientId, const std::vector<char> &packet, bool closeConnection) {
  PTcpClientData clientData;
  try {
    {
      std::lock_guard<std::mutex> clientsGuard(_clientsMutex);
      auto clientIterator = _clients.find(clientId);
      if (clientIterator == _clients.end()) return false;
      clientData = clientIterator->second;
    }

    clientData->socket->proofwrite((char *)packet.data(), packet.size());
    if (closeConnection) {
      _bl->fileDescriptorManager.close(clientData->fileDescriptor);
      if (_connectionClosedCallbackEx) _connectionClosedCallbackEx(clientData->id, 0, "");
      else if (_connectionClosedCallback) _connectionClosedCallback(clientData->id);
    }

    {
      auto time = BaseLib::HelperFunctions::getTimeMicroseconds();
      auto &average_mean_data = average_packets_per_minute_sent_.at(clientData->thread_index);
      auto interval = (double)(time - average_mean_data.last_measurement);
      if (interval == 0) interval = 1;
      double current_packets_per_minute = 60000000 / interval;
      average_mean_data.last_output = Math::metricExponentialMovingAverage(interval, 60000000, current_packets_per_minute, average_mean_data.last_output);
      average_mean_data.last_measurement = time;
    }

    return true;
  }
  catch (const std::exception &ex) {
    _bl->fileDescriptorManager.close(clientData->fileDescriptor);
    if (_connectionClosedCallbackEx) _connectionClosedCallbackEx(clientData->id, -300, std::string("Error sending data to client: ") + ex.what());
    else if (_connectionClosedCallback) _connectionClosedCallback(clientData->id);
  }
  return false;
}

void TcpSocket::closeClientConnection(int32_t clientId) {
  std::lock_guard<std::mutex> clientsGuard(_clientsMutex);
  auto clientIterator = _clients.find(clientId);
  if (clientIterator != _clients.end()) clientIterator->second->socket->close();
}

int32_t TcpSocket::clientCount() {
  std::lock_guard<std::mutex> clientsGuard(_clientsMutex);
  return _clients.size();
}

uint32_t TcpSocket::processingQueueSize() {
  return queueSize(0);
}

std::string TcpSocket::getClientCertDn(int32_t clientId) {
  std::lock_guard<std::mutex> clientsGuard(_clientsMutex);
  auto clientIterator = _clients.find(clientId);
  if (clientIterator != _clients.end()) return clientIterator->second->clientCertDn;
  return "";
}

std::string TcpSocket::getClientCertSerial(int32_t clientId) {
  std::lock_guard<std::mutex> clientsGuard(_clientsMutex);
  auto clientIterator = _clients.find(clientId);
  if (clientIterator != _clients.end()) return clientIterator->second->clientCertSerial;
  return "";
}

int64_t TcpSocket::getClientCertExpiration(int32_t clientId) {
  std::lock_guard<std::mutex> clientsGuard(_clientsMutex);
  auto clientIterator = _clients.find(clientId);
  if (clientIterator != _clients.end()) return clientIterator->second->clientCertExpiration;
  return 0;
}

void TcpSocket::serverThread(uint32_t thread_index) {
  sigset_t signal_set{};
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGPIPE);
  if (pthread_sigmask(SIG_BLOCK, &signal_set, nullptr) != 0) {
    _bl->out.printWarning("Warning: " + std::string("Could not block SIGPIPE."));
  }

  int32_t result = 0;
  int32_t socketDescriptor = -1;
  std::map<int32_t, PTcpClientData> clients;
  while (!_stopServer) {
    try {
      {
        std::lock_guard<std::mutex> socketDescriptorGuard(_socketDescriptorMutex);
        if (!_socketDescriptor || _socketDescriptor->descriptor == -1) {
          if (_stopServer) break;
          std::this_thread::sleep_for(std::chrono::milliseconds(5000));
          bindSocket();
          continue;
        }
        socketDescriptor = _socketDescriptor->descriptor;
      }

      { //Send backlog
        if (_packetReceivedCallback && queueIsStarted(0)) {
          for (auto &client: clients) {
            if (!client.second->fileDescriptor || client.second->fileDescriptor->descriptor == -1) continue;
            std::lock_guard<std::mutex> backlogGuard(client.second->backlogMutex);
            if (!client.second->backlog.empty() && !client.second->busy) {
              client.second->busy = true;
              std::shared_ptr<BaseLib::IQueueEntry> queueEntry = std::make_shared<QueueEntry>(client.second);
              enqueue(0, queueEntry);
            }
          }
        }
      }

      timeval timeout{};
      timeout.tv_sec = 0;
      timeout.tv_usec = 100000;
      fd_set readFileDescriptor;
      int32_t maxfd = 0;
      FD_ZERO(&readFileDescriptor);
      {
        auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
        fileDescriptorGuard.lock();
        maxfd = socketDescriptor;
        FD_SET(socketDescriptor, &readFileDescriptor);

        {
          for (auto &client: clients) {
            if (!client.second->fileDescriptor || client.second->fileDescriptor->descriptor == -1) continue;
            FD_SET(client.second->fileDescriptor->descriptor, &readFileDescriptor);
            if (client.second->fileDescriptor->descriptor > maxfd) maxfd = client.second->fileDescriptor->descriptor;
          }
        }
      }

      do {
        result = select(maxfd + 1, &readFileDescriptor, nullptr, nullptr, &timeout);
      } while (result == -1 && errno == EINTR);
      if (result == 0) {
        if (HelperFunctions::getTime() - _lastGarbageCollection > 60000 || _clients.size() >= _maxConnections) {
          collectGarbage();
          collectGarbage(clients);
        }
        continue;
      } else if (result == -1) {
        _bl->out.printError("Error: select returned -1: " + std::string(strerror(errno)));
        continue;
      }

      if (FD_ISSET(socketDescriptor, &readFileDescriptor) && !_stopServer) {
        struct sockaddr_storage clientInfo{};
        socklen_t addressSize = sizeof(addressSize);
        std::shared_ptr<BaseLib::FileDescriptor> clientFileDescriptor = _bl->fileDescriptorManager.add(accept(socketDescriptor, (struct sockaddr *)&clientInfo, &addressSize));
        if (!clientFileDescriptor || clientFileDescriptor->descriptor == -1) continue;
        if (clientFileDescriptor->descriptor >= FD_SETSIZE) {
          _bl->out.printError("Error: No more clients can connect to me as the maximum number of file descriptors is reached. Listen IP: " + _listenAddress + ", bound port: " + _listenPort);
          _bl->fileDescriptorManager.shutdown(clientFileDescriptor);
          continue;
        }

        try {
          getpeername(clientFileDescriptor->descriptor, (struct sockaddr *)&clientInfo, &addressSize);

          uint16_t port = 0;
          std::array<char, INET6_ADDRSTRLEN> ipString{};
          if (clientInfo.ss_family == AF_INET) {
            auto *s = (struct sockaddr_in *)&clientInfo;
            port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &s->sin_addr, ipString.data(), ipString.size());
          } else { // AF_INET6
            auto *s = (struct sockaddr_in6 *)&clientInfo;
            port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &s->sin6_addr, ipString.data(), ipString.size());
          }
          std::string address = std::string(ipString.data());

          if (_clients.size() > _maxConnections) {
            collectGarbage();
            if (_clients.size() > _maxConnections) {
              _bl->out.printError("Error: No more clients can connect to me as the maximum number of allowed connections is reached. Listen IP: " + _listenAddress + ", bound port: " + _listenPort + ", client IP: " + ipString.data());
              _bl->fileDescriptorManager.shutdown(clientFileDescriptor);
              continue;
            }
          }

          if (_stopServer) {
            _bl->fileDescriptorManager.shutdown(clientFileDescriptor);
            continue;
          }

          PTcpClientData clientData = std::make_shared<TcpClientData>();
          clientData->fileDescriptor = clientFileDescriptor;
          clientData->socket = std::make_shared<BaseLib::TcpSocket>(_bl, clientFileDescriptor);
          clientData->socket->setReadTimeout(100000);
          clientData->socket->setWriteTimeout(15000000);
          clientData->thread_index = thread_index;

          if (_useSsl) initClientSsl(clientData);

          int32_t currentClientId = 0;

          {
            std::lock_guard<std::mutex> clientsGuard(_clientsMutex);
            while (currentClientId == 0) {
              currentClientId = _currentClientId++;
            }
            clientData->id = currentClientId;
            _clients[currentClientId] = clientData;
          }

          clients[currentClientId] = clientData;

          if (_newConnectionCallback) _newConnectionCallback(currentClientId, address, port);
        }
        catch (const SocketSslHandshakeFailedException &ex) {
          _bl->fileDescriptorManager.shutdown(clientFileDescriptor);
          _bl->out.printInfo("Info: " + std::string(ex.what()));
        }
        catch (const std::exception &ex) {
          _bl->fileDescriptorManager.shutdown(clientFileDescriptor);
          _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
        }
        continue;
      }

      for (auto &client: clients) {
        if (client.second->fileDescriptor->descriptor == -1) continue;
        if (FD_ISSET(client.second->fileDescriptor->descriptor, &readFileDescriptor)) {
          readClient(client.second);
        }
      }
    }
    catch (const std::exception &ex) {
      _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
  }
  std::lock_guard<std::mutex> socketDescriptorGuard(_socketDescriptorMutex);
  _bl->fileDescriptorManager.close(_socketDescriptor);
}

void TcpSocket::processQueueEntry(int32_t index, std::shared_ptr<BaseLib::IQueueEntry> &entry) {
  try {
    std::shared_ptr<QueueEntry> queueEntry;
    queueEntry = std::dynamic_pointer_cast<QueueEntry>(entry);
    if (!queueEntry) return;

    std::unique_lock<std::mutex> backlogGuard(queueEntry->clientData->backlogMutex, std::defer_lock);

    //Send a maximum of 10 packets
    for (int32_t i = 0; i < 10; i++) {
      backlogGuard.lock();
      if (queueEntry->clientData->backlog.empty()) {
        queueEntry->clientData->busy = false;
        return;
      }
      std::shared_ptr<TcpPacket> packet = queueEntry->clientData->backlog.front();
      queueEntry->clientData->backlog.pop();
      backlogGuard.unlock();

      if (_packetReceivedCallback) _packetReceivedCallback(queueEntry->clientData->id, *packet);
    }
    backlogGuard.lock();
    queueEntry->clientData->busy = false;
    backlogGuard.unlock();
  }
  catch (const std::exception &ex) {
    _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void TcpSocket::collectGarbage() {
  _lastGarbageCollection = HelperFunctions::getTime();

  std::lock_guard<std::mutex> clientsGuard(_clientsMutex);
  std::vector<int32_t> clientsToRemove;
  {
    for (auto &client: _clients) {
      if (!client.second->fileDescriptor || client.second->fileDescriptor->descriptor == -1) clientsToRemove.push_back(client.first);
    }
  }
  for (auto &client: clientsToRemove) {
    _clients.erase(client);
  }
}

void TcpSocket::collectGarbage(std::map<int32_t, PTcpClientData> &clients) {
  std::vector<int32_t> clientsToRemove;
  for (auto &client: clients) {
    if (!client.second->fileDescriptor || client.second->fileDescriptor->descriptor == -1) clientsToRemove.push_back(client.first);
  }
  for (auto &client: clientsToRemove) {
    clients.erase(client);
  }
}
// }}}

PFileDescriptor TcpSocket::bindAndReturnSocket(FileDescriptorManager &fileDescriptorManager, const std::string &address, const std::string &port, uint32_t connectionBacklogSize, std::string &listenAddress, int32_t &listenPort) {
  PFileDescriptor socketDescriptor;
  addrinfo hostInfo{};
  addrinfo *serverInfo = nullptr;

  static constexpr int32_t yes = 1;

  memset(&hostInfo, 0, sizeof(hostInfo));

  hostInfo.ai_family = AF_UNSPEC;
  hostInfo.ai_socktype = SOCK_STREAM;
  hostInfo.ai_flags = AI_PASSIVE;
  std::array<char, 101> buffer{};
  int32_t result = 0;
  if ((result = getaddrinfo(address.c_str(), port.c_str(), &hostInfo, &serverInfo)) != 0) {
    throw SocketOperationException("Error: Could not get address information: " + std::string(gai_strerror(result)));
  }

  bool bound = false;
  int32_t error = 0;
  for (struct addrinfo *info = serverInfo; info != 0; info = info->ai_next) {
    socketDescriptor = fileDescriptorManager.add(socket(info->ai_family, info->ai_socktype, info->ai_protocol));
    if (socketDescriptor->descriptor == -1) continue;
    if (!(fcntl(socketDescriptor->descriptor, F_GETFL) & O_NONBLOCK)) {
      if (fcntl(socketDescriptor->descriptor, F_SETFL, fcntl(socketDescriptor->descriptor, F_GETFL) | O_NONBLOCK) < 0) throw SocketOperationException("Error: Could not set socket options.");
    }
    if (setsockopt(socketDescriptor->descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t)) == -1) throw SocketOperationException("Error: Could not set socket options.");
    if (bind(socketDescriptor->descriptor.load(), info->ai_addr, info->ai_addrlen) == -1) {
      socketDescriptor.reset();
      error = errno;
      continue;
    }
    switch (info->ai_family) {
      case AF_INET: inet_ntop(info->ai_family, &((struct sockaddr_in *)info->ai_addr)->sin_addr, buffer.data(), buffer.size() - 1);
        buffer.back() = '\0';
        listenAddress = std::string(buffer.data());
        break;
      case AF_INET6: inet_ntop(info->ai_family, &((struct sockaddr_in6 *)info->ai_addr)->sin6_addr, buffer.data(), buffer.size() - 1);
        buffer.back() = '\0';
        listenAddress = std::string(buffer.data());
        break;
    }
    bound = true;
    break;
  }
  freeaddrinfo(serverInfo);
  if (!bound) {
    fileDescriptorManager.shutdown(socketDescriptor);
    socketDescriptor.reset();
    if (errno == EADDRINUSE) throw SocketAddressInUseException("Error: Could not start listening on port " + port + ": " + std::string(strerror(error)));
    else throw SocketBindException("Error: Could not start listening on port " + port + ": " + std::string(strerror(error)));
  } else if (socketDescriptor->descriptor == -1 || listen(socketDescriptor->descriptor, connectionBacklogSize) == -1) {
    fileDescriptorManager.shutdown(socketDescriptor);
    socketDescriptor.reset();
    throw SocketOperationException("Error: Server could not start listening on port " + port + ": " + std::string(strerror(errno)));
  }

  struct sockaddr_in addressInfo{};
  socklen_t addressInfoLength = sizeof(addressInfo);
  if (getsockname(socketDescriptor->descriptor, (struct sockaddr *)&addressInfo, &addressInfoLength) == -1) {
    fileDescriptorManager.shutdown(socketDescriptor);
    socketDescriptor.reset();
    throw SocketOperationException("Error: Could get port listening on: " + std::string(strerror(error)));
  }
  listenPort = addressInfo.sin_port;

  try {
    if (listenAddress == "0.0.0.0") listenAddress = Net::getMyIpAddress();
    else if (listenAddress == "::") {
      listenAddress = Net::getMyIp6Address();
    }
  }
  catch (const std::exception &ex) {
    fileDescriptorManager.shutdown(socketDescriptor);
    socketDescriptor.reset();
    throw;
  }
  return socketDescriptor;
}

void TcpSocket::initSsl() {
  std::lock_guard<std::mutex> certificateCredentialsGuard(_certificateCredentialsMutex);
  int32_t result = 0;

  std::vector<uint8_t> dhParamContent; //Don't move. This variable must exist as long as dhParamsData exists.
  gnutls_datum_t dhParamsData{};
  if (_isServer && (!_dhParamFile.empty() || !_dhParamData.empty())) {
    if (!_dhParamFile.empty()) {
      try {
        dhParamContent = Io::getUBinaryFileContent(_dhParamFile);
        dhParamContent.push_back(0); //gnutls_datum_t.data needs to be null terminated
      }
      catch (const std::exception &ex) {
        throw SocketSslException("Error: Could not load DH parameters: " + std::string(ex.what()));
      }
      dhParamsData.data = dhParamContent.data();
      dhParamsData.size = static_cast<unsigned int>(dhParamContent.size());
    } else {
      dhParamsData.data = (unsigned char *)_dhParamData.c_str();
      dhParamsData.size = static_cast<unsigned int>(_dhParamData.size());
    }
  }

  if (_certificates.empty()) {
    if (_requireClientCert && _isServer) {
      throw SocketSslException("No CA certificates specified (1).");
    }

    if (!_isServer) {
      gnutls_certificate_credentials_t x509Credentials = nullptr;
      if ((result = gnutls_certificate_allocate_credentials(&x509Credentials)) != GNUTLS_E_SUCCESS) {
        x509Credentials = nullptr;
        throw SocketSslException("Could not allocate certificate credentials: " + std::string(gnutls_strerror(result)));
      }

      if ((result = gnutls_certificate_set_x509_system_trust(x509Credentials)) < 0) {
        gnutls_certificate_free_credentials(x509Credentials);
        x509Credentials = nullptr;
        throw SocketSslException("Could not load system certificates: " + std::string(gnutls_strerror(result)));
      }
      _certificateCredentials["*"] = std::make_shared<CertificateCredentials>(x509Credentials, dhParamsData);
    }
  }

  for (auto &certificateInfo: _certificates) {
    gnutls_certificate_credentials_t x509Credentials = nullptr;
    if ((result = gnutls_certificate_allocate_credentials(&x509Credentials)) != GNUTLS_E_SUCCESS) {
      x509Credentials = nullptr;
      _certificateCredentials.clear();
      throw SocketSslException("Could not allocate certificate credentials: " + std::string(gnutls_strerror(result)));
    }

    int32_t caCertificateCount = 0;
    if (!certificateInfo.second->caData.empty()) {
      gnutls_datum_t caData;
      caData.data = (unsigned char *)certificateInfo.second->caData.c_str();
      caData.size = certificateInfo.second->caData.size();
      if ((result = gnutls_certificate_set_x509_trust_mem(x509Credentials, &caData, GNUTLS_X509_FMT_PEM)) < 0) {
        gnutls_certificate_free_credentials(x509Credentials);
        x509Credentials = nullptr;
        _certificateCredentials.clear();
        throw SocketSslException("Could not load trusted certificates: " + std::string(gnutls_strerror(result)));
      }
      caCertificateCount = result;
    } else if (!certificateInfo.second->caFile.empty()) {
      if ((result = gnutls_certificate_set_x509_trust_file(x509Credentials, certificateInfo.second->caFile.c_str(), GNUTLS_X509_FMT_PEM)) < 0) {
        gnutls_certificate_free_credentials(x509Credentials);
        x509Credentials = nullptr;
        _certificateCredentials.clear();
        throw SocketSslException("Could not load trusted certificates from \"" + certificateInfo.second->caFile + "\": " + std::string(gnutls_strerror(result)));
      }
      caCertificateCount = result;
    } else if (_requireClientCert && _isServer) {
      throw SocketSslException(R"(Client certificate authentication is enabled, but "caFile" and "caData" are not specified.)");
    } else if (!_isServer) {
      if ((result = gnutls_certificate_set_x509_system_trust(x509Credentials)) < 0) {
        gnutls_certificate_free_credentials(x509Credentials);
        x509Credentials = nullptr;
        throw SocketSslException("Could not load system certificates: " + std::string(gnutls_strerror(result)));
      }
    }

    if (caCertificateCount == 0 && ((_verifyCertificate && !_isServer) || (_requireClientCert && _isServer))) {
      gnutls_certificate_free_credentials(x509Credentials);
      x509Credentials = nullptr;
      _certificateCredentials.clear();
      throw SocketSslException("No CA certificates specified (2).");
    }

    if (_isServer) {
      if (!certificateInfo.second->certData.empty() && (certificateInfo.second->keyData && !certificateInfo.second->keyData->empty())) {
        gnutls_datum_t certData;
        certData.data = (unsigned char *)certificateInfo.second->certData.c_str();
        certData.size = certificateInfo.second->certData.size();

        gnutls_datum_t keyData;
        keyData.data = (unsigned char *)certificateInfo.second->keyData->data();
        keyData.size = certificateInfo.second->keyData->size();

        if ((result = gnutls_certificate_set_x509_key_mem(x509Credentials, &certData, &keyData, GNUTLS_X509_FMT_PEM)) < 0) {
          gnutls_certificate_free_credentials(x509Credentials);
          x509Credentials = nullptr;
          _certificateCredentials.clear();
          throw SocketSslException("Could not load server certificate or key file: " + std::string(gnutls_strerror(result)));
        }

        std::fill(certificateInfo.second->keyData->begin(), certificateInfo.second->keyData->end(), 0);
      } else if (!certificateInfo.second->certFile.empty() && !certificateInfo.second->keyFile.empty()) {
        if ((result = gnutls_certificate_set_x509_key_file(x509Credentials, certificateInfo.second->certFile.c_str(), certificateInfo.second->keyFile.c_str(), GNUTLS_X509_FMT_PEM)) < 0) {
          gnutls_certificate_free_credentials(x509Credentials);
          x509Credentials = nullptr;
          _certificateCredentials.clear();
          throw SocketSslException("Could not load certificate or key file from \"" + certificateInfo.second->certFile + "\" or \"" + certificateInfo.second->keyFile + "\": " + std::string(gnutls_strerror(result)));
        }
      } else if (_useSsl) {
        throw SocketSslException("SSL is enabled but no certificates are specified.");
      }
    } else {
      if (!certificateInfo.second->certData.empty() && (certificateInfo.second->keyData && !certificateInfo.second->keyData->empty())) {
        gnutls_datum_t clientCertData{};
        clientCertData.data = (unsigned char *)certificateInfo.second->certData.c_str();
        clientCertData.size = static_cast<unsigned int>(certificateInfo.second->certData.size());

        gnutls_datum_t clientKeyData{};
        clientKeyData.data = (unsigned char *)certificateInfo.second->keyData->data();
        clientKeyData.size = static_cast<unsigned int>(certificateInfo.second->keyData->size());

        if ((result = gnutls_certificate_set_x509_key_mem(x509Credentials, &clientCertData, &clientKeyData, GNUTLS_X509_FMT_PEM)) < 0) {
          gnutls_certificate_free_credentials(x509Credentials);
          x509Credentials = nullptr;
          _certificateCredentials.clear();
          throw SocketSslException("Could not load client certificate or key: " + std::string(gnutls_strerror(result)));
        }

        std::fill(clientKeyData.data, clientKeyData.data + clientKeyData.size, 0);
      } else if (!certificateInfo.second->certFile.empty() && !certificateInfo.second->keyFile.empty()) {
        if ((result = gnutls_certificate_set_x509_key_file(x509Credentials, certificateInfo.second->certFile.c_str(), certificateInfo.second->keyFile.c_str(), GNUTLS_X509_FMT_PEM)) < 0) {
          gnutls_certificate_free_credentials(x509Credentials);
          x509Credentials = nullptr;
          _certificateCredentials.clear();
          throw SocketSslException("Could not load client certificate and key from \"" + certificateInfo.second->certFile + "\" and \"" + certificateInfo.second->keyFile + "\": " + std::string(gnutls_strerror(result)));
        }
      }
    }

    _certificateCredentials[certificateInfo.first] = std::make_shared<CertificateCredentials>(x509Credentials, dhParamsData);
  }
}

void TcpSocket::initTlsPriorityCache() {
  if (_tlsPriorityCache) gnutls_priority_deinit(_tlsPriorityCache);
  if (_isServer) {
    int result = 0;
    if ((result = gnutls_priority_init(&_tlsPriorityCache, "NORMAL", nullptr)) != GNUTLS_E_SUCCESS) {
      _tlsPriorityCache = nullptr;
      throw SocketSslException("Error: Could not initialize cipher priorities: " + std::string(gnutls_strerror(result)));
    }
  }
}

void TcpSocket::open() {
  _connecting = true;
  if (!_socketDescriptor || _socketDescriptor->descriptor < 0) getSocketDescriptor();
  else if (!connected()) {
    close();
    getSocketDescriptor();
  }
  _connecting = false;
}

void TcpSocket::autoConnect() {
  if (!_autoConnect) return;
  _connecting = true;
  if (!_socketDescriptor || _socketDescriptor->descriptor < 0) getSocketDescriptor();
  else if (!connected()) {
    close();
    getSocketDescriptor();
  }
  _connecting = false;
}

void TcpSocket::close() {
  std::unique_lock<std::mutex> readGuard(_readMutex, std::defer_lock);
  std::unique_lock<std::mutex> writeGuard(_writeMutex, std::defer_lock);
  std::lock(readGuard, writeGuard);
  _bl->fileDescriptorManager.close(_socketDescriptor);
}

int32_t TcpSocket::proofread(char *buffer, int32_t bufferSize) {
  bool moreData = false;
  return proofread(buffer, bufferSize, moreData);
}

int32_t TcpSocket::proofread(char *buffer, int32_t bufferSize, bool &moreData) {
  moreData = false;

  if (!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
  std::unique_lock<std::mutex> readGuard(_readMutex);
  if (_autoConnect && !connected()) {
    readGuard.unlock();
    autoConnect();
    readGuard.lock();
  }

  int32_t bytesRead = 0;
  if (_socketDescriptor->tlsSession && gnutls_record_check_pending(_socketDescriptor->tlsSession) > 0) {
    do {
      bytesRead = static_cast<int32_t>(gnutls_record_recv(_socketDescriptor->tlsSession, buffer, bufferSize));
    } while (bytesRead == GNUTLS_E_INTERRUPTED || bytesRead == GNUTLS_E_AGAIN);
    if (bytesRead > 0) {
      if (gnutls_record_check_pending(_socketDescriptor->tlsSession) > 0) moreData = true;
      if (bytesRead > bufferSize) bytesRead = bufferSize;
      return bytesRead;
    }
  }

  timeval timeout{};
  int32_t seconds = _readTimeout / 1000000;
  timeout.tv_sec = seconds;
  timeout.tv_usec = _readTimeout - (1000000 * seconds);
  fd_set readFileDescriptor{};
  FD_ZERO(&readFileDescriptor);
  auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
  fileDescriptorGuard.lock();
  int32_t nfds = _socketDescriptor->descriptor + 1;
  if (nfds <= 0) {
    fileDescriptorGuard.unlock();
    readGuard.unlock();
    close();
    throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (1).");
  }
  FD_SET(_socketDescriptor->descriptor, &readFileDescriptor);
  fileDescriptorGuard.unlock();
  do {
    bytesRead = select(nfds, &readFileDescriptor, nullptr, nullptr, &timeout);
  } while (bytesRead == -1 && errno == EINTR);
  if (bytesRead == 0) {
    throw SocketTimeOutException("Reading from socket timed out (1).", SocketTimeOutException::SocketTimeOutType::selectTimeout);
  }
  if (bytesRead == -1) {
    readGuard.unlock();
    close();
    throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (2).");
  }
  if (_socketDescriptor->tlsSession) {
    do {
      bytesRead = gnutls_record_recv(_socketDescriptor->tlsSession, buffer, bufferSize);
    } while (bytesRead == GNUTLS_E_INTERRUPTED || bytesRead == GNUTLS_E_AGAIN);

    if (gnutls_record_check_pending(_socketDescriptor->tlsSession) > 0) moreData = true;
  } else {
    do {
      bytesRead = read(_socketDescriptor->descriptor, buffer, bufferSize);
    } while (bytesRead < 0 && (errno == EAGAIN || errno == EINTR));
  }
  if (bytesRead <= 0) {
    if (bytesRead == -1) {
      if (errno == ETIMEDOUT) throw SocketTimeOutException("Reading from socket timed out (2).", SocketTimeOutException::SocketTimeOutType::readTimeout);
      else {
        readGuard.unlock();
        close();
        throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (3): " + strerror(errno));
      }
    } else {
      readGuard.unlock();
      close();
      throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (3).");
    }
  }
  if (bytesRead > bufferSize) bytesRead = bufferSize;
  return bytesRead;
}

int32_t TcpSocket::proofwrite(const std::shared_ptr<std::vector<char>> &data) {
  {
    std::unique_lock<std::mutex> writeGuard(_writeMutex);
    if (!connected()) {
      writeGuard.unlock();
      autoConnect();
    }
  }
  if (!data || data->empty()) return 0;
  return proofwrite(*data);
}

int32_t TcpSocket::proofwrite(const std::vector<char> &data) {
  if (!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
  std::unique_lock<std::mutex> writeGuard(_writeMutex);
  if (!connected()) {
    writeGuard.unlock();
    autoConnect();
    writeGuard.lock();
  }
  if (data.empty()) {
    return 0;
  }
  if (data.size() > 104857600) {
    throw SocketDataLimitException("Data size is larger than 100 MiB.");
  }

  int32_t totalBytesWritten = 0;
  while (totalBytesWritten < (signed)data.size()) {
    timeval timeout{};
    int32_t seconds = _writeTimeout / 1000000;
    timeout.tv_sec = seconds;
    timeout.tv_usec = _writeTimeout - (1000000 * seconds);
    fd_set writeFileDescriptor;
    FD_ZERO(&writeFileDescriptor);
    auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
    fileDescriptorGuard.lock();
    int32_t nfds = _socketDescriptor->descriptor + 1;
    if (nfds <= 0) {
      fileDescriptorGuard.unlock();
      writeGuard.unlock();
      close();
      throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (4).");
    }
    FD_SET(_socketDescriptor->descriptor, &writeFileDescriptor);
    fileDescriptorGuard.unlock();
    int32_t readyFds = 0;
    do {
      readyFds = select(nfds, nullptr, &writeFileDescriptor, nullptr, &timeout);
    } while (readyFds == -1 && errno == EINTR);
    if (readyFds == 0) {
      throw SocketTimeOutException("Writing to socket timed out.");
    }
    if (readyFds != 1) {
      writeGuard.unlock();
      close();
      throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (5).");
    }

    int32_t bytesToSend = data.size() - totalBytesWritten;
    int32_t bytesWritten = 0;
    if (_socketDescriptor->tlsSession) {
      do {
        bytesWritten = gnutls_record_send(_socketDescriptor->tlsSession, &data.at(totalBytesWritten), bytesToSend);
      } while (bytesWritten == GNUTLS_E_INTERRUPTED || bytesWritten == GNUTLS_E_AGAIN);
    } else {
      do {
        bytesWritten = send(_socketDescriptor->descriptor, &data.at(totalBytesWritten), bytesToSend, MSG_NOSIGNAL);
      } while (bytesWritten == -1 && (errno == EAGAIN || errno == EINTR));
    }
    if (bytesWritten <= 0) {
      writeGuard.unlock();
      close();
      writeGuard.lock();
      if (_socketDescriptor->tlsSession) {
        throw SocketOperationException(gnutls_strerror(bytesWritten));
      } else {
        throw SocketOperationException(strerror(errno));
      }
    }
    totalBytesWritten += bytesWritten;
  }
  return totalBytesWritten;
}

int32_t TcpSocket::proofwrite(const char *buffer, int32_t bytesToWrite) {
  if (!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
  std::unique_lock<std::mutex> writeGuard(_writeMutex);
  if (!connected()) {
    writeGuard.unlock();
    autoConnect();
    writeGuard.lock();
  }
  if (bytesToWrite <= 0) {
    return 0;
  }
  if (bytesToWrite > 104857600) {
    throw SocketDataLimitException("Data size is larger than 100 MiB.");
  }

  int32_t totalBytesWritten = 0;
  while (totalBytesWritten < bytesToWrite) {
    timeval timeout{};
    int32_t seconds = _writeTimeout / 1000000;
    timeout.tv_sec = seconds;
    timeout.tv_usec = _writeTimeout - (1000000 * seconds);
    fd_set writeFileDescriptor;
    FD_ZERO(&writeFileDescriptor);
    auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
    fileDescriptorGuard.lock();
    int32_t nfds = _socketDescriptor->descriptor + 1;
    if (nfds <= 0) {
      fileDescriptorGuard.unlock();
      writeGuard.unlock();
      close();
      throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (4).");
    }
    FD_SET(_socketDescriptor->descriptor, &writeFileDescriptor);
    fileDescriptorGuard.unlock();
    int32_t readyFds = 0;
    do {
      readyFds = select(nfds, nullptr, &writeFileDescriptor, nullptr, &timeout);
    } while (readyFds == -1 && errno == EINTR);
    if (readyFds == 0) {
      throw SocketTimeOutException("Writing to socket timed out.");
    }
    if (readyFds != 1) {
      writeGuard.unlock();
      close();
      throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (5).");
    }

    int32_t bytesToSend = bytesToWrite - totalBytesWritten;
    int32_t bytesWritten = 0;
    if (_socketDescriptor->tlsSession) {
      do {
        bytesWritten = gnutls_record_send(_socketDescriptor->tlsSession, buffer + totalBytesWritten, bytesToSend);
      } while (bytesWritten == GNUTLS_E_INTERRUPTED || bytesWritten == GNUTLS_E_AGAIN);
    } else {
      do {
        bytesWritten = send(_socketDescriptor->descriptor, buffer + totalBytesWritten, bytesToSend, MSG_NOSIGNAL);
      } while (bytesWritten == -1 && (errno == EAGAIN || errno == EINTR));
    }
    if (bytesWritten <= 0) {
      writeGuard.unlock();
      close();
      writeGuard.lock();
      if (_socketDescriptor->tlsSession) {
        throw SocketOperationException(gnutls_strerror(bytesWritten));
      } else {
        throw SocketOperationException(strerror(errno));
      }
    }
    totalBytesWritten += bytesWritten;
  }
  return totalBytesWritten;
}

int32_t TcpSocket::proofwrite(const std::string &data) {
  if (!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
  std::unique_lock<std::mutex> writeGuard(_writeMutex);
  if (!connected()) {
    writeGuard.unlock();
    autoConnect();
    writeGuard.lock();
  }
  if (data.empty()) {
    return 0;
  }
  if (data.size() > 104857600) {
    throw SocketDataLimitException("Data size is larger than 100 MiB.");
  }

  int32_t totalBytesWritten = 0;
  while (totalBytesWritten < (signed)data.size()) {
    timeval timeout{};
    int32_t seconds = _writeTimeout / 1000000;
    timeout.tv_sec = seconds;
    timeout.tv_usec = _writeTimeout - (1000000 * seconds);
    fd_set writeFileDescriptor;
    FD_ZERO(&writeFileDescriptor);
    auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
    fileDescriptorGuard.lock();
    int32_t nfds = _socketDescriptor->descriptor + 1;
    if (nfds <= 0) {
      fileDescriptorGuard.unlock();
      writeGuard.unlock();
      close();
      throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (6).");
    }
    FD_SET(_socketDescriptor->descriptor, &writeFileDescriptor);
    fileDescriptorGuard.unlock();
    int32_t readyFds = 0;
    do {
      readyFds = select(nfds, nullptr, &writeFileDescriptor, nullptr, &timeout);
    } while (readyFds == -1 && errno == EINTR);
    if (readyFds == 0) {
      throw SocketTimeOutException("Writing to socket timed out.");
    }
    if (readyFds != 1) {
      writeGuard.unlock();
      close();
      throw SocketClosedException("Connection to client number " + std::to_string(_socketDescriptor->id) + " closed (7).");
    }

    int32_t bytesToSend = data.size() - totalBytesWritten;
    int32_t bytesWritten = 0;
    if (_socketDescriptor->tlsSession) {
      do {
        bytesWritten = gnutls_record_send(_socketDescriptor->tlsSession, &data.at(totalBytesWritten), bytesToSend);
      } while (bytesWritten == GNUTLS_E_INTERRUPTED || bytesWritten == GNUTLS_E_AGAIN);
    } else {
      do {
        bytesWritten = send(_socketDescriptor->descriptor, &data.at(totalBytesWritten), bytesToSend, MSG_NOSIGNAL);
      } while (bytesWritten == -1 && (errno == EAGAIN || errno == EINTR));
    }
    if (bytesWritten <= 0) {
      writeGuard.unlock();
      close();
      writeGuard.lock();
      if (_socketDescriptor->tlsSession) {
        throw SocketOperationException(gnutls_strerror(bytesWritten));
      } else {
        throw SocketOperationException(strerror(errno));
      }
    }
    totalBytesWritten += bytesWritten;
  }
  return totalBytesWritten;
}

bool TcpSocket::connected() {
  if (!_socketDescriptor || _socketDescriptor->descriptor == -1 || _connecting) return false;
  char buffer[1];
  if (recv(_socketDescriptor->descriptor, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT) == -1 && errno != EWOULDBLOCK && errno != EAGAIN && errno != EINTR) return false;
  return true;
}

double TcpSocket::GetPacketsPerMinuteReceived() {
  double result = 0.0;
  for (auto &element: average_packets_per_minute_received_) {
    result += element.last_output;
  }
  return result;
}

double TcpSocket::GetPacketsPerMinuteSent() {
  double result = 0.0;
  for (auto &element: average_packets_per_minute_sent_) {
    result += element.last_output;
  }
  return result;
}

void TcpSocket::getSocketDescriptor() {
  std::unique_lock<std::mutex> readGuard(_readMutex, std::defer_lock);
  std::unique_lock<std::mutex> writeGuard(_writeMutex, std::defer_lock);
  std::lock(readGuard, writeGuard);
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Calling getFileDescriptor...");
  _bl->fileDescriptorManager.shutdown(_socketDescriptor);

  try {
    getConnection();
    if (!_socketDescriptor || _socketDescriptor->descriptor < 0) {
      throw SocketOperationException("Could not connect to server.");
    }

    if (_useSsl) getSsl();
  }
  catch (const std::exception &ex) {
    throw;
  }
}

void TcpSocket::getSsl() {
  if (!_socketDescriptor || _socketDescriptor->descriptor < 0) throw SocketSslException("Could not connect to server using SSL. File descriptor is invalid.");
  int32_t result = 0;
  //Disable TCP Nagle algorithm to improve performance
  const int32_t value = 1;
  if ((result = setsockopt(_socketDescriptor->descriptor, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value))) < 0) {
    _bl->fileDescriptorManager.shutdown(_socketDescriptor);
    throw SocketSslException("Could not disable Nagle algorithm.");
  }
  if ((result = gnutls_init(&_socketDescriptor->tlsSession, GNUTLS_CLIENT)) != GNUTLS_E_SUCCESS) {
    _bl->fileDescriptorManager.shutdown(_socketDescriptor);
    throw SocketSslException("Could not initialize TLS session: " + std::string(gnutls_strerror(result)));
  }
  if (!_socketDescriptor->tlsSession) throw SocketSslException("Could not initialize TLS session.");
  if ((result = gnutls_priority_set_direct(_socketDescriptor->tlsSession, "NORMAL", nullptr)) != GNUTLS_E_SUCCESS) {
    _bl->fileDescriptorManager.shutdown(_socketDescriptor);
    throw SocketSslException("Could not set cipher priorities: " + std::string(gnutls_strerror(result)));
  }

  {
    std::lock_guard<std::mutex> certificateCredentialsGuard(_certificateCredentialsMutex);
    if (_certificateCredentials.empty()) {
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketSslException("Could not connect to server using SSL. Certificate credentials are not initialized. Look for previous error messages.");
    }
    //Copy credentials so they can be replaced at any time.
    _currentClientCertificateCredentials = _certificateCredentials.begin()->second;
    if ((result = gnutls_credentials_set(_socketDescriptor->tlsSession, GNUTLS_CRD_CERTIFICATE, _currentClientCertificateCredentials->get())) != GNUTLS_E_SUCCESS) {
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketSslException("Could not set trusted certificates: " + std::string(gnutls_strerror(result)));
    }
  }

  gnutls_transport_set_ptr(_socketDescriptor->tlsSession, (gnutls_transport_ptr_t)(uintptr_t)_socketDescriptor->descriptor);
  if (!_hostname.empty()) {
    if ((result = gnutls_server_name_set(_socketDescriptor->tlsSession, GNUTLS_NAME_DNS, _hostname.c_str(), _hostname.length())) != GNUTLS_E_SUCCESS) {
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketSslException("Could not set server's hostname: " + std::string(gnutls_strerror(result)));
    }
  }
  do {
    result = gnutls_handshake(_socketDescriptor->tlsSession);
  } while (result < 0 && gnutls_error_is_fatal(result) == 0);
  if (result != GNUTLS_E_SUCCESS) {
    _bl->fileDescriptorManager.shutdown(_socketDescriptor);
    throw SocketSslHandshakeFailedException("Error during TLS handshake: " + std::string(gnutls_strerror(result)));
  }

  //Now verify the certificate
  uint32_t serverCertChainLength = 0;
  const gnutls_datum_t *const serverCertChain = gnutls_certificate_get_peers(_socketDescriptor->tlsSession, &serverCertChainLength);
  if (!serverCertChain || serverCertChainLength == 0) {
    _bl->fileDescriptorManager.shutdown(_socketDescriptor);
    throw SocketSslException("Could not get server certificate.");
  }
  uint32_t status = (uint32_t)-1;
  if ((result = gnutls_certificate_verify_peers2(_socketDescriptor->tlsSession, &status)) != GNUTLS_E_SUCCESS) {
    _bl->fileDescriptorManager.shutdown(_socketDescriptor);
    throw SocketSslException("Could not verify server certificate: " + std::string(gnutls_strerror(result)));
  }
  if (status > 0) {
    if (_verifyCertificate) {
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketSslException("Error verifying server certificate (Code: " + std::to_string(status) + "): " + HelperFunctions::getGNUTLSCertVerificationError(status));
    } else if ((status & GNUTLS_CERT_REVOKED) || (status & GNUTLS_CERT_INSECURE_ALGORITHM) || (status & GNUTLS_CERT_NOT_ACTIVATED) || (status & GNUTLS_CERT_EXPIRED)) {
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketSslException("Error verifying server certificate (Code: " + std::to_string(status) + "): " + HelperFunctions::getGNUTLSCertVerificationError(status));
    } else _bl->out.printWarning("Warning: Certificate verification failed (Code: " + std::to_string(status) + "): " + HelperFunctions::getGNUTLSCertVerificationError(status));
  } else //Verify hostname
  {
    gnutls_x509_crt_t serverCert = nullptr;
    if ((result = gnutls_x509_crt_init(&serverCert)) != GNUTLS_E_SUCCESS) {
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketSslException("Could not initialize server certificate structure: " + std::string(gnutls_strerror(result)));
    }
    //The peer certificate is the first certificate in the list
    if ((result = gnutls_x509_crt_import(serverCert, serverCertChain, GNUTLS_X509_FMT_DER)) != GNUTLS_E_SUCCESS) {
      gnutls_x509_crt_deinit(serverCert);
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketSslException("Could not import server certificate: " + std::string(gnutls_strerror(result)));
    }
    if (_verifyHostname && (result = gnutls_x509_crt_check_hostname(serverCert, _verificationHostname.c_str())) == 0) {
      gnutls_x509_crt_deinit(serverCert);
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketSslException("Server's hostname does not match the server certificate and hostname verification is enabled.");
    }
    gnutls_x509_crt_deinit(serverCert);
  }
}

void TcpSocket::getConnection() {
  _socketDescriptor.reset();
  if (_hostname.empty()) throw SocketInvalidParametersException("Hostname is empty");
  if (_port.empty()) throw SocketInvalidParametersException("Port is empty");
  if (_connectionRetries < 1) _connectionRetries = 1;
  else if (_connectionRetries > 10) _connectionRetries = 10;

  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Connecting to host " + _hostname + " on port " + _port + (_useSsl ? " using SSL" : "") + "...");

  for (int32_t i = 0; i < _connectionRetries; ++i) {
    struct addrinfo *serverInfo = nullptr;
    struct addrinfo hostInfo{};

    hostInfo.ai_family = AF_UNSPEC;
    hostInfo.ai_socktype = SOCK_STREAM;

    int32_t result = getaddrinfo(_hostname.c_str(), _port.c_str(), &hostInfo, &serverInfo);
    if (result != 0) {
      freeaddrinfo(serverInfo);
      throw SocketOperationException("Could not get address information: " + std::string(gai_strerror(result)));
    }

    char ipStringBuffer[INET6_ADDRSTRLEN + 1];
    if (serverInfo->ai_family == AF_INET) {
      auto s = (struct sockaddr_in *)serverInfo->ai_addr;
      inet_ntop(AF_INET, &s->sin_addr, ipStringBuffer, sizeof(ipStringBuffer));
    } else { // AF_INET6
      auto s = (struct sockaddr_in6 *)serverInfo->ai_addr;
      inet_ntop(AF_INET6, &s->sin6_addr, ipStringBuffer, sizeof(ipStringBuffer));
    }
    ipStringBuffer[INET6_ADDRSTRLEN] = '\0';
    _ipAddress = std::string(ipStringBuffer);

    _socketDescriptor = _bl->fileDescriptorManager.add(socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol));
    if (!_socketDescriptor || _socketDescriptor->descriptor == -1) {
      freeaddrinfo(serverInfo);
      throw SocketOperationException("Could not create socket for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
    }
    int32_t optValue = 1;
    if (setsockopt(_socketDescriptor->descriptor, SOL_SOCKET, SO_KEEPALIVE, (void *)&optValue, sizeof(int32_t)) == -1) {
      freeaddrinfo(serverInfo);
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
    }
    optValue = 30;
    //Set number of seconds a connection needs to be idle before sending keep-alive probes
    //Don't use SOL_TCP, as this constant doesn't exists in BSD. Also don't use getprotobyname() as this returns a nullptr on some systems.
    if (setsockopt(_socketDescriptor->descriptor, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&optValue, sizeof(int32_t)) == -1) {
      freeaddrinfo(serverInfo);
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
    }
    optValue = 4;
    //Set the maximum number of keep-alive probes before giving up and closing the connection
    if (setsockopt(_socketDescriptor->descriptor, IPPROTO_TCP, TCP_KEEPCNT, (void *)&optValue, sizeof(int32_t)) == -1) {
      freeaddrinfo(serverInfo);
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
    }
    optValue = 15;
    //Set number of seconds between keep-alive probes
    if (setsockopt(_socketDescriptor->descriptor, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&optValue, sizeof(int32_t)) == -1) {
      freeaddrinfo(serverInfo);
      _bl->fileDescriptorManager.shutdown(_socketDescriptor);
      throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
    }

    if (!(fcntl(_socketDescriptor->descriptor, F_GETFL) & O_NONBLOCK)) {
      if (fcntl(_socketDescriptor->descriptor, F_SETFL, fcntl(_socketDescriptor->descriptor, F_GETFL) | O_NONBLOCK) < 0) {
        freeaddrinfo(serverInfo);
        _bl->fileDescriptorManager.shutdown(_socketDescriptor);
        throw SocketOperationException("Could not set socket options for server " + _ipAddress + " on port " + _port + ": " + strerror(errno));
      }
    }

    int32_t connectResult = connect(_socketDescriptor->descriptor, serverInfo->ai_addr, serverInfo->ai_addrlen);
    int errorNumber = errno;
    freeaddrinfo(serverInfo);
    if (connectResult == -1 && errorNumber != EINPROGRESS) {
      if (i < _connectionRetries - 1) {
        _bl->fileDescriptorManager.shutdown(_socketDescriptor);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        continue;
      } else {
        _bl->fileDescriptorManager.shutdown(_socketDescriptor);
        throw SocketTimeOutException("Connecting to server " + _ipAddress + " on port " + _port + " timed out: " + strerror(errno));
      }
    }

    if (connectResult != 0) //We have to wait for the connection
    {
      pollfd pollstruct
          {
              (int)_socketDescriptor->descriptor,
              (short)(POLLIN | POLLOUT | POLLERR),
              (short)0
          };

      int32_t pollResult = poll(&pollstruct, 1, (int)(_readTimeout / 1000));
      if (pollResult < 0 || (pollstruct.revents & POLLERR)) {
        if (i < _connectionRetries - 1) {
          _bl->fileDescriptorManager.shutdown(_socketDescriptor);
          std::this_thread::sleep_for(std::chrono::milliseconds(200));
          continue;
        } else {
          _bl->fileDescriptorManager.shutdown(_socketDescriptor);
          throw SocketTimeOutException("Could not connect to server " + _ipAddress + " on port " + _port + ". Poll failed with error code: " + std::to_string(pollResult) + ".");
        }
      } else if (pollResult > 0) {
        socklen_t resultLength = sizeof(connectResult);
        if (getsockopt(_socketDescriptor->descriptor, SOL_SOCKET, SO_ERROR, &connectResult, &resultLength) < 0) {
          _bl->fileDescriptorManager.shutdown(_socketDescriptor);
          throw SocketOperationException("Could not connect to server " + _ipAddress + " on port " + _port + ": " + strerror(errno) + ".");
        }
        break;
      } else if (pollResult == 0) {
        if (i < _connectionRetries - 1) {
          _bl->fileDescriptorManager.shutdown(_socketDescriptor);
          continue;
        } else {
          _bl->fileDescriptorManager.shutdown(_socketDescriptor);
          throw SocketTimeOutException("Connecting to server " + _ipAddress + " on port " + _port + " timed out.");
        }
      }
    }
  }
  if (_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Connected to host " + _hostname + " on port " + _port + ". Client number is: " + std::to_string(_socketDescriptor->id));
}

}
