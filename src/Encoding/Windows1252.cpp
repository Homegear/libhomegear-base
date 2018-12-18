/* Copyright 2013-2017 Sathya Laufer
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

#include "Windows1252.h"
#include <iostream>

namespace BaseLib
{
Windows1252::Windows1252(bool toUtf8, bool toWindows1252) : _toUtf8(toUtf8), _toWindows1252(toWindows1252)
{
    if(toUtf8)
    {
        _utf8Lookup = std::vector<std::vector<uint8_t>>{
                std::vector<uint8_t>{ 0x20, 0xAC },            // 0x80
                std::vector<uint8_t>{},                        // 0x81
                std::vector<uint8_t>{ 0x20, 0x1A },            // 0x82
                std::vector<uint8_t>{ 0x01, 0x92 },            // 0x83
                std::vector<uint8_t>{ 0x20, 0x1E },            // 0x84
                std::vector<uint8_t>{ 0x20, 0x26 },            // 0x85
                std::vector<uint8_t>{ 0x20, 0x20 },            // 0x86
                std::vector<uint8_t>{ 0x20, 0x21 },            // 0x87
                std::vector<uint8_t>{ 0x02, 0xC6 },            // 0x88
                std::vector<uint8_t>{ 0x20, 0x30 },            // 0x89
                std::vector<uint8_t>{ 0x01, 0x60 },            // 0x8A
                std::vector<uint8_t>{ 0x20, 0x39 },            // 0x8B
                std::vector<uint8_t>{ 0x01, 0x52 },            // 0x8C
                std::vector<uint8_t>{},                        // 0x8D
                std::vector<uint8_t>{ 0x01, 0x7D },            // 0x8E
                std::vector<uint8_t>{},                        // 0x8F
                std::vector<uint8_t>{},                        // 0x90
                std::vector<uint8_t>{ 0x20, 0x18 },            // 0x91
                std::vector<uint8_t>{ 0x20, 0x19 },            // 0x92
                std::vector<uint8_t>{ 0x20, 0x1C },            // 0x93
                std::vector<uint8_t>{ 0x20, 0x1D },            // 0x94
                std::vector<uint8_t>{ 0x20, 0x22 },            // 0x95
                std::vector<uint8_t>{ 0x20, 0x13 },            // 0x96
                std::vector<uint8_t>{ 0x20, 0x14 },            // 0x97
                std::vector<uint8_t>{ 0x02, 0xDC },            // 0x98
                std::vector<uint8_t>{ 0x21, 0x22 },            // 0x99
                std::vector<uint8_t>{ 0x01, 0x61 },            // 0x9A
                std::vector<uint8_t>{ 0x20, 0x3A },            // 0x9B
                std::vector<uint8_t>{ 0x01, 0x53 },            // 0x9C
                std::vector<uint8_t>{},                        // 0x9D
                std::vector<uint8_t>{ 0x01, 0x7E },            // 0x9E
                std::vector<uint8_t>{ 0x01, 0x89 }             // 0x9F
        };
        _utf8Lookup.reserve(128);

        for(uint32_t i = 0xA0; i <= 0xFF; i++)
        {
            _utf8Lookup.push_back(std::vector<uint8_t>{ 0, (uint8_t)i });
        }
    }

    if(toWindows1252)
    {
        _windows1252Lookup = {
                std::pair<uint32_t, uint8_t> { 0x20AC, 0x80 },
                std::pair<uint32_t, uint8_t> { 0x201A, 0x82 },
                std::pair<uint32_t, uint8_t> { 0x0192, 0x83 },
                std::pair<uint32_t, uint8_t> { 0x201E, 0x84 },
                std::pair<uint32_t, uint8_t> { 0x2026, 0x85 },
                std::pair<uint32_t, uint8_t> { 0x2020, 0x86 },
                std::pair<uint32_t, uint8_t> { 0x2021, 0x86 },
                std::pair<uint32_t, uint8_t> { 0x02C6, 0x88 },
                std::pair<uint32_t, uint8_t> { 0x2030, 0x89 },
                std::pair<uint32_t, uint8_t> { 0x0160, 0x8A },
                std::pair<uint32_t, uint8_t> { 0x2039, 0x8B },
                std::pair<uint32_t, uint8_t> { 0x0152, 0x8C },
                std::pair<uint32_t, uint8_t> { 0x017D, 0x8E },
                std::pair<uint32_t, uint8_t> { 0x2018, 0x91 },
                std::pair<uint32_t, uint8_t> { 0x2019, 0x92 },
                std::pair<uint32_t, uint8_t> { 0x201C, 0x93 },
                std::pair<uint32_t, uint8_t> { 0x201D, 0x94 },
                std::pair<uint32_t, uint8_t> { 0x2022, 0x95 },
                std::pair<uint32_t, uint8_t> { 0x2013, 0x96 },
                std::pair<uint32_t, uint8_t> { 0x2014, 0x97 },
                std::pair<uint32_t, uint8_t> { 0x02DC, 0x98 },
                std::pair<uint32_t, uint8_t> { 0x2122, 0x99 },
                std::pair<uint32_t, uint8_t> { 0x0161, 0x9A },
                std::pair<uint32_t, uint8_t> { 0x203A, 0x9B },
                std::pair<uint32_t, uint8_t> { 0x0153, 0x9C },
                std::pair<uint32_t, uint8_t> { 0x017E, 0x9E },
                std::pair<uint32_t, uint8_t> { 0x0189, 0x9F }
        };
    }
}

std::string Windows1252::toUtf8(const std::string& windows1252String)
{
    if(!_toUtf8 || windows1252String.empty()) return "";
    std::vector<char> buffer(windows1252String.size() * 2 + 1);
    uint32_t pos = 0;
    for(uint32_t i = 0; i < windows1252String.size(); ++i)
    {
        if(windows1252String[i] == 0) break;
        if((uint8_t)windows1252String[i] < 128)
        {
            buffer[pos] = windows1252String[i];
            pos++;
        }
        else
        {
            std::vector<uint8_t>& utf8Char = _utf8Lookup[(uint8_t)windows1252String[i] - 128];
            if(!utf8Char.empty()) memcpy(buffer.data() + pos, utf8Char.data(), utf8Char.size());
            pos += utf8Char.size();
        }
    }
    buffer[pos] = 0;
    return std::string(buffer.data(), pos);
}

std::string Windows1252::toUtf8(const char* ansiString, uint32_t length)
{
    if(!_toUtf8 || length == 0) return "";
    std::vector<char> buffer(length * 2 + 1);
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
            if(!utf8Char.empty()) memcpy(buffer.data() + pos, utf8Char.data(), utf8Char.size());
            pos += utf8Char.size();
        }
    }
    buffer[pos] = 0;
    return std::string(buffer.data(), pos);
}

std::string Windows1252::toWindows1252(const std::string& utf8String)
{
    if(!_toWindows1252 || utf8String.empty()) return "";
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
            auto lookupIterator = _windows1252Lookup.find(currentUtfCharacter);
            if(lookupIterator == _windows1252Lookup.end()) buffer[pos] = '?';
            else buffer[pos] = (char)lookupIterator->second;
            pos++;
        }
    }
    buffer[pos] = 0;
    return std::string(&buffer[0], pos);
}

std::string Windows1252::toWindows1252(const char* utf8String, uint32_t length)
{
    if(!_toWindows1252 || length == 0) return "";
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
            auto lookupIterator = _windows1252Lookup.find(currentUtfCharacter);
            if(lookupIterator == _windows1252Lookup.end()) buffer[pos] = '?';
            else buffer[pos] = (char)lookupIterator->second;
            pos++;
        }
    }
    buffer[pos] = 0;
    return std::string(&buffer[0], pos);
}
}
