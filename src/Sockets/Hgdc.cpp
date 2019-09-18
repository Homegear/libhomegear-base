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

namespace BaseLib
{

Hgdc::Hgdc(SharedObjects* bl, uint16_t port) : IQueue(bl, 1, 100)
{
    _bl = bl;
    _port = port;

    signal(SIGPIPE, SIG_IGN);

    _out.setPrefix("HGDC: ");
    _binaryRpc.reset(new BaseLib::Rpc::BinaryRpc(bl));
    _rpcEncoder.reset(new BaseLib::Rpc::RpcEncoder(bl, true, true));
    _rpcDecoder.reset(new BaseLib::Rpc::RpcDecoder(bl, false, false));
}

Hgdc::~Hgdc()
{
    stop();
}

void Hgdc::start()
{
    try
    {
        stop();

        if(_port == 0)
        {
            _out.printError("Error: Cannot connect to Homegear Daisy Chain Connector, because port is invalid.");
            return;
        }

        startQueue(0, false, 2);

        _tcpSocket.reset(new BaseLib::TcpSocket(_bl, "localhost", std::to_string(_port)));
        _tcpSocket->setConnectionRetries(1);
        _tcpSocket->setReadTimeout(5000000);
        _tcpSocket->setWriteTimeout(5000000);

        try
        {
            _tcpSocket->open();
            if(_tcpSocket->connected())
            {
                _out.printInfo("Info: Successfully connected.");
                _stopped = false;
                std::shared_ptr<QueueEntry> queueEntry = std::make_shared<QueueEntry>();
                queueEntry->method = "reconnected";
                auto baseQueueEntry = static_cast<std::shared_ptr<IQueueEntry>>(queueEntry);
                enqueue(0, baseQueueEntry);
            }
        }
        catch(const std::exception& ex)
        {
            _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
        }

        _stopCallbackThread = false;
        _bl->threadManager.start(_listenThread, true, &Hgdc::listen, this);
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void Hgdc::stop()
{
    try
    {
        stopQueue(0);
        _stopCallbackThread = true;
        if(_tcpSocket) _tcpSocket->close();
        _bl->threadManager.join(_listenThread);
        _stopped = true;
        _tcpSocket.reset();
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

int32_t Hgdc::registerPacketReceivedEventHandler(int64_t familyId, std::function<void(int64_t, const std::string&, const std::vector<uint8_t>&)> value)
{
    try
    {
        int32_t eventHandlerId = -1;
        std::lock_guard<std::mutex> eventHandlersGuard(_packetReceivedEventHandlersMutex);
        while(eventHandlerId == -1) eventHandlerId = _currentEventHandlerId++;

        _packetReceivedEventHandlers[familyId].emplace_back(std::make_pair(eventHandlerId, std::move(value)));

        return eventHandlerId;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return -1;
}

void Hgdc::unregisterPacketReceivedEventHandler(int32_t eventHandlerId)
{
    try
    {
        if(eventHandlerId == -1) return;

        std::lock_guard<std::mutex> eventHandlersGuard(_packetReceivedEventHandlersMutex);
        for(auto& eventHandlers : _packetReceivedEventHandlers)
        {
            for(auto& eventHandler : eventHandlers.second)
            {
                if(eventHandler.first == eventHandlerId)
                {
                    _packetReceivedEventHandlers.erase(eventHandler.first);
                    break;
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

int32_t Hgdc::registerModuleUpdateEventHandler(std::function<void(const BaseLib::PVariable&)> value)
{
    try
    {
        int32_t eventHandlerId = -1;
        std::lock_guard<std::mutex> eventHandlersGuard(_moduleUpdateEventHandlersMutex);
        while(eventHandlerId == -1) eventHandlerId = _currentEventHandlerId++;

        _moduleUpdateEventHandlers.emplace(eventHandlerId, std::move(value));

        return eventHandlerId;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return -1;
}

void Hgdc::unregisterModuleUpdateEventHandler(int32_t eventHandlerId)
{
    try
    {
        if(eventHandlerId == -1) return;

        std::lock_guard<std::mutex> eventHandlersGuard(_moduleUpdateEventHandlersMutex);
        _moduleUpdateEventHandlers.erase(eventHandlerId);
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

int32_t Hgdc::registerReconnectedEventHandler(std::function<void()> value)
{
    try
    {
        int32_t eventHandlerId = -1;
        std::lock_guard<std::mutex> eventHandlersGuard(_reconnectedEventHandlersMutex);
        while(eventHandlerId == -1) eventHandlerId = _currentEventHandlerId++;

        _reconnectedEventHandlers.emplace(eventHandlerId, std::move(value));

        return eventHandlerId;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return -1;
}

void Hgdc::unregisterReconnectedEventHandler(int32_t eventHandlerId)
{
    try
    {
        if(eventHandlerId == -1) return;

        std::lock_guard<std::mutex> eventHandlersGuard(_reconnectedEventHandlersMutex);
        _reconnectedEventHandlers.erase(eventHandlerId);
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void Hgdc::listen()
{
    try
    {
        std::vector<char> buffer(4096);
        int32_t processedBytes = 0;
        while(!_stopCallbackThread)
        {
            try
            {
                if(_stopped || !_tcpSocket->connected())
                {
                    if(_stopCallbackThread) return;
                    if(_stopped) _out.printWarning("Warning: Connection to device closed. Trying to reconnect...");
                    _tcpSocket->close();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    _tcpSocket->open();
                    if(_tcpSocket->connected())
                    {
                        _out.printInfo("Info: Successfully connected.");
                        _stopped = false;
                        std::shared_ptr<QueueEntry> queueEntry = std::make_shared<QueueEntry>();
                        queueEntry->method = "reconnected";
                        auto baseQueueEntry = static_cast<std::shared_ptr<IQueueEntry>>(queueEntry);
                        enqueue(0, baseQueueEntry);
                    }
                    continue;
                }

                int32_t bytesRead = 0;
                try
                {
                    bytesRead = _tcpSocket->proofread(buffer.data(), buffer.size());
                }
                catch(BaseLib::SocketTimeOutException& ex)
                {
                    continue;
                }
                if(bytesRead <= 0) continue;
                if(bytesRead > (signed)buffer.size()) bytesRead = buffer.size();

                if(_bl->debugLevel >= 5) _out.printDebug("Debug: TCP packet received: " + BaseLib::HelperFunctions::getHexString(buffer.data(), bytesRead));

                processedBytes = 0;
                while(processedBytes < bytesRead)
                {
                    try
                    {
                        processedBytes += _binaryRpc->process(buffer.data() + processedBytes, bytesRead - processedBytes);
                        if(_binaryRpc->isFinished())
                        {
                            if(_binaryRpc->getType() == BaseLib::Rpc::BinaryRpc::Type::request)
                            {
                                std::shared_ptr<QueueEntry> queueEntry = std::make_shared<QueueEntry>();
                                auto baseQueueEntry = static_cast<std::shared_ptr<IQueueEntry>>(queueEntry);
                                queueEntry->parameters = _rpcDecoder->decodeRequest(_binaryRpc->getData(), queueEntry->method);
                                enqueue(0, baseQueueEntry);

                                BaseLib::PVariable response = std::make_shared<BaseLib::Variable>();
                                std::vector<char> data;
                                _rpcEncoder->encodeResponse(response, data);
                                _tcpSocket->proofwrite(data);
                            }
                            else if(_binaryRpc->getType() == BaseLib::Rpc::BinaryRpc::Type::response && _waitForResponse)
                            {
                                std::unique_lock<std::mutex> requestLock(_requestMutex);
                                _rpcResponse = _rpcDecoder->decodeResponse(_binaryRpc->getData());
                                requestLock.unlock();
                                _requestConditionVariable.notify_all();
                            }
                            _binaryRpc->reset();
                        }
                    }
                    catch(BaseLib::Rpc::BinaryRpcException& ex)
                    {
                        _binaryRpc->reset();
                        _out.printError("Error processing packet: " + std::string(ex.what()));
                    }
                }
            }
            catch(const std::exception& ex)
            {
                _stopped = true;
                _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
            }
        }
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

PVariable Hgdc::invoke(const std::string& methodName, const PArray& parameters)
{
    try
    {
        std::lock_guard<std::mutex> invokeGuard(_invokeMutex);

        std::unique_lock<std::mutex> requestLock(_requestMutex);
        _rpcResponse.reset();
        _waitForResponse = true;

        std::vector<char> encodedPacket;
        _rpcEncoder->encodeRequest(methodName, parameters, encodedPacket);

        int32_t i = 0;
        for(i = 0; i < 5; i++)
        {
            try
            {
                _tcpSocket->proofwrite(encodedPacket);
                break;
            }
            catch(const BaseLib::SocketOperationException& ex)
            {
                _out.printError("Error: " + std::string(ex.what()));
                if(i == 5) return BaseLib::Variable::createError(-32500, ex.what());
            }
        }

        i = 0;
        while(!_requestConditionVariable.wait_for(requestLock, std::chrono::milliseconds(1000), [&]
        {
            i++;
            return _rpcResponse || _stopped || i == 10;
        }));
        _waitForResponse = false;
        if(i == 10 || !_rpcResponse) return BaseLib::Variable::createError(-32500, "No RPC response received.");

        return _rpcResponse;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return BaseLib::Variable::createError(-32500, "Unknown application error. See log for more details.");
}

bool Hgdc::sendPacket(const std::string& serialNumber, const std::vector<uint8_t>& packet)
{
    try
    {
        if(!_tcpSocket || !_tcpSocket->connected()) return false;

        BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
        parameters->reserve(2);
        parameters->push_back(std::make_shared<BaseLib::Variable>(serialNumber));
        parameters->push_back(std::make_shared<BaseLib::Variable>(packet));

        auto result = invoke("sendPacket", parameters);
        if(result->errorStruct)
        {
            _out.printError("Error sending packet " + BaseLib::HelperFunctions::getHexString(packet) + ": " + result->structValue->at("faultString")->stringValue);
            return false;
        }

        return true;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

bool Hgdc::sendPacket(const std::string& serialNumber, const std::vector<char>& packet)
{
    try
    {
        if(!_tcpSocket || !_tcpSocket->connected()) return false;

        BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
        parameters->reserve(2);
        parameters->push_back(std::make_shared<BaseLib::Variable>(serialNumber));
        parameters->push_back(std::make_shared<BaseLib::Variable>(packet));

        auto result = invoke("sendPacket", parameters);
        if(result->errorStruct)
        {
            _out.printError("Error sending packet " + BaseLib::HelperFunctions::getHexString(packet) + ": " + result->structValue->at("faultString")->stringValue);
            return false;
        }

        return true;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

PVariable Hgdc::getModules(int64_t familyId)
{
    try
    {
        if(!_tcpSocket || !_tcpSocket->connected()) return BaseLib::Variable::createError(-32500, "Not connected.");

        BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
        parameters->push_back(std::make_shared<BaseLib::Variable>(familyId));

        return invoke("getModules", parameters);
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return BaseLib::Variable::createError(-32500, "Unknown application error. See log for more details.");
}

void Hgdc::processQueueEntry(int32_t index, std::shared_ptr<BaseLib::IQueueEntry>& entry)
{
    try
    {
        if(index == 0)
        {
            auto queueEntry = std::dynamic_pointer_cast<QueueEntry>(entry);
            if(!queueEntry) return;

            if(queueEntry->method == "packetReceived" && queueEntry->parameters && queueEntry->parameters->size() == 3 && !queueEntry->parameters->at(2)->binaryValue.empty())
            {
                std::lock_guard<std::mutex> eventHandlersGuard(_packetReceivedEventHandlersMutex);
                auto eventHandlersIterator = _packetReceivedEventHandlers.find(queueEntry->parameters->at(0)->integerValue64);
                if(eventHandlersIterator != _packetReceivedEventHandlers.end())
                {
                    for(auto& eventHandler : eventHandlersIterator->second)
                    {
                        if(eventHandler.second) eventHandler.second(queueEntry->parameters->at(0)->integerValue64, queueEntry->parameters->at(1)->stringValue, queueEntry->parameters->at(2)->binaryValue);
                    }
                }
            }
            else if(queueEntry->method == "moduleUpdate")
            {
                std::lock_guard<std::mutex> eventHandlersGuard(_moduleUpdateEventHandlersMutex);
                for(auto& eventHandler : _moduleUpdateEventHandlers)
                {
                    if(eventHandler.second) eventHandler.second(queueEntry->parameters->at(0));
                }
            }
            else if(queueEntry->method == "reconnected")
            {
                std::lock_guard<std::mutex> eventHandlersGuard(_reconnectedEventHandlersMutex);
                for(auto& eventHandler : _reconnectedEventHandlers)
                {
                    if(eventHandler.second) eventHandler.second();
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

}