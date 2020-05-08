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

#ifndef IDEVICEFAMILY_H_
#define IDEVICEFAMILY_H_

#include "FamilySettings.h"
#include "../Database/DatabaseTypes.h"
#include "ICentral.h"
#include "PhysicalInterfaceSettings.h"
#include "IPhysicalInterface.h"
#include "../Variable.h"
#include "../DeviceDescription/Devices.h"

#include <string>
#include <memory>

#include "PhysicalInterfaces.h"

namespace BaseLib
{

class SharedObjects;

namespace Systems
{
enum class FamilyType
{
    unknown = 0,
    sharedObject = 1,
    socket = 2
};

class IFamilyEventSink : public IEventSinkBase
{
public:
    //Hooks
    virtual void onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers) = 0;
    virtual void onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers) = 0;

    virtual void onRPCEvent(std::string source, uint64_t id, int32_t channel, std::string deviceAddress, std::shared_ptr<std::vector<std::string>> valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
    virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint) = 0;
    virtual void onRPCNewDevices(std::vector<uint64_t>& ids, std::shared_ptr<Variable> deviceDescriptions) = 0;
    virtual void onRPCDeleteDevices(std::vector<uint64_t>& ids, std::shared_ptr<Variable> deviceAddresses, std::shared_ptr<Variable> deviceInfo) = 0;
    virtual void onEvent(std::string source, uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> values) = 0;
    virtual void onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait) = 0;
    virtual BaseLib::PVariable onInvokeRpc(std::string& methodName, BaseLib::PArray& parameters) = 0;
    virtual int32_t onCheckLicense(int32_t moduleId, int32_t familyId, int32_t deviceId, const std::string& licenseKey) = 0;
    virtual uint64_t onGetRoomIdByName(std::string& name) = 0;

    //Device description
    virtual void onDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output) = 0;
};

class IDeviceFamily : public ICentral::ICentralEventSink, public DeviceDescription::Devices::IDevicesEventSink, public IEvents
{
public:
    IDeviceFamily(BaseLib::SharedObjects* bl, IFamilyEventSink* eventHandler, int32_t id, std::string name, FamilyType type);
    virtual ~IDeviceFamily();

    FamilyType type();
    virtual bool enabled();
    virtual bool init() = 0;
    virtual void dispose() {};

    virtual bool lifetick() = 0;
    virtual void lock();
    virtual void unlock();
    virtual bool locked();

    virtual int32_t getFamily();
    virtual FamilySettings::PFamilySetting getFamilySetting(const std::string& name);
    virtual void setFamilySetting(const std::string& name, const std::string& value);
    virtual void setFamilySetting(const std::string& name, int32_t value);
    virtual void setFamilySetting(const std::string& name, const std::vector<char>& value);
    virtual void deleteFamilySettingFromDatabase(const std::string& name);
    virtual void load() = 0;
    virtual void save(bool full) = 0;
    virtual std::string getName();
    virtual std::string handleCliCommand(std::string& command) = 0;

    /*
     * Executed when Homegear is fully started.
     */
    virtual void homegearStarted() = 0;

    /*
     * Executed before Homegear starts shutting down.
     */
    virtual void homegearShuttingDown() = 0;

    // {{{ RPC
    virtual std::shared_ptr<Variable> getPairingInfo() = 0;
    virtual std::shared_ptr<Variable> getParamsetDescription(PRpcClientInfo clientInfo, int32_t deviceId, int32_t firmwareVersion, int32_t channel, ParameterGroup::Type::Enum type) = 0;
    virtual PVariable listKnownDeviceTypes(PRpcClientInfo clientInfo, bool channels, std::set<std::string>& fields) = 0;
    // }}}
protected:
    BaseLib::SharedObjects* _bl = nullptr;
    FamilyType _type = FamilyType::unknown;
    std::shared_ptr<FamilySettings> _settings;
    std::atomic_bool _locked{false};
    bool _disposed = false;

    // {{{ Event handling
    //Hooks
    virtual void raiseAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers);
    virtual void raiseRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers);

    virtual void raiseRPCEvent(std::string& source, uint64_t id, int32_t channel, std::string& deviceAddress, std::shared_ptr<std::vector<std::string>>& valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>>& values);
    virtual void raiseRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
    virtual void raiseRPCNewDevices(std::vector<uint64_t>& ids, std::shared_ptr<Variable> deviceDescriptions);
    virtual void raiseRPCDeleteDevices(std::vector<uint64_t>& ids, std::shared_ptr<Variable> deviceAddresses, std::shared_ptr<Variable> deviceInfo);
    virtual void raiseEvent(std::string& source, uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>>& variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>>& values);
    virtual void raiseRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait);
    virtual BaseLib::PVariable raiseInvokeRpc(std::string& methodName, BaseLib::PArray& parameters);
    virtual int32_t raiseCheckLicense(int32_t moduleId, int32_t familyId, int32_t deviceId, const std::string& licenseKey);
    virtual uint64_t raiseGetRoomIdByName(std::string& name);

    // {{{ Device description event handling
    virtual void raiseDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output);
    // }}}
    // }}}

    // {{{ Device event handling
    //Hooks
    virtual void onAddWebserverEventHandler(BaseLib::Rpc::IWebserverEventSink* eventHandler, std::map<int32_t, PEventHandler>& eventHandlers);
    virtual void onRemoveWebserverEventHandler(std::map<int32_t, PEventHandler>& eventHandlers);

    virtual void onRPCEvent(std::string& source, uint64_t id, int32_t channel, std::string& deviceAddress, std::shared_ptr<std::vector<std::string>>& valueKeys, std::shared_ptr<std::vector<std::shared_ptr<Variable>>>& values);
    virtual void onRPCUpdateDevice(uint64_t id, int32_t channel, std::string address, int32_t hint);
    virtual void onRPCNewDevices(std::vector<uint64_t>& ids, std::shared_ptr<Variable> deviceDescriptions);
    virtual void onRPCDeleteDevices(std::vector<uint64_t>& ids, std::shared_ptr<Variable> deviceAddresses, std::shared_ptr<Variable> deviceInfo);
    virtual void onEvent(std::string& source, uint64_t peerID, int32_t channel, std::shared_ptr<std::vector<std::string>>& variables, std::shared_ptr<std::vector<std::shared_ptr<Variable>>>& values);
    virtual void onRunScript(ScriptEngine::PScriptInfo& scriptInfo, bool wait);
    virtual BaseLib::PVariable onInvokeRpc(std::string& methodName, BaseLib::PArray& parameters);
    virtual uint64_t onGetRoomIdByName(std::string& name);
    // }}}

    // {{{ Device description event handling
    virtual void onDecryptDeviceDescription(int32_t moduleId, const std::vector<char>& input, std::vector<char>& output);
    // }}}

    std::shared_ptr<DeviceDescription::Devices> _rpcDevices;

    virtual std::shared_ptr<ICentral> initializeCentral(uint32_t deviceId, int32_t address, std::string serialNumber) = 0;
    virtual void createCentral() = 0;
private:
    IDeviceFamily(const IDeviceFamily&) = delete;
    IDeviceFamily& operator=(const IDeviceFamily&) = delete;

    IFamilyEventSink* _eventHandler;
    int32_t _family = -1;
    std::string _name;
};

}
}
#endif
