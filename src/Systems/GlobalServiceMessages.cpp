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

#include "GlobalServiceMessages.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Systems
{

GlobalServiceMessages::GlobalServiceMessages()
{
}

GlobalServiceMessages::~GlobalServiceMessages()
{

}

void GlobalServiceMessages::init(SharedObjects* baseLib)
{
    _bl = baseLib;
    _rpcDecoder = std::unique_ptr<BaseLib::Rpc::RpcDecoder>(new BaseLib::Rpc::RpcDecoder(baseLib, false, false));
    _rpcEncoder = std::unique_ptr<BaseLib::Rpc::RpcEncoder>(new BaseLib::Rpc::RpcEncoder(baseLib, false, true));
}

void GlobalServiceMessages::load()
{
    try
    {
        std::lock_guard<std::mutex> serviceMessagesGuard(_serviceMessagesMutex);
        std::shared_ptr<BaseLib::Database::DataTable> rows = _bl->db->getServiceMessages(0);
        for(auto& row : *rows)
        {
            auto serviceMessage = std::make_shared<ServiceMessage>();
            serviceMessage->databaseId = (uint64_t)row.second.at(0)->intValue;
            serviceMessage->familyId = row.second.at(1)->intValue;
            serviceMessage->messageId = row.second.at(3)->intValue;
            serviceMessage->messageSubId = row.second.at(4)->textValue;
            serviceMessage->timestamp = row.second.at(5)->intValue;
            serviceMessage->message = row.second.at(7)->textValue;
            serviceMessage->value = row.second.at(6)->intValue;
            serviceMessage->data = _rpcDecoder->decodeResponse(*row.second.at(9)->binaryValue);
            _serviceMessages[row.second.at(1)->intValue][row.second.at(3)->intValue][row.second.at(4)->textValue].emplace(row.second.at(7)->textValue, std::move(serviceMessage));
        }
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
}

void GlobalServiceMessages::set(int32_t familyId, int32_t messageId, std::string messageSubId, int32_t timestamp, std::string message, std::list<std::string> variables, PVariable data, int64_t value)
{
    try
    {
        auto serviceMessage = std::make_shared<ServiceMessage>();
        serviceMessage->familyId = familyId;
        serviceMessage->messageId = messageId;
        serviceMessage->messageSubId = messageSubId;
        serviceMessage->timestamp = timestamp;
        serviceMessage->message = message;
        serviceMessage->variables = variables;
        serviceMessage->value = value;
        serviceMessage->data = data;

        {
            std::lock_guard<std::mutex> serviceMessagesGuard(_serviceMessagesMutex);
            _serviceMessages[familyId][messageId][messageSubId][message] = std::move(serviceMessage);
        }

        std::vector<char> variablesBlob;
        auto variablesIn = std::make_shared<Variable>(VariableType::tArray);
        variablesIn->arrayValue->reserve(variables.size());
        for(auto& variable : variables)
        {
            variablesIn->arrayValue->emplace_back(std::make_shared<Variable>(variable));
        }
        _rpcEncoder->encodeResponse(variablesIn, variablesBlob);

        std::vector<char> dataBlob;
        if(data) _rpcEncoder->encodeResponse(data, dataBlob);

        Database::DataRow data;
        data.push_back(std::make_shared<Database::DataColumn>(messageSubId)); //Selector for WHERE clause
        data.push_back(std::make_shared<Database::DataColumn>(message)); //Selector for WHERE clause
        data.push_back(std::make_shared<Database::DataColumn>(familyId)); //familyID
        data.push_back(std::make_shared<Database::DataColumn>(0)); //peerID
        data.push_back(std::make_shared<Database::DataColumn>(messageId)); //messageID
        data.push_back(std::make_shared<Database::DataColumn>(messageSubId)); //messageSubID
        data.push_back(std::make_shared<Database::DataColumn>(timestamp)); //timestamp
        data.push_back(std::make_shared<Database::DataColumn>(value)); //integerValue
        data.push_back(std::make_shared<Database::DataColumn>(message)); //message
        data.push_back(std::make_shared<Database::DataColumn>(variablesBlob)); //variables
        data.push_back(std::make_shared<Database::DataColumn>(dataBlob)); //binaryData
        _bl->db->saveGlobalServiceMessageAsynchronous(data);
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
}

void GlobalServiceMessages::unset(int32_t familyId, int32_t messageId, std::string messageSubId, std::string message)
{
    try
    {
        {
            std::lock_guard<std::mutex> serviceMessagesGuard(_serviceMessagesMutex);
            auto familyIterator = _serviceMessages.find(familyId);
            if(familyIterator != _serviceMessages.end())
            {
                auto messageIdIterator = familyIterator->second.find(messageId);
                if(messageIdIterator != familyIterator->second.end())
                {
                    auto messageSubIdIterator = messageIdIterator->second.find(messageSubId);
                    if(messageSubIdIterator != messageIdIterator->second.end())
                    {
                        auto messageIterator = messageSubIdIterator->second.find(message);
                        if(messageIterator != messageSubIdIterator->second.end())
                        {
                            messageSubIdIterator->second.erase(messageIterator);

                            _bl->db->deleteGlobalServiceMessage(familyId, messageId, messageSubId, message);
                        }

                        if(messageSubIdIterator->second.empty()) messageIdIterator->second.erase(messageSubIdIterator);
                    }

                    if(messageIdIterator->second.empty()) familyIterator->second.erase(messageIdIterator);
                }

                if(familyIterator->second.empty()) _serviceMessages.erase(familyId);
            }
        }
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
}

PVariable GlobalServiceMessages::get(PRpcClientInfo clientInfo)
{
    try
    {
        std::lock_guard<std::mutex> serviceMessagesGuard(_serviceMessagesMutex);
        PVariable serviceMessages(new Variable(VariableType::tArray));
        serviceMessages->arrayValue->reserve(100);
        for(auto& family : _serviceMessages)
        {
            for(auto& messageId : family.second)
            {
                for(auto& messageSubId : messageId.second)
                {
                    for(auto& message : messageSubId.second)
                    {
                        auto element = std::make_shared<Variable>(VariableType::tStruct);
                        element->structValue->emplace("TYPE", std::make_shared<Variable>(message.second->familyId == -1 ? 0 : 1));
                        if(message.second->familyId != -1) element->structValue->emplace("FAMILY_ID", std::make_shared<Variable>(message.second->familyId));
                        element->structValue->emplace("TIMESTAMP", std::make_shared<Variable>(message.second->timestamp));
                        element->structValue->emplace("MESSAGE_ID", std::make_shared<Variable>(message.second->messageId));
                        element->structValue->emplace("MESSAGE_SUBID", std::make_shared<Variable>(message.second->messageSubId));
                        element->structValue->emplace("MESSAGE", std::make_shared<Variable>(message.second->message));
                        auto variables = std::make_shared<Variable>(VariableType::tArray);
                        variables->arrayValue->reserve(message.second->variables.size());
                        for(auto& variable : message.second->variables)
                        {
                            variables->arrayValue->emplace_back(std::make_shared<Variable>(variable));
                        }
                        element->structValue->emplace("VARIABLES", variables);
                        element->structValue->emplace("DATA", message.second->data);
                        element->structValue->emplace("VALUE", std::make_shared<Variable>(message.second->value));
                        serviceMessages->arrayValue->push_back(element);
                        if(serviceMessages->arrayValue->size() == serviceMessages->arrayValue->capacity()) serviceMessages->arrayValue->reserve(serviceMessages->arrayValue->size() + 100);
                    }
                }
            }
        }
        return serviceMessages;
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return Variable::createError(-32500, "Unknown application error.");
}

}
}
