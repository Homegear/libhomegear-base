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

#ifndef ISCRIPTINFO_H_
#define ISCRIPTINFO_H_

#include "../Sockets/ServerInfo.h"
#include "../Encoding/Http.h"

#include <string>
#include <mutex>
#include <condition_variable>
#include <functional>

#include <c1-net/TcpSocket.h>

namespace BaseLib
{

class RpcClientInfo;
typedef std::shared_ptr<RpcClientInfo> PRpcClientInfo;

namespace ScriptEngine
{

class ScriptInfo;
class ScriptInfoCli;
class ScriptInfoWeb;
class ScriptInfoDevice;
typedef std::shared_ptr<ScriptInfo> PScriptInfo;
typedef std::shared_ptr<ScriptInfoCli> PScriptInfoCli;
typedef std::shared_ptr<ScriptInfoWeb> PScriptInfoWeb;
typedef std::shared_ptr<ScriptInfoDevice> PScriptInfoDevice;

/**
 * This class provides hooks into the script engine server so family modules can be notified about finished script executions.
 */
class ScriptInfo
{
public:
	enum class ScriptType
	{
		cli,
		device,
		device2,
		web,
		simpleNode,
		statefulNode
	};

	int32_t id = 0;

	// {{{ Input parameters
		std::string fullPath;
		std::string relativePath;
		std::string arguments;
		int32_t customId = 0;
		bool returnOutput = false;

		Http http; //Web
		Rpc::PServerInfo serverInfo; //Web
        PRpcClientInfo clientInfo; //Web
		std::string contentPath;

		std::string script; //Device
		int64_t peerId = 0; //Device

		PVariable nodeInfo; //Node
		uint32_t inputPort = 0; //Node
		PVariable message; //Node

		uint32_t maxThreadCount = 0; //Node
	// }}}

	// {{{ Output parameters
		bool started = false;
		bool finished = false;
		int32_t exitCode = -1;
		std::string output;
	// }}}


	std::function<void(PScriptInfo& scriptInfo, std::string& output, bool error)> scriptOutputCallback;
	std::function<void(PScriptInfo& scriptInfo, PVariable& headers)> scriptHeadersCallback;

	// {{{ Script finished notification. Can be combined.
		// Option 1: Call scriptFinishedCallback
		std::function<void(PScriptInfo& scriptInfo, int32_t exitCode)> scriptFinishedCallback;

		// Option 2: Wait for script
		std::mutex requestMutex;
		std::condition_variable requestConditionVariable;

		// Option 3: Write to socket
		C1Net::PTcpSocket socket;
	// }}}

	explicit ScriptInfo(ScriptType type);
	ScriptInfo(ScriptType type, std::string& fullPath, std::string& relativePath, std::string& arguments);
	ScriptInfo(ScriptType type, std::string& contentPath, std::string& fullPath, std::string& relativePath, Http& http, Rpc::PServerInfo& serverInfo, PRpcClientInfo& clientInfo);
	ScriptInfo(BaseLib::SharedObjects* bl, ScriptType type, std::string& contentPath, std::string& fullPath, std::string& relativePath, PVariable http, PVariable serverInfo, PVariable clientInfo);
	ScriptInfo(ScriptType type, std::string& fullPath, std::string& relativePath, std::string& script, std::string& arguments);
	ScriptInfo(ScriptType type, std::string& fullPath, std::string& relativePath, std::string& script, std::string& arguments, int64_t peerId);
	ScriptInfo(ScriptType type, PVariable nodeInfo, std::string& fullPath, std::string& relativePath, uint32_t inputPort, PVariable message);
	ScriptInfo(ScriptType type, PVariable nodeInfo, std::string& fullPath, std::string& relativePath, uint32_t maxThreadCount);
	virtual ~ScriptInfo() = default;
	ScriptType getType() { return _type; }
protected:
	ScriptType _type = ScriptType::cli;
};
}
}
#endif


