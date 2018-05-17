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

#include "Ssdp.h"
#include "../BaseLib.h"
#include "../Encoding/RapidXml/rapidxml.hpp"
#include <ifaddrs.h>

namespace BaseLib
{

SsdpInfo::SsdpInfo()
{
}

SsdpInfo::SsdpInfo(std::string ip, int32_t port, std::string path, PVariable info)
{
	_ip = ip;
    _port = port;
    _path = path;
	_info = info;
}

SsdpInfo::~SsdpInfo()
{

}

Ssdp::Ssdp(SharedObjects* baseLib)
{
	_bl = baseLib;
	getAddress();
}

Ssdp::~Ssdp()
{
}

void Ssdp::getAddress()
{
	try
	{
		std::string address;
		if(!_bl->settings.ssdpIpAddress().empty() && !Net::isIp(_bl->settings.ssdpIpAddress()))
		{
			//Assume address is interface name
			_address = BaseLib::Net::getMyIpAddress(_bl->settings.ssdpIpAddress());
		}
		else if(_bl->settings.ssdpIpAddress().empty() || _bl->settings.ssdpIpAddress() == "0.0.0.0" || _bl->settings.ssdpIpAddress() == "::")
		{
			_address = BaseLib::Net::getMyIpAddress();
			if(_address.empty()) _bl->out.printError("Error: No IP address could be found to bind the server to. Please specify the IP address manually in main.conf.");
		}
		else _address = _bl->settings.ssdpIpAddress();
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(Exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

std::shared_ptr<FileDescriptor> Ssdp::getSocketDescriptor(int32_t port, bool bindToMulticast)
{
	std::shared_ptr<FileDescriptor> serverSocketDescriptor;
	try
	{
		if(_address.empty()) getAddress();
		if(_address.empty()) return serverSocketDescriptor;
		serverSocketDescriptor = _bl->fileDescriptorManager.add(socket(AF_INET, SOCK_DGRAM, 0));
		if(serverSocketDescriptor->descriptor == -1)
		{
			_bl->out.printError("Error: Could not create socket.");
			return serverSocketDescriptor;
		}

		int32_t reuse = 1;
		if(setsockopt(serverSocketDescriptor->descriptor, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) == -1)
		{
			_bl->out.printWarning("Warning: Could set SSDP socket options: " + std::string(strerror(errno)));
		}

		if(_bl->debugLevel >= 5) _bl->out.printInfo("Debug: SSDP server: Binding to address: " + _address);

		char loopch = 0;
		if(setsockopt(serverSocketDescriptor->descriptor, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) == -1)
		{
			_bl->out.printWarning("Warning: Could set SSDP socket options: " + std::string(strerror(errno)));
		}

		struct in_addr localInterface;
		localInterface.s_addr = inet_addr(_address.c_str());
		if(setsockopt(serverSocketDescriptor->descriptor, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) == -1)
		{
			_bl->out.printWarning("Warning: Could set SSDP socket options: " + std::string(strerror(errno)));
		}

		struct sockaddr_in localSock;
		memset((char *) &localSock, 0, sizeof(localSock));
		localSock.sin_family = AF_INET;
		localSock.sin_port = htons(port);
		localSock.sin_addr.s_addr = inet_addr(bindToMulticast ? "239.255.255.250" : _address.c_str());

		if(bind(serverSocketDescriptor->descriptor, (struct sockaddr*)&localSock, sizeof(localSock)) == -1)
		{
			_bl->out.printError("Error: Binding to address " + _address + " failed: " + std::string(strerror(errno)));
			_bl->fileDescriptorManager.close(serverSocketDescriptor);
			return serverSocketDescriptor;
		}

        if(!bindToMulticast)
        {
            struct ip_mreq group;
            group.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
            group.imr_interface.s_addr = inet_addr(_address.c_str());

            if (setsockopt(serverSocketDescriptor->descriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &group, sizeof(group)) == -1)
            {
                _bl->out.printWarning("Warning: Could set SSDP socket options: " + std::string(strerror(errno)));
            }
        }
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(Exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return serverSocketDescriptor;
}

void Ssdp::sendSearchBroadcast(std::shared_ptr<FileDescriptor>& serverSocketDescriptor, const std::string& stHeader, uint32_t timeout)
{
	if(!serverSocketDescriptor || serverSocketDescriptor->descriptor == -1) return;
	struct sockaddr_in addessInfo;
	addessInfo.sin_family = AF_INET;
	addessInfo.sin_addr.s_addr = inet_addr("239.255.255.250");
	addessInfo.sin_port = htons(1900);

	if(timeout < 1000) timeout = 1000;

	std::string broadcastPacket("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: " + std::to_string(timeout / 1000) + "\r\nST: " + stHeader + "\r\nContent-Length: 0\r\n\r\n");

	if(sendto(serverSocketDescriptor->descriptor, &broadcastPacket.at(0), broadcastPacket.size(), 0, (struct sockaddr*)&addessInfo, sizeof(addessInfo)) == -1)
	{
		_bl->out.printWarning("Warning: Could send SSDP search broadcast packet: " + std::string(strerror(errno)));
	}
}

void Ssdp::searchDevices(const std::string& stHeader, uint32_t timeout, std::vector<SsdpInfo>& devices)
{
	std::shared_ptr<FileDescriptor> serverSocketDescriptor;
	try
	{
		if(stHeader.empty())
		{
			_bl->out.printError("Error: Cannot search for SSDP devices. ST header is empty.");
			return;
		}

		serverSocketDescriptor = getSocketDescriptor(_bl->settings.ssdpPort(), false);
		if(!serverSocketDescriptor || serverSocketDescriptor->descriptor == -1) return;
		if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Searching for SSDP devices ...");

		sendSearchBroadcast(serverSocketDescriptor, stHeader, timeout);
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
		sendSearchBroadcast(serverSocketDescriptor, stHeader, timeout);

		uint64_t startTime = _bl->hf.getTime();
		char buffer[1024];
		int32_t bytesReceived = 0;
		struct sockaddr_in si_other;
		socklen_t slen = sizeof(si_other);
		fd_set readFileDescriptor;
		timeval socketTimeout;
		int32_t nfds = 0;
		Http http;
		std::map<std::string, SsdpInfo> info;
		while(_bl->hf.getTime() - startTime <= (timeout + 500))
		{
			try
			{
				if(!serverSocketDescriptor || serverSocketDescriptor->descriptor == -1) break;

				socketTimeout.tv_sec = timeout / 1000;
				socketTimeout.tv_usec = 500000;
				FD_ZERO(&readFileDescriptor);
				auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
				fileDescriptorGuard.lock();
				nfds = serverSocketDescriptor->descriptor + 1;
				if(nfds <= 0)
				{
					fileDescriptorGuard.unlock();
					_bl->out.printError("Error: Socket closed (1).");
					_bl->fileDescriptorManager.shutdown(serverSocketDescriptor);
                    continue;
				}
				FD_SET(serverSocketDescriptor->descriptor, &readFileDescriptor);
				fileDescriptorGuard.unlock();
				bytesReceived = select(nfds, &readFileDescriptor, NULL, NULL, &socketTimeout);
				if(bytesReceived == 0) continue;
				if(bytesReceived != 1)
				{
					_bl->out.printError("Error: Socket closed (2).");
					_bl->fileDescriptorManager.shutdown(serverSocketDescriptor);
                    continue;
				}

				bytesReceived = recvfrom(serverSocketDescriptor->descriptor, buffer, 1024, 0, (struct sockaddr *)&si_other, &slen);
				if(bytesReceived == 0) continue;
                else if(bytesReceived == -1)
                {
                    _bl->out.printError("Error: Socket closed (3).");
                    _bl->fileDescriptorManager.shutdown(serverSocketDescriptor);
                    continue;
                }
				if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: SSDP response received:\n" + std::string(buffer, bytesReceived));
				http.reset();
				http.process(buffer, bytesReceived, false);
				if(http.headerIsFinished()) processPacket(http, stHeader, info);
			}
			catch(const std::exception& ex)
			{
				_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
			}
			catch(Exception& ex)
			{
				_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
			}
			catch(...)
			{
				_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
			}
		}
		getDeviceInfo(info, devices);
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(Exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	_bl->fileDescriptorManager.shutdown(serverSocketDescriptor);
}

void Ssdp::searchDevicesPassive(const std::string& stHeader, uint32_t timeout, std::vector<SsdpInfo>& devices, std::atomic_bool& abort)
{
	std::shared_ptr<FileDescriptor> serverSocketDescriptor;
	try
	{
		if(stHeader.empty())
		{
			_bl->out.printError("Error: Cannot search for SSDP devices. ST header is empty.");
			return;
		}

		serverSocketDescriptor = getSocketDescriptor(1900, true);
		if(!serverSocketDescriptor || serverSocketDescriptor->descriptor == -1) return;
		if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Searching for SSDP devices ...");

		uint64_t startTime = _bl->hf.getTime();
		char buffer[1024];
		int32_t bytesReceived = 0;
		struct sockaddr_in si_other;
		socklen_t slen = sizeof(si_other);
		fd_set readFileDescriptor;
		timeval socketTimeout;
		int32_t nfds = 0;
		Http http;
		std::map<std::string, SsdpInfo> info;
		while(_bl->hf.getTime() - startTime <= timeout && !abort)
		{
			try
			{
				if(!serverSocketDescriptor || serverSocketDescriptor->descriptor == -1) break;

				socketTimeout.tv_sec = 0;
				socketTimeout.tv_usec = 100000;
				FD_ZERO(&readFileDescriptor);
				auto fileDescriptorGuard = _bl->fileDescriptorManager.getLock();
				fileDescriptorGuard.lock();
				nfds = serverSocketDescriptor->descriptor + 1;
				if(nfds <= 0)
				{
					fileDescriptorGuard.unlock();
					_bl->out.printError("Error: Socket closed (1).");
					_bl->fileDescriptorManager.shutdown(serverSocketDescriptor);
                    continue;
				}
				FD_SET(serverSocketDescriptor->descriptor, &readFileDescriptor);
				fileDescriptorGuard.unlock();
				bytesReceived = select(nfds, &readFileDescriptor, NULL, NULL, &socketTimeout);
				if(bytesReceived == 0) continue;
				if(bytesReceived != 1)
				{
					_bl->out.printError("Error: Socket closed (2).");
					_bl->fileDescriptorManager.shutdown(serverSocketDescriptor);
                    continue;
				}

				bytesReceived = recvfrom(serverSocketDescriptor->descriptor, buffer, 1024, 0, (struct sockaddr *)&si_other, &slen);
				if(bytesReceived == 0) continue;
                else if(bytesReceived == -1)
                {
                    _bl->out.printError("Error: Socket closed (3).");
                    _bl->fileDescriptorManager.shutdown(serverSocketDescriptor);
                    continue;
                }
				if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: SSDP response received:\n" + std::string(buffer, bytesReceived));
				http.reset();
				http.process(buffer, bytesReceived, false);
				if(http.headerIsFinished()) processPacketPassive(http, stHeader, info);
			}
			catch(const std::exception& ex)
			{
				_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
			}
			catch(Exception& ex)
			{
				_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
			}
			catch(...)
			{
				_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
			}
		}
		getDeviceInfo(info, devices);
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(Exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	_bl->fileDescriptorManager.shutdown(serverSocketDescriptor);
}

void Ssdp::processPacket(Http& http, const std::string& stHeader, std::map<std::string, SsdpInfo>& info)
{
	try
	{
		Http::Header& header = http.getHeader();
		if(header.responseCode != 200 || header.fields.at("st") != stHeader) return;

		std::string location = header.fields.at("location");
		if(location.size() < 7) return;
		SsdpInfo currentInfo;
		currentInfo.setLocation(location);

		for(auto& field : header.fields)
		{
			currentInfo.addField(field.first, field.second);
		}

		info.emplace(location, currentInfo);
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(Exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void Ssdp::processPacketPassive(Http& http, const std::string& stHeader, std::map<std::string, SsdpInfo>& info)
{
    try
    {
        Http::Header& header = http.getHeader();
        if(header.method != "NOTIFY") return;
        auto headerIterator = header.fields.find("nt");
        if(headerIterator == header.fields.end() || headerIterator->second != stHeader) return;

        std::string location = header.fields.at("location");
        if(location.size() < 7) return;
        SsdpInfo currentInfo;
        currentInfo.setLocation(location);

        for(auto& field : header.fields)
        {
            currentInfo.addField(field.first, field.second);
        }

        info.emplace(location, currentInfo);
    }
    catch(const std::exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void Ssdp::getDeviceInfo(std::map<std::string, SsdpInfo>& info, std::vector<SsdpInfo>& devices)
{
	try
	{
		devices.reserve(info.size());
		for(auto& currentInfo : info)
        {
			std::string location = currentInfo.second.location();
			std::string::size_type posPort = location.find(':', 7);
			if(posPort == std::string::npos) return;
			std::string::size_type posPath = location.find('/', posPort);
			if(posPath == std::string::npos) posPath = location.size();
			std::string ip = location.substr(7, posPort - 7);
			std::string portString = location.substr(posPort + 1, posPath - posPort - 1);
			int32_t port = Math::getNumber(portString, false);
			if(port <= 0 || port > 65535) return;
			std::string path = posPath == location.size() ? "/" : location.substr(posPath);

			currentInfo.second.setIp(ip);
			currentInfo.second.setPort(port);
			currentInfo.second.setPath(path);

			HttpClient client(_bl, ip, port, false);
			std::string xml;
			client.get(path, xml);

			PVariable infoStruct;
			if(!xml.empty())
			{
				xml_document<> doc;
				doc.parse<parse_no_entity_translation | parse_validate_closing_tags>(&xml.at(0));
				xml_node<>* node = doc.first_node("root");
				if(node)
				{
					node = node->first_node("device");
					if(node)
					{
						infoStruct.reset(new Variable(node));
					}
				}
			}

			devices.push_back(currentInfo.second);
		}
	}
	catch(const std::exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(Exception& ex)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

}
