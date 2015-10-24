/* Copyright 2013-2015 Sathya Laufer
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

#include "Gpio.h"
#include "../BaseLib.h"

#include <sys/stat.h>

namespace BaseLib
{

Gpio::Gpio(BaseLib::Obj* baseLib)
{
	_bl = baseLib;
}

Gpio::~Gpio()
{
	_gpioMutex.lock();
	for(std::map<uint32_t, GpioInfo>::iterator i = _gpioInfo.begin(); i != _gpioInfo.end(); ++i)
	{
		_bl->fileDescriptorManager.close(i->second.fileDescriptor);
	}
	_gpioInfo.clear();
	_gpioMutex.unlock();
}

void Gpio::openDevice(uint32_t index, bool readOnly)
{
	_gpioMutex.lock();
	try
	{
		_gpioMutex.unlock();
		getPath(index); //Creates gpioInfo entry if it doesn't exist
		_gpioMutex.lock();
		if(_gpioInfo[index].path.empty())
		{
			_gpioInfo.erase(index);
			throw(Exception("Failed to open value file for GPIO with index " + std::to_string(index) + ": Unable to retrieve path."));
		}
		std::string path = _gpioInfo[index].path + "value";
		_gpioInfo[index].fileDescriptor = _bl->fileDescriptorManager.add(open(path.c_str(), readOnly ? O_RDONLY : O_RDWR));
		if (_gpioInfo[index].fileDescriptor->descriptor == -1) throw(Exception("Failed to open GPIO value file \"" + path + "\": " + std::string(strerror(errno))));
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
    _gpioMutex.unlock();
}

void Gpio::getPath(uint32_t index)
{
	_gpioMutex.lock();
	try
	{
		//Creates gpioInfo entry if it doesn't exist
		if(!_gpioInfo[index].path.empty())
		{
			_gpioMutex.unlock();
			return;
		}
		DIR* directory;
		struct dirent* entry;
		std::string gpioDir(_bl->settings.gpioPath());
		if((directory = opendir(gpioDir.c_str())) != 0)
		{
			while((entry = readdir(directory)) != 0)
			{
				struct stat dirStat;
				std::string dirName(gpioDir + std::string(entry->d_name));
				if(stat(dirName.c_str(), &dirStat) == -1)
				{
					_bl->out.printError("Error executing \"stat\" on entry \"" + dirName + "\": " + std::string(strerror(errno)));
					continue;
				}
				if(S_ISDIR(dirStat.st_mode))
				{
					try
					{
						int32_t pos = dirName.find_last_of('/');
						if(pos == (signed)std::string::npos || pos >= (signed)dirName.length()) continue;
						std::string subdirName = dirName.substr(pos + 1);
						if(subdirName.compare(0, 4, "gpio") != 0) continue;
						std::string number(std::to_string(index));
						if(subdirName.length() < 4 + number.length()) continue;
						if(subdirName.length() > 4 + number.length() && std::isdigit(subdirName.at(4 + number.length()))) continue;
						std::string number2(subdirName.substr(4, number.length()));
						if(number2 == number)
						{
							_bl->out.printDebug("Debug: GPIO path for GPIO with index " + std::to_string(index) + " set to \"" + dirName + "\".");
							if(dirName.back() != '/') dirName.push_back('/');
							_gpioInfo[index].path = dirName;
							closedir(directory);
							_gpioMutex.unlock();
							return;
						}
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
			closedir(directory);
		}
		else throw(Exception("Could not open directory \"" + _bl->settings.gpioPath() + "\"."));
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
    _gpioMutex.unlock();
}

void Gpio::closeDevice(uint32_t index)
{
	_gpioMutex.lock();
	try
	{
		std::map<uint32_t, GpioInfo>::iterator gpioIterator = _gpioInfo.find(index);
		if(gpioIterator != _gpioInfo.end())
		{
			_bl->fileDescriptorManager.close(gpioIterator->second.fileDescriptor);
		}
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
    _gpioMutex.unlock();
}

bool Gpio::get(uint32_t index)
{
	try
	{
		if(!isOpen(index))
		{
			_bl->out.printError("Failed to set GPIO with index \"" + std::to_string(index) + "\": Device not open.");
			return false;
		}
		std::vector<char> buffer(1);
		_gpioMutex.lock();
		if(read(_gpioInfo[index].fileDescriptor->descriptor, &buffer.at(0), 1) != 1)
		{
			_gpioMutex.unlock();
			_bl->out.printError("Could not read GPIO with index " + std::to_string(index) + ".");
			return false;
		}
		_gpioMutex.unlock();
		return buffer.at(0) == '1';
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
    return false;
}

void Gpio::set(uint32_t index, bool value)
{
	try
	{
		if(!isOpen(index))
		{
			_bl->out.printError("Failed to set GPIO with index \"" + std::to_string(index) + "\": Device not open.");
			return;
		}
		std::string temp(std::to_string((int32_t)value));
		_gpioMutex.lock();
		if(write(_gpioInfo[index].fileDescriptor->descriptor, temp.c_str(), temp.size()) <= 0)
		{
			_bl->out.printError("Could not write GPIO with index " + std::to_string(index) + ".");
		}
		_bl->out.printDebug("Debug: GPIO " + std::to_string(index) + " set to " + std::to_string(value) + ".");
		_gpioMutex.unlock();
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

void Gpio::setPermission(uint32_t index, int32_t userID, int32_t groupID, bool readOnly)
{
	try
	{
		getPath(index);
		_gpioMutex.lock();
		if(_gpioInfo[index].path.empty())
		{
			_gpioInfo.erase(index);
			throw(Exception("Error: Failed to get path for GPIO with index " + std::to_string(index) + "."));
		}
		std::string path = _gpioInfo[index].path + "value";
    	int32_t result = chown(path.c_str(), userID, groupID);
    	if(result == -1)
    	{
    		_bl->out.printError("Error: Could not set owner for GPIO value file " + path + ": " + std::string(strerror(errno)));
    	}
    	result = chmod(path.c_str(), readOnly ? (S_IRUSR | S_IRGRP) : (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
    	if(result == -1)
    	{
    		_bl->out.printError("Error: Could not set permissions for GPIO value file " + path + ": " + std::string(strerror(errno)));
    	}
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
    _gpioMutex.unlock();
}

bool Gpio::isOpen(uint32_t index)
{
	_gpioMutex.lock();
	try
	{
		std::map<uint32_t, GpioInfo>::iterator gpioIterator = _gpioInfo.find(index);
		if(gpioIterator != _gpioInfo.end() && gpioIterator->second.fileDescriptor && gpioIterator->second.fileDescriptor->descriptor != -1)
		{
			_gpioMutex.unlock();
			return true;
		}
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
    _gpioMutex.unlock();
    return false;
}

void Gpio::exportGpio(uint32_t index)
{
	try
	{
		_gpioMutex.lock();
		if(_gpioInfo[index].path.empty())
		{
			_gpioMutex.unlock();
			getPath(index);
			_gpioMutex.lock();
		}
		if(_gpioInfo[index].path.empty())
		{
			_gpioInfo.erase(index);
			throw(Exception("Error: Failed to get path for GPIO with index " + std::to_string(index) + "."));
		}
		std::string path;
		std::shared_ptr<FileDescriptor> fileDescriptor;
		std::string temp(std::to_string(index));
		if(!_gpioInfo[index].path.empty())
		{
			_bl->out.printDebug("Debug: Unexporting GPIO with index " + std::to_string(index) + " and number " + std::to_string(index) + ".");
			path = _bl->settings.gpioPath() + "unexport";
			fileDescriptor = _bl->fileDescriptorManager.add(open(path.c_str(), O_WRONLY));
			if (fileDescriptor->descriptor == -1) throw(Exception("Could not unexport GPIO with index " + std::to_string(index) + ". Failed to write to unexport file: " + std::string(strerror(errno))));
			if(write(fileDescriptor->descriptor, temp.c_str(), temp.size()) == -1)
			{
				_bl->out.printError("Error: Could not unexport GPIO with index " + std::to_string(index) + " and number " + temp + ": " + std::string(strerror(errno)));
			}
			_bl->fileDescriptorManager.close(fileDescriptor);
			_gpioInfo[index].path.clear();
		}

		_bl->out.printDebug("Debug: Exporting GPIO with index " + std::to_string(index) + " and number " + std::to_string(index) + ".");
		path = _bl->settings.gpioPath() + "export";
		fileDescriptor = _bl->fileDescriptorManager.add(open(path.c_str(), O_WRONLY));
		if (fileDescriptor->descriptor == -1) throw(Exception("Error: Could not export GPIO with index " + std::to_string(index) + ". Failed to write to export file: " + std::string(strerror(errno))));
		if(write(fileDescriptor->descriptor, temp.c_str(), temp.size()) == -1)
		{
			_bl->out.printError("Error: Could not export GPIO with index " + std::to_string(index) + " and number " + temp + ": " + std::string(strerror(errno)));
		}
		_bl->fileDescriptorManager.close(fileDescriptor);
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
    _gpioMutex.unlock();
}

void Gpio::setup(int32_t userId, int32_t groupId)
{
	try
	{
		for(std::vector<uint32_t>::iterator i = _bl->settings.exportGpios().begin(); i != _bl->settings.exportGpios().end(); ++i)
		{
			exportGpio(*i);
			setPermission(*i, userId, groupId, false);
		}
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

PFileDescriptor Gpio::getFileDescriptor(uint32_t index)
{
	_gpioMutex.lock();
	try
	{
		std::map<uint32_t, GpioInfo>::iterator gpioIterator = _gpioInfo.find(index);
		if(gpioIterator != _gpioInfo.end() && gpioIterator->second.fileDescriptor && gpioIterator->second.fileDescriptor->descriptor != -1)
		{
			_gpioMutex.unlock();
			return gpioIterator->second.fileDescriptor;
		}
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
    _gpioMutex.unlock();
    return PFileDescriptor();
}

void Gpio::setDirection(uint32_t index, GpioDirection::Enum direction)
{
	try
	{
		getPath(index);
		_gpioMutex.lock();
		if(_gpioInfo[index].path.empty())
		{
			_gpioInfo.erase(index);
			throw(Exception("Failed to open direction file for GPIO with index " + std::to_string(index) + ": Unable to retrieve path."));
		}
		std::string path(_gpioInfo[index].path + "direction");
		std::shared_ptr<FileDescriptor> fileDescriptor = _bl->fileDescriptorManager.add(open(path.c_str(), O_WRONLY));
		if (fileDescriptor->descriptor == -1) throw(Exception("Could not write to direction file (" + path + ") of GPIO with index " + std::to_string(index) + ": " + std::string(strerror(errno))));
		std::string temp((direction == GpioDirection::OUT) ? "out" : "in");
		if(write(fileDescriptor->descriptor, temp.c_str(), temp.size()) <= 0)
		{
			_bl->out.printError("Could not write to direction file \"" + path + "\": " + std::string(strerror(errno)));
		}
		_bl->fileDescriptorManager.close(fileDescriptor);
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
    _gpioMutex.unlock();
}

void Gpio::setEdge(uint32_t index, GpioEdge::Enum edge)
{
	try
	{
		getPath(index);
		_gpioMutex.lock();
		if(_gpioInfo[index].path.empty())
		{
			_gpioInfo.erase(index);
			throw(Exception("Failed to open edge file for GPIO with index " + std::to_string(index) + ": Unable to retrieve path."));
		}
		std::string path(_gpioInfo[index].path + "edge");
		std::shared_ptr<FileDescriptor> fileDescriptor = _bl->fileDescriptorManager.add(open(path.c_str(), O_WRONLY));
		if (fileDescriptor->descriptor == -1) throw(Exception("Could not write to edge file (" + path + ") of GPIO with index " + std::to_string(index) + ": " + std::string(strerror(errno))));
		std::string temp((edge == GpioEdge::RISING) ? "rising" : ((edge == GpioEdge::FALLING) ? "falling" : "both"));
		if(write(fileDescriptor->descriptor, temp.c_str(), temp.size()) <= 0)
		{
			_bl->out.printError("Could not write to edge file \"" + path + "\": " + std::string(strerror(errno)));
		}
		_bl->fileDescriptorManager.close(fileDescriptor);
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
    _gpioMutex.unlock();
}

}
