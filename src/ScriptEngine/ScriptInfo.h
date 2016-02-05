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

#include <string>
#include <mutex>
#include <condition_variable>

namespace BaseLib
{
namespace ScriptEngine
{

class ScriptInfo;
typedef std::shared_ptr<ScriptInfo> PScriptInfo;

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
	int32_t customId = 0;

	std::string path;			//cli, web
	std::string arguments;		//cli
	std::string script;			//device
	int64_t peerId = 0;			//device
	bool keepAlive = false;		//device
	int32_t interval = -1;		//device

	std::vector<char> output;

	// Option 1: Call scriptFinishedCallback
	std::function<void(PScriptInfo& scriptInfo, int32_t exitCode, std::string& output)> scriptFinishedCallback;

	// Option 2: Wait for script
	std::mutex requestMutex;
	std::condition_variable requestConditionVariable;

	virtual ~ScriptInfo() {}
};
}
}
#endif


