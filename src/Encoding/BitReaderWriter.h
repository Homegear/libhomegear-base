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

#ifndef BITREADERWRITER_H_
#define BITREADERWRITER_H_

#include <vector>
#include <cstdint>

namespace BaseLib
{

class BitReaderWriter
{
public:
	virtual ~BitReaderWriter();

	/**
	 * Reads any number of bits at any position from a byte array. It is ok for position + size to exceed the array boundaries.
	 *
	 * @param data The byte array to read from. Index 0 must be the most significant byte.
	 * @param position The position in bits starting with bit 7 of index 0 (position 1 is bit 6 of index 0 and so on). Example: If data is 00101000 00001000, position is 2 and size is 3 then the result is 00000101.
	 * @param size The size in bits of the data to read.
	 * @return The data is returned right aligned.
	 */
	static std::vector<uint8_t> getPosition(const std::vector<uint8_t>& data, uint32_t position, uint32_t size);

	/**
	 * Reads any number of bits at any position from a byte array. It is ok for position + size to exceed the array boundaries.
	 *
	 * @param data The byte array to read from. Index 0 must be the most significant byte.
	 * @param position The position in bits starting with bit 7 of index 0 (position 1 is bit 6 of index 0 and so on). Example: If data is 00101000 00001000, position is 2 and size is 3 then the result is 00000101.
	 * @param size The size in bits of the data to read.
	 * @return The data is returned right aligned.
	 */
	static std::vector<uint8_t> getPosition(const std::vector<char>& data, uint32_t position, uint32_t size);

	/**
	 * Reads up to 8 bits at any position from a byte array and returns it as an uint8_t.
	 *
	 * @param data The byte array to read from. Index 0 must be the most significant byte.
	 * @param position The position in bits starting with bit 7 of index 0 (position 1 is bit 6 of index 0 and so on). Example: If data is 00101000, position is 2 and size is 3 then the result is 00000101.
	 * @param size The size in bits of the data to read.
	 * @return The data is returned right aligned.
	 */
	static uint8_t getPosition8(const std::vector<uint8_t>& data, uint32_t position, uint32_t size);

	/**
	 * Reads up to 16 bits at any position from a byte array and returns it as an uint16_t.
	 *
	 * @param data The byte array to read from. Index 0 must be the most significant byte.
	 * @param position The position in bits starting with bit 7 of index 0 (position 1 is bit 6 of index 0 and so on). Example: If data is 00101000 00001000, position is 2 and size is 3 then the result is 00000101.
	 * @param size The size in bits of the data to read.
	 * @return The data is returned right aligned.
	 */
	static uint16_t getPosition16(const std::vector<uint8_t>& data, uint32_t position, uint32_t size);

	/**
	 * Reads up to 32 bits at any position from a byte array and returns it as an uint32_t.
	 *
	 * @param data The byte array to read from. Index 0 must be the most significant byte.
	 * @param position The position in bits starting with bit 7 of index 0 (position 1 is bit 6 of index 0 and so on). Example: If data is 00101000 00001000, position is 2 and size is 3 then the result is 00000101.
	 * @param size The size in bits of the data to read.
	 * @return The data is returned right aligned.
	 */
	static uint32_t getPosition32(const std::vector<uint8_t>& data, uint32_t position, uint32_t size);

	/**
	 * Reads up to 64 bits at any position from a byte array and returns it as an uint64_t.
	 *
	 * @param data The byte array to read from. Index 0 must be the most significant byte.
	 * @param position The position in bits starting with bit 7 of index 0 (position 1 is bit 6 of index 0 and so on). Example: If data is 00101000 00001000, position is 2 and size is 3 then the result is 00000101.
	 * @param size The size in bits of the data to read.
	 * @return The data is returned right aligned.
	 */
	static uint64_t getPosition64(const std::vector<uint8_t>& data, uint32_t position, uint32_t size);

	/**
	 * Sets any number of bits at any position in a byte array.
	 *
	 * @param position The position in bits starting with bit 7 of index 0 (position 1 is bit 6 of index 0 and so on). Example: If source is 00000101, position is 2, size is 3 and target.size() is 2 then the result is 00101000 00000000.
	 * @param size The size in bits of the data to set.
	 * @param target The byte array to write to. Index 0 is the most significant byte.
	 * @param source The right aligned byte array of the data to set. size bits and bytes are read LSB first by default and set at position in target (counted from the most significant bit of the most significant byte).
	 */
	static void setPosition(uint32_t position, uint32_t size, std::vector<uint8_t>& target, const std::vector<uint8_t>& source);

	/**
	 * Sets any number of bits at any position in a byte array.
	 *
	 * @param position The position in bits starting with bit 7 of index 0 (position 1 is bit 6 of index 0 and so on). Example: If source is 00000101, position is 2, size is 3 and target.size() is 2 then the result is 00101000 00000000. Example 2: If source is 0xABCD, position is 0 and size is 16, the result is 0xCDAB.
	 * @param size The size in bits of the data to set.
	 * @param target The byte array to write to. Index 0 is the most significant byte.
	 * @param source The right aligned byte array of the data to set. size bits and bytes are read LSB first by default and set at position in target (counted from the most significant bit of the most significant byte).
	 */
	static void setPosition(uint32_t position, uint32_t size, std::vector<char>& target, const std::vector<uint8_t>& source);
private:
	static const uint8_t _bitMaskGet[8];
	static const uint8_t _bitMaskSetSource[9];
	static const uint8_t _bitMaskSetTargetStart[8];
	static const uint8_t _bitMaskSetTargetEnd[8];

	BitReaderWriter();
};

}
#endif
