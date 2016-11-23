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
	Ansi::Ansi(bool ansiToUtf8, bool utf8ToAnsi) : _ansiToUtf8(ansiToUtf8), _utf8ToAnsi(utf8ToAnsi)
	{
		if(ansiToUtf8)
		{
			_utf8Lookup = std::vector<std::vector<uint8_t>>{
				std::vector<uint8_t>{ 0xE2, 0x82, 0xAC },      // 0x80
				std::vector<uint8_t>{},                        // 0x81
				std::vector<uint8_t>{ 0xE2, 0x80, 0x9A },      // 0x82
				std::vector<uint8_t>{ 0xC6, 0x92 },            // 0x83
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
			_utf8Lookup.reserve(128);

			for(uint32_t i = 0xC2A0; i <= 0xC2BF; i++)
			{
				_utf8Lookup.push_back(std::vector<uint8_t>{ (uint8_t)(i >> 8), (uint8_t)(i & 0xFF) });
			}

			for(uint32_t i = 0xC380; i <= 0xC3BF; i++)
			{
				_utf8Lookup.push_back(std::vector<uint8_t>{ (uint8_t)(i >> 8), (uint8_t)(i & 0xFF) });
			}
		}

		if(utf8ToAnsi)
		{
			_ansiLookup = {
				std::pair<uint32_t, uint8_t> { 0xE282AC, 0x80 },
				std::pair<uint32_t, uint8_t> { 0xE2809A, 0x82 },
				std::pair<uint32_t, uint8_t> { 0xC692,   0x83 },
				std::pair<uint32_t, uint8_t> { 0xE2809E, 0x84 },
				std::pair<uint32_t, uint8_t> { 0xE280A6, 0x85 },
				std::pair<uint32_t, uint8_t> { 0xE280A0, 0x86 },
				std::pair<uint32_t, uint8_t> { 0xE280A1, 0x86 },
				std::pair<uint32_t, uint8_t> { 0xCB86,   0x88 },
				std::pair<uint32_t, uint8_t> { 0xE280B0, 0x89 },
				std::pair<uint32_t, uint8_t> { 0xC5A0,   0x8A },
				std::pair<uint32_t, uint8_t> { 0xE280B9, 0x8B },
				std::pair<uint32_t, uint8_t> { 0xC592,   0x8C },
				std::pair<uint32_t, uint8_t> { 0xC5BD,   0x8E },
				std::pair<uint32_t, uint8_t> { 0xE28098, 0x91 },
				std::pair<uint32_t, uint8_t> { 0xE28099, 0x92 },
				std::pair<uint32_t, uint8_t> { 0xE2809C, 0x93 },
				std::pair<uint32_t, uint8_t> { 0xE2809D, 0x94 },
				std::pair<uint32_t, uint8_t> { 0xE280A2, 0x95 },
				std::pair<uint32_t, uint8_t> { 0xE28093, 0x96 },
				std::pair<uint32_t, uint8_t> { 0xE28094, 0x97 },
				std::pair<uint32_t, uint8_t> { 0xCB9C,   0x98 },
				std::pair<uint32_t, uint8_t> { 0xE284A2, 0x99 },
				std::pair<uint32_t, uint8_t> { 0xC5A1,   0x9A },
				std::pair<uint32_t, uint8_t> { 0xE280BA, 0x9B },
				std::pair<uint32_t, uint8_t> { 0xC593,   0x9C },
				std::pair<uint32_t, uint8_t> { 0xC5BE,   0x9E },
				std::pair<uint32_t, uint8_t> { 0xC5B8,   0x9F }
			};

			uint8_t pos = 0xA0;
			for(uint32_t i = 0xC2A0; i <= 0xC2BF; i++, pos++)
			{
				_ansiLookup[i] = pos;
			}

			for(uint32_t i = 0xC380; i <= 0xC3BF; i++, pos++)
			{
				_ansiLookup[i] = pos;
			}
		}
	}

	std::string Ansi::toUtf8(const std::string& ansiString)
	{
		if(!_ansiToUtf8 || ansiString.empty()) return "";
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
				std::vector<uint8_t>& utf8Char = _utf8Lookup[(uint8_t)ansiString[i] - 128];
				if(!utf8Char.empty()) memcpy(&buffer[pos], &utf8Char[0], utf8Char.size());
				pos += utf8Char.size();
			}
		}
		buffer[pos] = 0;
		return std::string(&buffer[0], pos);
	}

	std::string Ansi::toUtf8(const char* ansiString, uint32_t length)
	{
		if(!_ansiToUtf8 || length == 0) return "";
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
				std::vector<uint8_t>& utf8Char = _utf8Lookup[(uint8_t)ansiString[i] - 128];
				if(!utf8Char.empty()) memcpy(&buffer[pos], &utf8Char[0], utf8Char.size());
				pos += utf8Char.size();
			}
		}
		buffer[pos] = 0;
		return std::string(&buffer[0], pos);
	}

	std::string Ansi::toAnsi(const std::string& utf8String)
	{
		if(!_utf8ToAnsi || utf8String.empty()) return "";
		uint32_t currentUtfCharacter = 0;
		std::vector<char> buffer(utf8String.size() + 1);
		uint32_t characterSize = 0;
		uint32_t pos = 0;
		for(uint32_t i = 0; i < utf8String.size(); ++i)
		{
			uint8_t c = (uint8_t)utf8String.at(i);
			if(c == 0)
			{
				buffer[pos] = 0;
				if(pos == 0) return "";
				else return std::string(&buffer[0], pos);
			}
			else if(c < 128)
			{
				buffer[pos] = utf8String.at(i);
				pos++;
			}
			else
			{
				if ((c & 0xE0) == 0xC0) characterSize = 2;
				else if ((c & 0xF0) == 0xE0) characterSize = 3;
				else if ((c & 0xF8) == 0xF0) characterSize = 4;
				else return ""; //Invalid
				if(i + characterSize > utf8String.size())
				{
					buffer[pos] = 0;
					if(pos == 0) return "";
					else return std::string(&buffer[0], pos);
				}
				currentUtfCharacter = 0;
				for(int32_t j = characterSize - 1; j >= 0; j--)
				{
					currentUtfCharacter |= ((uint32_t)(uint8_t)utf8String.at(i + (characterSize - j - 1))) << (j * 8);
				}
				i += characterSize - 1;
				auto lookupIterator = _ansiLookup.find(currentUtfCharacter);
				if(lookupIterator == _ansiLookup.end()) buffer[pos] = '?';
				else buffer[pos] = (char)lookupIterator->second;
				pos++;
			}
		}
		buffer[pos] = 0;
		return std::string(&buffer[0], pos);
	}

	std::string Ansi::toAnsi(const char* utf8String, uint32_t length)
	{
		if(!_utf8ToAnsi || length == 0) return "";
		uint32_t currentUtfCharacter = 0;
		std::vector<char> buffer(length);
		uint32_t characterSize = 0;
		uint32_t pos = 0;
		for(uint32_t i = 0; i < length; ++i)
		{
			uint8_t c = (uint8_t)utf8String[i];
			if(c == 0)
			{
				buffer[pos] = 0;
				if(pos == 0) return "";
				else return std::string(&buffer[0], pos);
			}
			else if(c < 128)
			{
				buffer[pos] = utf8String[i];
				pos++;
			}
			else
			{
				if ((c & 0xE0) == 0xC0) characterSize = 2;
				else if ((c & 0xF0) == 0xE0) characterSize = 3;
				else if ((c & 0xF8) == 0xF0) characterSize = 4;
				else return ""; //Invalid
				if(i + characterSize > length)
				{
					buffer[pos] = 0;
					if(pos == 0) return "";
					else return std::string(&buffer[0], pos);
				}
				currentUtfCharacter = 0;
				for(int32_t j = characterSize - 1; j >= 0; j--)
				{
					currentUtfCharacter |= ((uint32_t)(uint8_t)utf8String[i + (characterSize - j - 1)]) << (j * 8);
				}
				i += characterSize - 1;
				std::map<uint32_t, uint8_t>::iterator lookupIterator = _ansiLookup.find(currentUtfCharacter);
				if(lookupIterator == _ansiLookup.end()) buffer[pos] = '?';
				else buffer[pos] = (char)lookupIterator->second;
				pos++;
			}
		}
		buffer[pos] = 0;
		return std::string(&buffer[0], pos);
	}
}
