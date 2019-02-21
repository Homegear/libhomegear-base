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

#ifndef LIBHOMEGEAR_BASE_ACLS_H
#define LIBHOMEGEAR_BASE_ACLS_H

#include "Acl.h"
#include "../Output/Output.h"

#include <mutex>

namespace BaseLib
{

class SharedObjects;

namespace Systems
{
    class Peer;
}

namespace Security
{

class Acls
{
private:
    BaseLib::SharedObjects* _bl = nullptr;
    int32_t _clientId = -1;
    BaseLib::Output _out;
    std::mutex _aclsMutex;
    std::vector<PAcl> _acls;
public:
    Acls(BaseLib::SharedObjects* bl, int32_t clientId);
    ~Acls();

    bool categoriesReadSet();
    bool categoriesWriteSet();
    bool rolesReadSet();
    bool rolesWriteSet();
    bool devicesReadSet();
    bool devicesWriteSet();
    bool roomsReadSet();
    bool roomsWriteSet();
    bool roomsCategoriesDevicesReadSet();
    bool roomsCategoriesDevicesWriteSet();
    bool variablesReadSet();
    bool variablesWriteSet();
    bool variablesRoomsCategoriesDevicesReadSet();
    bool variablesRoomsCategoriesDevicesWriteSet();
    bool variablesRoomsCategoriesReadSet();
    bool variablesRoomsCategoriesWriteSet();

    void clear();
    bool fromUser(std::string& userName);
    bool fromGroups(std::vector<uint64_t>& groupIds);

    /**
     * Checks if the ACLs grant access to a service.
     *
     * @param serviceName The name of the service to check.
     * @return "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the service is not in at least one of the ACLs. "true" if (1) the service is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkServiceAccess(std::string serviceName);

    /**
     * Checks if the ACLs grant access to one or more categories.
     *
     * @param categories The IDs of the categories to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkCategoriesReadAccess(std::set<uint64_t>& categories);

    /**
     * Checks if the ACLs grant access to one or more categories.
     *
     * @param categories The IDs of the categories to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkCategoriesWriteAccess(std::set<uint64_t>& categories);

    /**
     * Checks if the ACLs grant access to a category.
     *
     * @param roomId The ID of the room to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkCategoryReadAccess(uint64_t categoryId);

    /**
     * Checks if the ACLs grant access to a category.
     *
     * @param roomId The ID of the room to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkCategoryWriteAccess(uint64_t categoryId);

    /**
     * Checks if the ACLs grant access to a device. Also checks the room and categories assigned to the device.
     *
     * @param peer The peer to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkDeviceReadAccess(std::shared_ptr<Systems::Peer> peer);

    /**
     * Checks if the ACLs grant access to a device. Also checks the room and categories assigned to the device.
     *
     * @param peer The peer to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkDeviceWriteAccess(std::shared_ptr<Systems::Peer> peer);

    /**
     * Checks if the ACLs grant access to an event server method.
     *
     * @param methodName The name of the method to check.
     * @return "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the method is not in at least one of the ACLs. "true" if (1) the method is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkEventServerMethodAccess(std::string methodName);

    /**
     * Checks if the ACLs grant access to a method.
     *
     * @param methodName The name of the method to check.
     * @return "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the method is not in at least one of the ACLs. "true" if (1) the method is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkMethodAccess(std::string methodName);

    /**
     * Checks if the ACLs grant access to a method and category.
     *
     * @param methodName The name of the method to check.
     * @param categoryId The ID of the category to check.
     * @return "methodName" and "categoryId" are checked individually. This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkMethodAndCategoryReadAccess(std::string methodName, uint64_t categoryId);

    /**
     * Checks if the ACLs grant access to a method and category.
     *
     * @param methodName The name of the method to check.
     * @param categoryId The ID of the category to check.
     * @return "methodName" and "categoryId" are checked individually. This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkMethodAndCategoryWriteAccess(std::string methodName, uint64_t categoryId);

    /**
     * Checks if the ACLs grant access to a method and room.
     *
     * @param methodName The name of the method to check.
     * @param roomId The ID of the room to check.
     * @return "methodName" and "roomId" are checked individually. This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkMethodAndRoomReadAccess(std::string methodName, uint64_t roomId);

    /**
     * Checks if the ACLs grant access to a method and room.
     *
     * @param methodName The name of the method to check.
     * @param roomId The ID of the room to check.
     * @return "methodName" and "roomId" are checked individually. This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkMethodAndRoomWriteAccess(std::string methodName, uint64_t roomId);

    /**
     * Checks if the ACLs grant access to a method and device.
     *
     * @param methodName The name of the method to check.
     * @param peerId The ID of the peer to check.
     * @return "methodName" and "peerId" are checked individually. This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkMethodAndDeviceWriteAccess(std::string methodName, uint64_t peerId);

    /**
     * Checks if the ACLs grant access to a room.
     *
     * @param roomId The ID of the room to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkRoomReadAccess(uint64_t roomId);

    /**
     * Checks if the ACLs grant access to a room.
     *
     * @param roomId The ID of the room to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkRoomWriteAccess(uint64_t roomId);

    /**
     * Checks if the ACLs grant access to a system variable.
     *
     * @param systemVariable The system variable to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkSystemVariableReadAccess(Database::PSystemVariable systemVariable);

    /**
     * Checks if the ACLs grant access to a system variable.
     *
     * @param systemVariable The system variable to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkSystemVariableWriteAccess(Database::PSystemVariable systemVariable);

    /**
     * Checks if the ACLs grant access to a variable.
     *
     * @param peer The peer to check.
     * @param channel The channel to check.
     * @param variableName The variable name to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkVariableReadAccess(std::shared_ptr<Systems::Peer> peer, int32_t channel, const std::string& variableName);

    /**
     * Checks if the ACLs grant access to a variable.
     *
     * @param peer The peer to check.
     * @param channel The channel to check.
     * @param variableName The variable name to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkVariableWriteAccess(std::shared_ptr<Systems::Peer> peer, int32_t channel, const std::string& variableName);
};
typedef std::shared_ptr<Acls> PAcls;

}
}

#endif
