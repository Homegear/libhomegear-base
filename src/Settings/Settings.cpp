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
	_debugLevel = 3;
	_memoryDebugging = false;
	_enableUPnP = true;
	_uPnPIpAddress = "";
	_ssdpIpAddress = "";
	_ssdpPort = 1900;
	_enableMonitoring = true;
	_devLog = false;
    _ipcLog = false;
	_enableCoreDumps = true;
	_enableNodeBlue = true;
	_setDevicePermissions = true;
	_workingDirectory = _executablePath;
	_socketPath = _executablePath;
	_dataPath = _executablePath;
	_dataPathPermissions = 504;
	_dataPathUser = "";
	_dataPathGroup = "";
	_writeableDataPath = _dataPath;
	_writeableDataPathPermissions = 504;
	_writeableDataPathUser = "";
	_writeableDataPathGroup = "";
	_familyDataPath = _executablePath + "families/";
	_familyDataPathPermissions = 504;
	_familyDataPathUser = "";
	_familyDataPathGroup = "";
	_databaseSynchronous = true;
	_databaseMemoryJournal = false;
	_databaseWALJournal = true;
	_databasePath = "";
	_databaseBackupPath = "";
	_databaseMaxBackups = 10;
	_logfilePath = "/var/log/homegear/";
	_waitForCorrectTime = true;
	_prioritizeThreads = true;
	_secureMemorySize = 65536;
	_workerThreadWindow = 3000;
	_scriptEngineThreadCount = 10;
	_scriptEngineServerMaxConnections = 10;
	_scriptEngineMaxThreadsPerScript = 4;
	_scriptEngineMaxScriptsPerProcess = 50;
    _scriptEngineWatchdogTimeout = -1;
	_nodeBlueProcessingThreadCountServer = 10;
	_nodeBlueProcessingThreadCountNodes = 10;
	_nodeBlueServerMaxConnections = 20;
	_maxNodeThreadsPerProcess = 80;
	_nodeBlueWatchdogTimeout = -1;
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
	_nodeBluePath = "/var/lib/homegear/node-blue/";
	_nodeBluePathPermissions = 504;
	_nodeBluePathUser = "";
	_nodeBluePathGroup = "";
	_nodeBlueDataPath = "/var/lib/homegear/node-blue/data/";
	_nodeBlueDataPathPermissions = 504;
	_nodeBlueDataPathUser = "";
	_nodeBlueDataPathGroup = "";
	_nodeBlueDebugOutput = false;
	_nodeBlueEventLimit1 = 100;
    _nodeBlueEventLimit2 = 300;
    _nodeBlueEventLimit3 = 400;
	_adminUiPath = "/var/lib/homegear/admin-ui/";
	_adminUiPathPermissions = 504;
	_adminUiPathUser = "";
	_adminUiPathGroup = "";
	_uiPath = "/var/lib/homegear/ui/";
	_uiPathPermissions = 504;
	_uiPathUser = "";
	_uiPathGroup = "";
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
	_oauthCertPath = "";
	_oauthKeyPath = "";
	_oauthTokenLifetime = 3600;
	_oauthRefreshTokenLifetime = 5184000;
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
		if(_executablePath.back() != '/') _executablePath.push_back('/');
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
                else if(name == "waitforip4oninterface")
                {
                    _waitForIp4OnInterface = value;
                    _bl->out.printDebug("Debug: waitForIp4OnInterface set to " + _waitForIp4OnInterface);
                }
                else if(name == "waitforip6oninterface")
                {
                    _waitForIp6OnInterface = value;
                    _bl->out.printDebug("Debug: waitForIp6OnInterface set to " + _waitForIp6OnInterface);
                }
				else if(name == "enableupnp")
				{
					_enableUPnP = BaseLib::HelperFunctions::toLower(value) == "true";
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
					_devLog = HelperFunctions::toLower(value) == "true";
					_bl->out.printDebug("Debug: devLog set to " + std::to_string(_devLog));
				}
                else if(name == "ipclog")
                {
                    _ipcLog = HelperFunctions::toLower(value) == "true";
                    _bl->out.printDebug("Debug: ipcLog set to " + std::to_string(_ipcLog));
                }
				else if(name == "enablecoredumps")
				{
					if(HelperFunctions::toLower(value) == "false") _enableCoreDumps = false;
					_bl->out.printDebug("Debug: enableCoreDumps set to " + std::to_string(_enableCoreDumps));
				}
				else if(name == "enableflows" || name == "enablenodeblue")
				{
					_enableNodeBlue = HelperFunctions::toLower(value) == "true";
					_bl->out.printDebug("Debug: enableNodeBlue set to " + std::to_string(_enableNodeBlue));
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
					if(_writeableDataPath == _executablePath) _writeableDataPath = _dataPath;
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
				else if(name == "writeabledatapath")
				{
					_writeableDataPath = value;
					if(_writeableDataPath.empty()) _writeableDataPath = _dataPath.empty() ? _executablePath : _dataPath;
					if(_writeableDataPath.back() != '/') _writeableDataPath.push_back('/');
					_bl->out.printDebug("Debug: writeableDataPath set to " + _writeableDataPath);
				}
				else if(name == "writeabledatapathpermissions")
				{
					_writeableDataPathPermissions = Math::getOctalNumber(value);
					if(_writeableDataPathPermissions == 0) _writeableDataPathPermissions = 504;
					_bl->out.printDebug("Debug: writeableDataPathPermissions set to " + _writeableDataPathPermissions);
				}
				else if(name == "writeabledatapathuser")
				{
					_writeableDataPathUser = value;
					_bl->out.printDebug("Debug: writeableDataPathUser set to " + _writeableDataPathUser);
				}
				else if(name == "writeabledatapathgroup")
				{
					_writeableDataPathGroup = value;
					_bl->out.printDebug("Debug: writeableDataPathGroup set to " + _writeableDataPathGroup);
				}
				else if(name == "familydatapath")
				{
					_familyDataPath = value;
					if(_familyDataPath.empty()) _familyDataPath = _executablePath + "families";
					if(_familyDataPath.back() != '/') _familyDataPath.push_back('/');
					_bl->out.printDebug("Debug: familyDataPath set to " + _familyDataPath);
				}
				else if(name == "familydatapathpermissions")
				{
					_familyDataPathPermissions = Math::getOctalNumber(value);
					if(_familyDataPathPermissions == 0) _familyDataPathPermissions = 504;
					_bl->out.printDebug("Debug: familyDataPathPermissions set to " + _familyDataPathPermissions);
				}
				else if(name == "familydatapathuser")
				{
					_familyDataPathUser = value;
					_bl->out.printDebug("Debug: familyDataPathUser set to " + _familyDataPathUser);
				}
				else if(name == "familydatapathgroup")
				{
					_familyDataPathGroup = value;
					_bl->out.printDebug("Debug: familyDataPathGroup set to " + _familyDataPathGroup);
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
				else if(name == "waitforcorrecttime")
				{
					_waitForCorrectTime = (HelperFunctions::toLower(value) == "true");
					_bl->out.printDebug("Debug: waitForCorrectTime set to " + std::to_string(_waitForCorrectTime));
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
                else if(name == "scriptenginewatchdogtimeout")
                {
                    _scriptEngineWatchdogTimeout = Math::getNumber(value);
                    if(_scriptEngineWatchdogTimeout < 0) _scriptEngineWatchdogTimeout = -1;
                    else if(_scriptEngineWatchdogTimeout < 10000) _scriptEngineWatchdogTimeout = 10000;
                    _bl->out.printDebug("Debug: scriptEngineWatchdogTimeout set to " + std::to_string(_scriptEngineWatchdogTimeout));
                }
				else if(name == "scriptenginemanualclientstart")
				{
					_scriptEngineManualClientStart = HelperFunctions::toLower(value) == "true";
					_bl->out.printDebug("Debug: scriptEngineManualClientStart set to " + std::to_string(_scriptEngineManualClientStart));
				}
				else if(name == "flowsprocessingthreadcountserver" || name == "nodeblueprocessingthreadcountserver")
				{
					_nodeBlueProcessingThreadCountServer = Math::getNumber(value);
					_bl->out.printDebug("Debug: nodeBlueProcessingThreadCountServer set to " + std::to_string(_nodeBlueProcessingThreadCountServer));
				}
				else if(name == "flowsprocessingthreadcountnodes" || name == "nodeblueprocessingthreadcountnodes")
				{
					_nodeBlueProcessingThreadCountServer = Math::getNumber(value);
					_bl->out.printDebug("Debug: nodeBlueProcessingThreadCountNodes set to " + std::to_string(_nodeBlueProcessingThreadCountNodes));
				}
				else if(name == "flowsservermaxconnections" || name == "nodeblueservermaxconnections")
				{
					_nodeBlueServerMaxConnections = Math::getNumber(value);
					_bl->out.printDebug("Debug: nodeBlueServerMaxConnections set to " + std::to_string(_nodeBlueServerMaxConnections));
				}
				else if(name == "nodebluedebugoutput")
				{
					_nodeBlueDebugOutput = value == "true";
					_bl->out.printDebug("Debug: nodeBlueDebugOutput set to " + std::to_string(_nodeBlueDebugOutput));
				}
                else if(name == "nodeblueeventlimit1")
                {
                    _nodeBlueEventLimit1 = Math::getUnsignedNumber(value);
                    _bl->out.printDebug("Debug: nodeBlueEventLimit1 set to " + std::to_string(_nodeBlueEventLimit1));
                }
                else if(name == "nodeblueeventlimit2")
                {
                    _nodeBlueEventLimit2 = Math::getUnsignedNumber(value);
                    _bl->out.printDebug("Debug: nodeBlueEventLimit2 set to " + std::to_string(_nodeBlueEventLimit2));
                }
                else if(name == "nodeblueeventlimit3")
                {
                    _nodeBlueEventLimit3 = Math::getUnsignedNumber(value);
                    _bl->out.printDebug("Debug: nodeBlueEventLimit3 set to " + std::to_string(_nodeBlueEventLimit3));
                }
				else if(name == "maxnodethreadsperprocess")
				{
					_maxNodeThreadsPerProcess = Math::getNumber(value);
					_bl->out.printDebug("Debug: maxNodeThreadsPerProcess set to " + std::to_string(_maxNodeThreadsPerProcess));
				}
                else if(name == "flowswatchdogtimeout" || name == "nodebluewatchdogtimeout")
                {
                    _nodeBlueWatchdogTimeout = Math::getNumber(value);
                    if(_nodeBlueWatchdogTimeout < 0) _nodeBlueWatchdogTimeout = -1;
                    else if(_nodeBlueWatchdogTimeout < 10000) _nodeBlueWatchdogTimeout = 10000;
                    _bl->out.printDebug("Debug: nodeBlueWatchdogTimeout set to " + std::to_string(_nodeBlueWatchdogTimeout));
                }
				else if(name == "flowsmanualclientstart" || name == "nodebluemanualclientstart")
				{
					_nodeBlueManualClientStart = HelperFunctions::toLower(value) == "true";
					_bl->out.printDebug("Debug: nodeBlueManualClientStart set to " + std::to_string(_nodeBlueManualClientStart));
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
				else if(name == "flowspath" || name == "nodebluepath")
				{
					_nodeBluePath = value;
					if(_nodeBluePath.empty()) _nodeBluePath = "/var/lib/homegear/node-blue/";
					if(_nodeBluePath.back() != '/') _nodeBluePath.push_back('/');
					_bl->out.printDebug("Debug: nodeBluePath set to " + _nodeBluePath);
				}
				else if(name == "flowspathpermissions" || name == "nodebluepathpermissions")
				{
					_nodeBluePathPermissions = Math::getOctalNumber(value);
					if(_nodeBluePathPermissions == 0) _nodeBluePathPermissions = 504;
					_bl->out.printDebug("Debug: nodebluePathPermissions set to " + _nodeBluePathPermissions);
				}
				else if(name == "flowspathuser" || name == "nodebluepathuser")
				{
					_nodeBluePathUser = value;
					_bl->out.printDebug("Debug: nodeBluePathUser set to " + _nodeBluePathUser);
				}
				else if(name == "flowspathgroup" || name == "nodebluepathgroup")
				{
					_nodeBluePathGroup = value;
					_bl->out.printDebug("Debug: nodeBluePathGroup set to " + _nodeBluePathGroup);
				}
				else if(name == "flowsdatapath" || name == "nodebluedatapath")
				{
					_nodeBlueDataPath = value;
					if(_nodeBlueDataPath.empty()) _nodeBlueDataPath = "/var/lib/homegear/node-blue/data/";
					if(_nodeBlueDataPath.back() != '/') _nodeBlueDataPath.push_back('/');
					_bl->out.printDebug("Debug: nodeBlueDataPath set to " + _nodeBlueDataPath);
				}
				else if(name == "flowsdatapathpermissions" || name == "nodebluedatapathpermissions")
				{
					_nodeBlueDataPathPermissions = Math::getOctalNumber(value);
					if(_nodeBlueDataPathPermissions == 0) _nodeBlueDataPathPermissions = 504;
					_bl->out.printDebug("Debug: nodeBlueDataPathPermissions set to " + std::to_string(_nodeBlueDataPathPermissions));
				}
				else if(name == "flowsdatapathuser" || name == "nodebluedatapathuser")
				{
					_nodeBlueDataPathUser = value;
					_bl->out.printDebug("Debug: nodeBlueDataPathUser set to " + _nodeBlueDataPathUser);
				}
				else if(name == "flowsdatapathgroup" || name == "nodebluedatapathgroup")
				{
					_nodeBlueDataPathGroup = value;
					_bl->out.printDebug("Debug: nodeBlueDataPathGroup set to " + _nodeBlueDataPathGroup);
				}
				else if(name == "adminuipath")
				{
					_adminUiPath = value;
					if(_adminUiPath.empty()) _adminUiPath = "/var/lib/homegear/admin-ui/";
					if(_adminUiPath.back() != '/') _adminUiPath.push_back('/');
					_bl->out.printDebug("Debug: adminUiPath set to " + _adminUiPath);
				}
				else if(name == "adminuipathpermissions")
				{
					_adminUiPathPermissions = Math::getOctalNumber(value);
					if(_adminUiPathPermissions == 0) _adminUiPathPermissions = 504;
					_bl->out.printDebug("Debug: adminUiPathPermissions set to " + _adminUiPathPermissions);
				}
				else if(name == "adminuipathuser")
				{
					_adminUiPathUser = value;
					_bl->out.printDebug("Debug: adminUiPathUser set to " + _adminUiPathUser);
				}
				else if(name == "adminuipathgroup")
				{
					_adminUiPathGroup = value;
					_bl->out.printDebug("Debug: adminUiPathGroup set to " + _adminUiPathGroup);
				}
				else if(name == "uipath")
				{
					_uiPath = value;
					if(_uiPath.empty()) _uiPath = "/var/lib/homegear/ui/";
					if(_uiPath.back() != '/') _uiPath.push_back('/');
					_bl->out.printDebug("Debug: uiPath set to " + _uiPath);
				}
				else if(name == "uipathpermissions")
				{
					_uiPathPermissions = Math::getOctalNumber(value);
					if(_uiPathPermissions == 0) _uiPathPermissions = 504;
					_bl->out.printDebug("Debug: uiPathPermissions set to " + _uiPathPermissions);
				}
				else if(name == "uipathuser")
				{
					_uiPathUser = value;
					_bl->out.printDebug("Debug: uiPathUser set to " + _uiPathUser);
				}
				else if(name == "uipathgroup")
				{
					_uiPathGroup = value;
					_bl->out.printDebug("Debug: uiPathGroup set to " + _uiPathGroup);
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
				// {{{ OAuth
					else if(name == "oauthcertpath")
					{
						_oauthCertPath = value;
						_bl->out.printDebug("Debug: oauthCertPath set to " + _oauthCertPath);
					}
					else if(name == "oauthkeypath")
					{
						_oauthKeyPath = value;
						_bl->out.printDebug("Debug: oauthKeyPath set to " + _oauthKeyPath);
					}
					else if(name == "oauthtokenlifetime")
					{
						_oauthTokenLifetime = Math::getNumber(value);
						if(_oauthTokenLifetime < 0) _oauthTokenLifetime = 3600;
						_bl->out.printDebug("Debug: oauthTokenLifetime set to " + std::to_string(_oauthTokenLifetime));
					}
					else if(name == "oauthrefreshtokenlifetime")
					{
						_oauthRefreshTokenLifetime = Math::getNumber(value);
						if(_oauthRefreshTokenLifetime < 0) _oauthRefreshTokenLifetime = 5184000;
						_bl->out.printDebug("Debug: oauthRefreshTokenLifetime set to " + std::to_string(_oauthRefreshTokenLifetime));
					}
				// }}}
                //{{{ Deprecated settings
                    else if(name == "capath")
                    {
                        _bl->out.printWarning("Warning: The setting caPath has been moved from \"main.conf\" to \"rpcservers.conf\".");
                    }
                    else if(name == "certpath")
                    {
                        _bl->out.printWarning("Warning: The setting certPath has been moved from \"main.conf\" to \"rpcservers.conf\".");
                    }
                    else if(name == "keypath")
                    {
                        _bl->out.printWarning("Warning: The setting keyPath has been moved from \"main.conf\" to \"rpcservers.conf\".");
                    }
                    else if(name == "dhparampath")
                    {
                        _bl->out.printWarning("Warning: The setting dhParamPath has been moved from \"main.conf\" to \"rpcservers.conf\".");
                    }
                //}}}
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
}

}
