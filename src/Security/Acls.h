//
// Created by sathya on 25.02.18.
//

#ifndef LIBHOMEGEAR_BASE_ACLS_H
#define LIBHOMEGEAR_BASE_ACLS_H

#include "Acl.h"
#include "../Output/Output.h"

#include <mutex>

namespace BaseLib
{

class SharedObjects;

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
    bool devicesReadSet();
    bool devicesWriteSet();
    bool roomsReadSet();
    bool roomsWriteSet();

    void clear();
    bool fromUser(std::string& userName);
    bool fromGroups(std::vector<uint64_t>& groupIds);

    /**
     * Checks if the ACLs grant access to one or more categories.
     *
     * @param categories The IDs of the categories to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkCategoriesWriteAccess(std::set<uint64_t>& categories);

    /**
     * Checks if the ACLs grant access to a device.
     *
     * @param peerId The ID of the peer to check.
     * @return This method returns "false" if (1) access is explicitly denied in one of the ACLs, (2) on error or (3) if the checked entity is not in at least one of the ACLs. It returns "true" if (1) the checked entity is not part of all ACLs or (2) if access is granted in at least one ACL.
     */
    bool checkDeviceWriteAccess(uint64_t peerId);

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
    bool checkMethodAndCategoryWriteAccess(std::string methodName, uint64_t categoryId);

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
    bool checkRoomWriteAccess(uint64_t roomId);
};
typedef std::shared_ptr<Acls> PAcls;

}
}

#endif
