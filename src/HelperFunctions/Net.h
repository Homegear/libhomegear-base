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

#ifndef HOMEGEARNET_H_
#define HOMEGEARNET_H_

#include "../Exception.h"

#include <string>
#include <vector>
#include <memory>
#include <array>

namespace BaseLib
{
/**
 * Exception class for the HTTP client.
 *
 * @see HTTPClient
 */
class NetException : public Exception
{
public:
	NetException(std::string message) : Exception(message) {}
};

/**
 * Class with network related helper functions.
 */
class Net
{
public:
	struct RouteInfo
	{
	    bool ipv6 = false;
        std::array<uint8_t, 16> destinationAddress{};
        uint8_t sourceNetmask = 0;
        std::array<uint8_t, 16> sourceAddress{};
        uint8_t destinationNetmask = 0;
        std::array<uint8_t, 16> gateway{};
        std::string interfaceName;
	};

	typedef std::vector<std::shared_ptr<RouteInfo>> RouteInfoList;

	Net();

	/**
	 * Destructor.
	 * Does nothing.
	 */
	virtual ~Net();

	/**
	 * Checks is a string is a valid IPv4 or IPv6 address.
	 *
	 * @return Returns true if the string is an IP address.
	 */
	static bool isIp(const std::string& ipAddress);

	/**
	 * Tries to automatically determine the computers IPv4 address.
	 *
	 * @param interfaceName If specified, the IP address of this interface is returned.
	 * @return Returns the computers IPv4 address.
	 */
	static std::string getMyIpAddress(const std::string& interfaceName = "");

	/**
	 * Tries to automatically determine the computers IPv6 address.

	 * @param interfaceName If specified, the IP address of this interface is returned.
	 * @return Returns the computers IPv6 address.
	 */
	static std::string getMyIp6Address(std::string interfaceName = "");

	/**
	 * Tries to get the IP address for a hostname.
	 *
	 * @param hostname The hostname to get the IP address for. It is ok to pass an IP address as hostname.
	 * @return Returns the IP address or an empty String on error.
	 */
	static std::string resolveHostname(std::string& hostname);

	/**
	 * Returns a list of all defined network routes.
	 *
	 * @return The returned routes of type RouteInfoList.
	 */
	static RouteInfoList getRoutes();

	static std::vector<uint8_t> getMacAddress(bool allowLocallyAdministered, const std::string &interface = "");
private:
	static int32_t readNlSocket(int32_t sockFd, std::vector<char>& buffer, uint32_t messageIndex, uint32_t pid);
};

}

#endif
