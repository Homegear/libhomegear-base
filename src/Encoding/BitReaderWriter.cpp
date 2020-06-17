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

#include "BitReaderWriter.h"
#include <iostream>
#include "../HelperFunctions/HelperFunctions.h"

namespace BaseLib
{

const uint8_t BitReaderWriter::_bitMaskGet[8] = { 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };
const uint8_t BitReaderWriter::_bitMaskSetSource[9] = { 0xFF, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };
const uint8_t BitReaderWriter::_bitMaskSetTargetStart[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
const uint8_t BitReaderWriter::_bitMaskSetTargetEnd[8] = { 0x00, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

std::vector<uint8_t> BitReaderWriter::getPosition(const std::vector<uint8_t>& data, uint32_t position, uint32_t size)
{
	std::vector<uint8_t> result;
	if(size == 0) return result;

	uint32_t bytePosition = position / 8;
	uint32_t bitPosition = position & 7; // & 7 == % 8
	uint32_t relativeEndPosition = bitPosition + size;
	uint32_t sourceByteSize = relativeEndPosition / 8 + ((relativeEndPosition & 7) != 0 ? 1 : 0);
	uint32_t targetByteSize = size / 8 + ((size & 7) != 0 ? 1 : 0);
	result.resize(targetByteSize, 0);

	if(bytePosition >= data.size()) return result;

	uint8_t firstByte = data.at(bytePosition) & _bitMaskGet[bitPosition];
	if(sourceByteSize == 1)
	{
		result.at(0) = firstByte >> (8 - relativeEndPosition);
		return result;
	}

	uint32_t targetBytePosition = 0;
	uint32_t endSourceByte = bytePosition + sourceByteSize - 1;

	int32_t rightShiftCount = 8 - (size & 7) - bitPosition;
	bool leftShiftFirst = ((size & 7) == 0 || (rightShiftCount & 0x80000000));
	int32_t leftShiftCount = 0;
	if(rightShiftCount & 0x80000000) // < 0
	{
		leftShiftCount = rightShiftCount * -1;
		rightShiftCount = 8 - leftShiftCount;
	}
	else leftShiftCount = 8 - rightShiftCount;
	if(rightShiftCount == 8) rightShiftCount = 0;
	if(leftShiftFirst)
	{
		result.at(targetBytePosition) = firstByte << leftShiftCount;
		if(leftShiftCount == 0) targetBytePosition++;
	}
	else
	{
		result.at(targetBytePosition) = firstByte >> rightShiftCount;
		targetBytePosition++;
		result.at(targetBytePosition) = firstByte << leftShiftCount;
	}

	for(uint32_t i = bytePosition + 1; i < endSourceByte; i++)
	{
		if(i >= data.size()) return result;
		result.at(targetBytePosition) |= data.at(i) >> rightShiftCount;
		targetBytePosition++;
		if(leftShiftCount > 0) result.at(targetBytePosition) = data.at(i) << leftShiftCount;
	}

	if(endSourceByte >= data.size()) return result;
	result.at(targetBytePosition) |= data.at(endSourceByte) >> rightShiftCount;
	return result;
}

std::vector<uint8_t> BitReaderWriter::getPosition(const std::vector<char>& data, uint32_t position, uint32_t size)
{
	std::vector<uint8_t> result;
	if(size == 0) return result;

	uint32_t bytePosition = position / 8;
	uint32_t bitPosition = position & 7; // & 7 == % 8
	uint32_t relativeEndPosition = bitPosition + size;
	uint32_t sourceByteSize = relativeEndPosition / 8 + ((relativeEndPosition & 7) != 0 ? 1 : 0);
	uint32_t targetByteSize = size / 8 + ((size & 7) != 0 ? 1 : 0);
	result.resize(targetByteSize, 0);

	if(bytePosition >= data.size()) return result;

	uint8_t firstByte = (uint8_t)data.at(bytePosition) & _bitMaskGet[bitPosition];
	if(sourceByteSize == 1)
	{
		result.at(0) = firstByte >> (8 - relativeEndPosition);
		return result;
	}

	uint32_t targetBytePosition = 0;
	uint32_t endSourceByte = bytePosition + sourceByteSize - 1;

	int32_t rightShiftCount = 8 - (size & 7) - bitPosition;
	bool leftShiftFirst = ((size & 7) == 0 || rightShiftCount < 0);
	int32_t leftShiftCount = 0;
	if(rightShiftCount & 0x80000000) // < 0
	{
		leftShiftCount = rightShiftCount * -1;
		rightShiftCount = 8 - leftShiftCount;
	}
	else leftShiftCount = 8 - rightShiftCount;
	if(rightShiftCount == 8) rightShiftCount = 0;
	if(leftShiftFirst)
	{
		result.at(targetBytePosition) = firstByte << leftShiftCount;
		if(leftShiftCount == 0) targetBytePosition++;
	}
	else
	{
		result.at(targetBytePosition) = firstByte >> rightShiftCount;
		targetBytePosition++;
		result.at(targetBytePosition) = firstByte << leftShiftCount;
	}

	for(uint32_t i = bytePosition + 1; i < endSourceByte; i++)
	{
		if(i >= data.size()) return result;
		result.at(targetBytePosition) |= (uint8_t)data.at(i) >> rightShiftCount;
		targetBytePosition++;
		if(leftShiftCount > 0) result.at(targetBytePosition) = (uint8_t)data.at(i) << leftShiftCount;
	}

	if(endSourceByte >= data.size()) return result;
	result.at(targetBytePosition) |= (uint8_t)data.at(endSourceByte) >> rightShiftCount;
	return result;
}

uint8_t BitReaderWriter::getPosition8(const std::vector<uint8_t>& data, uint32_t position, uint32_t size)
{
	if(size > 8) size = 8;
	else if(size == 0) return 0;
	uint8_t result = 0;

	uint32_t bytePosition = position / 8;
	int32_t bitPosition = position % 8;
	int32_t sourceByteSize = (bitPosition + size) / 8 + ((bitPosition + size) % 8 != 0 ? 1 : 0);

	if(bytePosition >= data.size()) return 0;

	uint8_t firstByte = data.at(bytePosition) & _bitMaskGet[bitPosition];
	if(sourceByteSize == 1)
	{
		result = firstByte >> ((8 - ((bitPosition + size) % 8)) % 8);
		return result;
	}

	result |= (uint16_t)firstByte << (size - (8 - bitPosition));

	if(bytePosition + 1 >= data.size()) return result;
	result |= data.at(bytePosition + 1) >> ((8 - ((bitPosition + size) % 8)) % 8);
	return result;
}

uint16_t BitReaderWriter::getPosition16(const std::vector<uint8_t>& data, uint32_t position, uint32_t size)
{
	if(size > 16) size = 16;
	else if(size == 0) return 0;
	uint16_t result = 0;

	uint32_t bytePosition = position / 8;
	int32_t bitPosition = position % 8;
	int32_t sourceByteSize = (bitPosition + size) / 8 + ((bitPosition + size) % 8 != 0 ? 1 : 0);

	if(bytePosition >= data.size()) return 0;

	uint8_t firstByte = data.at(bytePosition) & _bitMaskGet[bitPosition];
	if(sourceByteSize == 1)
	{
		result = firstByte >> ((8 - ((bitPosition + size) % 8)) % 8);
		return result;
	}

	int32_t bitsLeft = size - (8 - bitPosition);
	result |= (uint16_t)firstByte << bitsLeft;
	bitsLeft -= 8;

	for(uint32_t i = bytePosition + 1; i < bytePosition + sourceByteSize - 1; i++)
	{
		if(i >= data.size()) return result;
		result |= (uint16_t)data.at(i) << bitsLeft;
		bitsLeft -= 8;
	}

	if(bytePosition + sourceByteSize - 1 >= data.size()) return result;
	result |= data.at(bytePosition + sourceByteSize - 1) >> ((8 - ((bitPosition + size) % 8)) % 8);

	return result;
}

uint32_t BitReaderWriter::getPosition32(const std::vector<uint8_t>& data, uint32_t position, uint32_t size)
{
	if(size > 32) size = 32;
	else if(size == 0) return 0;
	uint32_t result = 0;

	uint32_t bytePosition = position / 8;
	int32_t bitPosition = position % 8;
	int32_t sourceByteSize = (bitPosition + size) / 8 + ((bitPosition + size) % 8 != 0 ? 1 : 0);

	if(bytePosition >= data.size()) return 0;

	uint8_t firstByte = data.at(bytePosition) & _bitMaskGet[bitPosition];
	if(sourceByteSize == 1)
	{
		result = firstByte >> ((8 - ((bitPosition + size) % 8)) % 8);
		return result;
	}

	int32_t bitsLeft = size - (8 - bitPosition);
	result |= (uint32_t)firstByte << bitsLeft;
	bitsLeft -= 8;

	for(uint32_t i = bytePosition + 1; i < bytePosition + sourceByteSize - 1; i++)
	{
		if(i >= data.size()) return result;
		result |= (uint32_t)data.at(i) << bitsLeft;
		bitsLeft -= 8;
	}

	if(bytePosition + sourceByteSize - 1 >= data.size()) return result;
	result |= data.at(bytePosition + sourceByteSize - 1) >> ((8 - ((bitPosition + size) % 8)) % 8);
	return result;
}

uint64_t BitReaderWriter::getPosition64(const std::vector<uint8_t>& data, uint32_t position, uint32_t size)
{
	if(size > 64) size = 64;
	else if(size == 0) return 0;
	uint64_t result = 0;

	uint32_t bytePosition = position / 8;
	int32_t bitPosition = position % 8;
	int32_t sourceByteSize = (bitPosition + size) / 8 + ((bitPosition + size) % 8 != 0 ? 1 : 0);

	if(bytePosition >= data.size()) return 0;

	uint8_t firstByte = data.at(bytePosition) & _bitMaskGet[bitPosition];
	if(sourceByteSize == 1)
	{
		result = firstByte >> ((8 - ((bitPosition + size) % 8)) % 8);
		return result;
	}

	int32_t bitsLeft = size - (8 - bitPosition);
	result |= (uint64_t)firstByte << bitsLeft;
	bitsLeft -= 8;

	for(uint32_t i = bytePosition + 1; i < bytePosition + sourceByteSize - 1; i++)
	{
		if(i >= data.size()) return result;
		result |= (uint64_t)data.at(i) << bitsLeft;
		bitsLeft -= 8;
	}

	if(bytePosition + sourceByteSize - 1 >= data.size()) return result;
	result |= data.at(bytePosition + sourceByteSize - 1) >> ((8 - ((bitPosition + size) % 8)) % 8);
	return result;
}

void BitReaderWriter::setPositionLE(uint32_t position, uint32_t size, std::vector<uint8_t>& target, const std::vector<uint8_t>& source)
{
	if(size == 0) return;

	uint32_t bytePosition = position / 8;
	uint32_t bitPosition = position % 8;
	uint32_t relativeEndPosition = bitPosition + size;
	uint32_t targetBytePosition = bytePosition;
	uint32_t targetByteCount = relativeEndPosition / 8 + ((relativeEndPosition & 7) != 0 ? 1 : 0);
	uint32_t endIndex = bytePosition + (targetByteCount - 1);
	uint32_t sourceByteCount = size / 8 + ((size & 7) != 0 ? 1 : 0);
	uint32_t requiredSize = bytePosition + targetByteCount;
	if(target.size() < requiredSize) target.resize(requiredSize, 0);

	if(endIndex == bytePosition) target.at(bytePosition) &= (_bitMaskSetTargetStart[bitPosition] | _bitMaskSetTargetEnd[relativeEndPosition & 7]);
	else
	{
		target.at(bytePosition) &= _bitMaskSetTargetStart[bitPosition];
		for(uint32_t i = bytePosition + 1; i < endIndex; i++)
		{
			target.at(i) = 0;
		}
		target.at(endIndex) &= _bitMaskSetTargetEnd[relativeEndPosition & 7];
	}

	uint8_t firstByte = sourceByteCount > source.size() ? 0 : source.at(sourceByteCount - 1) & _bitMaskSetSource[size & 7];

	int32_t leftShiftCount = 8 - (size & 7) - bitPosition;
	bool rightShiftFirst =  (size & 7) == 0 || (leftShiftCount & 0x80000000);
	int32_t rightShiftCount = 0;
	if(leftShiftCount & 0x80000000) // < 0
	{
		rightShiftCount = leftShiftCount * -1;
		leftShiftCount = 8 - rightShiftCount;
	}
	else rightShiftCount = 8 - leftShiftCount;
	if(leftShiftCount == 8) leftShiftCount = 0;

	if(rightShiftFirst)
	{
		target.at(targetBytePosition) |= firstByte >> rightShiftCount;
		targetBytePosition++;
		if(rightShiftCount > 0) target.at(targetBytePosition) |= firstByte << leftShiftCount;
	}
	else target.at(targetBytePosition) |= firstByte << leftShiftCount;

	for(int32_t i = sourceByteCount - 2; i >= 0; i--)
	{
		if((unsigned)i < source.size())
		{
			target.at(targetBytePosition) |= source.at(i) >> rightShiftCount;
			targetBytePosition++;
			if(rightShiftCount > 0) target.at(targetBytePosition) |= source.at(i) << leftShiftCount;
		}
		else targetBytePosition++;
	}
}

void BitReaderWriter::setPositionLE(uint32_t position, uint32_t size, std::vector<char>& target, const std::vector<uint8_t>& source)
{
	if(size == 0) return;

	uint32_t bytePosition = position / 8;
	uint32_t bitPosition = position % 8;
	uint32_t relativeEndPosition = bitPosition + size;
	uint32_t targetBytePosition = bytePosition;
	uint32_t targetByteCount = relativeEndPosition / 8 + ((relativeEndPosition & 7) != 0 ? 1 : 0);
	uint32_t endIndex = bytePosition + (targetByteCount - 1);
	uint32_t sourceByteCount = size / 8 + ((size & 7) != 0 ? 1 : 0);
	uint32_t requiredSize = bytePosition + targetByteCount;
	if(target.size() < requiredSize) target.resize(requiredSize, 0);

	if(endIndex == bytePosition) target.at(bytePosition) &= (char)(_bitMaskSetTargetStart[bitPosition] | _bitMaskSetTargetEnd[relativeEndPosition & 7]);
	else
	{
		target.at(bytePosition) &= (char)_bitMaskSetTargetStart[bitPosition];
		for(uint32_t i = bytePosition + 1; i < endIndex; i++)
		{
			target.at(i) = 0;
		}
		target.at(endIndex) &= (char)_bitMaskSetTargetEnd[relativeEndPosition & 7];
	}

	uint8_t firstByte = sourceByteCount > source.size() ? 0 : source.at(sourceByteCount - 1) & _bitMaskSetSource[size & 7];

	int32_t leftShiftCount = 8 - (size & 7) - bitPosition;
	bool rightShiftFirst =  (size & 7) == 0 || (leftShiftCount & 0x80000000);
	int32_t rightShiftCount = 0;
	if(leftShiftCount & 0x80000000) // < 0
	{
		rightShiftCount = leftShiftCount * -1;
		leftShiftCount = 8 - rightShiftCount;
	}
	else rightShiftCount = 8 - leftShiftCount;
	if(leftShiftCount == 8) leftShiftCount = 0;

	if(rightShiftFirst)
	{
		target.at(targetBytePosition) |= (char)(firstByte >> rightShiftCount);
		targetBytePosition++;
		if(rightShiftCount > 0) target.at(targetBytePosition) |= (char)(firstByte << leftShiftCount);
	}
	else target.at(targetBytePosition) |= (char)(firstByte << leftShiftCount);

	for(int32_t i = sourceByteCount - 2; i >= 0; i--)
	{
		if((unsigned)i < source.size())
		{
			target.at(targetBytePosition) |= (char)(source.at(i) >> rightShiftCount);
			targetBytePosition++;
			if(rightShiftCount > 0) target.at(targetBytePosition) |= (char)(source.at(i) << leftShiftCount);
		}
		else targetBytePosition++;
	}
}

void BitReaderWriter::setPositionBE(uint32_t position, uint32_t size, std::vector<uint8_t>& target, const std::vector<uint8_t>& source)
{
    if(size == 0) return;

    uint32_t bytePosition = position / 8;
    uint32_t bitPosition = position % 8;
    uint32_t relativeEndPosition = bitPosition + size;
    uint32_t targetBytePosition = bytePosition;
    uint32_t targetByteCount = relativeEndPosition / 8 + ((relativeEndPosition & 7) != 0 ? 1 : 0);
    uint32_t endIndex = bytePosition + (targetByteCount - 1);
    uint32_t sourceByteCount = size / 8 + ((size & 7) != 0 ? 1 : 0);
    int32_t sourceByteOffset = source.size() - sourceByteCount;
    uint32_t requiredSize = bytePosition + targetByteCount;
    if(target.size() < requiredSize) target.resize(requiredSize, 0);

    if(endIndex == bytePosition) target.at(bytePosition) &= (_bitMaskSetTargetStart[bitPosition] | _bitMaskSetTargetEnd[relativeEndPosition & 7]);
    else
    {
        target.at(bytePosition) &= _bitMaskSetTargetStart[bitPosition];
        for(uint32_t i = bytePosition + 1; i < endIndex; i++)
        {
            target.at(i) = 0;
        }
        target.at(endIndex) &= _bitMaskSetTargetEnd[relativeEndPosition & 7];
    }

    uint8_t firstByte = (sourceByteOffset < 0 || source.size() <= (unsigned)sourceByteOffset) ? 0 : source.at(sourceByteOffset) & _bitMaskSetSource[size & 7];

    int32_t leftShiftCount = 8 - (size & 7) - bitPosition;
    bool rightShiftFirst =  (size & 7) == 0 || (leftShiftCount & 0x80000000);
    int32_t rightShiftCount = 0;
    if(leftShiftCount & 0x80000000) // < 0
    {
        rightShiftCount = leftShiftCount * -1;
        leftShiftCount = 8 - rightShiftCount;
    }
    else rightShiftCount = 8 - leftShiftCount;
    if(leftShiftCount == 8) leftShiftCount = 0;

    if(rightShiftFirst)
    {
        target.at(targetBytePosition) |= firstByte >> rightShiftCount;
        targetBytePosition++;
        if(rightShiftCount > 0) target.at(targetBytePosition) |= firstByte << leftShiftCount;
    }
    else target.at(targetBytePosition) |= firstByte << leftShiftCount;

    for(int32_t i = 1; (unsigned)i < sourceByteCount; i++)
    {
        if(i + sourceByteOffset < 0)
        {
            targetBytePosition++;
            continue;
        }
        else if((unsigned)(i + sourceByteOffset) < source.size())
        {
            target.at(targetBytePosition) |= source.at(i + sourceByteOffset) >> rightShiftCount;
            targetBytePosition++;
            if(rightShiftCount > 0) target.at(targetBytePosition) |= source.at(i + sourceByteOffset) << leftShiftCount;
        }
        else break;
    }
}

void BitReaderWriter::setPositionBE(uint32_t position, uint32_t size, std::vector<char>& target, const std::vector<uint8_t>& source)
{
    if(size == 0) return;

    uint32_t bytePosition = position / 8;
    uint32_t bitPosition = position % 8;
    uint32_t relativeEndPosition = bitPosition + size;
    uint32_t targetBytePosition = bytePosition;
    uint32_t targetByteCount = relativeEndPosition / 8 + ((relativeEndPosition & 7) != 0 ? 1 : 0);
    uint32_t endIndex = bytePosition + (targetByteCount - 1);
    uint32_t sourceByteCount = size / 8 + ((size & 7) != 0 ? 1 : 0);
    int32_t sourceByteOffset = source.size() - sourceByteCount;
    uint32_t requiredSize = bytePosition + targetByteCount;
    if(target.size() < requiredSize) target.resize(requiredSize, 0);

    if(endIndex == bytePosition) target.at(bytePosition) &= (char)(_bitMaskSetTargetStart[bitPosition] | _bitMaskSetTargetEnd[relativeEndPosition & 7]);
    else
    {
        target.at(bytePosition) &= (char)_bitMaskSetTargetStart[bitPosition];
        for(uint32_t i = bytePosition + 1; i < endIndex; i++)
        {
            target.at(i) = 0;
        }
        target.at(endIndex) &= (char)_bitMaskSetTargetEnd[relativeEndPosition & 7];
    }

    uint8_t firstByte = (sourceByteOffset < 0 || source.size() <= (unsigned)sourceByteOffset) ? 0 : source.at(sourceByteOffset) & _bitMaskSetSource[size & 7];

    int32_t leftShiftCount = 8 - (size & 7) - bitPosition;
    bool rightShiftFirst =  (size & 7) == 0 || (leftShiftCount & 0x80000000);
    int32_t rightShiftCount = 0;
    if(leftShiftCount & 0x80000000) // < 0
    {
        rightShiftCount = leftShiftCount * -1;
        leftShiftCount = 8 - rightShiftCount;
    }
    else rightShiftCount = 8 - leftShiftCount;
    if(leftShiftCount == 8) leftShiftCount = 0;

    if(rightShiftFirst)
    {
        target.at(targetBytePosition) |= (char)(firstByte >> rightShiftCount);
        targetBytePosition++;
        if(rightShiftCount > 0) target.at(targetBytePosition) |= (char)(firstByte << leftShiftCount);
    }
    else target.at(targetBytePosition) |= (char)(firstByte << leftShiftCount);

    for(int32_t i = 1; (unsigned)i < sourceByteCount; i++)
    {
        if(i + sourceByteOffset < 0)
        {
            targetBytePosition++;
            continue;
        }
        else if((unsigned)(i + sourceByteOffset) < source.size())
        {
            target.at(targetBytePosition) |= (char)(source.at(i + sourceByteOffset) >> rightShiftCount);
            targetBytePosition++;
            if(rightShiftCount > 0) target.at(targetBytePosition) |= (char)(source.at(i + sourceByteOffset) << leftShiftCount);
        }
        else break;
    }
}

}
