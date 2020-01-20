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

#ifndef BASELIB_H_
#define BASELIB_H_

#include "Database/IDatabaseController.h"
#include "Encoding/Ansi.h"
#include "Encoding/XmlrpcDecoder.h"
#include "Encoding/XmlrpcEncoder.h"
#include "Encoding/RpcDecoder.h"
#include "Encoding/RpcEncoder.h"
#include "Encoding/RpcMethod.h"
#include "Encoding/BinaryRpc.h"
#include "Encoding/JsonDecoder.h"
#include "Encoding/JsonEncoder.h"
#include "Encoding/Http.h"
#include "Encoding/Html.h"
#include "Encoding/WebSocket.h"
#include "Encoding/BitReaderWriter.h"
#include "Managers/SerialDeviceManager.h"
#include "Managers/FileDescriptorManager.h"
#include "Managers/ThreadManager.h"
#include "HelperFunctions/HelperFunctions.h"
#include "HelperFunctions/Color.h"
#include "HelperFunctions/Math.h"
#include "HelperFunctions/Base64.h"
#include "HelperFunctions/Net.h"
#include "HelperFunctions/Pid.h"
#include "HelperFunctions/Io.h"
#include "LowLevel/Gpio.h"
#include "LowLevel/Spi.h"
#include "Output/Output.h"
#include "DeviceDescription/Devices.h"
#include "DeviceDescription/DeviceTranslations.h"
#include "ScriptEngine/ScriptInfo.h"
#include "Security/Acls.h"
#include "Security/Hash.h"
#include "Security/Mac.h"
#include "Security/Gcrypt.h"
#include "Settings/Settings.h"
#include "Sockets/Hgdc.h"
#include "Sockets/IWebserverEventSink.h"
#include "Sockets/ServerInfo.h"
#include "Sockets/RpcClientInfo.h"
#include "Systems/DeviceFamily.h"
#include "Systems/GlobalServiceMessages.h"
#include "Systems/Peer.h"
#include "Systems/SystemFactory.h"
#include "Systems/UpdateInfo.h"
#include "Licensing/LicensingFactory.h"
#include "Sockets/Ssdp.h"
#include "IQueue.h"
#include "ITimedQueue.h"
#include "Sockets/HttpClient.h"
#include "Sockets/HttpServer.h"
#include "Sockets/Modbus.h"
#include "Sockets/TcpSocket.h"
#include "Sockets/UdpSocket.h"

namespace BaseLib
{

/**
 * This is the base library main class.
 * It is used to share objects and data between all modules and the main program.
 */
class SharedObjects
{
public:
    SharedObjects(const SharedObjects&) = delete;  //Copy constructor
    SharedObjects& operator=(const SharedObjects&) = delete; //Copy assignment operator

	/**
	 * The current debug level for logging.
	 */
	int32_t debugLevel = 3;

	/*
	 * User ID of the Homegear process.
	 */
	uid_t userId = 0;

	/*
	 * Group ID of the Homegear process.
	 */
	gid_t groupId = 0;

	/**
	 * True when Homegear is still starting. It is set to false, when start up is complete and isOpen() of all interfaces is "true" (plus 30 seconds).
	 */
	std::atomic_bool booting{ true };

	/**
	 * True when Homegear received signal 15.
	 */
	std::atomic_bool shuttingDown{ false };

	/**
	 * The FileDescriptorManager object where all file or socket descriptors should be registered.
	 * This should be done to avoid errors as it can happen, that a closed file descriptor is reopened and suddenly valid again without the object using the old descriptor noticing it.
	 */
	FileDescriptorManager fileDescriptorManager;

	/**
	 * The serial device manager can be used to access one serial device across multiple modules.
	 */
	SerialDeviceManager serialDeviceManager;

	/**
	 * The main.conf settings.
	 */
	Settings settings;

	/**
	 * Provides database access.
	 */
	std::shared_ptr<Database::IDatabaseController> db;

	/**
	 * Port, the non-ssl RPC server listens on.
	 */
	uint32_t rpcPort = 0;

	/**
	 * Return the time of the creation of the object (the Homegear start time).
	 *
	 * @return The unix epoch time in milliseconds.
	 */
	int64_t getStartTime();

	/**
	 * Set the start time.
	 */
	void setStartTime(int64_t time);

	/**
	 * Object to store information about running updates and to only allow one update at a time.
	 */
	Systems::UpdateInfo deviceUpdateInfo;

	/**
	 * Functions to ease your life for a lot of standard operations. As all methods are static, this variable is only
	 * kept for backwards compatibility.
	 */
	HelperFunctions hf;

	/**
	 * Functions for io operations.
	 */
	Io io;

	/**
	 * The main output object to print text to the standard and error output.
	 */
	Output out;

	/**
	 * The thread manager.
	 */
	ThreadManager threadManager;

	/**
	 * Global service messages.
	 */
	Systems::GlobalServiceMessages globalServiceMessages;

	/**
	 * Homegear Daisy Chain
	 */
	std::shared_ptr<Hgdc> hgdc;

	/**
	 * Default signal mask
	 */
	static sigset_t defaultSignalMask;

	/**
	 * Main constructor.
	 *
	 * @param testMaxThreadCount If set to "true", the library tests the maximum number of threads possible. This takes some time.
	 */
	SharedObjects(bool testMaxThreadCount = false);

	/**
	 * Destructor.
	 */
	virtual ~SharedObjects();

	/**
	 * Returns the Homegear version.
	 * @return The Homegear version string.
	 */
	static std::string version();

	/**
	 * Sets a callback function which will be called for all error messages. First parameter is the error level (1 = critical, 2 = error, 3 = warning), second parameter is the error string.
	 */
	void setErrorCallback(std::function<void(int32_t, std::string)>* errorCallback);
private:
	int64_t _startTime = 0;
};
}
#endif
