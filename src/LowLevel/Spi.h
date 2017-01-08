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

#ifndef SPI_H_
#define SPI_H_

#include "../Exception.h"

#include <type_traits>
#include <vector>
#include <mutex>
#include <memory>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

namespace BaseLib
{

class SharedObjects;
class FileDescriptor;

namespace LowLevel
{

/**
 * Exception class for SPI.
 *
 * @see Spi
 */
class SpiException : public Exception
{
public:
	SpiException(std::string message) : Exception(message) {}
};

enum class SpiModes : uint8_t
{
	none = 0,

	/**
	 * Enable SPI loopback.
	 */
	loop = SPI_LOOP,

	/**
	 * Data is valid on clock trailing edge (default on clock leading edge)
	 */
	cpha = SPI_CPHA,

	/**
	 * Clock is high when active (default low)
	 */
	cpol = SPI_CPOL,

	/**
	 * Least significant bit first (default most significant bit first)
	 */
	lsbFirst = SPI_LSB_FIRST,

	/**
	 * Enable line is active high (default low)
	 */
	csHigh = SPI_CS_HIGH,

	/**
	 * 3-Wire SPI (only one data line).
	 */
	threeWire = SPI_3WIRE,

	/**
	 * No enable line
	 */
	noCs = SPI_NO_CS,

	/**
	 * ?
	 */
	ready = SPI_READY
};

inline SpiModes operator|(SpiModes lhs, SpiModes rhs)
{
    return (SpiModes)(static_cast<std::underlying_type<SpiModes>::type>(lhs) | static_cast<std::underlying_type<SpiModes>::type>(rhs));
}

inline SpiModes& operator|=(SpiModes& lhs, SpiModes rhs)
{
    lhs = (SpiModes)(static_cast<std::underlying_type<SpiModes>::type>(lhs) | static_cast<std::underlying_type<SpiModes>::type>(rhs));
    return lhs;
}

inline SpiModes operator&(SpiModes lhs, SpiModes rhs)
{
    return (SpiModes)(static_cast<std::underlying_type<SpiModes>::type>(lhs) & static_cast<std::underlying_type<SpiModes>::type>(rhs));
}

inline SpiModes& operator&=(SpiModes& lhs, SpiModes rhs)
{
    lhs = (SpiModes)(static_cast<std::underlying_type<SpiModes>::type>(lhs) & static_cast<std::underlying_type<SpiModes>::type>(rhs));
    return lhs;
}

class Spi
{
public:
	/**
	 * Constructor.
	 *
	 * @param baseLib Pointer to SharedObjects
	 * @param device The SPI device to open (e. g. /dev/spidev0.0)
	 * @param mode The SPI mode
	 * @param bitsPerWord The number of bits per transfert (between 1 and 64, typically 8)
	 * @param speed The speed in Hz (e. g. 1000000 for 1 MHz).
	 */
	Spi(BaseLib::SharedObjects* baseLib, std::string device, SpiModes mode, uint8_t bitsPerWord, uint32_t speed);

	virtual ~Spi();

	void open();
	void close();
	void readwrite(std::vector<uint8_t>& data);

	bool isOpen();
protected:
	BaseLib::SharedObjects* _bl = nullptr;
	std::shared_ptr<FileDescriptor> _fileDescriptor;
	std::string _lockfile;
	std::mutex _sendMutex;
	struct spi_ioc_transfer _transfer;

	std::string _device;
	SpiModes _mode = SpiModes::none;
	uint8_t _bitsPerWord = 8;
	uint32_t _speed = 1000000;

	void setup();
};

}
}
#endif
