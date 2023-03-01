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

#include "Hgdc.h"
#include "../BaseLib.h"

namespace BaseLib {

Hgdc::Hgdc(SharedObjects *bl, uint16_t port) : IQueue(bl, 1, 100) {
  _bl = bl;
  _port = port;

  signal(SIGPIPE, SIG_IGN);

  _out.setPrefix("HGDC: ");
  _binaryRpc.reset(new BaseLib::Rpc::BinaryRpc(bl));
  _rpcEncoder.reset(new BaseLib::Rpc::RpcEncoder(bl, true, true));
  _rpcDecoder.reset(new BaseLib::Rpc::RpcDecoder(bl, false, false));
}

Hgdc::~Hgdc() {
  stop();
}

void Hgdc::start() {
  try {
    stop();

    if (_port == 0) {
      _out.printError("Error: Cannot connect to Homegear Daisy Chain Connector, because port is invalid.");
      return;
    }

    startQueue(0, false, 2);

    C1Net::TcpSocketInfo tcp_socket_info;
    tcp_socket_info.read_timeout = 5000;
    tcp_socket_info.write_timeout = 5000;
    tcp_socket_info.log_callback = std::bind(&Hgdc::Log, this, std::placeholders::_1, std::placeholders::_2);;

    C1Net::TcpSocketHostInfo tcp_socket_host_info;
    tcp_socket_host_info.host = "localhost";
    tcp_socket_host_info.port = _port;
    tcp_socket_host_info.auto_connect = false;
    tcp_socket_host_info.connection_retries = 2;

    _tcpSocket = std::make_unique<C1Net::TcpSocket>(tcp_socket_info, tcp_socket_host_info);

    try {
      _tcpSocket->Open();
      if (_tcpSocket->Connected()) {
        _out.printInfo("Info: Successfully connected.");
        _stopped = false;
        std::shared_ptr<QueueEntry> queueEntry = std::make_shared<QueueEntry>();
        queueEntry->method = "reconnected";
        auto baseQueueEntry = static_cast<std::shared_ptr<IQueueEntry>>(queueEntry);
        enqueue(0, baseQueueEntry);
      }
    }
    catch (const std::exception &ex) {
      _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    _stopCallbackThread = false;
    _bl->threadManager.start(_listenThread, true, &Hgdc::listen, this);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Hgdc::stop() {
  try {
    stopQueue(0);
    _stopCallbackThread = true;
    _bl->threadManager.join(_listenThread);
    _stopped = true;
    if (_tcpSocket) _tcpSocket->Shutdown();
    _tcpSocket.reset();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

int32_t Hgdc::registerPacketReceivedEventHandler(int64_t familyId, std::function<void(int64_t, const std::string &, const std::vector<uint8_t> &)> value) {
  try {
    int32_t eventHandlerId = -1;
    std::lock_guard<std::mutex> eventHandlersGuard(_packetReceivedEventHandlersMutex);
    while (eventHandlerId == -1) eventHandlerId = _currentEventHandlerId++;

    _packetReceivedEventHandlers[familyId].emplace_back(std::make_pair(eventHandlerId, std::move(value)));

    return eventHandlerId;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return -1;
}

void Hgdc::unregisterPacketReceivedEventHandler(int32_t eventHandlerId) {
  try {
    if (eventHandlerId == -1) return;

    std::lock_guard<std::mutex> eventHandlersGuard(_packetReceivedEventHandlersMutex);
    for (auto &eventHandlers: _packetReceivedEventHandlers) {
      for (auto &eventHandler: eventHandlers.second) {
        if (eventHandler.first == eventHandlerId) {
          _packetReceivedEventHandlers.erase(eventHandler.first);
          break;
        }
      }
    }
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

int32_t Hgdc::registerModuleUpdateEventHandler(std::function<void(const BaseLib::PVariable &)> value) {
  try {
    int32_t eventHandlerId = -1;
    std::lock_guard<std::mutex> eventHandlersGuard(_moduleUpdateEventHandlersMutex);
    while (eventHandlerId == -1) eventHandlerId = _currentEventHandlerId++;

    _moduleUpdateEventHandlers.emplace(eventHandlerId, std::move(value));

    return eventHandlerId;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return -1;
}

void Hgdc::unregisterModuleUpdateEventHandler(int32_t eventHandlerId) {
  try {
    if (eventHandlerId == -1) return;

    std::lock_guard<std::mutex> eventHandlersGuard(_moduleUpdateEventHandlersMutex);
    _moduleUpdateEventHandlers.erase(eventHandlerId);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

int32_t Hgdc::registerReconnectedEventHandler(std::function<void()> value) {
  try {
    int32_t eventHandlerId = -1;
    std::lock_guard<std::mutex> eventHandlersGuard(_reconnectedEventHandlersMutex);
    while (eventHandlerId == -1) eventHandlerId = _currentEventHandlerId++;

    _reconnectedEventHandlers.emplace(eventHandlerId, std::move(value));

    return eventHandlerId;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return -1;
}

void Hgdc::unregisterReconnectedEventHandler(int32_t eventHandlerId) {
  try {
    if (eventHandlerId == -1) return;

    std::lock_guard<std::mutex> eventHandlersGuard(_reconnectedEventHandlersMutex);
    _reconnectedEventHandlers.erase(eventHandlerId);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Hgdc::Log(uint32_t log_level, const std::string &message) {
  _out.printMessage("Core TCP client: " + message, (int32_t)log_level, log_level < 3);
}

void Hgdc::listen() {
  try {
    std::vector<uint8_t> buffer(4096);
    std::size_t processedBytes = 0;
    bool more_data = false;
    while (!_stopCallbackThread) {
      try {
        if (_stopped || !_tcpSocket->Connected()) {
          if (_stopCallbackThread) return;
          if (_stopped) _out.printWarning("Warning: Connection to device closed. Trying to reconnect...");
          _tcpSocket->Shutdown();
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          if (_stopCallbackThread) return;
          _tcpSocket->Shutdown();
          if (_tcpSocket->Connected()) {
            _out.printInfo("Info: Successfully connected.");
            _stopped = false;
            std::shared_ptr<QueueEntry> queueEntry = std::make_shared<QueueEntry>();
            queueEntry->method = "reconnected";
            auto baseQueueEntry = static_cast<std::shared_ptr<IQueueEntry>>(queueEntry);
            enqueue(0, baseQueueEntry);
          }
          continue;
        }

        std::size_t bytesRead = 0;
        try {
          bytesRead = _tcpSocket->Read(buffer.data(), buffer.size(), more_data);
        }
        catch (BaseLib::SocketTimeOutException &ex) {
          continue;
        }
        if (bytesRead <= 0) continue;
        if (bytesRead > buffer.size()) bytesRead = buffer.size();

        if (_bl->debugLevel >= 5) _out.printDebug("Debug: TCP packet received: " + BaseLib::HelperFunctions::getHexString(buffer.data(), bytesRead));

        processedBytes = 0;
        while (processedBytes < bytesRead) {
          try {
            processedBytes += _binaryRpc->process((char *)buffer.data() + processedBytes, bytesRead - processedBytes);
            if (_binaryRpc->isFinished()) {
              if (_binaryRpc->getType() == BaseLib::Rpc::BinaryRpc::Type::request) {
                std::shared_ptr<QueueEntry> queueEntry = std::make_shared<QueueEntry>();
                auto baseQueueEntry = static_cast<std::shared_ptr<IQueueEntry>>(queueEntry);
                queueEntry->parameters = _rpcDecoder->decodeRequest(_binaryRpc->getData(), queueEntry->method);
                enqueue(0, baseQueueEntry);

                BaseLib::PVariable response = std::make_shared<BaseLib::Variable>();
                std::vector<uint8_t> data;
                _rpcEncoder->encodeResponse(response, data);
                _tcpSocket->Send(data);
              } else if (_binaryRpc->getType() == BaseLib::Rpc::BinaryRpc::Type::response) {
                auto response = _rpcDecoder->decodeResponse(_binaryRpc->getData());

                if (response->arrayValue->size() < 3) {
                  _out.printError("Error: Response has wrong array size.");
                } else {
                  pthread_t threadId = response->arrayValue->at(0)->integerValue64;
                  int32_t packetId = response->arrayValue->at(1)->integerValue;

                  std::lock_guard<std::mutex> requestInfoGuard(_requestInfoMutex);
                  auto requestIterator = _requestInfo.find(threadId);
                  if (requestIterator != _requestInfo.end()) {
                    std::unique_lock<std::mutex> waitLock(requestIterator->second->waitMutex);

                    {
                      std::lock_guard<std::mutex> responseGuard(_rpcResponsesMutex);
                      auto responseIterator = _rpcResponses[threadId].find(packetId);
                      if (responseIterator != _rpcResponses[threadId].end()) {
                        PRpcResponse element = responseIterator->second;
                        if (element) {
                          element->response = response;
                          element->packetId = packetId;
                          element->finished = true;
                        }
                      }
                    }

                    waitLock.unlock();
                    requestIterator->second->conditionVariable.notify_all();
                  }
                }
              }
              _binaryRpc->reset();
            }
          }
          catch (BaseLib::Rpc::BinaryRpcException &ex) {
            _binaryRpc->reset();
            _out.printError("Error processing packet: " + std::string(ex.what()));
          }
        }
      }
      catch (const std::exception &ex) {
        _stopped = true;
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
      }
    }
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

PVariable Hgdc::invoke(const std::string &methodName, const PArray &parameters, int32_t timeout) {
  try {
    auto threadId = pthread_self();
    std::unique_lock<std::mutex> requestInfoGuard(_requestInfoMutex);
    PRequestInfo requestInfo = _requestInfo.emplace(std::piecewise_construct, std::make_tuple(threadId), std::make_tuple(std::make_shared<RequestInfo>())).first->second;
    requestInfoGuard.unlock();

    int32_t packetId;
    {
      std::lock_guard<std::mutex> packetIdGuard(_packetIdMutex);
      packetId = _currentPacketId++;
    }
    auto array = std::make_shared<BaseLib::Array>();
    array->reserve(3);
    array->emplace_back(std::move(std::make_shared<BaseLib::Variable>((int64_t)threadId)));
    array->emplace_back(std::move(std::make_shared<BaseLib::Variable>(packetId)));
    array->emplace_back(std::move(std::make_shared<BaseLib::Variable>(parameters)));
    std::vector<uint8_t> encodedPacket;
    _rpcEncoder->encodeRequest(methodName, array, encodedPacket);

    PRpcResponse response;
    {
      std::lock_guard<std::mutex> responseGuard(_rpcResponsesMutex);
      auto result = _rpcResponses[threadId].emplace(packetId, std::make_shared<RpcResponse>());
      if (result.second) response = result.first->second;
    }
    if (!response) {
      _out.printError("Critical: Could not insert response struct into map.");
      return BaseLib::Variable::createError(-32500, "Unknown application error.");
    }

    int32_t i = 0;
    for (i = 0; i < 5; i++) {
      try {
        _tcpSocket->Send(encodedPacket);
        break;
      }
      catch (const BaseLib::SocketOperationException &ex) {
        _out.printError("Error: " + std::string(ex.what()));
        if (i == 5) {
          std::lock_guard<std::mutex> responseGuard(_rpcResponsesMutex);
          _rpcResponses[threadId].erase(packetId);
          if (_rpcResponses[threadId].empty()) _rpcResponses.erase(threadId);
          return BaseLib::Variable::createError(-32500, ex.what());
        }
      }
    }

    auto startTime = BaseLib::HelperFunctions::getTime();
    std::unique_lock<std::mutex> waitLock(requestInfo->waitMutex);
    while (!requestInfo->conditionVariable.wait_for(waitLock, std::chrono::milliseconds(1000), [&] {
      return response->finished || _stopped || (timeout > 0 && BaseLib::HelperFunctions::getTime() - startTime > timeout);
    }));

    BaseLib::PVariable result;
    if (!response->finished || response->response->arrayValue->size() != 3 || response->packetId != packetId) {
      _out.printError("Error: No response received to RPC request. Method: " + methodName);
      result = BaseLib::Variable::createError(-1, "No response received.");
    } else result = response->response->arrayValue->at(2);

    {
      std::lock_guard<std::mutex> responseGuard(_rpcResponsesMutex);
      _rpcResponses[threadId].erase(packetId);
      if (_rpcResponses[threadId].empty()) _rpcResponses.erase(threadId);
    }

    {
      requestInfoGuard.lock();
      _requestInfo.erase(threadId);
    }

    return result;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return BaseLib::Variable::createError(-32500, "Unknown application error. See log for more details.");
}

bool Hgdc::sendPacket(const std::string &serialNumber, const std::vector<uint8_t> &packet) {
  try {
    if (!_tcpSocket || !_tcpSocket->Connected()) return false;

    BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<BaseLib::Variable>(serialNumber));
    parameters->push_back(std::make_shared<BaseLib::Variable>(packet));

    auto result = invoke("sendPacket", parameters);
    if (result->errorStruct) {
      _out.printError("Error sending packet " + BaseLib::HelperFunctions::getHexString(packet) + ": " + result->structValue->at("faultString")->stringValue);
      return false;
    }

    return true;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Hgdc::sendPacket(const std::string &serialNumber, const std::vector<char> &packet) {
  try {
    if (!_tcpSocket || !_tcpSocket->Connected()) return false;

    BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<BaseLib::Variable>(serialNumber));
    parameters->push_back(std::make_shared<BaseLib::Variable>(packet));

    auto result = invoke("sendPacket", parameters);
    if (result->errorStruct) {
      _out.printError("Error sending packet " + BaseLib::HelperFunctions::getHexString(packet) + ": " + result->structValue->at("faultString")->stringValue);
      return false;
    }

    return true;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Hgdc::moduleReset(const std::string &serialNumber) {
  try {
    if (!_tcpSocket || !_tcpSocket->Connected()) return false;

    BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
    parameters->push_back(std::make_shared<BaseLib::Variable>(serialNumber));

    auto result = invoke("moduleModuleReset", parameters);
    if (result->errorStruct) {
      _out.printError("Error resetting module: " + result->structValue->at("faultString")->stringValue);
      return false;
    }

    return true;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Hgdc::setMode(const std::string &serialNumber, uint8_t mode) {
  try {
    if (!_tcpSocket || !_tcpSocket->Connected()) return false;

    BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<BaseLib::Variable>(serialNumber));
    parameters->push_back(std::make_shared<BaseLib::Variable>((int64_t)mode));

    auto result = invoke("moduleSetMode", parameters);
    if (result->errorStruct) {
      _out.printError("Error setting mode: " + result->structValue->at("faultString")->stringValue);
      return false;
    }

    return true;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Hgdc::isMaster() {
  try {
    if (!_tcpSocket || !_tcpSocket->Connected()) return false;

    BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();

    return invoke("coreIsMaster", parameters)->booleanValue;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

PVariable Hgdc::getModules(int64_t familyId) {
  try {
    if (!_tcpSocket || !_tcpSocket->Connected()) return BaseLib::Variable::createError(-32500, "Not connected.");

    BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
    parameters->push_back(std::make_shared<BaseLib::Variable>(familyId));

    return invoke("getModules", parameters);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return BaseLib::Variable::createError(-32500, "Unknown application error. See log for more details.");
}

void Hgdc::processQueueEntry(int32_t index, std::shared_ptr<BaseLib::IQueueEntry> &entry) {
  try {
    if (index == 0) {
      auto queueEntry = std::dynamic_pointer_cast<QueueEntry>(entry);
      if (!queueEntry) return;

      if (queueEntry->method == "packetReceived" && queueEntry->parameters && queueEntry->parameters->size() == 3 && !queueEntry->parameters->at(2)->binaryValue.empty()) {
        std::lock_guard<std::mutex> eventHandlersGuard(_packetReceivedEventHandlersMutex);
        auto eventHandlersIterator = _packetReceivedEventHandlers.find(queueEntry->parameters->at(0)->integerValue64);
        if (eventHandlersIterator != _packetReceivedEventHandlers.end()) {
          for (auto &eventHandler: eventHandlersIterator->second) {
            if (eventHandler.second) eventHandler.second(queueEntry->parameters->at(0)->integerValue64, queueEntry->parameters->at(1)->stringValue, queueEntry->parameters->at(2)->binaryValue);
          }
        }
      } else if (queueEntry->method == "moduleUpdate") {
        std::lock_guard<std::mutex> eventHandlersGuard(_moduleUpdateEventHandlersMutex);
        for (auto &eventHandler: _moduleUpdateEventHandlers) {
          if (eventHandler.second) eventHandler.second(queueEntry->parameters->at(0));
        }
      } else if (queueEntry->method == "reconnected") {
        std::lock_guard<std::mutex> eventHandlersGuard(_reconnectedEventHandlersMutex);
        for (auto &eventHandler: _reconnectedEventHandlers) {
          if (eventHandler.second) eventHandler.second();
        }
      }
    }
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}