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

#ifndef GLOBALSERVICEMESSAGES_H_
#define GLOBALSERVICEMESSAGES_H_

#include "../Variable.h"
#include "../Sockets/RpcClientInfo.h"
#include "../Encoding/RpcDecoder.h"
#include "../Encoding/RpcEncoder.h"

#include <mutex>

namespace BaseLib
{

class SharedObjects;

namespace Systems
{
class GlobalServiceMessages
{
public:
    GlobalServiceMessages();
    virtual ~GlobalServiceMessages();

    void init(BaseLib::SharedObjects* baseLib);
    void load();

    void set(int32_t familyId, int32_t messageId, std::string messageSubId, int32_t timestamp, std::string message, std::list<std::string> variables, PVariable data = PVariable(), int64_t value = 0);
    void unset(int32_t familyId, int32_t messageId, std::string messageSubId, std::string message);

    std::shared_ptr<Variable> get(PRpcClientInfo clientInfo);
protected:
    struct ServiceMessage
    {
        uint64_t databaseId = 0;
        int32_t familyId = 0;
        int32_t messageId = 0;
        std::string messageSubId;
        int32_t timestamp = 0;
        std::string message;
        std::list<std::string> variables;
        int64_t value = 0;
        PVariable data;
    };
    typedef std::shared_ptr<ServiceMessage> PServiceMessage;
    typedef int32_t FamilyId;
    typedef int32_t MessageId;
    typedef std::string MessageSubId;
    typedef std::string MessageType;

    BaseLib::SharedObjects* _bl = nullptr;

    std::unique_ptr<Rpc::RpcDecoder> _rpcDecoder;
    std::unique_ptr<Rpc::RpcEncoder> _rpcEncoder;

    std::mutex _serviceMessagesMutex;
    std::unordered_map<FamilyId, std::unordered_map<MessageId, std::unordered_map<MessageSubId, std::unordered_map<MessageType, PServiceMessage>>>> _serviceMessages;
};

}
}
#endif
