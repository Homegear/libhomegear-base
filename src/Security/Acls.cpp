//
// Created by sathya on 25.02.18.
//

#include "Acls.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Security
{

Acls::Acls(BaseLib::SharedObjects* bl, int32_t clientId)
{
    _bl = bl;
    _out.init(bl);
    _clientId = clientId;
    _out.setPrefix("Client " + std::to_string(clientId) + " ACLs: ");
}

Acls::~Acls()
{
    clear();
}

bool Acls::categoriesReadSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->categoriesReadSet()) return true;
    }

    return false;
}

bool Acls::categoriesWriteSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->categoriesWriteSet()) return true;
    }

    return false;
}

bool Acls::devicesReadSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->devicesReadSet()) return true;
    }

    return false;
}

bool Acls::devicesWriteSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->devicesWriteSet()) return true;
    }

    return false;
}

bool Acls::roomsReadSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->roomsReadSet()) return true;
    }

    return false;
}

bool Acls::roomsWriteSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->roomsWriteSet()) return true;
    }

    return false;
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
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return false;
}

bool Acls::fromGroups(std::vector<uint64_t>& groupIds)
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    try
    {
        std::string outputPrefix = "Client " + std::to_string(_clientId) + " ACLs (groups ";
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
            outputPrefix += std::to_string(group) + ", ";
        }

        outputPrefix = outputPrefix.substr(0, outputPrefix.size() - 2) + "): ";
        _out.setPrefix(outputPrefix);

        return true;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    _acls.clear();
    return false;
}

bool Acls::checkCategoriesWriteAccess(std::set<uint64_t>& categories)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkCategoriesWriteAccess(categories);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(!acceptSet) _out.printError("Error: Access denied to categories (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet) _out.printError("Error: Access denied to categories (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    return false;
}

bool Acls::checkDeviceWriteAccess(std::shared_ptr<Systems::Peer> peer)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkDeviceWriteAccess(peer);
            if(result == AclResult::error || result == AclResult::deny)
            {
                _out.printError("Error: Access denied to peer ID " + std::to_string(peer->getID()) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet) _out.printError("Error: Access denied to peer ID " + std::to_string(peer->getID()) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

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
            if(result == AclResult::error || result == AclResult::deny)
            {
                _out.printError("Error: Access denied to method " + methodName + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet) _out.printError("Error: Access denied to method " + methodName + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
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
            if(result == AclResult::error || result == AclResult::deny)
            {
                _out.printError("Error: Access denied to method " + methodName + " or category " + std::to_string(categoryId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet) _out.printError("Error: Access denied to method " + methodName + " or category " + std::to_string(categoryId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
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
            if(result == AclResult::error || result == AclResult::deny)
            {
                _out.printError("Error: Access denied to method " + methodName + " or room " + std::to_string(roomId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet) _out.printError("Error: Access denied to method " + methodName + " or room " + std::to_string(roomId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    return false;
}

bool Acls::checkMethodAndDeviceWriteAccess(std::string methodName, uint64_t peerId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkMethodAndDeviceWriteAccess(methodName, peerId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                _out.printError("Error: Access denied to method " + methodName + " or peer " + std::to_string(peerId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet) _out.printError("Error: Access denied to method " + methodName + " or peer " + std::to_string(peerId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    return false;
}

bool Acls::checkRoomWriteAccess(uint64_t roomId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkRoomWriteAccess(roomId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                _out.printError("Error: Access denied to room " + std::to_string(roomId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet) _out.printError("Error: Access denied to room " + std::to_string(roomId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }

    return false;
}

}
}