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

#ifndef GPIO_H_
#define GPIO_H_

#include <map>
#include <mutex>
#include <memory>
#include <vector>

namespace BaseLib
{

class SharedObjects;
class FileDescriptor;

namespace LowLevel
{

class Gpio
{
public:
	struct GpioDirection
	{
			enum Enum
			{
					IN,
					OUT
			};
	};

	struct GpioEdge
	{
			enum Enum
			{
					RISING,
					FALLING,
					BOTH
			};
	};

	Gpio(BaseLib::SharedObjects* baseLib, std::string gpioPath);
	virtual ~Gpio();

	virtual void openDevice(uint32_t index, bool readOnly);
	virtual void getPath(uint32_t index);
	virtual void closeDevice(uint32_t index);
	virtual bool get(uint32_t index);

	/**
	 * Waits for a GPIO input to change and returns the input value.
	 *
	 * @param index The index of the GPIO
	 * @param timeout The timeout in milliseconds after which to return regardless if the input changed
	 * @param debounce If true the method waits 30 milliseconds after the input change and then returns the value (use this for e. g. switches)
	 * @return Returns -1 on error, -2 on timeout, 0 for input low and 1 for input high
	 */
	virtual int32_t poll(uint32_t index, int32_t timeout, bool debounce);

	virtual void set(uint32_t index, bool value);
	virtual void setPermission(uint32_t index, int32_t userID, int32_t groupID, bool readOnly);
	virtual void exportGpio(uint32_t index);
	virtual void setDirection(uint32_t index, GpioDirection::Enum direction);
	virtual void setEdge(uint32_t index, GpioEdge::Enum edge);
	virtual bool isOpen(uint32_t index);
	virtual void setup(int32_t userId, int32_t groupId, bool setPermissions, std::vector<uint32_t>& exportGpios);
	virtual std::shared_ptr<FileDescriptor> getFileDescriptor(uint32_t index);
protected:
	class GpioInfo
	{
	public:
		std::string path;
		std::shared_ptr<FileDescriptor> fileDescriptor;
	};

	BaseLib::SharedObjects* _bl = nullptr;
	std::string _gpioPath;
	std::mutex _gpioMutex;
	std::map<uint32_t, GpioInfo> _gpioInfo;
};

}
}
#endif
