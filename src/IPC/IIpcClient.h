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

#ifndef IIPCCLIENT_H_
#define IIPCCLIENT_H_

#include "IpcResponse.h"
#include "../Encoding/RpcMethod.h"
#include "../IQueue.h"
#include "../Output/Output.h"

#include <thread>
#include <mutex>
#include <string>

namespace BaseLib
{

class FileDescriptor;

namespace Rpc
{
	class BinaryRpc;
	class RpcDecoder;
	class RpcEncoder;
}

namespace Ipc
{

class IIpcClient : public IQueue
{
public:
	IIpcClient(SharedObjects* bl, std::string socketPath);
	virtual ~IIpcClient();
	virtual void dispose();

	virtual void start();
	virtual void stop();
protected:
	struct RequestInfo
	{
		std::mutex waitMutex;
		std::condition_variable conditionVariable;
	};

	class QueueEntry : public IQueueEntry
	{
	public:
		QueueEntry() {}
		QueueEntry(std::vector<char>& packet, bool isRequest) { this->packet = packet; this->isRequest = isRequest; }
		virtual ~QueueEntry() {}

		std::vector<char> packet;
		bool isRequest = false;
	};

	SharedObjects* _bl = nullptr;
	std::mutex _disposeMutex;
	bool _disposing = false;
	Output _out;
	std::string _socketPath;
	std::shared_ptr<FileDescriptor> _fileDescriptor;
	int64_t _lastGargabeCollection = 0;
	std::atomic_bool _stopped;
	std::atomic_bool _closed;
	std::mutex _sendMutex;
	std::mutex _rpcResponsesMutex;
	std::unordered_map<int64_t, std::unordered_map<int32_t, PIpcResponse>> _rpcResponses;
	std::shared_ptr<RpcClientInfo> _dummyClientInfo;
	std::map<std::string, std::function<PVariable(PArray& parameters)>> _localRpcMethods;
	std::thread _mainThread;
	std::thread _maintenanceThread;
	std::mutex _requestInfoMutex;
	std::map<int64_t, RequestInfo> _requestInfo;
	std::mutex _packetIdMutex;
	int32_t _currentPacketId = 0;

	std::unique_ptr<Rpc::BinaryRpc> _binaryRpc;
	std::unique_ptr<Rpc::RpcDecoder> _rpcDecoder;
	std::unique_ptr<Rpc::RpcEncoder> _rpcEncoder;

	void connect();
	void mainThread();
	PVariable invoke(std::string methodName, PArray& parameters);
	void sendResponse(PVariable& packetId, PVariable& variable);

	void processQueueEntry(int32_t index, std::shared_ptr<IQueueEntry>& entry);
	PVariable send(std::vector<char>& data);

	virtual void registerRpcMethods() = 0;

	// {{{ RPC methods
		virtual BaseLib::PVariable broadcastEvent(BaseLib::PArray& parameters) { return BaseLib::PVariable(new BaseLib::Variable()); }
		virtual BaseLib::PVariable broadcastNewDevices(BaseLib::PArray& parameters) { return BaseLib::PVariable(new BaseLib::Variable()); }
		virtual BaseLib::PVariable broadcastDeleteDevices(BaseLib::PArray& parameters) { return BaseLib::PVariable(new BaseLib::Variable()); }
		virtual BaseLib::PVariable broadcastUpdateDevice(BaseLib::PArray& parameters) { return BaseLib::PVariable(new BaseLib::Variable()); }
	// }}}
};

}
}
#endif
