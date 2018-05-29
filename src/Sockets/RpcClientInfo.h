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

#ifndef RPCCLIENTINFO_H_
#define RPCCLIENTINFO_H_

#include "../Variable.h"
#include <memory>
#include <condition_variable>
#include <atomic>

namespace BaseLib
{

class FileDescriptor;
class TcpSocket;

namespace Security
{
    class Acls;
    typedef std::shared_ptr<Acls> PAcls;
}

enum class RpcClientType
{
	generic,
	ipsymcon,
	ccu2,
	homematicconfigurator
};

enum class RpcType
{
	unknown,
	xml,
	binary,
	json,
	websocket,
	mqtt,
	rest
};

class RpcClientInfo
{
public:
	int32_t id = -1;
    bool sendEventsToRpcServer = false;
	bool closed = false;
	bool addon = false;
	bool flowsServer = false;
	bool scriptEngineServer = false;
    bool ipcServer = false;
    bool mqttClient = false;
    bool familyModule = false;
	std::string webSocketClientId;
	std::string address;
	int32_t port = 0;
	std::string initUrl;
	std::string initInterfaceId;
	std::string language = "en-US";
	std::string user;
	Security::PAcls acls;

	RpcType rpcType = RpcType::unknown;
	RpcClientType clientType = RpcClientType::generic;
	bool initKeepAlive = false;
	bool initBinaryMode = false;
	bool initNewFormat = false;
	bool initSubscribePeers = false;
	bool initJsonMode = false;
	bool initSendNewDevices = true;

    std::shared_ptr<FileDescriptor> socketDescriptor;
    std::shared_ptr<TcpSocket> socket;

	//{{{ Invoke variables
	std::mutex invokeMutex;
	std::mutex requestMutex;
	std::condition_variable requestConditionVariable;
	PVariable rpcResponse;
	std::atomic_bool waitForResponse;
	//}}}

	RpcClientInfo() = default;
	virtual ~RpcClientInfo() = default;
};

typedef std::shared_ptr<RpcClientInfo> PRpcClientInfo;

}

#endif
