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

#ifndef SERVICEMESSAGES_H_
#define SERVICEMESSAGES_H_

#include "../Variable.h"
#include "../Encoding/BinaryEncoder.h"
#include "../Encoding/BinaryDecoder.h"
#include "../IEvents.h"
#include "../Database/DatabaseTypes.h"
#include "../Sockets/RpcClientInfo.h"

#include <string>
#include <iomanip>
#include <memory>
#include <chrono>
#include <map>
#include <mutex>
#include <vector>

namespace BaseLib
{

class SharedObjects;

namespace Systems
{
class ServiceMessages : public IEvents
{
public:
	//Event handling
	class IServiceEventSink : public IEventSinkBase
	{
	public:
		virtual void onConfigPending(bool configPending) = 0;

		virtual void onEvent(uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
		virtual void onRPCEvent(uint64_t peerId, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
		virtual void onSaveParameter(std::string name, uint32_t channel, std::vector<uint8_t>& data) = 0;
		virtual std::shared_ptr<Database::DataTable> onGetServiceMessages() = 0;
		virtual void onSaveServiceMessage(Database::DataRow& data) = 0;
		virtual void onDeleteServiceMessage(uint64_t databaseID) = 0;
		virtual void onEnqueuePendingQueues() = 0;
	};
	//End event handling

	ServiceMessages(BaseLib::SharedObjects* baseLib, int32_t familyId, uint64_t peerId, std::string peerSerial, IServiceEventSink* eventHandler);
	virtual ~ServiceMessages();

	virtual void setFamilyId(uint64_t familyId) { _familyId = familyId; }
	virtual void setPeerId(uint64_t peerId) { _peerId = peerId; }
	virtual void setPeerSerial(std::string peerSerial) { _peerSerial = peerSerial; }

	virtual void load();
	virtual void save(int64_t timestamp, uint32_t index, bool value);
	virtual void save(int64_t timestamp, int32_t channel, std::string id, uint8_t value);
	virtual bool set(std::string id, bool value);
	virtual void set(std::string id, uint8_t value, uint32_t channel);
	virtual std::shared_ptr<Variable> get(PRpcClientInfo clientInfo, bool returnID);

	virtual void setConfigPending(bool value);
	virtual bool getConfigPending() { return _configPending; }
	virtual void resetConfigPendingSetTime();
	virtual int64_t getConfigPendingSetTime() { return _configPendingSetTime; }

	virtual void setUnreach(bool value, bool requeue);
	virtual bool getUnreach() { return _unreach; }
	virtual void checkUnreach(int32_t cyclicTimeout, uint32_t lastPacketReceived);
    virtual void endUnreach();

    virtual bool getLowbat() { return _lowbat; }
protected:
    struct ErrorInfo
    {
        int64_t timestamp = 0;
        uint8_t value = 0;
    };

    BaseLib::SharedObjects* _bl = nullptr;
    std::map<uint32_t, uint32_t> _variableDatabaseIDs;
	int32_t _familyId = -1;
    uint64_t _peerId = 0;
    std::string _peerSerial;
    bool _disposing = false;
    bool _configPending = false;
    int64_t _configPendingTime = 0;
    int64_t _configPendingSetTime = 0;
    int32_t _unreachResendCounter = 0;
    bool _unreach = false;
    int64_t _unreachTime = 0;
	bool _stickyUnreach = false;
    int64_t _stickyUnreachTime = 0;
	bool _lowbat = false;
    int64_t _lowbatTime = 0;

	std::mutex _errorMutex;
	std::map<uint32_t, std::map<std::string, ErrorInfo>> _errors;

	//Event handling
	virtual void raiseConfigPending(bool configPending);

	virtual void raiseEvent(uint64_t peerId, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<BaseLib::Variable>>> values);
	virtual void raiseRPCEvent(uint64_t peerId, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values);
	virtual void raiseSaveParameter(std::string name, uint32_t channel, std::vector<uint8_t>& data);
	virtual std::shared_ptr<Database::DataTable> raiseGetServiceMessages();
	virtual void raiseSaveServiceMessage(Database::DataRow& data);
	virtual void raiseDeleteServiceMessage(uint64_t databaseID);
	virtual void raiseEnqueuePendingQueues();
	//End event handling
};

}
}
#endif /* SERVICEMESSAGES_H_ */
