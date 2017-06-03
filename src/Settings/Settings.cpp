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

#include "Settings.h"
#include "../BaseLib.h"

namespace BaseLib
{

Settings::Settings()
{
}

void Settings::init(BaseLib::SharedObjects* baseLib)
{
	_bl = baseLib;
}

void Settings::reset()
{
	_runAsUser = "";
	_runAsGroup = "";
	_certPath = "/etc/homegear/homegear.crt";
	_keyPath = "/etc/homegear/homegear.key";
	_dhParamPath = "/etc/homegear/dh2048.pem";
	_debugLevel = 3;
	_memoryDebugging = false;
	_enableUPnP = true;
	_uPnPIpAddress = "";
	_ssdpIpAddress = "";
	_ssdpPort = 1900;
	_enableMonitoring = true;
	_devLog = false;
	_enableCoreDumps = true;
	_enableFlows = true;
	_setDevicePermissions = true;
	_workingDirectory = _executablePath;
	_socketPath = _executablePath;
	_dataPath = _executablePath;
	_dataPathPermissions = 504;
	_dataPathUser = "";
	_dataPathGroup = "";
	_databaseSynchronous = true;
	_databaseMemoryJournal = false;
	_databaseWALJournal = true;
	_databasePath = "";
	_databaseBackupPath = "";
	_databaseMaxBackups = 10;
	_logfilePath = "/var/log/homegear/";
	_prioritizeThreads = true;
	_secureMemorySize = 65536;
	_workerThreadWindow = 3000;
	_scriptEngineThreadCount = 10;
	_scriptEngineServerMaxConnections = 10;
	_scriptEngineMaxThreadsPerScript = 4;
	_scriptEngineMaxScriptsPerProcess = 50;
	_flowsProcessingThreadCountServer = 10;
	_flowsProcessingThreadCountNodes = 10;
	_flowsServerMaxConnections = 20;
	_maxNodeThreadsPerProcess = 50;
	_ipcThreadCount = 10;
	_ipcServerMaxConnections = 20;
	_cliServerMaxConnections = 50;
	_rpcServerMaxConnections = 50;
	_rpcServerThreadPriority = 0;
	_rpcServerThreadPolicy = SCHED_OTHER;
	_rpcClientMaxServers = 50;
	_rpcClientThreadPriority = 0;
	_rpcClientThreadPolicy = SCHED_OTHER;
	_workerThreadPriority = 0;
	_workerThreadPolicy = SCHED_OTHER;
	_packetQueueThreadPriority = 45;
	_packetQueueThreadPolicy = SCHED_FIFO;
	_packetReceivedThreadPriority = 0;
	_packetReceivedThreadPolicy = SCHED_OTHER;
	_eventThreadCount = 5;
	_eventThreadPriority = 0;
	_eventThreadPolicy = SCHED_OTHER;
	_familyConfigPath = "/etc/homegear/families/";
	_deviceDescriptionPath = "/etc/homegear/devices/";
	_clientSettingsPath = "/etc/homegear/rpcclients.conf";
	_serverSettingsPath = "/etc/homegear/rpcservers.conf";
	_mqttSettingsPath = "/etc/homegear/mqtt.conf";
	_modulePath = "/var/lib/homegear/modules/";
	_scriptPath = "/var/lib/homegear/scripts/";
	_scriptPathPermissions = 360;
	_scriptPathUser = "";
	_scriptPathGroup = "";
	_flowsPath = "/var/lib/homegear/flows/";
	_flowsPathPermissions = 504;
	_flowsPathUser = "";
	_flowsPathGroup = "";
	_flowsDataPath = "/var/lib/homegear/flows/data/";
	_flowsDataPathPermissions = 504;
	_flowsDataPathUser = "";
	_flowsDataPathGroup = "";
	_firmwarePath = "/usr/share/homegear/firmware/";
	_tempPath = "/var/lib/homegear/tmp/";
	_lockFilePath = "/var/lock/";
	_lockFilePathPermissions = 0;
	_lockFilePathUser = "";
	_lockFilePathGroup = "";
	_phpIniPath = "/etc/homegear/php.ini";
	_tunnelClients.clear();
	_clientAddressesToReplace.clear();
	_gpioPath = "/sys/class/gpio/";
	_exportGpios.clear();
}

bool Settings::changed()
{
	if(_bl->io.getFileLastModifiedTime(_path) != _lastModified ||
		_bl->io.getFileLastModifiedTime(_clientSettingsPath) != _clientSettingsLastModified ||
		_bl->io.getFileLastModifiedTime(_serverSettingsPath) != _serverSettingsLastModified)
	{
		return true;
	}
	return false;
}

void Settings::load(std::string filename, std::string executablePath)
{
	try
	{
		_executablePath = executablePath;
		reset();
		_path = filename;
		char input[1024];
		FILE *fin;
		int32_t len, ptr;
		bool found = false;

		if (!(fin = fopen(filename.c_str(), "r")))
		{
			_bl->out.printError("Unable to open config file: " + filename + ". " + strerror(errno));
			return;
		}

		while (fgets(input, 1024, fin))
		{
			if(input[0] == '#') continue;
			len = strlen(input);
			if (len < 2) continue;
			if (input[len-1] == '\n') input[len-1] = '\0';
			ptr = 0;
			found = false;
			while(ptr < len)
			{
				if (input[ptr] == '=')
				{
					found = true;
					input[ptr++] = '\0';
					break;
				}
				ptr++;
			}
			if(found)
			{
				std::string name(input);
				HelperFunctions::toLower(name);
				HelperFunctions::trim(name);
				std::string value(&input[ptr]);
				HelperFunctions::trim(value);
				if(name == "runasuser")
				{
					_runAsUser = value;
					_bl->out.printDebug("Debug: runAsUser set to " + _runAsUser);
				}
				else if(name == "runasgroup")
				{
					_runAsGroup = value;
					_bl->out.printDebug("Debug: runAsGroup set to " + _runAsGroup);
				}
				else if(name == "certpath")
				{
					_certPath = value;
					if(_certPath.empty()) _certPath = "/etc/homegear/homegear.crt";
					_bl->out.printDebug("Debug: certPath set to " + _certPath);
				}
				else if(name == "keypath")
				{
					_keyPath = value;
					if(_keyPath.empty()) _keyPath = "/etc/homegear/homegear.key";
					_bl->out.printDebug("Debug: keyPath set to " + _keyPath);
				}
				else if(name == "loaddhparamsfromfile")
				{
					if(HelperFunctions::toLower(value) == "true") _loadDHParamsFromFile = true;
					else if(HelperFunctions::toLower(value) == "false") _loadDHParamsFromFile = false;
					_bl->out.printDebug("Debug: loadDHParamsFromFile set to " + std::to_string(_loadDHParamsFromFile));
				}
				else if(name == "dhparampath")
				{
					_dhParamPath = value;
					if(_dhParamPath.empty()) _dhParamPath = "/etc/homegear/dh2048.pem";
					_bl->out.printDebug("Debug: dhParamPath set to " + _dhParamPath);
				}
				else if(name == "debuglevel")
				{
					_debugLevel = Math::getNumber(value);
					if(_debugLevel < 0) _debugLevel = 3;
					_bl->debugLevel = _debugLevel;
					_bl->out.printDebug("Debug: debugLevel set to " + std::to_string(_debugLevel));
				}
				else if(name == "memorydebugging")
				{
					if(HelperFunctions::toLower(value) == "true") _memoryDebugging = true;
					_bl->out.printDebug("Debug: memoryDebugging set to " + std::to_string(_memoryDebugging));
				}
				else if(name == "enableupnp")
				{
					if(HelperFunctions::toLower(value) == "false") _enableUPnP = false;
					_bl->out.printDebug("Debug: enableUPnP set to " + std::to_string(_enableUPnP));
				}
				else if(name == "upnpipaddress")
				{
					_uPnPIpAddress = value;
					_bl->out.printDebug("Debug: uPnPIpAddress set to " + _uPnPIpAddress);
				}
				else if(name == "ssdpipaddress")
				{
					_ssdpIpAddress = value;
					_bl->out.printDebug("Debug: ssdpIpAddress set to " + _ssdpIpAddress);
				}
				else if(name == "ssdpport")
				{
					_ssdpPort = Math::getNumber(value);
					if(_ssdpPort < 1 || _ssdpPort > 65535) _ssdpPort = 1900;
					_bl->out.printDebug("Debug: ssdpPort set to " + std::to_string(_ssdpPort));
				}
				else if(name == "enablemonitoring")
				{
					if(HelperFunctions::toLower(value) == "false") _enableMonitoring = false;
					_bl->out.printDebug("Debug: enableMonitoring set to " + std::to_string(_enableMonitoring));
				}
				else if(name == "devlog")
				{
					if(HelperFunctions::toLower(value) == "true") _devLog = true;
					_bl->out.printDebug("Debug: devLog set to " + std::to_string(_devLog));
				}
				else if(name == "enablecoredumps")
				{
					if(HelperFunctions::toLower(value) == "false") _enableCoreDumps = false;
					_bl->out.printDebug("Debug: enableCoreDumps set to " + std::to_string(_enableCoreDumps));
				}
				else if(name == "enableflows")
				{
					_enableFlows = HelperFunctions::toLower(value) == "true";
					_bl->out.printDebug("Debug: enableFlows set to " + std::to_string(_enableFlows));
				}
				else if(name == "setdevicepermissions")
				{
					_setDevicePermissions = HelperFunctions::toLower(value) == "true";
					_bl->out.printDebug("Debug: setDevicePermissions set to " + std::to_string(_setDevicePermissions));
				}
				else if(name == "workingdirectory")
				{
					_workingDirectory = value;
					if(_workingDirectory.empty()) _workingDirectory = _executablePath;
					if(_workingDirectory.back() != '/') _workingDirectory.push_back('/');
					_bl->out.printDebug("Debug: workingDirectory set to " + _workingDirectory);
				}
				else if(name == "socketpath")
				{
					_socketPath = value;
					if(_socketPath.empty()) _socketPath = _executablePath;
					if(_socketPath.back() != '/') _socketPath.push_back('/');
					_bl->out.printDebug("Debug: socketPath set to " + _socketPath);
				}
				else if(name == "datapath")
				{
					_dataPath = value;
					if(_dataPath.empty()) _dataPath = _executablePath;
					if(_dataPath.back() != '/') _dataPath.push_back('/');
					_bl->out.printDebug("Debug: dataPath set to " + _dataPath);
				}
				else if(name == "databasepath" && _dataPath.empty() && !value.empty())
				{
					_dataPath = value.substr(0, value.find_last_of("/") + 1);;
					if(_dataPath.empty()) _dataPath = _executablePath;
					if(_dataPath.back() != '/') _dataPath.push_back('/');
					_bl->out.printDebug("Debug: dataPath set to " + _dataPath);
				}
				else if(name == "datapathpermissions")
				{
					_dataPathPermissions = Math::getOctalNumber(value);
					if(_dataPathPermissions == 0) _dataPathPermissions = 504;
					_bl->out.printDebug("Debug: dataPathPermissions set to " + _dataPathPermissions);
				}
				else if(name == "datapathuser")
				{
					_dataPathUser = value;
					_bl->out.printDebug("Debug: dataPathUser set to " + _dataPathUser);
				}
				else if(name == "datapathgroup")
				{
					_dataPathGroup = value;
					_bl->out.printDebug("Debug: dataPathGroup set to " + _dataPathGroup);
				}
				else if(name == "databasesynchronous")
				{
					if(HelperFunctions::toLower(value) == "false") _databaseSynchronous = false;
					_bl->out.printDebug("Debug: databaseSynchronous set to " + std::to_string(_databaseSynchronous));
				}
				else if(name == "databasememoryjournal")
				{
					if(HelperFunctions::toLower(value) == "true") _databaseMemoryJournal = true;
					_bl->out.printDebug("Debug: databaseMemoryJournal set to " + std::to_string(_databaseMemoryJournal));
				}
				else if(name == "databasewaljournal")
				{
					if(HelperFunctions::toLower(value) == "false") _databaseWALJournal = false;
					_bl->out.printDebug("Debug: databaseWALJournal set to " + std::to_string(_databaseWALJournal));
				}
				else if(name == "databasepath")
				{
					_databasePath = value;
					if(!_databasePath.empty() && _databasePath.back() != '/') _databasePath.push_back('/');
					_bl->out.printDebug("Debug: databasePath set to " + _databasePath);
				}
				else if(name == "databasebackuppath")
				{
					_databaseBackupPath = value;
					if(!_databaseBackupPath.empty() && _databaseBackupPath.back() != '/') _databaseBackupPath.push_back('/');
					_bl->out.printDebug("Debug: databaseBackupPath set to " + _databaseBackupPath);
				}
				else if(name == "databasemaxbackups")
				{
					_databaseMaxBackups = Math::getNumber(value);
					if(_databaseMaxBackups > 10000) _databaseMaxBackups = 10000;
					_bl->out.printDebug("Debug: databaseMaxBackups set to " + std::to_string(_databaseMaxBackups));
				}
				else if(name == "logfilepath")
				{
					_logfilePath = value;
					if(_logfilePath.empty()) _logfilePath = "/var/log/homegear/";
					if(_logfilePath.back() != '/') _logfilePath.push_back('/');
					_bl->out.printDebug("Debug: logfilePath set to " + _logfilePath);
				}
				else if(name == "prioritizethreads")
				{
					if(HelperFunctions::toLower(value) == "false") _prioritizeThreads = false;
					_bl->out.printDebug("Debug: prioritizeThreads set to " + std::to_string(_prioritizeThreads));
				}
				else if(name == "securememorysize")
				{
					_secureMemorySize = Math::getNumber(value);
					//Allow 0 => disable secure memory. 16384 is minimum size. Values smaller than 16384 are set to 16384 by gcrypt: https://gnupg.org/documentation/manuals/gcrypt-devel/Controlling-the-library.html
					if(_secureMemorySize < 0) _secureMemorySize = 1;
					_bl->out.printDebug("Debug: secureMemorySize set to " + std::to_string(_secureMemorySize));
				}
				else if(name == "workerthreadwindow")
				{
					_workerThreadWindow = Math::getNumber(value);
					if(_workerThreadWindow > 3600000) _workerThreadWindow = 3600000;
					_bl->out.printDebug("Debug: workerThreadWindow set to " + std::to_string(_workerThreadWindow));
				}
				else if(name == "scriptenginethreadcount")
				{
					_scriptEngineThreadCount = Math::getNumber(value);
					_bl->out.printDebug("Debug: scriptEngineThreadCount set to " + std::to_string(_scriptEngineThreadCount));
				}
				else if(name == "scriptengineservermaxconnections")
				{
					_scriptEngineServerMaxConnections = Math::getNumber(value);
					_bl->out.printDebug("Debug: scriptEngineServerMaxConnections set to " + std::to_string(_scriptEngineServerMaxConnections));
				}
				else if(name == "scriptenginemaxthreadsperscript")
				{
					_scriptEngineMaxThreadsPerScript = Math::getNumber(value);
					_bl->out.printDebug("Debug: scriptEngineMaxThreadsPerScript set to " + std::to_string(_scriptEngineMaxThreadsPerScript));
				}
				else if(name == "scriptenginemaxscriptsperprocess")
				{
					_scriptEngineMaxScriptsPerProcess = Math::getNumber(value);
					_bl->out.printDebug("Debug: scriptEngineMaxScriptsPerProcess set to " + std::to_string(_scriptEngineMaxScriptsPerProcess));
				}
				else if(name == "flowsprocessingthreadcountserver")
				{
					_flowsProcessingThreadCountServer = Math::getNumber(value);
					_bl->out.printDebug("Debug: flowsProcessingThreadCountServer set to " + std::to_string(_flowsProcessingThreadCountServer));
				}
				else if(name == "flowsprocessingthreadcountnodes")
				{
					_flowsProcessingThreadCountServer = Math::getNumber(value);
					_bl->out.printDebug("Debug: flowsProcessingThreadCountNodes set to " + std::to_string(_flowsProcessingThreadCountNodes));
				}
				else if(name == "flowsservermaxconnections")
				{
					_flowsServerMaxConnections = Math::getNumber(value);
					_bl->out.printDebug("Debug: flowsServerMaxConnections set to " + std::to_string(_flowsServerMaxConnections));
				}
				else if(name == "maxnodethreadsperprocess")
				{
					_maxNodeThreadsPerProcess = Math::getNumber(value);
					_bl->out.printDebug("Debug: maxNodeThreadsPerProcess set to " + std::to_string(_maxNodeThreadsPerProcess));
				}
				else if(name == "ipcthreadcount")
				{
					_ipcThreadCount = Math::getNumber(value);
					_bl->out.printDebug("Debug: ipcThreadCount set to " + std::to_string(_ipcThreadCount));
				}
				else if(name == "ipcservermaxconnections")
				{
					_ipcServerMaxConnections = Math::getNumber(value);
					_bl->out.printDebug("Debug: ipsServerMaxConnections set to " + std::to_string(_ipcServerMaxConnections));
				}
				else if(name == "cliservermaxconnections")
				{
					_cliServerMaxConnections = Math::getNumber(value);
					_bl->out.printDebug("Debug: cliServerMaxConnections set to " + std::to_string(_cliServerMaxConnections));
				}
				else if(name == "rpcservermaxconnections")
				{
					_rpcServerMaxConnections = Math::getNumber(value);
					_bl->out.printDebug("Debug: rpcServerMaxConnections set to " + std::to_string(_rpcServerMaxConnections));
				}
				else if(name == "rpcserverthreadpriority")
				{
					_rpcServerThreadPriority = Math::getNumber(value);
					if(_rpcServerThreadPriority > 99) _rpcServerThreadPriority = 99;
					if(_rpcServerThreadPriority < 0) _rpcServerThreadPriority = 0;
					_bl->out.printDebug("Debug: rpcServerThreadPriority set to " + std::to_string(_rpcServerThreadPriority));
				}
				else if(name == "rpcserverthreadpolicy")
				{
					_rpcServerThreadPolicy = ThreadManager::getThreadPolicyFromString(value);
					_rpcServerThreadPriority = ThreadManager::parseThreadPriority(_rpcServerThreadPriority, _rpcServerThreadPolicy);
					_bl->out.printDebug("Debug: rpcServerThreadPolicy set to " + std::to_string(_rpcServerThreadPolicy));
				}
				else if(name == "rpcclientmaxservers")
				{
					_rpcClientMaxServers = Math::getNumber(value);
					_bl->out.printDebug("Debug: rpcClientMaxThreads set to " + std::to_string(_rpcClientMaxServers));
				}
				else if(name == "rpcclientthreadpriority")
				{
					_rpcClientThreadPriority = Math::getNumber(value);
					if(_rpcClientThreadPriority > 99) _rpcClientThreadPriority = 99;
					if(_rpcClientThreadPriority < 0) _rpcClientThreadPriority = 0;
					_bl->out.printDebug("Debug: rpcClientThreadPriority set to " + std::to_string(_rpcClientThreadPriority));
				}
				else if(name == "rpcclientthreadpolicy")
				{
					_rpcClientThreadPolicy = ThreadManager::getThreadPolicyFromString(value);
					_rpcClientThreadPriority = ThreadManager::parseThreadPriority(_rpcClientThreadPriority, _rpcClientThreadPolicy);
					_bl->out.printDebug("Debug: rpcClientThreadPolicy set to " + std::to_string(_rpcClientThreadPolicy));
				}
				else if(name == "workerthreadpriority")
				{
					_workerThreadPriority = Math::getNumber(value);
					if(_workerThreadPriority > 99) _workerThreadPriority = 99;
					if(_workerThreadPriority < 0) _workerThreadPriority = 0;
					_bl->out.printDebug("Debug: workerThreadPriority set to " + std::to_string(_workerThreadPriority));
				}
				else if(name == "workerthreadpolicy")
				{
					_workerThreadPolicy = ThreadManager::getThreadPolicyFromString(value);
					_workerThreadPriority = ThreadManager::parseThreadPriority(_workerThreadPriority, _workerThreadPolicy);
					_bl->out.printDebug("Debug: workerThreadPolicy set to " + std::to_string(_workerThreadPolicy));
				}
				else if(name == "packetqueuethreadpriority")
				{
					_packetQueueThreadPriority = Math::getNumber(value);
					if(_packetQueueThreadPriority > 99) _packetQueueThreadPriority = 99;
					if(_packetQueueThreadPriority < 0) _packetQueueThreadPriority = 0;
					_bl->out.printDebug("Debug: physicalInterfaceThreadPriority set to " + std::to_string(_packetQueueThreadPriority));
				}
				else if(name == "packetqueuethreadpolicy")
				{
					_packetQueueThreadPolicy = ThreadManager::getThreadPolicyFromString(value);
					_packetQueueThreadPriority = ThreadManager::parseThreadPriority(_packetQueueThreadPriority, _packetQueueThreadPolicy);
					_bl->out.printDebug("Debug: physicalInterfaceThreadPolicy set to " + std::to_string(_packetQueueThreadPolicy));
				}
				else if(name == "packetreceivedthreadpriority")
				{
					_packetReceivedThreadPriority = Math::getNumber(value);
					if(_packetReceivedThreadPriority > 99) _packetReceivedThreadPriority = 99;
					if(_packetReceivedThreadPriority < 0) _packetReceivedThreadPriority = 0;
					_bl->out.printDebug("Debug: packetReceivedThreadPriority set to " + std::to_string(_packetReceivedThreadPriority));
				}
				else if(name == "packetreceivedthreadpolicy")
				{
					_packetReceivedThreadPolicy = ThreadManager::getThreadPolicyFromString(value);
					_packetReceivedThreadPriority = ThreadManager::parseThreadPriority(_packetReceivedThreadPriority, _packetReceivedThreadPolicy);
					_bl->out.printDebug("Debug: packetReceivedThreadPolicy set to " + std::to_string(_packetReceivedThreadPolicy));
				}
				else if(name == "eventthreadcount")
				{
					_eventThreadCount = Math::getNumber(value);
					_bl->out.printDebug("Debug: eventThreadCount set to " + std::to_string(_eventThreadCount));
				}
				else if(name == "eventthreadpriority")
				{
					_eventThreadPriority = Math::getNumber(value);
					if(_eventThreadPriority > 99) _eventThreadPriority = 99;
					if(_eventThreadPriority < 0) _eventThreadPriority = 0;
					_bl->out.printDebug("Debug: eventThreadPriority set to " + std::to_string(_eventThreadPriority));
				}
				else if(name == "eventthreadpolicy")
				{
					_eventThreadPolicy = ThreadManager::getThreadPolicyFromString(value);
					_eventThreadPriority = ThreadManager::parseThreadPriority(_eventThreadPriority, _eventThreadPolicy);
					_bl->out.printDebug("Debug: eventThreadPolicy set to " + std::to_string(_eventThreadPolicy));
				}
				else if(name == "familyconfigpath")
				{
					_familyConfigPath = value;
					if(_familyConfigPath.empty()) _familyConfigPath = "/etc/homegear/families";
					if(_familyConfigPath.back() != '/') _familyConfigPath.push_back('/');
					_bl->out.printDebug("Debug: familyConfigPath set to " + _familyConfigPath);
				}
				else if(name == "devicedescriptionpath")
				{
					_deviceDescriptionPath = value;
					if(_deviceDescriptionPath.empty()) _deviceDescriptionPath = "/etc/homegear/devices";
					if(_deviceDescriptionPath.back() != '/') _deviceDescriptionPath.push_back('/');
					_bl->out.printDebug("Debug: deviceDescriptionPath set to " + _deviceDescriptionPath);
				}
				else if(name == "serversettingspath")
				{
					_serverSettingsPath = value;
					if(_serverSettingsPath.empty()) _serverSettingsPath = "/etc/homegear/rpcservers.conf";
					_bl->out.printDebug("Debug: serverSettingsPath set to " + _serverSettingsPath);
				}
				else if(name == "clientsettingspath")
				{
					_clientSettingsPath = value;
					if(_clientSettingsPath.empty()) _clientSettingsPath = "/etc/homegear/rpcclients.conf";
					_bl->out.printDebug("Debug: clientSettingsPath set to " + _clientSettingsPath);
				}
				else if(name == "mqttsettingspath")
				{
					_mqttSettingsPath = value;
					if(_mqttSettingsPath.empty()) _mqttSettingsPath = "/etc/homegear/mqtt.conf";
					_bl->out.printDebug("Debug: mqttSettingsPath set to " + _mqttSettingsPath);
				}
				else if(name == "modulepath")
				{
					_modulePath = value;
					if(_modulePath.empty()) _modulePath = "/var/lib/homegear/modules/";
					if(_modulePath.back() != '/') _modulePath.push_back('/');
					_bl->out.printDebug("Debug: libraryPath set to " + _modulePath);
				}
				else if(name == "scriptpath")
				{
					_scriptPath = value;
					if(_scriptPath.empty()) _scriptPath = "/var/lib/homegear/scripts/";
					if(_scriptPath.back() != '/') _scriptPath.push_back('/');
					_bl->out.printDebug("Debug: scriptPath set to " + _scriptPath);
				}
				else if(name == "scriptpathpermissions")
				{
					_scriptPathPermissions = Math::getOctalNumber(value);
					if(_scriptPathPermissions == 0) _scriptPathPermissions = 360;
					_bl->out.printDebug("Debug: scriptPathPermissions set to " + _scriptPathPermissions);
				}
				else if(name == "scriptpathuser")
				{
					_scriptPathUser = value;
					_bl->out.printDebug("Debug: scriptPathUser set to " + _scriptPathUser);
				}
				else if(name == "scriptpathgroup")
				{
					_scriptPathGroup = value;
					_bl->out.printDebug("Debug: scriptPathGroup set to " + _scriptPathGroup);
				}
				else if(name == "flowspath")
				{
					_flowsPath = value;
					if(_flowsPath.empty()) _flowsPath = "/var/lib/homegear/flows/";
					if(_flowsPath.back() != '/') _flowsPath.push_back('/');
					_bl->out.printDebug("Debug: flowsPath set to " + _flowsPath);
				}
				else if(name == "flowspathpermissions")
				{
					_flowsPathPermissions = Math::getOctalNumber(value);
					if(_flowsPathPermissions == 0) _flowsPathPermissions = 504;
					_bl->out.printDebug("Debug: flowsPathPermissions set to " + _flowsPathPermissions);
				}
				else if(name == "flowspathuser")
				{
					_flowsPathUser = value;
					_bl->out.printDebug("Debug: flowsPathUser set to " + _flowsPathUser);
				}
				else if(name == "flowspathgroup")
				{
					_flowsPathGroup = value;
					_bl->out.printDebug("Debug: flowsPathGroup set to " + _flowsPathGroup);
				}
				else if(name == "flowsdatapath")
				{
					_flowsDataPath = value;
					if(_flowsDataPath.empty()) _flowsDataPath = "/var/lib/homegear/flows/data/";
					if(_flowsDataPath.back() != '/') _flowsDataPath.push_back('/');
					_bl->out.printDebug("Debug: flowsDataPath set to " + _flowsDataPath);
				}
				else if(name == "flowsdatapathpermissions")
				{
					_flowsDataPathPermissions = Math::getOctalNumber(value);
					if(_flowsDataPathPermissions == 0) _flowsDataPathPermissions = 504;
					_bl->out.printDebug("Debug: flowsDataPathPermissions set to " + _flowsDataPathPermissions);
				}
				else if(name == "flowsdatapathuser")
				{
					_flowsDataPathUser = value;
					_bl->out.printDebug("Debug: flowsDataPathUser set to " + _flowsDataPathUser);
				}
				else if(name == "flowsdatapathgroup")
				{
					_flowsDataPathGroup = value;
					_bl->out.printDebug("Debug: flowsDataPathGroup set to " + _flowsDataPathGroup);
				}
				else if(name == "firmwarepath")
				{
					_firmwarePath = value;
					if(_firmwarePath.empty()) _firmwarePath = "/usr/share/homegear/firmware/";
					if(_firmwarePath.back() != '/') _firmwarePath.push_back('/');
					_bl->out.printDebug("Debug: firmwarePath set to " + _firmwarePath);
				}
				else if(name == "temppath")
				{
					_tempPath = value;
					if(_tempPath.empty()) _tempPath = "/var/lib/homegear/tmp/";
					if(_tempPath.back() != '/') _tempPath.push_back('/');
					_bl->out.printDebug("Debug: tempPath set to " + _tempPath);
				}
				else if(name == "lockfilepath")
				{
					_lockFilePath = value;
					if(_lockFilePath.empty()) _lockFilePath = _executablePath;
					if(_lockFilePath.back() != '/') _lockFilePath.push_back('/');
					_bl->out.printDebug("Debug: lockFilePath set to " + _lockFilePath);
				}
				else if(name == "lockfilepathpermissions")
				{
					_lockFilePathPermissions = Math::getOctalNumber(value);
					_bl->out.printDebug("Debug: lockFilePathPermissions set to " + std::to_string(_lockFilePathPermissions));
				}
				else if(name == "lockfilepathuser")
				{
					_lockFilePathUser = value;
					_bl->out.printDebug("Debug: lockFilePathUser set to " + _lockFilePathUser);
				}
				else if(name == "lockfilepathgroup")
				{
					_lockFilePathGroup = value;
					_bl->out.printDebug("Debug: lockFilePathGroup set to " + _lockFilePathGroup);
				}
				else if(name == "phpinipath")
				{
					_phpIniPath = value;
					if(_phpIniPath.empty()) _phpIniPath = "/etc/homegear/php.ini";
					_bl->out.printDebug("Debug: phpIniPath set to " + _phpIniPath);
				}
				else if(name == "redirecttosshtunnel")
				{
					if(!value.empty()) _tunnelClients[HelperFunctions::toLower(value)] = true;
				}
				else if(name == "replaceclientserveraddress")
				{
					if(!value.empty())
					{
						std::pair<std::string, std::string> addresses = _bl->hf.splitLast(HelperFunctions::toLower(value), ' ');
						if(!value.empty()) _clientAddressesToReplace[addresses.first] = addresses.second;
						_bl->out.printDebug("Debug: Added replaceClientServerAddress " + addresses.first + " " + addresses.second);
					}
				}
				else if(name == "gpiopath")
				{
					_gpioPath = value;
					if(_gpioPath.empty()) _gpioPath = "/sys/class/gpio/";
					if(_gpioPath.back() != '/') _gpioPath.push_back('/');
					_bl->out.printDebug("Debug: gpioPath set to " + _gpioPath);
				}
				else if(name == "exportgpios")
				{
					std::vector<std::string> stringValues = BaseLib::HelperFunctions::splitAll(value, ',');
					for(std::vector<std::string>::iterator i = stringValues.begin(); i != stringValues.end(); ++i)
					{
						uint32_t gpio = BaseLib::Math::getNumber(*i);
						_exportGpios.push_back(gpio);
						_bl->out.printDebug("Debug: Added " + std::to_string(gpio) + " to exportGpios.");
					}
				}
				else
				{
					_bl->out.printWarning("Warning: Setting not found: " + std::string(input));
				}
			}
		}

		fclose(fin);
		_lastModified = _bl->io.getFileLastModifiedTime(filename);
		_clientSettingsLastModified = _bl->io.getFileLastModifiedTime(_clientSettingsPath);
		_serverSettingsLastModified = _bl->io.getFileLastModifiedTime(_serverSettingsPath);
		_mqttSettingsLastModified = _bl->io.getFileLastModifiedTime(_mqttSettingsPath);
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
}

}
