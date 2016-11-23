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

#include "Spi.h"
#include "../BaseLib.h"

namespace BaseLib
{

Spi::Spi(BaseLib::SharedObjects* baseLib, std::string device, SpiModes mode, uint8_t bitsPerWord, uint32_t speed) : _device(device), _mode(mode), _bitsPerWord(bitsPerWord), _speed(speed)
{
	_bl = baseLib;
	_transfer =  { (uint64_t)0, (uint64_t)0, (uint32_t)0, (uint32_t)speed, (uint16_t)0, (uint8_t)bitsPerWord, (uint8_t)0, (uint32_t)0 };
}

Spi::~Spi()
{
	close();
}

void Spi::open()
{
	if(_fileDescriptor && _fileDescriptor->descriptor != -1) close();
	if(_device.empty()) throw SpiException("\"device\" is empty.");
	_lockfile = _bl->settings.lockFilePath() + "LCK.." + _device.substr(_device.find_last_of('/') + 1);
	int lockfileDescriptor = ::open(_lockfile.c_str(), O_WRONLY | O_EXCL | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if(lockfileDescriptor == -1)
	{
		if(errno != EEXIST) throw SpiException("Couldn't create lockfile " + _lockfile + ": " + strerror(errno));

		int processID = 0;
		std::ifstream lockfileStream(_lockfile.c_str());
		lockfileStream >> processID;
		if(getpid() != processID && kill(processID, 0) == 0) throw SpiException("Rf device is in use: " + _device);
		unlink(_lockfile.c_str());
		lockfileDescriptor = ::open(_lockfile.c_str(), O_WRONLY | O_EXCL | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if(lockfileDescriptor == -1) throw SpiException("Couldn't create lockfile " + _lockfile + ": " + strerror(errno));
	}
	dprintf(lockfileDescriptor, "%10i", getpid());
	::close(lockfileDescriptor);

	_fileDescriptor = _bl->fileDescriptorManager.add(::open(_device.c_str(), O_RDWR | O_NONBLOCK));
	usleep(100);

	if(_fileDescriptor->descriptor == -1) throw SpiException("Couldn't open rf device \"" + _device + "\": " + strerror(errno));

	setup();
}

void Spi::setup()
{
	if(_fileDescriptor->descriptor == -1) return;

	if(ioctl(_fileDescriptor->descriptor, SPI_IOC_WR_MODE, (uint8_t*)(&_mode))) throw SpiException("Couldn't set spi mode on device " + _device);
	if(ioctl(_fileDescriptor->descriptor, SPI_IOC_RD_MODE, (uint8_t*)(&_mode))) throw SpiException("Couldn't get spi mode off device " + _device);

	if(ioctl(_fileDescriptor->descriptor, SPI_IOC_WR_BITS_PER_WORD, &_bitsPerWord)) throw SpiException("Couldn't set bits per word on device " + _device);
	if(ioctl(_fileDescriptor->descriptor, SPI_IOC_RD_BITS_PER_WORD, &_bitsPerWord)) throw SpiException("Couldn't get bits per word off device " + _device);

	if((bool)(_mode & SpiModes::LsbFirst))
	{
		uint8_t lsb = 1;
		if(ioctl(_fileDescriptor->descriptor, SPI_IOC_WR_LSB_FIRST, &lsb)) throw SpiException("Couldn't set bits per word on device " + _device);
		if(ioctl(_fileDescriptor->descriptor, SPI_IOC_RD_LSB_FIRST, &lsb)) throw SpiException("Couldn't get bits per word off device " + _device);
	}

	if(ioctl(_fileDescriptor->descriptor, SPI_IOC_WR_MAX_SPEED_HZ, &_speed)) throw SpiException("Couldn't set speed on device " + _device);
	if(ioctl(_fileDescriptor->descriptor, SPI_IOC_RD_MAX_SPEED_HZ, &_speed)) throw SpiException("Couldn't get speed off device " + _device);
}

void Spi::close()
{
	_bl->fileDescriptorManager.close(_fileDescriptor);
	unlink(_lockfile.c_str());
}

void Spi::readwrite(std::vector<uint8_t>& data)
{
	std::lock_guard<std::mutex> sendGuard(_sendMutex);
	_transfer.tx_buf = (uint64_t)&data[0];
	_transfer.rx_buf = (uint64_t)&data[0];
	_transfer.len = (uint32_t)data.size();
	if(!ioctl(_fileDescriptor->descriptor, SPI_IOC_MESSAGE(1), &_transfer)) throw SpiException("Couldn't write to device " + _device + ": " + std::string(strerror(errno)));
}

}
