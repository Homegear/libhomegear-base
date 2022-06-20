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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string>
#include <map>
#include <vector>

namespace BaseLib {
class SharedObjects;

class Settings {
 public:
  Settings();
  virtual ~Settings() {}
  void init(BaseLib::SharedObjects *baseLib);
  void load(const std::string &filename, const std::string &executablePath, bool hideOutput = false);
  bool changed();

  std::string runAsUser() { return _runAsUser; }
  std::string runAsGroup() { return _runAsGroup; }
  int32_t debugLevel() { return _debugLevel; }
  bool memoryDebugging() { return _memoryDebugging; }
  std::string waitForIp4OnInterface() { return _waitForIp4OnInterface; }
  std::string waitForIp6OnInterface() { return _waitForIp6OnInterface; }
  bool enableUPnP() { return _enableUPnP; }
  std::string uPnPIpAddress() { return _uPnPIpAddress; }
  std::string ssdpIpAddress() { return _ssdpIpAddress; }
  int32_t ssdpPort() { return _ssdpPort; }
  bool enableMonitoring() { return _enableMonitoring; };
  bool enableHgdc() { return _enableHgdc; };
  int32_t hgdcPort() { return _hgdcPort; }
  bool devLog() { return _devLog; }
  bool ipcLog() { return _ipcLog; }
  bool enableCoreDumps() { return _enableCoreDumps; };
  bool enableNodeBlue() { return _enableNodeBlue; }
  bool setDevicePermissions() { return _setDevicePermissions; }
  std::string workingDirectory() { return _workingDirectory; }
  std::string socketPath() { return _socketPath; }
  std::string dataPath() { return _dataPath; }
  uint32_t dataPathPermissions() { return _dataPathPermissions; }
  std::string dataPathUser() { return _dataPathUser; }
  std::string dataPathGroup() { return _dataPathGroup; }
  std::string writeableDataPath() { return _writeableDataPath; }
  uint32_t writeableDataPathPermissions() { return _writeableDataPathPermissions; }
  std::string writeableDataPathUser() { return _writeableDataPathUser; }
  std::string writeableDataPathGroup() { return _writeableDataPathGroup; }
  std::string familyDataPath() { return _familyDataPath; }
  uint32_t familyDataPathPermissions() { return _familyDataPathPermissions; }
  std::string familyDataPathUser() { return _familyDataPathUser; }
  std::string familyDataPathGroup() { return _familyDataPathGroup; }
  bool databaseSynchronous() { return _databaseSynchronous; }
  bool databaseMemoryJournal() { return _databaseMemoryJournal; }
  bool databaseWALJournal() { return _databaseWALJournal; }
  std::string databasePath() { return _databasePath; }
  std::string factoryDatabasePath() { return _factoryDatabasePath; }
  std::string databaseBackupPath() { return _databaseBackupPath; }
  std::string factoryDatabaseBackupPath() { return _factoryDatabaseBackupPath; }
  uint32_t databaseMaxBackups() { return _databaseMaxBackups; }
  std::string logfilePath() { return _logfilePath; }
  bool waitForCorrectTime() { return _waitForCorrectTime; }
  bool prioritizeThreads() { return _prioritizeThreads; }
  uint32_t maxTotalThreadCount() { return _maxTotalThreadCount; }
  void setPrioritizeThreads(bool value) { _prioritizeThreads = value; }
  uint32_t secureMemorySize() { return _secureMemorySize; }
  uint32_t workerThreadWindow() { return _workerThreadWindow; }
  uint32_t scriptEngineThreadCount() { return _scriptEngineThreadCount; }
  uint32_t scriptEngineServerMaxConnections() { return _scriptEngineServerMaxConnections; }
  uint32_t scriptEngineMaxThreadsPerScript() { return _scriptEngineMaxThreadsPerScript; }
  int32_t scriptEngineMaxScriptsPerProcess() { return _scriptEngineMaxScriptsPerProcess; }
  int32_t scriptEngineWatchdogTimeout() { return _scriptEngineWatchdogTimeout; }
  bool scriptEngineManualClientStart() { return _scriptEngineManualClientStart; }
  uint32_t nodeBlueProcessingThreadCountServer() { return _nodeBlueProcessingThreadCountServer; }
  uint32_t nodeBlueProcessingThreadCountNodes() { return _nodeBlueProcessingThreadCountNodes; }
  uint32_t nodeBlueServerMaxConnections() { return _nodeBlueServerMaxConnections; }
  int32_t maxNodeThreadsPerProcess() { return _maxNodeThreadsPerProcess; }
  int32_t nodeBlueWatchdogTimeout() { return _nodeBlueWatchdogTimeout; }
  bool nodeBlueManualClientStart() { return _nodeBlueManualClientStart; }
  std::string nodeRedJsPath() { return _nodeRedJsPath; }
  uint16_t nodeRedPort() { return _nodeRedPort; }
  std::string nodeOptions() { return _nodeOptions; }
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
  std::string familyConfigPath() { return _familyConfigPath; }
  std::string deviceDescriptionPath() { return _deviceDescriptionPath; }
  std::string clientSettingsPath() { return _clientSettingsPath; }
  std::string serverSettingsPath() { return _serverSettingsPath; }
  std::string mqttSettingsPath() { return _mqttSettingsPath; }
  std::string cloudUserMapPath() { return _cloudUserMapPath; }
  std::string modulePath() { return _modulePath; }
  std::string scriptPath() { return _scriptPath; }
  uint32_t scriptPathPermissions() { return _scriptPathPermissions; }
  std::string scriptPathUser() { return _scriptPathUser; }
  std::string scriptPathGroup() { return _scriptPathGroup; }
  std::string nodeBluePath() { return _nodeBluePath; }
  uint32_t nodeBluePathPermissions() { return _nodeBluePathPermissions; }
  std::string nodeBluePathUser() { return _nodeBluePathUser; }
  std::string nodeBluePathGroup() { return _nodeBluePathGroup; }
  std::string nodeBlueDataPath() { return _nodeBlueDataPath; }
  uint32_t nodeBlueDataPathPermissions() { return _nodeBlueDataPathPermissions; }
  std::string nodeBlueDataPathUser() { return _nodeBlueDataPathUser; }
  std::string nodeBlueDataPathGroup() { return _nodeBlueDataPathGroup; }
  bool nodeBlueDebugOutput() { return _nodeBlueDebugOutput; }
  uint32_t nodeBlueEventLimit1() { return _nodeBlueEventLimit1; }
  uint32_t nodeBlueEventLimit2() { return _nodeBlueEventLimit2; }
  uint32_t nodeBlueEventLimit3() { return _nodeBlueEventLimit3; }
  uint32_t nodeBlueFrontendHistorySize() { return _nodeBlueFrontendHistorySize; }
  std::string nodeBlueUriPathsExcludedFromLogin() { return _nodeBlueUriPathsExcludedFromLogin; }
  std::string adminUiPath() { return _adminUiPath; }
  uint32_t adminUiPathPermissions() { return _adminUiPathPermissions; }
  std::string adminUiPathUser() { return _adminUiPathUser; }
  std::string adminUiPathGroup() { return _adminUiPathGroup; }
  std::string uiPath() { return _uiPath; }
  uint32_t uiPathPermissions() { return _uiPathPermissions; }
  std::string uiPathUser() { return _uiPathUser; }
  std::string uiPathGroup() { return _uiPathGroup; }
  std::string uiTranslationPath() { return _uiTranslationPath; }
  std::string webSshPath() { return _webSshPath; }
  uint32_t webSshPathPermissions() { return _webSshPathPermissions; }
  std::string webSshPathUser() { return _webSshPathUser; }
  std::string webSshPathGroup() { return _webSshPathGroup; }
  bool reloadRolesOnStartup() { return _reloadRolesOnStartup; }
  std::string firmwarePath() { return _firmwarePath; }
  std::string tempPath() { return _tempPath; }
  std::string lockFilePath() { return _lockFilePath; }
  void setLockFilePath(const std::string &value) { _lockFilePath = value; }
  uint32_t lockFilePathPermissions() { return _lockFilePathPermissions; }
  std::string lockFilePathUser() { return _lockFilePathUser; }
  std::string lockFilePathGroup() { return _lockFilePathGroup; }
  std::string phpIniPath() { return _phpIniPath; }
  std::map<std::string, bool> &tunnelClients() { return _tunnelClients; }
  std::string gpioPath() { return _gpioPath; }
  std::vector<uint32_t> exportGpios() { return _exportGpios; }
  std::string oauthCertPath() { return _oauthCertPath; }
  std::string oauthKeyPath() { return _oauthKeyPath; }
  int32_t oauthTokenLifetime() { return _oauthTokenLifetime; }
  int32_t oauthRefreshTokenLifetime() { return _oauthRefreshTokenLifetime; }
  uint32_t maxWaitForPhysicalInterfaces() { return _maxWaitForPhysicalInterfaces; }
 private:
  BaseLib::SharedObjects *_bl = nullptr;
  std::string _executablePath;
  std::string _path;
  int32_t _lastModified = -1;
  int32_t _clientSettingsLastModified = -1;
  int32_t _serverSettingsLastModified = -1;
  int32_t _mqttSettingsLastModified = -1;

