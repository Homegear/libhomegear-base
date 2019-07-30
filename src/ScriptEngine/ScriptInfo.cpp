#include <utility>

#include <utility>

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

#include "ScriptInfo.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace ScriptEngine
{

ScriptInfo::ScriptInfo(ScriptInfo::ScriptType type)
{
    _type = type;
}

ScriptInfo::ScriptInfo(ScriptInfo::ScriptType type, std::string& fullPath, std::string& relativePath, std::string& arguments)
{
    _type = type;
    this->fullPath = fullPath;
    this->relativePath = relativePath;
    this->arguments = arguments;
}


ScriptInfo::ScriptInfo(ScriptInfo::ScriptType type, std::string& contentPath, std::string& fullPath, std::string& relativePath, Http& http, Rpc::PServerInfo& serverInfo, PRpcClientInfo& clientInfo)
{
    _type = type;
    this->contentPath = contentPath;
    this->fullPath = fullPath;
    this->relativePath = relativePath;
    this->http = http;
    this->serverInfo = serverInfo;
    this->clientInfo = clientInfo;
}

ScriptInfo::ScriptInfo(BaseLib::SharedObjects* bl, ScriptInfo::ScriptType type, std::string& contentPath, std::string& fullPath, std::string& relativePath, PVariable http, PVariable serverInfo, PVariable clientInfo)
{
    _type = type;
    this->contentPath = contentPath;
    this->fullPath = fullPath;
    this->relativePath = relativePath;
    this->http.unserialize(std::move(http));
    this->serverInfo.reset(new Rpc::ServerInfo::Info());
    this->serverInfo->unserialize(std::move(serverInfo));
    this->clientInfo.reset(new RpcClientInfo());
    this->clientInfo->unserialize(bl, std::move(clientInfo));
}

ScriptInfo::ScriptInfo(ScriptInfo::ScriptType type, std::string& fullPath, std::string& relativePath, std::string& script, std::string& arguments)
{
    _type = type;
    this->fullPath = fullPath;
    this->relativePath = relativePath;
    this->script = script;
    this->arguments = arguments;
}

ScriptInfo::ScriptInfo(ScriptInfo::ScriptType type, std::string& fullPath, std::string& relativePath, std::string& script, std::string& arguments, int64_t peerId)
{
    _type = type;
    this->fullPath = fullPath;
    this->relativePath = relativePath;
    this->script = script;
    this->arguments = arguments;
    this->peerId = peerId;
}

ScriptInfo::ScriptInfo(ScriptInfo::ScriptType type, PVariable nodeInfo, std::string& fullPath, std::string& relativePath, uint32_t inputPort, PVariable message)
{
    _type = type;
    this->fullPath = fullPath;
    this->relativePath = relativePath;
    this->nodeInfo = std::move(nodeInfo);
    this->inputPort = inputPort;
    this->message = std::move(message);
}

ScriptInfo::ScriptInfo(ScriptInfo::ScriptType type, PVariable nodeInfo, std::string& fullPath, std::string& relativePath, uint32_t maxThreadCount)
{
    _type = type;
    this->fullPath = fullPath;
    this->relativePath = relativePath;
    this->nodeInfo = std::move(nodeInfo);
    this->maxThreadCount = maxThreadCount;
}

}
}
