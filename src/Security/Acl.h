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

#ifndef BASELIB_SECURITY_ACL_H_
#define BASELIB_SECURITY_ACL_H_

#include "../Exception.h"
#include "../Variable.h"

#include <set>

namespace BaseLib
{
namespace Security
{

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
 * This class is used to store ACL rules. The elements are checked in the following order. If a field is set to "no access", access is denied immediately without checking further rules.
 *
 * 1. Methods
 * 2. Variables
 * 3. Devices
 * 4. Rooms
 * 5. Categories
 */
class Acl
{
private:
    /**
     * Key: Room ID; value: access/no access
     *
     * Grant all entry: 0 (all rooms) => true
     * Deny all entry: 0 (all rooms) => true
     */
    std::unordered_map<uint64_t, bool> _roomsRead;

    /**
     * Key: Room ID; value: access/no access
     *
     * Grant all entry: 0 (all rooms) => true
     * Deny all entry: 0 (all rooms) => true
     */
    std::unordered_map<uint64_t, bool> _roomsWrite;

    /**
     * Key: Category ID; value: access/no access
     *
     * Grant all entry: 0 (all categories) => true
     * Deny all entry: 0 (all categories) => true
     */
    std::unordered_map<uint64_t, bool> _categoriesRead;

    /**
     * Key: Category ID; value: access/no access
     *
     * Grant all entry: 0 (all categories) => true
     * Deny all entry: 0 (all categories) => true
     */
    std::unordered_map<uint64_t, bool> _categoriesWrite;

    /**
     * Key: Peer ID; value: access/no access
     *
     * Grant all entry: 0 (all devices) => true
     * Deny all entry: 0 (all devices) => true
     */
    std::unordered_map<uint64_t, bool> _devicesRead;

    /**
     * Key: Peer ID; value: access/no access
     *
     * Grant all entry: 0 (all devices) => true
     * Deny all entry: 0 (all devices) => true
     */
    std::unordered_map<uint64_t, bool> _devicesWrite;

    /**
     * Key: Channel; key 2: variable name; value: access/no access
     *
     * Grant all entry: -2 (all channels) => "*" => true
     * Deny all entry: -2 (all channels) => "*" => true
     */
    std::unordered_map<int32_t, std::unordered_map<std::string, bool>> _variablesRead;

    /**
     * Key: Channel; key 2: variable name; value: access/no access
     *
     * Grant all entry: -2 (all channels) => "*" => true
     * Deny all entry: -2 (all channels) => "*" => true
     */
    std::unordered_map<int32_t, std::unordered_map<std::string, bool>> _variablesWrite;

    /**
     * Key: Method name; value: access/no access.
     *
     * Grant all entry: "*" => true
     * Deny all entry: "*" => true
     */
    std::unordered_map<std::string, bool> _methods;

public:
    Acl();

    /**
     * Destructor.
     */
    virtual ~Acl();

    PVariable serialize();
    void unserialize(PVariable serializedData);
};

typedef std::shared_ptr<Acl> PAcl;

}
}
#endif
