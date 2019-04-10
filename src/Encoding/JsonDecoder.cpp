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

#include "JsonDecoder.h"
#include "../HelperFunctions/Math.h"
#include "../BaseLib.h"

namespace BaseLib
{
namespace Rpc
{

std::shared_ptr<Variable> JsonDecoder::decode(const std::string& json)
{
    uint32_t pos = 0;
    std::shared_ptr<Variable> variable(new Variable());
    skipWhitespace(json, pos);
    if(!posValid(json, pos)) return variable;

    while(pos < json.length())
    {
        switch(json[pos])
        {
            case '{':
                decodeObject(json, pos, variable);
                return variable;
            case '[':
                decodeArray(json, pos, variable);
                return variable;
            default:
                throw JsonDecoderException("JSON does not start with '{' or '['.");
        }
    }

    return variable;
}

std::shared_ptr<Variable> JsonDecoder::decode(const std::string& json, uint32_t& bytesRead)
{
    bytesRead = 0;
    std::shared_ptr<Variable> variable(new Variable());
    skipWhitespace(json, bytesRead);
    if(!posValid(json, bytesRead)) return variable;

    while(bytesRead < json.length())
    {
        switch(json[bytesRead])
        {
            case '{':
                decodeObject(json, bytesRead, variable);
                return variable;
            case '[':
                decodeArray(json, bytesRead, variable);
                return variable;
            default:
                throw JsonDecoderException("JSON does not start with '{' or '['.");
        }
    }

    return variable;
}

std::shared_ptr<Variable> JsonDecoder::decode(const std::vector<char>& json)
{
    uint32_t pos = 0;
    std::shared_ptr<Variable> variable(new Variable());
    skipWhitespace(json, pos);
    if(!posValid(json, pos)) return variable;

    while(pos < json.size())
    {
        switch(json[pos])
        {
            case '{':
                decodeObject(json, pos, variable);
                return variable;
            case '[':
                decodeArray(json, pos, variable);
                return variable;
            default:
                throw JsonDecoderException("JSON does not start with '{' or '['.");
        }
    }

    return variable;
}

std::shared_ptr<Variable> JsonDecoder::decode(const std::vector<char>& json, uint32_t& bytesRead)
{
    bytesRead = 0;
    std::shared_ptr<Variable> variable(new Variable());
    skipWhitespace(json, bytesRead);
    if(!posValid(json, bytesRead)) return variable;

    while(bytesRead < json.size())
    {
        switch(json[bytesRead])
        {
            case '{':
                decodeObject(json, bytesRead, variable);
                return variable;
            case '[':
                decodeArray(json, bytesRead, variable);
                return variable;
            default:
                throw JsonDecoderException("JSON does not start with '{' or '['.");
        }
    }

    return variable;
}

bool JsonDecoder::posValid(const std::string& json, uint32_t pos)
{
    return pos < json.length();
}

bool JsonDecoder::posValid(const std::vector<char>& json, uint32_t pos)
{
    return pos < json.size();
}

void JsonDecoder::skipWhitespace(const std::string& json, uint32_t& pos)
{
    while(pos < json.length() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\r' || json[pos] == '\t'))
    {
        pos++;
    }
}

void JsonDecoder::skipWhitespace(const std::vector<char>& json, uint32_t& pos)
{
    while(pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\r' || json[pos] == '\t'))
    {
        pos++;
    }
}

void JsonDecoder::decodeObject(const std::string& json, uint32_t& pos, std::shared_ptr<Variable>& variable)
{
    variable->type = VariableType::tStruct;
    if(!posValid(json, pos)) return;
    if(json[pos] == '{')
    {
        pos++;
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
    }
    skipWhitespace(json, pos);
    if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
    if(json[pos] == '}')
    {
        pos++;
        return; //Empty object
    }

    while(pos < json.length())
    {
        if(json[pos] != '"') throw JsonDecoderException("Object element has no name.");
        std::string name;
        decodeString(json, pos, name);
        skipWhitespace(json, pos);
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
        if(json[pos] != ':')
        {
            variable->structValue->insert(StructElement(name, std::shared_ptr<Variable>(new Variable(VariableType::tVoid))));
            if(json[pos] == ',')
            {
                pos++;
                skipWhitespace(json, pos);
                if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
                continue;
            }
            if(json[pos] == '}')
            {
                pos++;
                return;
            }
            throw JsonDecoderException("Invalid data after object name.");
        }
        pos++;
        skipWhitespace(json, pos);
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
        std::shared_ptr<Variable> element(new Variable(VariableType::tVoid));
        decodeValue(json, pos,element);
        variable->structValue->insert(StructElement(name, element));
        skipWhitespace(json, pos);
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
        if(json[pos] == ',')
        {
            pos++;
            skipWhitespace(json, pos);
            if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
            continue;
        }
        if(json[pos] == '}')
        {
            pos++;
            return;
        }
        throw JsonDecoderException("No closing '}' found.");
    }
}

void JsonDecoder::decodeObject(const std::vector<char>& json, uint32_t& pos, std::shared_ptr<Variable>& variable)
{
    variable->type = VariableType::tStruct;
    if(!posValid(json, pos)) return;
    if(json[pos] == '{')
    {
        pos++;
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
    }
    skipWhitespace(json, pos);
    if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
    if(json[pos] == '}')
    {
        pos++;
        return; //Empty object
    }

    while(pos < json.size())
    {
        if(json[pos] != '"') throw JsonDecoderException("Object element has no name.");
        std::string name;
        decodeString(json, pos, name);
        skipWhitespace(json, pos);
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
        if(json[pos] != ':')
        {
            variable->structValue->insert(StructElement(name, std::shared_ptr<Variable>(new Variable(VariableType::tVoid))));
            if(json[pos] == ',')
            {
                pos++;
                skipWhitespace(json, pos);
                if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
                continue;
            }
            if(json[pos] == '}')
            {
                pos++;
                return;
            }
            throw JsonDecoderException("Invalid data after object name.");
        }
        pos++;
        skipWhitespace(json, pos);
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
        std::shared_ptr<Variable> element(new Variable(VariableType::tVoid));
        decodeValue(json, pos,element);
        variable->structValue->insert(StructElement(name, element));
        skipWhitespace(json, pos);
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
        if(json[pos] == ',')
        {
            pos++;
            skipWhitespace(json, pos);
            if(!posValid(json, pos)) throw JsonDecoderException("No closing '}' found.");
            continue;
        }
        if(json[pos] == '}')
        {
            pos++;
            return;
        }
        throw JsonDecoderException("No closing '}' found.");
    }
}

void JsonDecoder::decodeArray(const std::string& json, uint32_t& pos, std::shared_ptr<Variable>& variable)
{
    variable->type = VariableType::tArray;
    if(!posValid(json, pos)) return;
    if(json[pos] == '[')
    {
        pos++;
        if(!posValid(json, pos)) throw JsonDecoderException("No closing ']' found.");
    }

    skipWhitespace(json, pos);
    if(!posValid(json, pos)) throw JsonDecoderException("No closing ']' found.");
    if(json[pos] == ']')
    {
        pos++;
        return; //Empty array
    }

    while(pos < json.length())
    {
        std::shared_ptr<Variable> element(new Variable(VariableType::tVoid));
        decodeValue(json, pos, element);
        variable->arrayValue->push_back(element);
        skipWhitespace(json, pos);
        if(!posValid(json, pos)) throw JsonDecoderException("No closing ']' found.");
        if(json[pos] == ',')
        {
            pos++;
            skipWhitespace(json, pos);
            if(!posValid(json, pos)) throw JsonDecoderException("No closing ']' found.");
            continue;
        }
        if(json[pos] == ']')
        {
            pos++;
            return;
        }
        throw JsonDecoderException("No closing ']' found.");
    }
}

void JsonDecoder::decodeArray(const std::vector<char>& json, uint32_t& pos, std::shared_ptr<Variable>& variable)
{
    variable->type = VariableType::tArray;
    if(!posValid(json, pos)) return;
    if(json[pos] == '[')
    {
        pos++;
        if(!posValid(json, pos)) throw JsonDecoderException("No closing ']' found.");
    }

    skipWhitespace(json, pos);
    if(!posValid(json, pos)) throw JsonDecoderException("No closing ']' found.");
    if(json[pos] == ']')
    {
        pos++;
        return; //Empty array
    }

    while(pos < json.size())
    {
        std::shared_ptr<Variable> element(new Variable(VariableType::tVoid));
        decodeValue(json, pos, element);
        variable->arrayValue->push_back(element);
        skipWhitespace(json, pos);
        if(!posValid(json, pos)) throw JsonDecoderException("No closing ']' found.");
        if(json[pos] == ',')
        {
            pos++;
            skipWhitespace(json, pos);
            if(!posValid(json, pos)) throw JsonDecoderException("No closing ']' found.");
            continue;
        }
        if(json[pos] == ']')
        {
            pos++;
            return;
        }
        throw JsonDecoderException("No closing ']' found.");
    }
}

void JsonDecoder::decodeString(const std::string& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    value->type = VariableType::tString;
    std::string s;
    decodeString(json, pos, value->stringValue);
}

void JsonDecoder::decodeString(const std::vector<char>& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    value->type = VariableType::tString;
    std::string s;
    decodeString(json, pos, value->stringValue);
}

#if __GNUC__ > 4

std::string JsonDecoder::decodeString(const std::string& s)
{
    std::string utf8; //String is expected to be UTF-8, except "\uXXXX". This is how Webapps encode JSONs.
    utf8.reserve(s.size());
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t > converter;
    for(int32_t i = 0; i < (signed)s.size(); i++)
    {
        char c = s[i];
        if(c == '\\')
        {
            i++;
            if(!posValid(s, i)) break;
            c = s[i];
            switch(c)
            {
                case 'b':
                    utf8.push_back('\b');
                    break;
                case 'f':
                    utf8.push_back('\f');
                    break;
                case 'n':
                    utf8.push_back('\n');
                    break;
                case 'r':
                    utf8.push_back('\r');
                    break;
                case 't':
                    utf8.push_back('\t');
                    break;
                case 'u':
                {
                    i += 4;
                    if(!posValid(s, i)) break;
                    std::string hex1(s.data() + (i - 3), 2);
                    std::string hex2(s.data() + (i - 1), 2);
                    char16_t c16 = ((char16_t)(uint16_t)(BaseLib::Math::getNumber(hex1, true) << 8)) | ((char16_t)(uint16_t)BaseLib::Math::getNumber(hex2, true));
                    if(c16 != 0 && ((uint16_t)c16 < 0xDC00 || (uint16_t)c16 > 0xDFFF)) //Ignore low surrogates as first character
                    {
                        if((uint16_t)c16 >= 0xD800 && (uint16_t)c16 <= 0xDBFF) //High surrogate => a second character follows
                        {
                            std::u16string utf16;
                            utf16.reserve(2);
                            utf16.push_back(c16);
                            i += 6;
                            if(!posValid(s, i)) break;
                            if(s.at(i - 5) != '\\' || s.at(i - 4) != 'u') throw JsonDecoderException("Invalid UTF-16 in JSON.");
                            std::string hex3(s.data() + (i - 3), 2);
                            std::string hex4(s.data() + (i - 1), 2);
                            c16 = ((char16_t)(uint16_t)(BaseLib::Math::getNumber(hex3, true) << 8)) | ((char16_t)(uint16_t)BaseLib::Math::getNumber(hex4, true));
                            utf16.push_back(c16);
                            auto utf8Char = converter.to_bytes(utf16);
                            if(!utf8Char.empty()) utf8.insert(utf8.end(), utf8Char.begin(), utf8Char.end());
                        }
                        else
                        {
                            auto utf8Char = converter.to_bytes(c16);
                            if(!utf8Char.empty()) utf8.insert(utf8.end(), utf8Char.begin(), utf8Char.end());
                        }
                    }
                    break;
                }
                default:
                    utf8.push_back(s[i]);
            }
        }
        else utf8.push_back(s[i]);
    }

    utf8.shrink_to_fit();
    return utf8;
}

void JsonDecoder::decodeString(const std::string& json, uint32_t& pos, std::string& s)
{
    s.clear(); //String is expected to be UTF-8, except "\uXXXX". This is how Webapps encode JSONs.
    s.reserve(1024);
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t > converter;
    if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
    if(json[pos] == '"')
    {
        pos++;
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
    }
    while(pos < json.length())
    {
        char c = json[pos];
        if(c == '\\')
        {
            pos++;
            if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
            c = json[pos];
            switch(c)
            {
                case 'b':
                    s.push_back('\b');
                    break;
                case 'f':
                    s.push_back('\f');
                    break;
                case 'n':
                    s.push_back('\n');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'u':
                {
                    pos += 4;
                    if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
                    std::string hex1(json.data() + (pos - 3), 2);
                    std::string hex2(json.data() + (pos - 1), 2);
                    char16_t c16 = ((char16_t)(uint16_t)(BaseLib::Math::getNumber(hex1, true) << 8)) | ((char16_t)(uint16_t)BaseLib::Math::getNumber(hex2, true));
                    if(c16 != 0 && ((uint16_t)c16 < 0xDC00 || (uint16_t)c16 > 0xDFFF)) //Ignore low surrogates as first character
                    {
                        if((uint16_t)c16 >= 0xD800 && (uint16_t)c16 <= 0xDBFF) //High surrogate => a second character follows
                        {
                            std::u16string utf16;
                            utf16.reserve(2);
                            utf16.push_back(c16);
                            pos += 6;
                            if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
                            if(json.at(pos - 5) != '\\' || json.at(pos - 4) != 'u') throw JsonDecoderException("Invalid UTF-16 in JSON.");
                            std::string hex3(json.data() + (pos - 3), 2);
                            std::string hex4(json.data() + (pos - 1), 2);
                            c16 = ((char16_t)(uint16_t)(BaseLib::Math::getNumber(hex3, true) << 8)) | ((char16_t)(uint16_t)BaseLib::Math::getNumber(hex4, true));
                            utf16.push_back(c16);
                            auto utf8Char = converter.to_bytes(utf16);
                            if(!utf8Char.empty()) s.insert(s.end(), utf8Char.begin(), utf8Char.end());
                        }
                        else
                        {
                            auto utf8Char = converter.to_bytes(c16);
                            if(!utf8Char.empty()) s.insert(s.end(), utf8Char.begin(), utf8Char.end());
                        }
                    }
                }
                    break;
                default:
                    s.push_back(json[pos]);
            }
        }
        else if(c == '"')
        {
            pos++;
            s.shrink_to_fit();
            return;
        }
        else s.push_back(json[pos]);
        pos++;
        if(s.size() + 4 > s.capacity()) s.reserve(s.capacity() + 1024);
    }
    throw JsonDecoderException("No closing '\"' found.");
}

void JsonDecoder::decodeString(const std::vector<char>& json, uint32_t& pos, std::string& s)
{
    s.clear(); //String is expected to be UTF-8, except "\uXXXX". This is how Webapps encode JSONs.
    s.reserve(1024);
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t > converter;
    if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
    if(json[pos] == '"')
    {
        pos++;
        if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
    }
    while(pos < json.size())
    {
        char c = json[pos];
        if(c == '\\')
        {
            pos++;
            if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
            c = json[pos];
            switch(c)
            {
                case 'b':
                    s.push_back('\b');
                    break;
                case 'f':
                    s.push_back('\f');
                    break;
                case 'n':
                    s.push_back('\n');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'u':
                {
                    pos += 4;
                    if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
                    std::string hex1(json.data() + (pos - 3), 2);
                    std::string hex2(json.data() + (pos - 1), 2);
                    char16_t c16 = ((char16_t)(uint16_t)(BaseLib::Math::getNumber(hex1, true) << 8)) | ((char16_t)(uint16_t)BaseLib::Math::getNumber(hex2, true));
                    if(c16 != 0 && ((uint16_t)c16 < 0xDC00 || (uint16_t)c16 > 0xDFFF)) //Ignore low surrogates as first character
                    {
                        if((uint16_t)c16 >= 0xD800 && (uint16_t)c16 <= 0xDBFF) //High surrogate => a second character follows
                        {
                            std::u16string utf16;
                            utf16.reserve(2);
                            utf16.push_back(c16);
                            pos += 6;
                            if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
                            if(json.at(pos - 5) != '\\' || json.at(pos - 4) != 'u') throw JsonDecoderException("Invalid UTF-16 in JSON.");
                            std::string hex3(json.data() + (pos - 3), 2);
                            std::string hex4(json.data() + (pos - 1), 2);
                            c16 = ((char16_t)(uint16_t)(BaseLib::Math::getNumber(hex3, true) << 8)) | ((char16_t)(uint16_t)BaseLib::Math::getNumber(hex4, true));
                            utf16.push_back(c16);
                            auto utf8Char = converter.to_bytes(utf16);
                            if(!utf8Char.empty()) s.insert(s.end(), utf8Char.begin(), utf8Char.end());
                        }
                        else
                        {
                            auto utf8Char = converter.to_bytes(c16);
                            if(!utf8Char.empty()) s.insert(s.end(), utf8Char.begin(), utf8Char.end());
                        }
                    }
                }
                    break;
                default:
                    s.push_back(json[pos]);
            }
        }
        else if(c == '"')
        {
            pos++;
            s.shrink_to_fit();
            return;
        }
        else s.push_back(json[pos]);
        pos++;
        if(s.size() + 4 > s.capacity()) s.reserve(s.capacity() + 1024);
    }
    throw JsonDecoderException("No closing '\"' found.");
}

#else

std::string JsonDecoder::decodeString(const std::string& s)
{
	std::string result;
	result.reserve(s.size());
	for(int32_t i = 0; i < (int32_t)s.size(); i++)
	{
		char c = s[i];
		if(c == '\\')
		{
			i++;
			if(!posValid(s, i)) return result;
			c = s[i];
			switch(c)
			{
				case 'b':
					result.push_back('\b');
					break;
				case 'f':
					result.push_back('\f');
					break;
				case 'n':
					result.push_back('\n');
					break;
				case 'r':
					result.push_back('\r');
					break;
				case 't':
					result.push_back('\t');
					break;
				case 'u':
				{
					i += 4;
					if(!posValid(s, i)) return result;
					std::string hex1(s.data() + (i - 3), 2);
					std::string hex2(s.data() + (i - 1), 2);
                    char char1 = (char)(uint8_t)BaseLib::Math::getNumber(hex1, true);
                    if(char1 != 0) result.push_back(char1);
					result.push_back((char) (uint8_t) BaseLib::Math::getNumber(hex2, true));
				}
					break;
				default:
					result.push_back(s[i]);
			}
		}
		else if((unsigned) c < 0x20) throw JsonDecoderException("Invalid character in string: " + std::to_string((int32_t) c) + ". String so far: " + s);
		else result.push_back(s[i]);
	}
	return result;
}

void JsonDecoder::decodeString(const std::string& json, uint32_t& pos, std::string& s)
{
	s.clear();
	if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
	if(json[pos] == '"')
	{
		pos++;
		if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
	}
	while(pos < json.length())
	{
		char c = json[pos];
		if(c == '\\')
		{
			pos++;
			if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
			c = json[pos];
			switch(c)
			{
			case 'b':
				s.push_back('\b');
				break;
			case 'f':
				s.push_back('\f');
				break;
			case 'n':
				s.push_back('\n');
				break;
			case 'r':
				s.push_back('\r');
				break;
			case 't':
				s.push_back('\t');
				break;
			case 'u':
				{
					pos += 4;
					if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
					std::string hex1(json.data() + (pos - 3), 2);
					std::string hex2(json.data() + (pos - 1), 2);
					s.push_back((char)(uint8_t)BaseLib::Math::getNumber(hex1, true));
					s.push_back((char)(uint8_t)BaseLib::Math::getNumber(hex2, true));
				}
				break;
			default:
				s.push_back(json[pos]);
			}
		}
		else if(c == '"')
		{
			pos++;
			return;
		}
		else if((unsigned)c < 0x20) throw JsonDecoderException("Invalid character in string: " + std::to_string((int32_t)c) + ". String so far: " + s);
		else s.push_back(json[pos]);
		pos++;
	}
	throw JsonDecoderException("No closing '\"' found.");
}

void JsonDecoder::decodeString(const std::vector<char>& json, uint32_t& pos, std::string& s)
{
	s.clear();
	if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
	if(json[pos] == '"')
	{
		pos++;
		if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
	}
	while(pos < json.size())
	{
		char c = json[pos];
		if(c == '\\')
		{
			pos++;
			if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
			c = json[pos];
			switch(c)
			{
			case 'b':
				s.push_back('\b');
				break;
			case 'f':
				s.push_back('\f');
				break;
			case 'n':
				s.push_back('\n');
				break;
			case 'r':
				s.push_back('\r');
				break;
			case 't':
				s.push_back('\t');
				break;
			case 'u':
				{
					pos += 4;
					if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
					std::string hex1(json.data() + (pos - 3), 2);
					std::string hex2(json.data() + (pos - 1), 2);
					s.push_back((char)(uint8_t)BaseLib::Math::getNumber(hex1, true));
					s.push_back((char)(uint8_t)BaseLib::Math::getNumber(hex2, true));
				}
				break;
			default:
				s.push_back(json[pos]);
			}
		}
		else if(c == '"')
		{
			pos++;
			return;
		}
		else if((unsigned)c < 0x20) throw JsonDecoderException("Invalid character in string.");
		else s.push_back(json[pos]);
		pos++;
	}
	throw JsonDecoderException("No closing '\"' found.");
}

#endif

void JsonDecoder::decodeValue(const std::string& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
    switch (json[pos]) {
        case 'n':
            decodeNull(json, pos, value);
            break;
        case 't':
            decodeBoolean(json, pos, value);
            break;
        case 'f':
            decodeBoolean(json, pos, value);
            break;
        case '"':
            decodeString(json, pos, value);
            break;
        case '{':
            decodeObject(json, pos, value);
            break;
        case '[':
            decodeArray(json, pos, value);
            break;
        default:
            decodeNumber(json, pos, value);
            break;
    }
}

void JsonDecoder::decodeValue(const std::vector<char>& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    if(!posValid(json, pos)) throw JsonDecoderException("No closing '\"' found.");
    switch (json[pos]) {
        case 'n':
            decodeNull(json, pos, value);
            break;
        case 't':
            decodeBoolean(json, pos, value);
            break;
        case 'f':
            decodeBoolean(json, pos, value);
            break;
        case '"':
            decodeString(json, pos, value);
            break;
        case '{':
            decodeObject(json, pos, value);
            break;
        case '[':
            decodeArray(json, pos, value);
            break;
        default:
            decodeNumber(json, pos, value);
            break;
    }
}

void JsonDecoder::decodeBoolean(const std::string& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    value->type = VariableType::tBoolean;
    if(!posValid(json, pos)) return;
    if(json[pos] == 't')
    {
        value->booleanValue = true;
        pos += 4;
    }
    else
    {
        value->booleanValue = false;
        pos += 5;
    }
}

void JsonDecoder::decodeBoolean(const std::vector<char>& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    value->type = VariableType::tBoolean;
    if(!posValid(json, pos)) return;
    if(json[pos] == 't')
    {
        value->booleanValue = true;
        pos += 4;
    }
    else
    {
        value->booleanValue = false;
        pos += 5;
    }
}

void JsonDecoder::decodeNull(const std::string& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    value->type = VariableType::tVoid;
    pos += 4;
}

void JsonDecoder::decodeNull(const std::vector<char>& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    value->type = VariableType::tVoid;
    pos += 4;
}

void JsonDecoder::decodeNumber(const std::string& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    value->type = VariableType::tInteger;
    if(!posValid(json, pos)) return;
    bool minus = false;
    if(json[pos] == '-')
    {
        minus = true;
        pos++;
        if(!posValid(json, pos)) return;
    }
    else if(json[pos] == '+')
    {
        pos++;
        if(!posValid(json, pos)) return;
    }

    bool isDouble = false;
    int64_t number = 0;
    if(json[pos] == '0')
    {
        number = 0;
        pos++;
        if(!posValid(json, pos)) return;
    }
    else if(json[pos] >= '1' && json[pos] <= '9')
    {
        while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9')
        {
            if(number >= 922337203685477580ll)
            {
                value->type = VariableType::tFloat;
                isDouble = true;
                value->floatValue = number;
                break;
            }
            number = number * 10 + (json[pos] - '0');
            pos++;
        }
    }
    else throw JsonDecoderException("Tried to decode invalid number.");

    if(isDouble)
    {
        while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9')
        {
            value->floatValue = value->floatValue * 10 + (json[pos] - '0');
            pos++;
        }
    }

    if(!posValid(json, pos)) return;
    int32_t exponent = 0;
    if(json[pos] == '.')
    {
        if(!isDouble)
        {
            value->type = VariableType::tFloat;
            isDouble = true;
            value->floatValue = number;
        }
        pos++;
        while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9')
        {
            value->floatValue = value->floatValue * 10 + (json[pos] - '0');
            pos++;
            exponent--;
        }
    }

    if(!posValid(json, pos)) return;
    int32_t exponent2 = 0;
    if(json[pos] == 'e' || json[pos] == 'E')
    {
        pos++;
        if(!posValid(json, pos)) return;

        bool negative = false;
        if(json[pos] == '-')
        {
            negative = true;
            pos++;
            if(!posValid(json, pos)) return;
        }
        else if(json[pos] == '+')
        {
            pos++;
            if(!posValid(json, pos)) return;
        }
        if (json[pos] >= '0' && json[pos] <= '9')
        {
            exponent2 = json[pos] - '0';
            pos++;
            if(!posValid(json, pos)) return;
            while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9')
            {
                exponent2 = exponent2 * 10 + (json[pos] - '0');
            }
        }
        if(negative) exponent2 *= -1;
    }

    if(isDouble)
    {
        exponent += exponent2;
        if(exponent < -308) exponent = -308;
        else if(exponent > 308) exponent = 308;
        value->floatValue = (exponent >= 0) ? value->floatValue * Math::Pow10(exponent) : value->floatValue / Math::Pow10(-exponent);
        if(minus) value->floatValue *= -1;
        value->integerValue64 = std::llround(value->floatValue);
        value->integerValue = std::lround(value->floatValue);
    }
    else
    {
        value->integerValue64 = minus ? -((int64_t)number) : number;

        if(value->integerValue64 > 2147483647ll || value->integerValue64 < -2147483648ll)
        {
            value->type = VariableType::tInteger64;
        }

        value->integerValue = value->integerValue64;
        value->floatValue = value->integerValue64;
    }
}

void JsonDecoder::decodeNumber(const std::vector<char>& json, uint32_t& pos, std::shared_ptr<Variable>& value)
{
    value->type = VariableType::tInteger;
    if(!posValid(json, pos)) return;
    bool minus = false;
    if(json[pos] == '-')
    {
        minus = true;
        pos++;
        if(!posValid(json, pos)) return;
    }
    else if(json[pos] == '+')
    {
        pos++;
        if(!posValid(json, pos)) return;
    }

    bool isDouble = false;
    int64_t number = 0;
    if(json[pos] == '0')
    {
        number = 0;
        pos++;
        if(!posValid(json, pos)) return;
    }
    else if(json[pos] >= '1' && json[pos] <= '9')
    {
        while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9')
        {
            if(number >= 922337203685477580ll)
            {
                value->type = VariableType::tFloat;
                isDouble = true;
                value->floatValue = number;
                break;
            }
            number = number * 10 + (json[pos] - '0');
            pos++;
        }
    }
    else throw JsonDecoderException("Tried to decode invalid number.");

    if(isDouble)
    {
        while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9')
        {
            value->floatValue = value->floatValue * 10 + (json[pos] - '0');
            pos++;
        }
    }

    if(!posValid(json, pos)) return;
    int32_t exponent = 0;
    if(json[pos] == '.')
    {
        if(!isDouble)
        {
            value->type = VariableType::tFloat;
            isDouble = true;
            value->floatValue = number;
        }
        pos++;
        while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9')
        {
            value->floatValue = value->floatValue * 10 + (json[pos] - '0');
            pos++;
            exponent--;
        }
    }

    if(!posValid(json, pos)) return;
    int32_t exponent2 = 0;
    if(json[pos] == 'e' || json[pos] == 'E')
    {
        pos++;
        if(!posValid(json, pos)) return;

        bool negative = false;
        if(json[pos] == '-')
        {
            negative = true;
            pos++;
            if(!posValid(json, pos)) return;
        }
        else if(json[pos] == '+')
        {
            pos++;
            if(!posValid(json, pos)) return;
        }
        if (json[pos] >= '0' && json[pos] <= '9')
        {
            exponent2 = json[pos] - '0';
            pos++;
            if(!posValid(json, pos)) return;
            while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9')
            {
                exponent2 = exponent2 * 10 + (json[pos] - '0');
            }
        }
        if(negative) exponent2 *= -1;
    }

    if(isDouble)
    {
        exponent += exponent2;
        if(exponent < -308) exponent = -308;
        else if(exponent > 308) exponent = 308;
        value->floatValue = (exponent >= 0) ? value->floatValue * Math::Pow10(exponent) : value->floatValue / Math::Pow10(-exponent);
        if(minus) value->floatValue *= -1;
        value->integerValue64 = std::llround(value->floatValue);
        value->integerValue = std::lround(value->floatValue);
    }
    else
    {

        value->integerValue64 = minus ? -((int64_t)number) : number;

        if(value->integerValue64 > 2147483647ll || value->integerValue64 < -2147483648ll)
        {
            value->type = VariableType::tInteger64;
        }

        value->integerValue = value->integerValue64;
        value->floatValue = value->integerValue64;
    }
}

}
}
