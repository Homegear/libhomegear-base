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

#ifndef BASELIB_SECURITY_ACL_H_
#define BASELIB_SECURITY_ACL_H_

#include "../Exception.h"
#include "../Variable.h"
#include "../Database/IDatabaseController.h"

#include <set>

namespace BaseLib
{

namespace Systems
{
    class Peer;
}

namespace Security
{

enum class AclResult
{
    error = -3,
    notInList = -2,
    deny = -1,
    accept = 0
};

/**
 * Exception class for Acl.
 *
 * @see Acl
 */
class AclException : public Exception
{
public:
    AclException(std::string message) : Exception(message) {}
};

/**
 * This class is used to store ACL rules. The elements are checked in the following order. Unset elements are skipped. If a field is set to "no access", access is denied immediately without checking further rules. When set to "access", the rule checking continues.
 *
 * 1. Variables
 * 2. Devices
 * 3. Rooms
 * 4. Categories
 * 5. Roles
 * 6. Methods
 * 7. Event Server Methods
 *
 * Security needs to be applied here:
 *
 * RpcServer:
 *  - Most RPC methods are defined in file RPCMethods.cpp
 *  - Variable with all methods from RPCMethods.cpp: _rpcMethods
 *  - If not found in map => IpcServer::callRpcMethod()
 *
 * IpcServer:
 *  - All methods on clients are called within callRpcMethod()
 *  - Server methods are defined in _rpcMethods and _localRpcMethods
 *  - Variable with all methods from RPCMethods.cpp: _rpcMethods
 *  - Has broadcast methods
 *  - Has it's own RPC methods
 *
 * NodeBlueServer:
 *  - Server methods are defined in _rpcMethods and _localRpcMethods
 *  - Variable with all methods from RPCMethods.cpp: _rpcMethods
 *  - If not found in above maps => IpcServer::callRpcMethod()
 *  - Has broadcast methods
 *  - Has it's own RPC methods
 *
 * ScriptEngineServer:
 *  - Server methods are defined in _rpcMethods and _localRpcMethods
 *  - Variable with all methods from RPCMethods.cpp: _rpcMethods
 *  - If not found in above maps => IpcServer::callRpcMethod()
 *  - Has broadcast methods
 *  - Has it's own RPC methods
 *
 * RPC::Client
 *  - Has broadcast methods
 *
 * Mqtt
 *  - Check method access
 */
class Acl
{
private:
    /**
     * When set to "true", _devicesRead is included in the ACL checks.
     */
    bool _devicesReadSet = false;

    /**
     * Key: Peer ID; value: access/no access
     *
     * Grant all entry: 0 (all devices) => true, can be combined with "no access" entries.
     */
    std::unordered_map<uint64_t, bool> _devicesRead;

    /**
     * When set to "true", _devicesWrite is included in the ACL checks.
     */
    bool _devicesWriteSet = false;

    /**
     * Key: Peer ID; value: access/no access
     *
     * Grant all entry: 0 (all devices) => true, can be combined with "no access" entries.
     */
    std::unordered_map<uint64_t, bool> _devicesWrite;

    /**
     * When set to "true", _variablesRead is included in the ACL checks.
     */
    bool _variablesReadSet = false;

    /**
     * Key1: Device; key2: channel; key 3: variable name; value: access/no access
     *
     * For system variables key1 is "0",  and channel is "-1".
     *
     * Grant all entry for device variables: 0 (all devices) => -3 (all channels) => "*" => true, can be combined with "no access" entries.
     * Grant all entry for metadata: 0 (all devices) => -2 => "*" => true, can be combined with "no access" entries.
     * Grant all entry for system variables: 0 (all devices) => -1 => "*" => true, can be combined with "no access" entries.
     * Grant all entry for specific metadata: Peer ID => -2 => "*" => true, can be combined with "no access" entries.
     * Grant all entry for all channels of a peer: Peer ID => -3 => "*" => true, can be combined with "no access" entries.
     */
    std::unordered_map<uint64_t, std::unordered_map<int32_t, std::unordered_map<std::string, bool>>> _variablesRead;

    /**
     * When set to "true", _variablesWrite is included in the ACL checks.
     */
    bool _variablesWriteSet = false;

    /**
     * Key1: Device; key2: Channel; key 3: variable name; value: access/no access
     *
     * For system variables key1 is "0",  and channel is "-1".
     *
     * Grant all entry for device variables: 0 (all devices) => -3 (all channels) => "*" => true, can be combined with "no access" entries.
     * Grant all entry for metadata: 0 (all devices) => -2 => "*" => true, can be combined with "no access" entries.
     * Grant all entry for system variables: 0 (all devices) => -1 => "*" => true, can be combined with "no access" entries.
     * Grant all entry for specific metadata: Peer ID => -2 => "*" => true, can be combined with "no access" entries.
     * Grant all entry for all channels of a peer: Peer ID => -3 => "*" => true, can be combined with "no access" entries.
     */
    std::unordered_map<uint64_t, std::unordered_map<int32_t, std::unordered_map<std::string, bool>>> _variablesWrite;

    /**
     * When set to "true", _roomsRead is included in the ACL checks.
     */
    bool _roomsReadSet = false;

    /**
     * Key: Room ID; value: access/no access
     */
    std::unordered_map<uint64_t, bool> _roomsRead;

    /**
     * When set to "true", _roomsWrite is included in the ACL checks.
     */
    bool _roomsWriteSet = false;

