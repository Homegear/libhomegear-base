/* Copyright 2013-2017 Sathya Laufer
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

#include "IIpcClient.h"
#include "../BaseLib.h"

#include <sys/un.h>
#include <sys/socket.h>

namespace BaseLib
{
namespace Ipc
{

IIpcClient::IIpcClient(SharedObjects* bl, std::string socketPath) : IQueue(bl, 1, 1000)
{
	_bl = bl;
	_socketPath = socketPath;
	_fileDescriptor = std::shared_ptr<FileDescriptor>(new FileDescriptor);
	_out.init(_bl);

	_dummyClientInfo.reset(new RpcClientInfo());

	_closed = true;
	_stopped = true;

	_binaryRpc = std::unique_ptr<Rpc::BinaryRpc>(new Rpc::BinaryRpc(_bl));
	_rpcDecoder = std::unique_ptr<Rpc::RpcDecoder>(new Rpc::RpcDecoder(_bl, false, false));
	_rpcEncoder = std::unique_ptr<Rpc::RpcEncoder>(new Rpc::RpcEncoder(_bl, true));

	_localRpcMethods.emplace("broadcastEvent", std::bind(&IIpcClient::broadcastEvent, this, std::placeholders::_1));
	_localRpcMethods.emplace("broadcastNewDevices", std::bind(&IIpcClient::broadcastNewDevices, this, std::placeholders::_1));
	_localRpcMethods.emplace("broadcastDeleteDevices", std::bind(&IIpcClient::broadcastDeleteDevices, this, std::placeholders::_1));
	_localRpcMethods.emplace("broadcastUpdateDevice", std::bind(&IIpcClient::broadcastUpdateDevice, this, std::placeholders::_1));
}

IIpcClient::~IIpcClient()
{
	dispose();
}

void IIpcClient::dispose()
{
	try
	{
		if(_disposing) return;
		std::lock_guard<std::mutex> disposeGuard(_disposeMutex);
		_disposing = true;
		stop();
		_rpcResponses.clear();
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void IIpcClient::stop()
{
	try
	{
		if(_stopped) return;
		_stopped = true;
		if(_mainThread.joinable()) _mainThread.join();
		if (_maintenanceThread.joinable()) _maintenanceThread.join();
		_closed = true;
		stopQueue(0);
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void IIpcClient::connect()
{
	try
	{
		for(int32_t i = 0; i < 2; i++)
		{
			_fileDescriptor = _bl->fileDescriptorManager.add(socket(AF_LOCAL, SOCK_STREAM | SOCK_NONBLOCK, 0));
			if(!_fileDescriptor || _fileDescriptor->descriptor == -1)
			{
				_out.printError("Could not create socket.");
				return;
			}

			if(_bl->debugLevel >= 4 && i == 0) _out.printInfo("Info: Trying to connect...");
			sockaddr_un remoteAddress;
			remoteAddress.sun_family = AF_LOCAL;
			//104 is the size on BSD systems - slightly smaller than in Linux
			if(_socketPath.length() > 104)
			{
				//Check for buffer overflow
				_out.printCritical("Critical: Socket path is too long.");
				return;
			}
			strncpy(remoteAddress.sun_path, _socketPath.c_str(), 104);
			remoteAddress.sun_path[103] = 0; //Just to make sure it is null terminated.
			if(::connect(_fileDescriptor->descriptor, (struct sockaddr*)&remoteAddress, strlen(remoteAddress.sun_path) + 1 + sizeof(remoteAddress.sun_family)) == -1)
			{
				if(i == 0)
				{
					_out.printDebug("Debug: Socket closed. Trying again...");
					//When socket was not properly closed, we sometimes need to reconnect
					std::this_thread::sleep_for(std::chrono::milliseconds(2000));
					continue;
				}
				else
				{
					_out.printError("Could not connect to socket. Error: " + std::string(strerror(errno)));
					return;
				}
			}
			else break;
		}
		_closed = false;

		if (_maintenanceThread.joinable()) _maintenanceThread.join();
		_maintenanceThread = std::thread(&IIpcClient::registerRpcMethods, this);

		if(_bl->debugLevel >= 4) _out.printMessage("Connected.");
	}
	catch(const std::exception& ex)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(Exception& ex)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void IIpcClient::start()
{
	try
	{
		_stopped = false;

		startQueue(0, 5, 0, SCHED_OTHER);

		if(_bl->debugLevel >= 5) _out.printDebug("Debug: Socket path is " + _socketPath);

		if(_mainThread.joinable()) _mainThread.join();
		_mainThread = std::thread(&IIpcClient::mainThread, this);
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void IIpcClient::mainThread()
{
	try
	{
		connect();

		std::vector<char> buffer(1024);
		int32_t result = 0;
		int32_t bytesRead = 0;
		int32_t processedBytes = 0;
		while(!_stopped)
		{
			if(_closed)
			{
				connect();
				if(_closed || _fileDescriptor->descriptor == -1)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(10000));
					continue;
				}
			}

			timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 100000;
			fd_set readFileDescriptor;
			FD_ZERO(&readFileDescriptor);
			{
				auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
				fileDescriptorGuard.lock();
				FD_SET(_fileDescriptor->descriptor, &readFileDescriptor);
			}

			result = select(_fileDescriptor->descriptor + 1, &readFileDescriptor, NULL, NULL, &timeout);
			if(result == 0) continue;
			else if(result == -1)
			{
				if(errno == EINTR) continue;
				_out.printMessage("Connection to IPC server closed (1).");
				_closed = true;
				std::this_thread::sleep_for(std::chrono::milliseconds(10000));
				continue;
			}

			bytesRead = read(_fileDescriptor->descriptor, &buffer[0], 1024);
			if(bytesRead <= 0) //read returns 0, when connection is disrupted.
			{
				_out.printMessage("Connection to IPC server closed (2).");
				_closed = true;
				std::this_thread::sleep_for(std::chrono::milliseconds(10000));
				continue;
			}

			if(bytesRead > (signed)buffer.size()) bytesRead = buffer.size();

			try
			{
				processedBytes = 0;
				while(processedBytes < bytesRead)
				{
					processedBytes += _binaryRpc->process(&buffer[processedBytes], bytesRead - processedBytes);
					if(_binaryRpc->isFinished())
					{
						std::shared_ptr<IQueueEntry> queueEntry(new QueueEntry(_binaryRpc->getData(), _binaryRpc->getType() == Rpc::BinaryRpc::Type::request));
						enqueue(0, queueEntry);
						_binaryRpc->reset();
					}
				}
			}
			catch(Rpc::BinaryRpcException& ex)
			{
				_out.printError("Error processing packet: " + ex.what());
				_binaryRpc->reset();
			}
		}
		buffer.clear();
	}
	catch(const std::exception& ex)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(Exception& ex)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void IIpcClient::processQueueEntry(int32_t index, std::shared_ptr<IQueueEntry>& entry)
{
	try
	{
		if(_disposing) return;
		std::shared_ptr<QueueEntry> queueEntry;
		queueEntry = std::dynamic_pointer_cast<QueueEntry>(entry);
		if(!queueEntry) return;

		if(queueEntry->isRequest)
		{
			std::string methodName;
			PArray parameters = _rpcDecoder->decodeRequest(queueEntry->packet, methodName);

			if(parameters->size() < 2)
			{
				_out.printError("Error: Wrong parameter count while calling method " + methodName);
				return;
			}
			std::map<std::string, std::function<PVariable(PArray& parameters)>>::iterator localMethodIterator = _localRpcMethods.find(methodName);
			if(localMethodIterator == _localRpcMethods.end())
			{
				_out.printError("Warning: RPC method not found: " + methodName);
				PVariable error = Variable::createError(-32601, ": Requested method not found.");
				sendResponse(parameters->at(0), error);
				return;
			}

			if(_bl->debugLevel >= 4) _out.printInfo("Info: Server is calling RPC method: " + methodName);

			PVariable result = localMethodIterator->second(parameters->at(1)->arrayValue);
			if(_bl->debugLevel >= 5)
			{
				_out.printDebug("Response: ");
				result->print(true, false);
			}
			sendResponse(parameters->at(0), result);
		}
		else
		{
			PVariable response = _rpcDecoder->decodeResponse(queueEntry->packet);
			if(response->arrayValue->size() < 3)
			{
				_out.printError("Error: Response has wrong array size.");
				return;
			}
			int64_t threadId = response->arrayValue->at(0)->integerValue64;
			int32_t packetId = response->arrayValue->at(1)->integerValue;

			{
				std::lock_guard<std::mutex> responseGuard(_rpcResponsesMutex);
				auto responseIterator = _rpcResponses[threadId].find(packetId);
				if(responseIterator != _rpcResponses[threadId].end())
				{
					PIpcResponse element = responseIterator->second;
					if(element)
					{
						element->response = response;
						element->packetId = packetId;
						element->finished = true;
					}
				}
			}
			std::lock_guard<std::mutex> requestInfoGuard(_requestInfoMutex);
			std::map<int64_t, RequestInfo>::iterator requestIterator = _requestInfo.find(threadId);
			if (requestIterator != _requestInfo.end()) requestIterator->second.conditionVariable.notify_all();
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

PVariable IIpcClient::send(std::vector<char>& data)
{
	try
	{
		int32_t totallySentBytes = 0;
		std::lock_guard<std::mutex> sendGuard(_sendMutex);
		while (totallySentBytes < (signed)data.size())
		{
			int32_t sentBytes = ::send(_fileDescriptor->descriptor, &data.at(0) + totallySentBytes, data.size() - totallySentBytes, MSG_NOSIGNAL);
			if(sentBytes <= 0)
			{
				if(errno == EAGAIN) continue;
				_out.printError("Could not send data to client " + std::to_string(_fileDescriptor->descriptor) + ". Sent bytes: " + std::to_string(totallySentBytes) + " of " + std::to_string(data.size()) + (sentBytes == -1 ? ". Error message: " + std::string(strerror(errno)) : ""));
				return Variable::createError(-32500, "Unknown application error.");
			}
			totallySentBytes += sentBytes;
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return PVariable(new Variable());
}

PVariable IIpcClient::sendRequest(std::string methodName, PArray& parameters)
{
	try
	{
		int64_t threadId = pthread_self();
		std::unique_lock<std::mutex> requestInfoGuard(_requestInfoMutex);
		RequestInfo& requestInfo = _requestInfo[threadId];
		requestInfoGuard.unlock();

		int32_t packetId;
		{
			std::lock_guard<std::mutex> packetIdGuard(_packetIdMutex);
			packetId = _currentPacketId++;
		}
		PArray array(new Array{ PVariable(new Variable(threadId)), PVariable(new Variable(packetId)), PVariable(new Variable(parameters)) });
		std::vector<char> data;
		_rpcEncoder->encodeRequest(methodName, array, data);

		PIpcResponse response;
		{
			std::lock_guard<std::mutex> responseGuard(_rpcResponsesMutex);
			auto result = _rpcResponses[threadId].emplace(packetId, std::make_shared<IpcResponse>());
			if(result.second) response = result.first->second;
		}
		if(!response)
		{
			_out.printError("Critical: Could not insert response struct into map.");
			return Variable::createError(-32500, "Unknown application error.");
		}

		PVariable result = send(data);
		if(result->errorStruct)
		{
			std::lock_guard<std::mutex> responseGuard(_rpcResponsesMutex);
			_rpcResponses[threadId].erase(packetId);
			if (_rpcResponses[threadId].empty()) _rpcResponses.erase(threadId);
			return result;
		}

		std::unique_lock<std::mutex> waitLock(requestInfo.waitMutex);
		while (!requestInfo.conditionVariable.wait_for(waitLock, std::chrono::milliseconds(10000), [&]
		{
			return response->finished || _closed || _stopped || _disposing;
		}));

		if(!response->finished || response->response->arrayValue->size() != 3 || response->packetId != packetId)
		{
			_out.printError("Error: No response received to RPC request. Method: " + methodName);
			result = Variable::createError(-1, "No response received.");
		}
		else result = response->response->arrayValue->at(2);

		{
			std::lock_guard<std::mutex> responseGuard(_rpcResponsesMutex);
			_rpcResponses[threadId].erase(packetId);
			if (_rpcResponses[threadId].empty()) _rpcResponses.erase(threadId);
		}

		return result;
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return Variable::createError(-32500, "Unknown application error.");
}

void IIpcClient::sendResponse(PVariable& packetId, PVariable& variable)
{
	try
	{
		PVariable array(new Variable(PArray(new Array{ packetId, variable })));
		std::vector<char> data;
		_rpcEncoder->encodeResponse(array, data);

		send(data);
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

}
}
