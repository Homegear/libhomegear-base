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

PVariable Acls::toVariable()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    auto serializedData = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
    serializedData->arrayValue->reserve(_acls.size());
    for(auto& acl : _acls)
    {
        serializedData->arrayValue->emplace_back(std::move(acl->toVariable()));
    }
    return serializedData;
}

void Acls::fromVariable(PVariable serializedData)
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    _acls.clear();
    _acls.reserve(serializedData->arrayValue->size());
    for(auto& element : *serializedData->arrayValue)
    {
        auto acl = std::make_shared<Acl>();
        acl->fromVariable(element);
        _acls.emplace_back(std::move(acl));
    }
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

bool Acls::rolesReadSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->rolesReadSet()) return true;
    }

    return false;
}

bool Acls::rolesWriteSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->rolesWriteSet()) return true;
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

bool Acls::roomsCategoriesRolesDevicesReadSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->roomsReadSet() || acl->categoriesReadSet() || acl->rolesReadSet() || acl->devicesReadSet()) return true;
    }

    return false;
}

bool Acls::roomsCategoriesRolesDevicesWriteSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->roomsWriteSet() || acl->categoriesWriteSet() || acl->rolesWriteSet() || acl->devicesWriteSet()) return true;
    }

    return false;
}

bool Acls::variablesReadSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->variablesReadSet()) return true;
    }

    return false;
}

bool Acls::variablesWriteSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->variablesWriteSet()) return true;
    }

    return false;
}

bool Acls::variablesRoomsCategoriesRolesReadSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->variablesReadSet() || acl->roomsReadSet() || acl->categoriesReadSet() || acl->rolesReadSet()) return true;
    }

    return false;
}

bool Acls::variablesRoomsCategoriesRolesWriteSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->variablesWriteSet() || acl->roomsWriteSet() || acl->categoriesWriteSet() || acl->rolesWriteSet()) return true;
    }

    return false;
}

bool Acls::variablesRoomsCategoriesRolesDevicesReadSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->variablesReadSet() || acl->roomsReadSet() || acl->categoriesReadSet() || acl->rolesReadSet() || acl->devicesReadSet()) return true;
    }

    return false;
}