  std::string _runAsUser;
  std::string _runAsGroup;
  int32_t _debugLevel = 3;
  bool _memoryDebugging = false;
  std::string _waitForIp4OnInterface;
  std::string _waitForIp6OnInterface;
  bool _enableUPnP = true;
  std::string _uPnPIpAddress;
  std::string _ssdpIpAddress;
  int32_t _ssdpPort = 1900;
  bool _enableMonitoring = true;
  bool _enableHgdc = false;
  int32_t _hgdcPort = 2017;
  bool _devLog = false;
  bool _ipcLog = false;
  bool _enableCoreDumps = true;
  bool _enableNodeBlue = true;
  bool _setDevicePermissions = true;
  std::string _workingDirectory;
  std::string _socketPath;
  std::string _dataPath;
  uint32_t _dataPathPermissions = 504;
  std::string _dataPathUser;
  std::string _dataPathGroup;
  std::string _writeableDataPath;
  uint32_t _writeableDataPathPermissions = 504;
  std::string _writeableDataPathUser;
  std::string _writeableDataPathGroup;
  std::string _familyDataPath;
  uint32_t _familyDataPathPermissions = 504;
  std::string _familyDataPathUser;
  std::string _familyDataPathGroup;
  bool _databaseSynchronous = true;
  bool _databaseMemoryJournal = false;
  bool _databaseWALJournal = true;
  std::string _databasePath;
  std::string _factoryDatabasePath;
  std::string _databaseBackupPath;
  std::string _factoryDatabaseBackupPath;
  uint32_t _databaseMaxBackups = 10;
  std::string _logfilePath;
  bool _waitForCorrectTime = true;
  bool _prioritizeThreads = true;
  uint32_t _maxTotalThreadCount = 0;
  uint32_t _secureMemorySize = 65536;
  uint32_t _workerThreadWindow = 3000;
  uint32_t _scriptEngineThreadCount = 10;
  uint32_t _scriptEngineServerMaxConnections = 20;
  uint32_t _scriptEngineMaxThreadsPerScript = 4;
  int32_t _scriptEngineMaxScriptsPerProcess = -1;
  int32_t _scriptEngineWatchdogTimeout = -1;
  bool _scriptEngineManualClientStart = false;
  uint32_t _nodeBlueProcessingThreadCountServer = 10;
  uint32_t _nodeBlueProcessingThreadCountNodes = 10;
  uint32_t _nodeBlueServerMaxConnections = 20;
  int32_t _maxNodeThreadsPerProcess = 80;
  int32_t _nodeBlueWatchdogTimeout = -1;
  bool _nodeBlueManualClientStart = false;
  std::string _nodeRedJsPath;
  uint16_t _nodeRedPort = 1999;
  std::string _nodeOptions;
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
  std::string _familyConfigPath;
  std::string _deviceDescriptionPath;
  std::string _clientSettingsPath;
  std::string _serverSettingsPath;
  std::string _mqttSettingsPath;
  std::string _cloudUserMapPath;
  std::string _modulePath;
  std::string _scriptPath;
  uint32_t _scriptPathPermissions = 360;
  std::string _scriptPathUser;
  std::string _scriptPathGroup;
  std::string _nodeBluePath;
  uint32_t _nodeBluePathPermissions = 504;
  std::string _nodeBluePathUser;
  std::string _nodeBluePathGroup;
  std::string _nodeBlueDataPath;
  uint32_t _nodeBlueDataPathPermissions = 504;
  std::string _nodeBlueDataPathUser;
  std::string _nodeBlueDataPathGroup;
  bool _nodeBlueDebugOutput = false;
  uint32_t _nodeBlueEventLimit1 = 100;
  uint32_t _nodeBlueEventLimit2 = 300;
  uint32_t _nodeBlueEventLimit3 = 400;
  uint32_t _nodeBlueFrontendHistorySize = 50;
  std::string _nodeBlueUriPathsExcludedFromLogin;
  std::string _adminUiPath;
  uint32_t _adminUiPathPermissions = 504;
  std::string _adminUiPathUser;
  std::string _adminUiPathGroup;
  std::string _uiPath;
  uint32_t _uiPathPermissions = 504;
  std::string _uiPathUser;
  std::string _uiPathGroup;
  std::string _uiTranslationPath;
  std::string _webSshPath;
  uint32_t _webSshPathPermissions = 360;
  std::string _webSshPathUser;
  std::string _webSshPathGroup;
  bool _reloadRolesOnStartup = true;
  std::string _firmwarePath;
  std::string _tempPath;
  std::string _lockFilePath;
  uint32_t _lockFilePathPermissions = 504;
  std::string _lockFilePathUser;
  std::string _lockFilePathGroup;
  std::string _phpIniPath;
  std::map<std::string, bool> _tunnelClients;
  std::string _gpioPath;
  std::vector<uint32_t> _exportGpios;
  uint32_t _maxWaitForPhysicalInterfaces = 180;

  // {{{ OAuth
  std::string _oauthCertPath;
  std::string _oauthKeyPath;
  int32_t _oauthTokenLifetime = 3600;
  int32_t _oauthRefreshTokenLifetime = 5184000;
  // }}}

  void reset();
};

}
#endif /* SETTINGS_H_ */
