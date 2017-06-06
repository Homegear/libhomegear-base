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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace BaseLib
{
class SharedObjects;

class Settings
{
public:
	Settings();
	virtual ~Settings() {}
	void init(BaseLib::SharedObjects* baseLib);
	void load(std::string filename, std::string executablePath);
	bool changed();

	std::string runAsUser() { return _runAsUser; }
	std::string runAsGroup() { return _runAsGroup; }
	std::string certPath() { return _certPath; }
	std::string keyPath() { return _keyPath;  }
	bool loadDHParamsFromFile() { return _loadDHParamsFromFile; }
	std::string dhParamPath() { return _dhParamPath;  }
	int32_t debugLevel() { return _debugLevel; }
	bool memoryDebugging() { return _memoryDebugging; }
	bool enableUPnP() { return _enableUPnP; }
	std::string uPnPIpAddress() { return _uPnPIpAddress; }
	std::string ssdpIpAddress() { return _ssdpIpAddress; }
	int32_t ssdpPort() { return _ssdpPort; }
	bool enableMonitoring() { return _enableMonitoring; };
	bool devLog() { return _devLog; }
	bool enableCoreDumps() { return _enableCoreDumps; };
	bool enableFlows() { return _enableFlows; }
	bool setDevicePermissions() { return _setDevicePermissions; }
	std::string workingDirectory() { return _workingDirectory; }
	std::string socketPath() { return _socketPath; }
	std::string dataPath() { return _dataPath; }
	uint32_t dataPathPermissions() { return _dataPathPermissions; }
	std::string dataPathUser() { return _dataPathUser; }
	std::string dataPathGroup() { return _dataPathGroup; }
	std::string familyDataPath() { return _familyDataPath; }
	uint32_t familyDataPathPermissions() { return _familyDataPathPermissions; }
	std::string familyDataPathUser() { return _familyDataPathUser; }
	std::string familyDataPathGroup() { return _familyDataPathGroup; }
	bool databaseSynchronous() { return _databaseSynchronous; }
	bool databaseMemoryJournal() { return _databaseMemoryJournal; }
	bool databaseWALJournal() { return _databaseWALJournal; }
	std::string databasePath() { return _databasePath; }
	std::string databaseBackupPath() { return _databaseBackupPath; }
	uint32_t databaseMaxBackups() { return _databaseMaxBackups; }
	std::string logfilePath() { return _logfilePath; }
	bool prioritizeThreads() { return _prioritizeThreads; }
	void setPrioritizeThreads(bool value) { _prioritizeThreads = value; }
	uint32_t secureMemorySize() { return _secureMemorySize; }
	uint32_t workerThreadWindow() { return _workerThreadWindow; }
	uint32_t scriptEngineThreadCount() { return _scriptEngineThreadCount; }
	uint32_t scriptEngineServerMaxConnections() { return _scriptEngineServerMaxConnections; }
	uint32_t scriptEngineMaxThreadsPerScript() { return _scriptEngineMaxThreadsPerScript; }
	int32_t scriptEngineMaxScriptsPerProcess() { return _scriptEngineMaxScriptsPerProcess; }
	uint32_t flowsProcessingThreadCountServer() { return _flowsProcessingThreadCountServer; }
	uint32_t flowsProcessingThreadCountNodes() { return _flowsProcessingThreadCountNodes; }
	uint32_t flowsServerMaxConnections() { return _flowsServerMaxConnections; }
	int32_t maxNodeThreadsPerProcess() { return _maxNodeThreadsPerProcess; }
	uint32_t ipcThreadCount() { return _ipcThreadCount; }
	uint32_t ipcServerMaxConnections() { return _ipcServerMaxConnections; }
	uint32_t cliServerMaxConnections() { return _cliServerMaxConnections; }
	uint32_t rpcServerMaxConnections() { return _rpcServerMaxConnections; }
	int32_t rpcServerThreadPriority() { return _rpcServerThreadPriority; }
	int32_t rpcServerThreadPolicy() { return _rpcServerThreadPolicy; }
	uint32_t rpcClientMaxServers() { return _rpcClientMaxServers; }
	int32_t rpcClientThreadPriority() { return _rpcClientThreadPriority; }
	int32_t rpcClientThreadPolicy() { return _rpcClientThreadPolicy; }
	int32_t workerThreadPriority() { return _workerThreadPriority; }
	int32_t workerThreadPolicy() { return _workerThreadPolicy; }
	int32_t packetQueueThreadPriority() { return _packetQueueThreadPriority; }
	int32_t packetQueueThreadPolicy() { return _packetQueueThreadPolicy; }
	int32_t packetReceivedThreadPriority() { return _packetReceivedThreadPriority; }
	int32_t packetReceivedThreadPolicy() { return _packetReceivedThreadPolicy; }
	uint32_t eventThreadCount() { return _eventThreadCount; }
	int32_t eventThreadPriority() { return _eventThreadPriority; }
	int32_t eventThreadPolicy() { return _eventThreadPolicy; }
	std::string familyConfigPath() { return _familyConfigPath; }
	std::string deviceDescriptionPath() { return _deviceDescriptionPath; }
	std::string clientSettingsPath() { return _clientSettingsPath; }
	std::string serverSettingsPath() { return _serverSettingsPath; }
	std::string mqttSettingsPath() { return _mqttSettingsPath; }
	std::string modulePath() { return _modulePath; }
	std::string scriptPath() { return _scriptPath; }
	uint32_t scriptPathPermissions() { return _scriptPathPermissions; }
	std::string scriptPathUser() { return _scriptPathUser; }
	std::string scriptPathGroup() { return _scriptPathGroup; }
	std::string flowsPath() { return _flowsPath; }
	uint32_t flowsPathPermissions() { return _flowsPathPermissions; }
	std::string flowsPathUser() { return _flowsPathUser; }
	std::string flowsPathGroup() { return _flowsPathGroup; }
	std::string flowsDataPath() { return _flowsDataPath; }
	uint32_t flowsDataPathPermissions() { return _flowsDataPathPermissions; }
	std::string flowsDataPathUser() { return _flowsDataPathUser; }
	std::string flowsDataPathGroup() { return _flowsDataPathGroup; }
	std::string firmwarePath() { return _firmwarePath; }
	std::string tempPath() { return _tempPath; }
	std::string lockFilePath() { return _lockFilePath; }
	uint32_t lockFilePathPermissions() { return _lockFilePathPermissions; }
	std::string lockFilePathUser() { return _lockFilePathUser; }
	std::string lockFilePathGroup() { return _lockFilePathGroup; }
	std::string phpIniPath() { return _phpIniPath; }
	std::map<std::string, bool>& tunnelClients() { return _tunnelClients; }
	std::map<std::string, std::string>& clientAddressesToReplace() { return _clientAddressesToReplace; }
	std::string gpioPath() { return _gpioPath; }
	std::vector<uint32_t> exportGpios() { return _exportGpios; }
private:
	BaseLib::SharedObjects* _bl = nullptr;
	std::string _executablePath;
	std::string _path;
	int32_t _lastModified = -1;
	int32_t _clientSettingsLastModified = -1;
	int32_t _serverSettingsLastModified = -1;
	int32_t _mqttSettingsLastModified = -1;

	std::string _runAsUser;
	std::string _runAsGroup;
	std::string _certPath;
	std::string _keyPath;
	bool _loadDHParamsFromFile = true;
	std::string _dhParamPath;
	int32_t _debugLevel = 3;
	bool _memoryDebugging = false;
	bool _enableUPnP = true;
	std::string _uPnPIpAddress;
	std::string _ssdpIpAddress;
	int32_t _ssdpPort = 1900;
	bool _enableMonitoring = true;
	bool _devLog = false;
	bool _enableCoreDumps = true;
	bool _enableFlows = true;
	bool _setDevicePermissions = true;
	std::string _workingDirectory;
	std::string _socketPath;
	std::string _dataPath;
	uint32_t _dataPathPermissions = 504;
	std::string _dataPathUser;
	std::string _dataPathGroup;
	std::string _familyDataPath;
	uint32_t _familyDataPathPermissions = 504;
	std::string _familyDataPathUser;
	std::string _familyDataPathGroup;
	bool _databaseSynchronous = true;
	bool _databaseMemoryJournal = false;
	bool _databaseWALJournal = true;
	std::string _databasePath;
	std::string _databaseBackupPath;
	uint32_t _databaseMaxBackups = 10;
	std::string _logfilePath;
	bool _prioritizeThreads = true;
	uint32_t _secureMemorySize = 65536;
	uint32_t _workerThreadWindow = 3000;
	uint32_t _scriptEngineThreadCount = 10;
	uint32_t _scriptEngineServerMaxConnections = 20;
	uint32_t _scriptEngineMaxThreadsPerScript = 4;
	int32_t _scriptEngineMaxScriptsPerProcess = -1;
	uint32_t _flowsProcessingThreadCountServer = 10;
	uint32_t _flowsProcessingThreadCountNodes = 10;
	uint32_t _flowsServerMaxConnections = 20;
	int32_t _maxNodeThreadsPerProcess = 50;
	uint32_t _ipcThreadCount = 10;
	uint32_t _ipcServerMaxConnections = 20;
	uint32_t _cliServerMaxConnections = 50;
	uint32_t _rpcServerMaxConnections = 50;
	int32_t _rpcServerThreadPriority = 0;
	int32_t _rpcServerThreadPolicy = SCHED_OTHER;
	uint32_t _rpcClientMaxServers = 50;
	int32_t _rpcClientThreadPriority = 0;
	int32_t _rpcClientThreadPolicy = SCHED_OTHER;
	int32_t _workerThreadPriority = 0;
	int32_t _workerThreadPolicy = SCHED_OTHER;
	int32_t _packetQueueThreadPriority = 45;
	int32_t _packetQueueThreadPolicy = SCHED_FIFO;
	int32_t _packetReceivedThreadPriority = 0;
	int32_t _packetReceivedThreadPolicy = SCHED_OTHER;
	uint32_t _eventThreadCount = 5;
	int32_t _eventThreadPriority = 0;
	int32_t _eventThreadPolicy = SCHED_OTHER;
	std::string _familyConfigPath;
	std::string _deviceDescriptionPath;
	std::string _clientSettingsPath;
	std::string _serverSettingsPath;
	std::string _mqttSettingsPath;
	std::string _modulePath;
	std::string _scriptPath;
	uint32_t _scriptPathPermissions = 360;
	std::string _scriptPathUser;
	std::string _scriptPathGroup;
	std::string _flowsPath;
	uint32_t _flowsPathPermissions = 504;
	std::string _flowsPathUser;
	std::string _flowsPathGroup;
	std::string _flowsDataPath;
	uint32_t _flowsDataPathPermissions = 504;
	std::string _flowsDataPathUser;
	std::string _flowsDataPathGroup;
	std::string _firmwarePath;
	std::string _tempPath;
	std::string _lockFilePath;
	uint32_t _lockFilePathPermissions = 504;
	std::string _lockFilePathUser;
	std::string _lockFilePathGroup;
	std::string _phpIniPath;
	std::map<std::string, bool> _tunnelClients;
	std::map<std::string, std::string> _clientAddressesToReplace;
	std::string _gpioPath;
	std::vector<uint32_t> _exportGpios;

	void reset();
};

}
#endif /* SETTINGS_H_ */