bool Acls::variablesRoomsCategoriesRolesDevicesWriteSet()
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    for(auto& acl : _acls)
    {
        if(acl->variablesWriteSet() || acl->roomsWriteSet() || acl->categoriesWriteSet() || acl->rolesWriteSet() || acl->devicesWriteSet()) return true;
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
        if(userId == 0) return false;
        auto groups = _bl->db->getUsersGroups(userId);
        if(groups.empty()) return false;
        return fromGroups(groups);
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

bool Acls::fromGroups(std::vector<uint64_t>& groupIds)
{
    std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
    try
    {
        if(groupIds.empty()) return false;
        std::string outputPrefix = "Client " + std::to_string(_clientId) + " ACLs (groups ";
        _acls.clear();
        _acls.reserve(groupIds.size());
        for(auto& group : groupIds)
        {
            auto aclData = _bl->db->getAcl(group);
            if(aclData->errorStruct)
            {
                _out.printError("Error: Could not get ACLs of group " + std::to_string(group) + ": " + aclData->structValue->at("faultString")->stringValue);
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

    _acls.clear();
    return false;
}

bool Acls::checkServiceAccess(std::string serviceName)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkServiceAccess(serviceName);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to service " + serviceName + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Error: Access denied to service " + serviceName + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkCategoriesReadAccess(std::set<uint64_t>& categories)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkCategoriesReadAccess(categories);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to categories (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to categories (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

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
                if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to categories (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to categories (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkCategoryReadAccess(uint64_t categoryId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkCategoryReadAccess(categoryId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to categories (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to categories (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkCategoryWriteAccess(uint64_t categoryId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkCategoryWriteAccess(categoryId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to categories (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to categories (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkRolesReadAccess(std::set<uint64_t>& roles)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkRolesReadAccess(roles);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to roles (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to roles (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkRolesWriteAccess(std::set<uint64_t>& roles)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkRolesWriteAccess(roles);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to roles (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to roles (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkRoleReadAccess(uint64_t roleId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkRoleReadAccess(roleId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to role (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to role (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkRoleWriteAccess(uint64_t roleId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkRoleWriteAccess(roleId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to role (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to role (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkDeviceReadAccess(std::shared_ptr<Systems::Peer> peer)
{
    try
    {
        if(!peer) return false;
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkDeviceReadAccess(peer);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to peer ID " + std::to_string(peer->getID()) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to peer ID " + std::to_string(peer->getID()) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkDeviceWriteAccess(std::shared_ptr<Systems::Peer> peer)
{
    try
    {
        if(!peer) return false;
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkDeviceWriteAccess(peer);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to peer ID " + std::to_string(peer->getID()) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to peer ID " + std::to_string(peer->getID()) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkEventServerMethodAccess(std::string methodName)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkEventServerMethodAccess(methodName);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to event server method " + methodName + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Error: Access denied to event server method " + methodName + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
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
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Error: Access denied to method " + methodName + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkMethodAndCategoryReadAccess(std::string methodName, uint64_t categoryId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkMethodAndCategoryReadAccess(methodName, categoryId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or category " + std::to_string(categoryId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or category " + std::to_string(categoryId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
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
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or category " + std::to_string(categoryId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or category " + std::to_string(categoryId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkMethodAndRoleReadAccess(std::string methodName, uint64_t roleId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkMethodAndRoleReadAccess(methodName, roleId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or role " + std::to_string(roleId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or role " + std::to_string(roleId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkMethodAndRoleWriteAccess(std::string methodName, uint64_t roleId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkMethodAndRoleWriteAccess(methodName, roleId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or role " + std::to_string(roleId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or role " + std::to_string(roleId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkMethodAndRoomReadAccess(std::string methodName, uint64_t roomId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkMethodAndRoomReadAccess(methodName, roomId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or room " + std::to_string(roomId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or room " + std::to_string(roomId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
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
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or room " + std::to_string(roomId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or room " + std::to_string(roomId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
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
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or peer " + std::to_string(peerId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to method " + methodName + " or peer " + std::to_string(peerId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkRoomReadAccess(uint64_t roomId)
{
    try
    {
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkRoomReadAccess(roomId);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to room " + std::to_string(roomId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to room " + std::to_string(roomId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
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
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to room " + std::to_string(roomId) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to room " + std::to_string(roomId) + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkSystemVariableReadAccess(Database::PSystemVariable systemVariable)
{
    try
    {
        if(!systemVariable) return false;
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkSystemVariableReadAccess(systemVariable);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to system variable " + systemVariable->name + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to system variable " + systemVariable->name + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkSystemVariableWriteAccess(Database::PSystemVariable systemVariable)
{
    try
    {
        if(!systemVariable) return false;
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkSystemVariableWriteAccess(systemVariable);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to system variable " + systemVariable->name + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to system variable " + systemVariable->name + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkVariableReadAccess(std::shared_ptr<Systems::Peer> peer, int32_t channel, const std::string& variableName)
{
    try
    {
        if(!peer) return false;
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkVariableReadAccess(peer, channel, variableName);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to variable " + variableName + " on channel " + std::to_string(channel) + " of peer " + std::to_string(peer->getID()) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to system variable " + variableName + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

bool Acls::checkVariableWriteAccess(std::shared_ptr<Systems::Peer> peer, int32_t channel, const std::string& variableName)
{
    try
    {
        if(!peer) return false;
        std::lock_guard<std::mutex> aclsGuard(_aclsMutex);
        bool acceptSet = false;
        for(auto& acl : _acls)
        {
            auto result = acl->checkVariableWriteAccess(peer, channel, variableName);
            if(result == AclResult::error || result == AclResult::deny)
            {
                if(_bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to variable " + variableName + " on channel " + std::to_string(channel) + " of peer " + std::to_string(peer->getID()) + " (1).");
                return false;
            }
            else if(result == AclResult::accept) acceptSet = true;
        }

        if(!acceptSet && _bl->debugLevel >= 5) _out.printDebug("Debug: Access denied to system variable " + variableName + " (2).");
        return acceptSet;
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    return false;
}

}
}