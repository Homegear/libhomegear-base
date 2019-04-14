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

#include "ServerInfo.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Rpc
{
PVariable ServerInfo::Info::serialize()
{
	if(serializedInfo) return serializedInfo;

	serializedInfo = std::make_shared<Variable>(VariableType::tArray);
    serializedInfo->arrayValue->reserve(20 + validGroups.size());
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(index)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(name)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(interface)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(port)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(ssl)));
	serializedInfo->arrayValue->emplace_back(std::make_shared<Variable>(caPath));
	serializedInfo->arrayValue->emplace_back(std::make_shared<Variable>(certPath));
	serializedInfo->arrayValue->emplace_back(std::make_shared<Variable>(keyPath));
	serializedInfo->arrayValue->emplace_back(std::make_shared<Variable>(dhParamPath));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable((int32_t)authType)));
    serializedInfo->arrayValue->emplace_back(std::make_shared<Variable>(validGroups.size()));
	for(auto group : validGroups)
	{
		serializedInfo->arrayValue->emplace_back(std::make_shared<Variable>(group));
	}
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(contentPath)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(webServer)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(webSocket)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable((int32_t)websocketAuthType)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(xmlrpcServer)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(jsonrpcServer)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(restServer)));
    serializedInfo->arrayValue->emplace_back(PVariable(new Variable(cacheAssets)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(redirectTo)));
	serializedInfo->arrayValue->emplace_back(PVariable(new Variable(address)));

	// Module settings are not serialized

	return serializedInfo;
}

void ServerInfo::Info::unserialize(PVariable data)
{
	if(!data) return;
	int32_t pos = 0;
	index = data->arrayValue->at(pos)->integerValue; pos++;
	name = data->arrayValue->at(pos)->stringValue; pos++;
	interface = data->arrayValue->at(pos)->stringValue; pos++;
	port = data->arrayValue->at(pos)->integerValue; pos++;
	ssl = data->arrayValue->at(pos)->booleanValue; pos++;
	caPath = data->arrayValue->at(pos)->stringValue; pos++;
	certPath = data->arrayValue->at(pos)->stringValue; pos++;
	keyPath = data->arrayValue->at(pos)->stringValue; pos++;
	dhParamPath = data->arrayValue->at(pos)->stringValue; pos++;
	authType = (AuthType)data->arrayValue->at(pos)->integerValue; pos++;
	int32_t validUsersSize = data->arrayValue->at(pos)->integerValue; pos++;
	for(int32_t i = 0; i < validUsersSize; i++)
	{
		validGroups.emplace((uint64_t)data->arrayValue->at(pos)->integerValue64); pos++;
	}
	contentPath = data->arrayValue->at(pos)->stringValue; pos++;
	webServer = data->arrayValue->at(pos)->booleanValue; pos++;
	webSocket = data->arrayValue->at(pos)->booleanValue; pos++;
	websocketAuthType = (AuthType)data->arrayValue->at(pos)->integerValue; pos++;
	xmlrpcServer = data->arrayValue->at(pos)->booleanValue; pos++;
	jsonrpcServer = data->arrayValue->at(pos)->booleanValue; pos++;
	restServer = data->arrayValue->at(pos)->booleanValue; pos++;
    cacheAssets = data->arrayValue->at(pos)->integerValue; pos++;
	redirectTo = data->arrayValue->at(pos)->stringValue; pos++;
	address = data->arrayValue->at(pos)->stringValue;
}

ServerInfo::ServerInfo()
{
}

ServerInfo::ServerInfo(BaseLib::SharedObjects* baseLib)
{
	_bl = baseLib;
}

void ServerInfo::reset()
{
	_servers.clear();
}

void ServerInfo::init(BaseLib::SharedObjects* baseLib)
{
	_bl = baseLib;
}

