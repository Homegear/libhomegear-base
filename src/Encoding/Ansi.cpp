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

#include "Ansi.h"
#include <iostream>

namespace BaseLib
{
	Ansi::Ansi()
	{
		_lookup = std::vector<std::vector<uint8_t>>{
			std::vector<uint8_t>{ 0xE2, 0x82, 0xAC },      // 0x80
			std::vector<uint8_t>{},                        // 0x81
			std::vector<uint8_t>{ 0xE2, 0x80, 0x9A },      // 0x82
			std::vector<uint8_t>{ 0xC6, 0x92},             // 0x83
			std::vector<uint8_t>{ 0xE2, 0x80, 0x9E },      // 0x84
			std::vector<uint8_t>{ 0xE2, 0x80, 0xA6 },      // 0x85
			std::vector<uint8_t>{ 0xE2, 0x80, 0xA0 },      // 0x86
			std::vector<uint8_t>{ 0xE2, 0x80, 0xA1 },      // 0x87
			std::vector<uint8_t>{ 0xCB, 0x86 },            // 0x88
			std::vector<uint8_t>{ 0xE2, 0x80, 0xB0 },      // 0x89
			std::vector<uint8_t>{ 0xC5, 0xA0 },            // 0x8A
			std::vector<uint8_t>{ 0xE2, 0x80, 0xB9 },      // 0x8B
			std::vector<uint8_t>{ 0xC5, 0x92 },            // 0x8C
			std::vector<uint8_t>{},                        // 0x8D
			std::vector<uint8_t>{ 0xC5, 0xBD },            // 0x8E
			std::vector<uint8_t>{},                        // 0x8F
			std::vector<uint8_t>{},                        // 0x90
			std::vector<uint8_t>{ 0xE2, 0x80, 0x98 },      // 0x91
			std::vector<uint8_t>{ 0xE2, 0x80, 0x99 },      // 0x92
			std::vector<uint8_t>{ 0xE2, 0x80, 0x9C },      // 0x93
			std::vector<uint8_t>{ 0xE2, 0x80, 0x9D },      // 0x94
			std::vector<uint8_t>{ 0xE2, 0x80, 0xA2 },      // 0x95
			std::vector<uint8_t>{ 0xE2, 0x80, 0x93 },      // 0x96
			std::vector<uint8_t>{ 0xE2, 0x80, 0x94 },      // 0x97
			std::vector<uint8_t>{ 0xCB, 0x9C },            // 0x98
			std::vector<uint8_t>{ 0xE2, 0x84, 0xA2 },      // 0x99
			std::vector<uint8_t>{ 0xC5, 0xA1 },            // 0x9A
			std::vector<uint8_t>{ 0xE2, 0x80, 0xBA },      // 0x9B
			std::vector<uint8_t>{ 0xC5, 0x93 },            // 0x9C
			std::vector<uint8_t>{},                        // 0x9D
			std::vector<uint8_t>{ 0xC5, 0xBE },            // 0x9E
			std::vector<uint8_t>{ 0xC5, 0xB8 }             // 0x9F
		};
		_lookup.reserve(128);

		for(uint32_t i = 0xC2A0; i <= 0xC2BF; i++)
		{
			_lookup.push_back(std::vector<uint8_t>{ (uint8_t)(i >> 8), (uint8_t)(i & 0xFF) });
		}

		for(uint32_t i = 0xC380; i <= 0xC3BF; i++)
		{
			_lookup.push_back(std::vector<uint8_t>{ (uint8_t)(i >> 8), (uint8_t)(i & 0xFF) });
		}
	}

	std::string Ansi::toUtf8(const std::string& ansiString)
	{
		std::vector<char> buffer(ansiString.size() * 3 + 1);
		uint32_t pos = 0;
		for(uint32_t i = 0; i < ansiString.size(); ++i)
		{
			if((uint8_t)ansiString[i] < 128)
			{
				buffer[pos] = ansiString[i];
				pos++;
			}
			else
			{
				std::vector<uint8_t>& utf8Char = _lookup[(uint8_t)ansiString[i] - 128];
				if(!utf8Char.empty()) memcpy(&buffer[pos], &utf8Char[0], utf8Char.size());
				pos += utf8Char.size();
			}
		}
		buffer[pos] = 0;
		return std::string(&buffer[0], pos);
	}

	std::string Ansi::toUtf8(const char* ansiString, uint32_t length)
	{
		std::vector<char> buffer(length * 3 + 1);
		uint32_t pos = 0;
		for(uint32_t i = 0; i < length; ++i)
		{
			if((uint8_t)ansiString[i] < 128)
			{
				buffer[pos] = ansiString[i];
				pos++;
			}
			else
			{
				std::vector<uint8_t>& utf8Char = _lookup[(uint8_t)ansiString[i] - 128];
				if(!utf8Char.empty()) memcpy(&buffer[pos], &utf8Char[0], utf8Char.size());
				pos += utf8Char.size();
			}
		}
		buffer[pos] = 0;
		return std::string(&buffer[0], pos);
	}
}
