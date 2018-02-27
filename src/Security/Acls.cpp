//
// Created by sathya on 25.02.18.
//

#include "Acls.h"

namespace BaseLib
{
namespace Security
{

Acls::Acls(BaseLib::SharedObjects* bl)
{
    _bl = bl;
}

Acls::~Acls()
{
    clear();
}

void Acls::clear()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    _acls.clear();
}

bool Acls::fromUser(std::string& userName)
{
    try
    {
        uint64_t userId = _bl->db->getUserId(userName);
        auto groups = _bl->db->getUsersGroups(userId);
        return fromGroups(groups);
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return false;
}

bool Acls::fromGroups(std::vector<uint64_t>& groupIds)
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    try
    {
        _acls.clear();
        _acls.reserve(groupIds.size());
        for(auto& group : groupIds)
        {
            auto aclData = _bl->db->getAcl(group);
            if(aclData->errorStruct)
            {
                _acls.clear();
                return false;
            }

            PAcl acl = std::make_shared<Acl>();
            acl->fromVariable(aclData); //Throws AclException on error => return false
            _acls.push_back(acl);
        }

        return true;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    _acls.clear();
    return false;
}

bool Acls::checkMethodAccess(std::string methodName)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkMethodAccess(methodName);
            if(result == AclResult::error || result == AclResult::deny) return false;
            else if(result == AclResult::accept) acceptSet = true;
        }

        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    return false;
}

bool Acls::checkMethodAndCategoryWriteAccess(std::string methodName, uint64_t categoryId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkMethodAndCategoryWriteAccess(methodName, categoryId);
            if(result == AclResult::error || result == AclResult::deny) return false;
            else if(result == AclResult::accept) acceptSet = true;
        }

        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    return false;
}

bool Acls::checkMethodAndRoomWriteAccess(std::string methodName, uint64_t roomId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkMethodAndRoomWriteAccess(methodName, roomId);
            if(result == AclResult::error || result == AclResult::deny) return false;
            else if(result == AclResult::accept) acceptSet = true;
        }

        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    return false;
}

}
}