void ServerInfo::load(std::string filename)
{
	try
	{
		std::unordered_map<int32_t, PFileDescriptor> socketInfo;

		for(auto& server : _servers)
		{
			if(server.second->socketDescriptor && server.second->socketDescriptor->descriptor != -1) socketInfo.emplace(server.second->port, server.second->socketDescriptor);
		}

		reset();
		int32_t index = 0;
		char input[1024];
		FILE *fin;
		int32_t len, ptr;
		bool found = false;

		if (!(fin = fopen(filename.c_str(), "r")))
		{
			_bl->out.printError("Unable to open RPC server config file: " + filename + ". " + strerror(errno));
			return;
		}

		std::shared_ptr<Info> info(new Info());
		while (fgets(input, 1024, fin))
		{
			if(input[0] == '#') continue;
			len = strlen(input);
			if (len < 2) continue;
			if (input[len-1] == '\n') input[len-1] = '\0';
			ptr = 0;
			if(input[0] == '[')
			{
				while(ptr < len)
				{
					if (input[ptr] == ']')
					{
						input[ptr] = '\0';
						if(info->port > 0)
						{
							info->index = index;

							auto socketDescriptorIterator = socketInfo.find(info->port);
							if(socketDescriptorIterator != socketInfo.end()) info->socketDescriptor = socketDescriptorIterator->second;

							_servers[index++] = info;
						}
						info.reset(new Info());
						info->name = std::string(&input[1]);
						break;
					}
					ptr++;
				}
				continue;
			}
			found = false;
			while(ptr < len)
			{
				if (input[ptr] == '=')
				{
					found = true;
					input[ptr++] = '\0';
					break;
				}
				ptr++;
			}
			if(found)
			{
				std::string name(input);
				HelperFunctions::toLower(name);
				HelperFunctions::trim(name);
				std::string value(&input[ptr]);
				HelperFunctions::trim(value);
				if(name == "interface")
				{
					info->interface = value;
					if(info->interface.empty()) info->interface = "::";
					_bl->out.printDebug("Debug: interface of server " + info->name + " set to " + info->interface);
				}
				else if(name == "port")
				{
					info->port = Math::getNumber(value);
					_bl->out.printDebug("Debug: port of server " + info->name + " set to " + std::to_string(info->port));
				}
				else if(name == "ssl")
				{
					HelperFunctions::toLower(value);
					info->ssl = value == "true";;
					_bl->out.printDebug("Debug: ssl of server " + info->name + " set to " + std::to_string(info->ssl));
				}
				else if(name == "capath")
				{
					info->caPath = value;
					_bl->out.printDebug("Debug: caPath of server " + info->name + " set to " + info->caPath);
				}
				else if(name == "certpath")
				{
					info->certPath = value;
					_bl->out.printDebug("Debug: certPath of server " + info->name + " set to " + info->certPath);
				}
				else if(name == "keypath")
				{
					info->keyPath = value;
					_bl->out.printDebug("Debug: keyPath of server " + info->name + " set to " + info->keyPath);
				}
				else if(name == "dhparampath")
				{
					info->dhParamPath = value;
					_bl->out.printDebug("Debug: dhParamPath of server " + info->name + " set to " + info->dhParamPath);
				}
				else if(name == "authtype")
				{
                    info->authType = Info::AuthType::none;
					HelperFunctions::toLower(value);
                    auto fields = HelperFunctions::splitAll(value, ',');
                    for(auto& field : fields)
                    {
                        HelperFunctions::trim(field);
                        if(field == "none") info->authType = (Info::AuthType)((int32_t)info->authType | Info::AuthType::none);
                        else if(field == "basic") info->authType = (Info::AuthType)((int32_t)info->authType | Info::AuthType::basic);
                        else if(field == "cert") info->authType = (Info::AuthType)((int32_t)info->authType | Info::AuthType::cert);
                        else if(field == "oauth2local") info->authType = (Info::AuthType)((int32_t)info->authType | Info::AuthType::oauth2Local);
                    }
					_bl->out.printDebug("Debug: authType of server " + info->name + " set to " + std::to_string(info->authType));
				}
				else if(name == "validgroups")
				{
					std::stringstream stream(value);
					std::string element;
					while(std::getline(stream, element, ','))
					{
						HelperFunctions::toLower(HelperFunctions::trim(element));
						uint64_t group = BaseLib::Math::getNumber64(element);
						if(group > 0) info->validGroups.emplace(group);
					}
				}
				else if(name == "contentpath")
				{
					info->contentPath = value;
					if(info->contentPath.back() != '/') info->contentPath.push_back('/');
					_bl->out.printDebug("Debug: contentPath of RPC server " + info->name + " set to " + info->contentPath);
				}
				else if(name == "contentpathpermissions")
				{
					info->contentPathPermissions = Math::getOctalNumber(value);
					if(info->contentPathPermissions == 0) info->contentPathPermissions = 360;
					_bl->out.printDebug("Debug: contentPathPermissions of RPC server " + info->name + " set to " + std::to_string(info->contentPathPermissions));
				}
				else if(name == "contentpathuser")
				{
					info->contentPathUser = value;
					_bl->out.printDebug("Debug: contentPathUser of RPC server " + info->name + " set to " + info->contentPathUser);
				}
				else if(name == "contentpathgroup")
				{
					info->contentPathGroup = value;
					_bl->out.printDebug("Debug: contentPathGroup of RPC server " + info->name + " set to " + info->contentPathGroup);
				}
				else if(name == "xmlrpcserver")
				{
					HelperFunctions::toLower(value);
					info->xmlrpcServer = value == "true";;
					_bl->out.printDebug("Debug: xmlrpcServer of server " + info->name + " set to " + std::to_string(info->xmlrpcServer));
				}
				else if(name == "jsonrpcserver")
				{
					HelperFunctions::toLower(value);
					info->jsonrpcServer = value == "true";
					_bl->out.printDebug("Debug: jsonrpcServer of server " + info->name + " set to " + std::to_string(info->jsonrpcServer));
				}
				else if(name == "restserver")
				{
					HelperFunctions::toLower(value);
					info->restServer = value == "true";
					_bl->out.printDebug("Debug: restServer of server " + info->name + " set to " + std::to_string(info->restServer));
				}
				else if(name == "webserver")
				{
					HelperFunctions::toLower(value);
					info->webServer = value == "true";;
					_bl->out.printDebug("Debug: webServer of server " + info->name + " set to " + std::to_string(info->webServer));
				}
				else if(name == "familyserver")
				{
					HelperFunctions::toLower(value);
					info->familyServer = value == "true";
					_bl->out.printDebug("Debug: familyServer of server " + info->name + " set to " + std::to_string(info->familyServer));
				}
				else if(name == "websocket")
				{
					HelperFunctions::toLower(value);
					info->webSocket = value == "true";;
					_bl->out.printDebug("Debug: webSocket of server " + info->name + " set to " + std::to_string(info->webSocket));
				}
				else if(name == "websocketauthtype")
				{
					HelperFunctions::toLower(value);
					if(value == "none") info->websocketAuthType = Info::AuthType::none;
					else if(value == "basic") info->websocketAuthType = Info::AuthType::basic;
					else if(value == "session") info->websocketAuthType = Info::AuthType::session;
					_bl->out.printDebug("Debug: webSocketAuthType of server " + info->name + " set to " + std::to_string(info->websocketAuthType));
				}
                else if(name == "cacheassets")
                {
                    info->cacheAssets = Math::getNumber(value);
                    _bl->out.printDebug("Debug: cacheAssets of server " + info->name + " set to " + std::to_string(info->cacheAssets));
                }
				else if(name == "redirectto")
				{
					info->redirectTo = value;
					_bl->out.printDebug("Debug: redirectToPort of server " + info->name + " set to " + info->redirectTo);
				}
				else
				{
					info->modSettings[name].push_back(value);
					_bl->out.printDebug("Debug: Setting " + value + " added to module " + name + " of server " + info->name + ".");
				}
			}
		}
		if(info->port > 0)
		{
			info->index = index;

			auto socketDescriptorIterator = socketInfo.find(info->port);
			if(socketDescriptorIterator != socketInfo.end()) info->socketDescriptor = socketDescriptorIterator->second;

			_servers[index] = info;
		}

		fclose(fin);
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

}
}
