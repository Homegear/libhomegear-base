/* Copyright 2013-2016 Sathya Laufer
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

#include "../Sockets/SocketOperations.h"

#include <string>
#include <mutex>
#include <condition_variable>

namespace BaseLib
{
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
		web
	};

	int32_t id = 0;

	// {{{ Input parameters
		std::string path;
		std::string arguments;
		int32_t customId = 0;
		bool returnOutput = false;

		PVariable http; //Web
		PVariable serverInfo; //Web

		std::string script; //Device
		int64_t peerId = 0; //Device
	// }}}

	// {{{ Output parameters
		int32_t exitCode = -1;
		std::string output;
	// }}}


	std::function<void(PScriptInfo& scriptInfo, std::string& output)> scriptOutputCallback;

	// {{{ Script finished notification. Can be combined.
		// Option 1: Call scriptFinishedCallback
		std::function<void(PScriptInfo& scriptInfo, int32_t exitCode)> scriptFinishedCallback;

		// Option 2: Wait for script
		std::mutex requestMutex;
		std::condition_variable requestConditionVariable;

		// Option 3: Write to socket
		PSocketOperations socket;
	// }}}

	ScriptInfo(ScriptType type) { _type = type; }
	ScriptInfo(ScriptType type, std::string& path, std::string& arguments) { _type = type; this->path = path; this->arguments = arguments; }
	ScriptInfo(ScriptType type, std::string& path, PVariable http, PVariable serverInfo) { _type = type; this->path = path; this->http = http; this->serverInfo = serverInfo; }
	ScriptInfo(ScriptType type, std::string& path, std::string& script, std::string& arguments, int64_t peerId) { _type = type; this->path = path; this->script = script; this->arguments = arguments; this->peerId = peerId; }
	virtual ~ScriptInfo() {}
	ScriptType getType() { return _type; }
protected:
	ScriptType _type = ScriptType::cli;
};
}
}
#endif