    /**
     * Key: Room ID; value: access/no access
     *
     * ID 0 stands for "no room assigned"
     */
    std::unordered_map<uint64_t, bool> _roomsWrite;

    /**
     * When set to "true", _categoriesRead is included in the ACL checks.
     */
    bool _categoriesReadSet = false;

    /**
     * Key: Category ID; value: access/no access
     *
     * ID 0 stands for "no category assigned"
     */
    std::unordered_map<uint64_t, bool> _categoriesRead;

    /**
     * When set to "true", _categoriesWrite is included in the ACL checks.
     */
    bool _categoriesWriteSet = false;

    /**
     * Key: Category ID; value: access/no access
     */
    std::unordered_map<uint64_t, bool> _categoriesWrite;

    /**
     * When set to "true", _rolesRead is included in the ACL checks.
     */
    bool _rolesReadSet = false;

    /**
     * Key: Role ID; value: access/no access
     *
     * ID 0 stands for "no role assigned"
     */
    std::unordered_map<uint64_t, bool> _rolesRead;

    /**
     * When set to "true", _rolesWrite is included in the ACL checks.
     */
    bool _rolesWriteSet = false;

    /**
     * Key: Role ID; value: access/no access
     */
    std::unordered_map<uint64_t, bool> _rolesWrite;

    /**
     * When set to "true", _methods is included in the ACL checks.
     */
    bool _methodsSet = false;

    /**
     * Key: Method name; value: access/no access.
     *
     * Grant all entry: "*" => true
     * Deny all entry: "*" => false
     */
    std::unordered_map<std::string, bool> _methods;

    /**
     * When set to "true", _eventServerMethods is included in the ACL checks.
     */
    bool _eventServerMethodsSet = false;

    /**
     * Key: Method name; value: access/no access.
     *
     * Grant all entry: "*" => true
     * Deny all entry: "*" => false
     */
    std::unordered_map<std::string, bool> _eventServerMethods;

    /**
     * When set to "true", _services is included in the ACL checks.
     */
    bool _servicesSet = false;

    /**
     * Key: Service name; value: access/no access.
     *
     * Grant all entry: "*" => true
     * Deny all entry: "*" => false
     */
    std::unordered_map<std::string, bool> _services;
public:
    Acl();

    /**
     * Destructor.
     */
    virtual ~Acl();

    bool categoriesReadSet() { return _categoriesReadSet; }
    bool categoriesWriteSet() { return _categoriesWriteSet; }
    bool rolesReadSet() { return _rolesReadSet; }
    bool rolesWriteSet() { return _rolesWriteSet; }
    bool devicesReadSet() { return _devicesReadSet; }
    bool devicesWriteSet() { return _devicesWriteSet; }
    bool roomsReadSet() { return _roomsReadSet; }
    bool roomsWriteSet() { return _roomsWriteSet; }
    bool variablesReadSet() { return _variablesReadSet; }
    bool variablesWriteSet() { return _variablesWriteSet; }

    PVariable toVariable();

    /**
     * Converts a Variable structure to an ACL. This is not thread safe, so make sure no checks are being executed when calling this method!
     *
     * @param serializedData The structure to convert to an ACL.
     */
    void fromVariable(PVariable serializedData);

    AclResult checkServiceAccess(std::string& serviceName);
    AclResult checkCategoriesReadAccess(std::set<uint64_t>& categories);
    AclResult checkCategoriesWriteAccess(std::set<uint64_t>& categories);
    AclResult checkCategoryReadAccess(uint64_t category);
    AclResult checkCategoryWriteAccess(uint64_t category);
    AclResult checkRolesReadAccess(std::set<uint64_t>& roles);
    AclResult checkRolesWriteAccess(std::set<uint64_t>& roles);
    AclResult checkRoleReadAccess(uint64_t role);
    AclResult checkRoleWriteAccess(uint64_t role);
    AclResult checkDeviceReadAccess(std::shared_ptr<Systems::Peer> peer);
    AclResult checkDeviceWriteAccess(std::shared_ptr<Systems::Peer> peer);
    AclResult checkEventServerMethodAccess(std::string& methodName);
    AclResult checkMethodAccess(std::string& methodName);
    AclResult checkMethodAndCategoryReadAccess(std::string& methodName, uint64_t categoryId);
    AclResult checkMethodAndCategoryWriteAccess(std::string& methodName, uint64_t categoryId);
    AclResult checkMethodAndRoleReadAccess(std::string& methodName, uint64_t roleId);
    AclResult checkMethodAndRoleWriteAccess(std::string& methodName, uint64_t roleId);
    AclResult checkMethodAndRoomReadAccess(std::string& methodName, uint64_t roomId);
    AclResult checkMethodAndRoomWriteAccess(std::string& methodName, uint64_t roomId);
    AclResult checkMethodAndDeviceWriteAccess(std::string& methodName, uint64_t peerId);
    AclResult checkRoomReadAccess(uint64_t roomId);
    AclResult checkRoomWriteAccess(uint64_t roomId);
    AclResult checkSystemVariableReadAccess(Database::PSystemVariable systemVariable);
    AclResult checkSystemVariableWriteAccess(Database::PSystemVariable systemVariable);
    AclResult checkVariableReadAccess(std::shared_ptr<Systems::Peer> peer, int32_t channel, const std::string& variableName);
    AclResult checkVariableWriteAccess(std::shared_ptr<Systems::Peer> peer, int32_t channel, const std::string& variableName);

    std::string toString(int32_t indentation = 0);
};

typedef std::shared_ptr<Acl> PAcl;

}
}
#endif
