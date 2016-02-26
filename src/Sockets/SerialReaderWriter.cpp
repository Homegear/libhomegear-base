/* Copyright 2013-2016 Sathya Laufer
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

#include "SerialReaderWriter.h"
#include "../BaseLib.h"

namespace BaseLib
{

SerialReaderWriter::SerialReaderWriter(BaseLib::Obj* baseLib, std::string device, int32_t baudrate, int32_t flags, bool createLockFile, int32_t readThreadPriority)
{
	_fileDescriptor = std::shared_ptr<FileDescriptor>(new FileDescriptor());
	_bl = baseLib;
	_device = device;
	_baudrate = baudrate;
	_flags = flags;
	if(_flags == 0) _flags = O_RDWR | O_NOCTTY | O_NDELAY;
	_createLockFile = createLockFile;
	_readThreadPriority = readThreadPriority;
}

SerialReaderWriter::~SerialReaderWriter()
{
	_handles = 0;
	closeDevice();
}

void SerialReaderWriter::openDevice(bool parity, bool oddParity, bool events)
{
	_handles++;
	if(_fileDescriptor->descriptor > -1) return;
	if(_createLockFile) createLockFile();
	_fileDescriptor = _bl->fileDescriptorManager.add(open(_device.c_str(), _flags));
	if(_fileDescriptor->descriptor == -1) throw SerialReaderWriterException("Couldn't open device \"" + _device + "\": " + strerror(errno));

	tcflag_t baudrate;
	switch(_baudrate)
	{
	case 50:
		baudrate = B50;
		break;
	case 75:
		baudrate = B75;
		break;
	case 110:
		baudrate = B110;
		break;
	case 134:
		baudrate = B134;
		break;
	case 150:
		baudrate = B150;
		break;
	case 200:
		baudrate = B200;
		break;
	case 300:
		baudrate = B300;
		break;
	case 600:
		baudrate = B600;
		break;
	case 1200:
		baudrate = B1200;
		break;
	case 1800:
		baudrate = B1800;
		break;
	case 2400:
		baudrate = B2400;
		break;
	case 4800:
		baudrate = B4800;
		break;
	case 9600:
		baudrate = B9600;
		break;
	case 19200:
		baudrate = B19200;
		break;
	case 38400:
		baudrate = B38400;
		break;
	case 57600:
		baudrate = B57600;
		break;
	case 115200:
		baudrate = B115200;
		break;
	case 230400:
		baudrate = B230400;
		break;
	case 460800:
		baudrate = B460800;
		break;
	case 500000:
		baudrate = B500000;
		break;
	case 576000:
		baudrate = B576000;
		break;
	case 921600:
		baudrate = B921600;
		break;
	case 1000000:
		baudrate = B1000000;
		break;
	case 1152000:
		baudrate = B1152000;
		break;
	case 1500000:
		baudrate = B1500000;
		break;
	case 2000000:
		baudrate = B2000000;
		break;
	case 2500000:
		baudrate = B2500000;
		break;
	case 3000000:
		baudrate = B3000000;
		break;
	case 3500000:
		baudrate = B3500000;
		break;
	case 4000000:
		baudrate = B4000000;
		break;
	default:
		throw SerialReaderWriterException("Couldn't setup device \"" + _device + "\": Unsupported baudrate.");
	}
	memset(&_termios, 0, sizeof(termios));
	_termios.c_cflag = baudrate | CS8 | CREAD;
	if(parity) _termios.c_cflag |= PARENB;
	if(oddParity) _termios.c_cflag |= PARENB | PARODD;
	_termios.c_iflag = 0;
	_termios.c_oflag = 0;
	_termios.c_lflag = 0;
	_termios.c_cc[VMIN] = 1;
	_termios.c_cc[VTIME] = 0;
	cfsetispeed(&_termios, baudrate);
	cfsetospeed(&_termios, baudrate);
	if(tcflush(_fileDescriptor->descriptor, TCIFLUSH) == -1) throw SerialReaderWriterException("Couldn't flush device " + _device);
	if(tcsetattr(_fileDescriptor->descriptor, TCSANOW, &_termios) == -1) throw SerialReaderWriterException("Couldn't set device settings for device " + _device);

	int flags = fcntl(_fileDescriptor->descriptor, F_GETFL);
	if(!(flags & O_NONBLOCK))
	{
		if(fcntl(_fileDescriptor->descriptor, F_SETFL, flags | O_NONBLOCK) == -1) throw SerialReaderWriterException("Couldn't set device to non blocking mode: " + _device);
	}

	if(events)
	{
		_readThreadMutex.lock();
		_bl->threadManager.join(_readThread);
		_stopped = false;
		_stopReadThread = false;
		if(_readThreadPriority > -1) _bl->threadManager.start(_readThread, true, _readThreadPriority, SCHED_FIFO, &SerialReaderWriter::readThread, this, parity, oddParity);
		else _bl->threadManager.start(_readThread, true, &SerialReaderWriter::readThread, this, parity, oddParity);
		_readThreadMutex.unlock();
	}
}

void SerialReaderWriter::closeDevice()
{
	_handles--;
	if(_handles > 0) return;
	_readThreadMutex.lock();
	_stopped = true;
	_stopReadThread = true;
	_bl->threadManager.join(_readThread);
	_readThreadMutex.unlock();
	_openDeviceThreadMutex.lock();
	_bl->threadManager.join(_openDeviceThread);
	_openDeviceThreadMutex.unlock();
	_bl->fileDescriptorManager.close(_fileDescriptor);
	unlink(_lockfile.c_str());
}

void SerialReaderWriter::createLockFile()
{
	_lockfile = "/var/lock" + _device.substr(_device.find_last_of('/')) + ".lock";
	std::shared_ptr<FileDescriptor> lockfileDescriptor = _bl->fileDescriptorManager.add(open(_lockfile.c_str(), O_WRONLY | O_EXCL | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
	if(lockfileDescriptor->descriptor == -1)
	{
		if(errno != EEXIST)
		{
			throw SerialReaderWriterException("Couldn't create lockfile " + _lockfile + ": " + strerror(errno));
		}

		int processID = 0;
		std::ifstream lockfileStream(_lockfile.c_str());
		lockfileStream >> processID;
		if(getpid() != processID && kill(processID, 0) == 0)
		{
			throw SerialReaderWriterException("COC device is in use: " + _device);
		}
		unlink(_lockfile.c_str());
		lockfileDescriptor = _bl->fileDescriptorManager.add(open(_lockfile.c_str(), O_WRONLY | O_EXCL | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
		if(lockfileDescriptor->descriptor == -1)
		{
			throw SerialReaderWriterException("Couldn't create lockfile " + _lockfile + ": " + strerror(errno));
			return;
		}
	}
	dprintf(lockfileDescriptor->descriptor, "%10i", getpid());
	_bl->fileDescriptorManager.close(lockfileDescriptor);
}

int32_t SerialReaderWriter::readChar(char& data, uint32_t timeout)
{
	int32_t i;
	fd_set readFileDescriptor;
	while(!_stopReadThread)
	{
		if(_fileDescriptor->descriptor == -1)
		{
			_bl->out.printError("Error: File descriptor is invalid.");
			return -1;
		}
		FD_ZERO(&readFileDescriptor);
		FD_SET(_fileDescriptor->descriptor, &readFileDescriptor);
		//Timeout needs to be set every time, so don't put it outside of the while loop
		timeval timeval;
		timeval.tv_sec = timeout / 1000000;
		timeval.tv_usec = timeout % 1000000;
		i = select(_fileDescriptor->descriptor + 1, &readFileDescriptor, NULL, NULL, &timeval);
		switch(i)
		{
			case 0: //Timeout
				return 1;
			case 1:
				break;
			default:
				//Error
				_bl->fileDescriptorManager.close(_fileDescriptor);
				return -1;
		}
		i = read(_fileDescriptor->descriptor, &data, 1);
		if(i == -1)
		{
			if(errno == EAGAIN) continue;
			_bl->fileDescriptorManager.close(_fileDescriptor);
			continue;
		}
		return 0;
	}
	return -1;
}

int32_t SerialReaderWriter::readLine(std::string& data, uint32_t timeout)
{
	data.clear();
	int32_t i;
	char localBuffer[1];
	fd_set readFileDescriptor;
	while(!_stopReadThread)
	{
		if(_fileDescriptor->descriptor == -1)
		{
			_bl->out.printError("Error: File descriptor is invalid.");
			return -1;
		}
		FD_ZERO(&readFileDescriptor);
		FD_SET(_fileDescriptor->descriptor, &readFileDescriptor);
		//Timeout needs to be set every time, so don't put it outside of the while loop
		timeval timeval;
		timeval.tv_sec = timeout / 1000000;
		timeval.tv_usec = timeout % 1000000;
		i = select(_fileDescriptor->descriptor + 1, &readFileDescriptor, NULL, NULL, &timeval);
		switch(i)
		{
			case 0: //Timeout
				return 1;
			case 1:
				break;
			default:
				//Error
				_bl->fileDescriptorManager.close(_fileDescriptor);
				return -1;
		}
		i = read(_fileDescriptor->descriptor, localBuffer, 1);
		if(i == -1)
		{
			if(errno == EAGAIN) continue;
			_bl->fileDescriptorManager.close(_fileDescriptor);
			continue;
		}
		data.push_back(localBuffer[0]);
		if(data.size() > 1024)
		{
			//Something is wrong
			_bl->fileDescriptorManager.close(_fileDescriptor);
		}
		if(localBuffer[0] == '\n') return 0;
	}
	return -1;
}

void SerialReaderWriter::writeLine(std::string& data)
{
    try
    {
        if(!_fileDescriptor || _fileDescriptor->descriptor == -1) throw SerialReaderWriterException("Couldn't write to device \"" + _device + "\", because the file descriptor is not valid.");
        if(data.empty() == 0) return;
        if(data.back() != '\n') data.push_back('\n');
        int32_t bytesWritten = 0;
        int32_t i;
        _sendMutex.lock();
        while(bytesWritten < (signed)data.length())
        {
        	if(_bl->debugLevel >= 5) _bl->out.printDebug("Debug: Writing: " + data);
            i = write(_fileDescriptor->descriptor, data.c_str() + bytesWritten, data.length() - bytesWritten);
            if(i == -1)
            {
                if(errno == EAGAIN) continue;
                throw SerialReaderWriterException("Error writing to device \"" + _device + "\" (3, " + std::to_string(errno) + ").");
            }
            bytesWritten += i;
        }
    }
    catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    _sendMutex.unlock();
}

void SerialReaderWriter::readThread(bool parity, bool oddParity)
{
	std::string data;
	while(!_stopReadThread)
	{
		if(_fileDescriptor->descriptor == -1)
		{
			closeDevice();
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			_openDeviceThreadMutex.lock();
			_bl->threadManager.join(_openDeviceThread);
			_bl->threadManager.start(_openDeviceThread, true, &SerialReaderWriter::openDevice, this, parity, oddParity, true);
			_openDeviceThreadMutex.unlock();
			return;
		}
		if(readLine(data) == 0)
		{
			EventHandlers eventHandlers = getEventHandlers();
			for(EventHandlers::const_iterator i = eventHandlers.begin(); i != eventHandlers.end(); ++i)
			{
				i->second->lock();
				if(i->second->handler()) ((ISerialReaderWriterEventSink*)i->second->handler())->lineReceived(data);
				i->second->unlock();
			}
		}
	}
}
}
