/* Copyright 2013-2019 Homegear GmbH
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

#include "RpcClientInfo.h"
#include "../Security/Acls.h"

BaseLib::RpcClientInfo::RpcClientInfo()
{
    lastReceivedPacket.store(0);
}

BaseLib::RpcClientInfo::RpcClientInfo(const BaseLib::RpcClientInfo& rhs)
{
    id = rhs.id;
    sendEventsToRpcServer = rhs.sendEventsToRpcServer;
    closed = rhs.closed;
    addon = rhs.addon;
    flowsServer = rhs.flowsServer;
    scriptEngineServer = rhs.scriptEngineServer;
    ipcServer = rhs.ipcServer;
    mqttClient = rhs.mqttClient;
    familyModule = rhs.familyModule;
    webSocketClientId = rhs.webSocketClientId;
    address = rhs.address;
    port = rhs.port;
    initUrl = rhs.initUrl;
    initInterfaceId = rhs.initInterfaceId;
    language = rhs.language;
    user = rhs.user;
    acls = rhs.acls;
    rpcType = rhs.rpcType;
    clientType = rhs.clientType;
    initKeepAlive = rhs.initKeepAlive;
    initBinaryMode = rhs.initBinaryMode;
    initNewFormat = rhs.initNewFormat;
    initSubscribePeers = rhs.initSubscribePeers;
    initJsonMode = rhs.initJsonMode;
    initSendNewDevices = rhs.initSendNewDevices;
    peerId = rhs.peerId;
}

BaseLib::RpcClientInfo& BaseLib::RpcClientInfo::operator=(const BaseLib::RpcClientInfo& rhs)
{
    if(&rhs == this) return *this;

    id = rhs.id;
    sendEventsToRpcServer = rhs.sendEventsToRpcServer;
    closed = rhs.closed;
    addon = rhs.addon;
    flowsServer = rhs.flowsServer;
    scriptEngineServer = rhs.scriptEngineServer;
    ipcServer = rhs.ipcServer;
    mqttClient = rhs.mqttClient;
    familyModule = rhs.familyModule;
    webSocketClientId = rhs.webSocketClientId;
    address = rhs.address;
    port = rhs.port;
    initUrl = rhs.initUrl;
    initInterfaceId = rhs.initInterfaceId;
    language = rhs.language;
    user = rhs.user;
    acls = rhs.acls;
    rpcType = rhs.rpcType;
    clientType = rhs.clientType;
    initKeepAlive = rhs.initKeepAlive;
    initBinaryMode = rhs.initBinaryMode;
    initNewFormat = rhs.initNewFormat;
    initSubscribePeers = rhs.initSubscribePeers;
    initJsonMode = rhs.initJsonMode;
    initSendNewDevices = rhs.initSendNewDevices;
    peerId = rhs.peerId;

    return *this;
}

BaseLib::PVariable BaseLib::RpcClientInfo::serialize()
{
    if(serializedInfo) return serializedInfo;

    serializedInfo = std::make_shared<Variable>(VariableType::tArray);
    serializedInfo->arrayValue->reserve(28);
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(id));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(sendEventsToRpcServer));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(closed));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(addon));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(flowsServer));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(scriptEngineServer));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(ipcServer));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(mqttClient));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(familyModule));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(webSocketClientId));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(address));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(port));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(initUrl));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(initInterfaceId));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(language));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(user));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(hasClientCertificate));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(distinguishedName));
    serializedInfo->arrayValue->emplace_back(acls->toVariable());
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>((int32_t)rpcType));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>((int32_t)clientType));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(initKeepAlive));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(initBinaryMode));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(initNewFormat));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(initSubscribePeers));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(initJsonMode));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(initSendNewDevices));
    serializedInfo->arrayValue->emplace_back(std::make_shared<BaseLib::Variable>(peerId));
    return serializedInfo;
}

void BaseLib::RpcClientInfo::unserialize(BaseLib::SharedObjects* bl, BaseLib::PVariable data)
{
    if(!data) return;
    int32_t pos = 0;
    id = data->arrayValue->at(pos)->integerValue; pos++;
    sendEventsToRpcServer = data->arrayValue->at(pos)->booleanValue; pos++;
    closed = data->arrayValue->at(pos)->booleanValue; pos++;
    addon = data->arrayValue->at(pos)->booleanValue; pos++;
    flowsServer = data->arrayValue->at(pos)->booleanValue; pos++;
    scriptEngineServer = data->arrayValue->at(pos)->booleanValue; pos++;
    ipcServer = data->arrayValue->at(pos)->booleanValue; pos++;
    mqttClient = data->arrayValue->at(pos)->booleanValue; pos++;
    familyModule = data->arrayValue->at(pos)->booleanValue; pos++;
    webSocketClientId = data->arrayValue->at(pos)->stringValue; pos++;
    address = data->arrayValue->at(pos)->stringValue; pos++;
    port = data->arrayValue->at(pos)->integerValue; pos++;
    initUrl = data->arrayValue->at(pos)->stringValue; pos++;
    initInterfaceId = data->arrayValue->at(pos)->stringValue; pos++;
    language = data->arrayValue->at(pos)->stringValue; pos++;
    user = data->arrayValue->at(pos)->stringValue; pos++;
    hasClientCertificate = data->arrayValue->at(pos)->booleanValue; pos++;
    distinguishedName = data->arrayValue->at(pos)->stringValue; pos++;
    acls = std::make_shared<BaseLib::Security::Acls>(bl, id);
    acls->fromVariable(data->arrayValue->at(pos)); pos++;
    rpcType = (RpcType)data->arrayValue->at(pos)->integerValue; pos++;
    clientType = (RpcClientType)data->arrayValue->at(pos)->integerValue; pos++;
    initKeepAlive = data->arrayValue->at(pos)->booleanValue; pos++;
    initBinaryMode = data->arrayValue->at(pos)->booleanValue; pos++;
    initNewFormat = data->arrayValue->at(pos)->booleanValue; pos++;
    initSubscribePeers = data->arrayValue->at(pos)->booleanValue; pos++;
    initJsonMode = data->arrayValue->at(pos)->booleanValue; pos++;
    initSendNewDevices = data->arrayValue->at(pos)->booleanValue; pos++;
    peerId = data->arrayValue->at(pos)->integerValue64; pos++;
}