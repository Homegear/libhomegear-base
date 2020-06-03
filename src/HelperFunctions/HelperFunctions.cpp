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

#include "HelperFunctions.h"
#include "../BaseLib.h"

#include <iomanip>

#include <pwd.h>
#include <grp.h>
#include <sys/resource.h>

namespace BaseLib
{

const std::array<int32_t, 23> HelperFunctions::_asciiToBinaryTable{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10 ,11 ,12, 13, 14, 15};

const std::array<int32_t, 16> HelperFunctions::_binaryToASCIITable{0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};

HelperFunctions::HelperFunctions()
{
}

int64_t HelperFunctions::getTime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t HelperFunctions::getTimeMicroseconds()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t HelperFunctions::getTimeSeconds()
{
    auto time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
    if(time < 0) time = 0;
    return time;
}

std::string HelperFunctions::getTimeString(int64_t time)
{
    const char timeFormat[] = "%x %X";
    std::time_t t;
    int32_t milliseconds;
    if(time > 0)
    {
        t = std::time_t(time / 1000);
        milliseconds = time % 1000;
    }
    else
    {
        const auto timePoint = std::chrono::system_clock::now();
        t = std::chrono::system_clock::to_time_t(timePoint);
        milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count() % 1000;
    }
    char timeString[50];
    std::tm localTime;
    localtime_r(&t, &localTime);
    strftime(&timeString[0], 50, &timeFormat[0], &localTime);
    std::ostringstream timeStream;
    timeStream << timeString << "." << std::setw(3) << std::setfill('0') << milliseconds;
    return timeStream.str();
}

std::string HelperFunctions::getTimeString(std::string format, int64_t time)
{
    std::time_t t;
    int32_t milliseconds;
    if(time > 0)
    {
        t = std::time_t(time / 1000);
        milliseconds = time % 1000;
    }
    else
    {
        const auto timePoint = std::chrono::system_clock::now();
        t = std::chrono::system_clock::to_time_t(timePoint);
        milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count() % 1000;
    }
    char timeString[50];
    std::tm localTime;
    localtime_r(&t, &localTime);
    strftime(&timeString[0], 50, format.c_str(), &localTime);
    std::ostringstream timeStream;
    timeStream << timeString << "." << std::setw(3) << std::setfill('0') << milliseconds;
    return timeStream.str();
}

std::string HelperFunctions::getTimeUuid(int64_t time)
{
    std::string uuid;
    uuid = BaseLib::HelperFunctions::getHexString(time == 0 ? getTimeMicroseconds() : time, 16);
    uuid.reserve(53);
    uuid.push_back('-');
    uuid.append(BaseLib::HelperFunctions::getHexString(BaseLib::HelperFunctions::getRandomNumber(-2147483648, 2147483647), 8) + "-");
    uuid.append(BaseLib::HelperFunctions::getHexString(BaseLib::HelperFunctions::getRandomNumber(0, 65535), 4) + "-");
    uuid.append(BaseLib::HelperFunctions::getHexString(BaseLib::HelperFunctions::getRandomNumber(0, 65535), 4) + "-");
    uuid.append(BaseLib::HelperFunctions::getHexString(BaseLib::HelperFunctions::getRandomNumber(0, 65535), 4) + "-");
    uuid.append(BaseLib::HelperFunctions::getHexString(BaseLib::HelperFunctions::getRandomNumber(-2147483648, 2147483647), 8));
    uuid.append(BaseLib::HelperFunctions::getHexString(BaseLib::HelperFunctions::getRandomNumber(0, 65535), 4));
    return uuid;
}

std::string HelperFunctions::stripNonAlphaNumeric(const std::string& s)
{
    std::string strippedString;
    strippedString.reserve(s.size());
    for(std::string::const_iterator i = s.begin(); i != s.end(); ++i)
    {
        if(isalpha(*i) || isdigit(*i) || (*i == '_') || (*i == '-')) strippedString.push_back(*i);
    }
    return strippedString;
}

std::string HelperFunctions::stripNonPrintable(const std::string& s)
{
    std::string strippedString;
    strippedString.reserve(s.size());
    for(std::string::const_iterator i = s.begin(); i != s.end(); ++i)
    {
        if(std::isprint(*i, std::locale("en_US.UTF-8"))) strippedString.push_back(*i);
    }
    return strippedString;
}

PVariable HelperFunctions::xml2variable(const xml_node* node)
{
    auto nodeVariable = std::make_shared<Variable>(VariableType::tStruct);

    bool hasElements = false;
    for(xml_attribute* attr = node->first_attribute(); attr; attr = attr->next_attribute())
    {
        std::string attributeName(attr->name());
        std::string attributeValue(attr->value());

        nodeVariable->structValue->emplace("@" + attributeName, std::make_shared<Variable>(attributeValue));
    }

    for(xml_node* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
    {
        std::string nodeName(subNode->name());
        if(nodeName.empty()) continue;
        std::string nodeValue(subNode->value());

        hasElements = true;

        auto variableIterator = nodeVariable->structValue->find(nodeName);
        if(variableIterator == nodeVariable->structValue->end())
        {
            variableIterator = nodeVariable->structValue->emplace(nodeName, std::make_shared<Variable>(VariableType::tArray)).first;
            variableIterator->second->arrayValue->reserve(100);
        }

        if(variableIterator->second->arrayValue->size() == variableIterator->second->arrayValue->capacity()) variableIterator->second->arrayValue->reserve(variableIterator->second->arrayValue->size() + 100);

        auto subNodeVariable = xml2variable(subNode);
        if(!subNodeVariable->errorStruct) variableIterator->second->arrayValue->emplace_back(subNodeVariable);
    }

    if(!hasElements)
    {
        std::string nodeValue = std::string(node->value());
        PVariable jsonNodeValue;
        try
        {
            jsonNodeValue = Rpc::JsonDecoder::decode(nodeValue);
        }
        catch(const std::exception& ex)
        {
            jsonNodeValue = std::make_shared<Variable>(nodeValue);
        }

        if(nodeVariable->structValue->empty()) return jsonNodeValue;
        else
        {
            if(!nodeValue.empty()) nodeVariable->structValue->emplace("@@value", jsonNodeValue);
            return nodeVariable;
        }
    }
    else return nodeVariable;
}

void HelperFunctions::variable2xml(xml_document* doc, xml_node* parentNode, const PVariable& variable)
{
    std::string tempString;

    for(auto& structElement : *variable->structValue)
    {
        if(structElement.first.empty()) continue;
        else if(structElement.first == "@@value") //Value when node has attributes
        {
            tempString = structElement.second->toString();
            parentNode->value(doc->allocate_string(tempString.c_str(), tempString.size() + 1));
        }
        else if(structElement.first.front() == '@' && structElement.first.size() > 1) //Attribute
        {
            tempString = structElement.second->toString();
            auto* attr = doc->allocate_attribute(structElement.first.c_str() + 1, doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            parentNode->append_attribute(attr);
        }
        else //Element
        {
            if(structElement.second->type == VariableType::tStruct)
            {
                auto* node = doc->allocate_node(node_element, structElement.first.c_str());
                parentNode->append_node(node);
                variable2xml(doc, node, structElement.second);
            }
            else if(structElement.second->type == VariableType::tArray)
            {
                auto* node = doc->allocate_node(node_element, "element");
                parentNode->append_node(node);
                variable2xml(doc, node, structElement.second);
            }
            else
            {
                tempString = structElement.second->toString();
                auto* node = doc->allocate_node(node_element, structElement.first.c_str(), doc->allocate_string(tempString.c_str(), tempString.size() + 1));
                parentNode->append_node(node);
            }
        }
    }
}

int32_t HelperFunctions::getRandomNumber(int32_t min, int32_t max)
{
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int32_t> distribution(min, max);
    return distribution(generator);
}

std::vector<uint8_t> HelperFunctions::getRandomBytes(uint32_t size)
{
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<uint8_t> distribution(0, 255);
    std::vector<uint8_t> bytes;
    bytes.reserve(size);
    for(uint32_t i = 0; i < size; i++)
    {
        bytes.push_back(distribution(generator));
    }
    return bytes;
}

void HelperFunctions::memcpyBigEndian(char* to, const char* from, const uint32_t& length)
{
    static bool bigEndian = isBigEndian();

    if(bigEndian) memcpy(to, from, length);
    else
    {
        uint32_t last = length - 1;
        for(uint32_t i = 0; i < length; i++)
        {
            to[i] = from[last - i];
        }
    }
}

void HelperFunctions::memcpyBigEndian(uint8_t* to, const uint8_t* from, const uint32_t& length)
{
    memcpyBigEndian((char*)to, (char*)from, length);
}

void HelperFunctions::memcpyBigEndian(int32_t& to, const std::vector<uint8_t>& from)
{
    static bool bigEndian = isBigEndian();

    to = 0; //Necessary if length is < 4
    if(from.empty()) return;
    uint32_t length = from.size();
    if(length > 4) length = 4;
    if(bigEndian) memcpyBigEndian(((uint8_t*)&to) + (4 - length), &from.at(0), length);
    else memcpyBigEndian(((uint8_t*)&to), &from.at(0), length);
}

void HelperFunctions::memcpyBigEndian(std::vector<uint8_t>& to, const int32_t& from)
{
    static bool bigEndian = isBigEndian();

    if(!to.empty()) to.clear();
    int32_t length = 4;
    if(from < 0) length = 4;
    else if(from < 256) length = 1;
    else if(from < 65536) length = 2;
    else if(from < 16777216) length = 3;
    to.resize(length, 0);
    if(bigEndian) memcpyBigEndian(&to.at(0), (uint8_t*)&from + (4 - length), length);
    else memcpyBigEndian(&to.at(0), (uint8_t*)&from, length);
}

void HelperFunctions::memcpyBigEndian(int64_t& to, const std::vector<uint8_t>& from)
{
    static bool bigEndian = isBigEndian();

    to = 0; //Necessary if length is < 8
    if(from.empty()) return;
    uint32_t length = from.size();
    if(length > 8) length = 8;
    if(bigEndian) memcpyBigEndian(((uint8_t*)&to) + (8 - length), &from.at(0), length);
    else memcpyBigEndian(((uint8_t*)&to), &from.at(0), length);
}

void HelperFunctions::memcpyBigEndian(std::vector<uint8_t>& to, const int64_t& from)
{
    static bool bigEndian = isBigEndian();

    if(!to.empty()) to.clear();
    int32_t length = 8;
    if(from < 0) length = 8;
    else if(from <= 0xFF) length = 1;
    else if(from <= 0xFFFF) length = 2;
    else if(from <= 0xFFFFFF) length = 3;
    else if(from <= 0xFFFFFFFFll) length = 4;
    else if(from <= 0xFFFFFFFFFFll) length = 5;
    else if(from <= 0xFFFFFFFFFFFFll) length = 6;
    else if(from <= 0xFFFFFFFFFFFFFFll) length = 7;
    to.resize(length, 0);
    if(bigEndian) memcpyBigEndian(&to.at(0), (uint8_t*)&from + (8 - length), length);
    else memcpyBigEndian(&to.at(0), (uint8_t*)&from, length);
}

std::string& HelperFunctions::stringReplace(std::string& haystack, const std::string& search, const std::string& replace)
{
    if(search.empty()) return haystack;
    int32_t pos = 0;
    while(true)
    {
        pos = haystack.find(search, pos);
        if (pos == (signed)std::string::npos) break;
        haystack.replace(pos, search.size(), replace);
        pos += replace.size();
    }
    return haystack;
}

std::string& HelperFunctions::regexReplace(std::string& haystack, const std::string& search, const std::string& replace, bool ignoreCase)
{
    std::regex regex(search, std::regex::icase);

    std::string result = std::regex_replace(haystack, regex, replace);
    haystack = result;

    return haystack;
}

std::pair<std::string, std::string> HelperFunctions::splitFirst(std::string string, char delimiter)
{
    int32_t pos = string.find_first_of(delimiter);
    if(pos == -1) return std::pair<std::string, std::string>(string, "");
    if((unsigned)pos + 1 >= string.size()) return std::pair<std::string, std::string>(string.substr(0, pos), "");
    return std::pair<std::string, std::string>(string.substr(0, pos), string.substr(pos + 1));
}

std::pair<std::string, std::string> HelperFunctions::splitLast(std::string string, char delimiter)
{
    int32_t pos = string.find_last_of(delimiter);
    if(pos == -1) return std::pair<std::string, std::string>(string, "");
    if((unsigned)pos + 1 >= string.size()) return std::pair<std::string, std::string>(string.substr(0, pos), "");
    return std::pair<std::string, std::string>(string.substr(0, pos), string.substr(pos + 1));
}

std::vector<std::string> HelperFunctions::splitAll(std::string string, char delimiter)
{
    std::vector<std::string> elements;
    std::stringstream stringStream(string);
    std::string element;
    while (std::getline(stringStream, element, delimiter))
    {
        elements.push_back(element);
    }
    if(string.back() == delimiter) elements.push_back(std::string());
    return elements;
}

std::vector<uint8_t> HelperFunctions::hexToBin(const std::string& data)
{
    std::vector<uint8_t> bin;
    bin.reserve(data.size() / 2);
    for(uint32_t i = 0; i < data.size(); i+=2)
    {
        try	{ bin.push_back(std::stoi(data.substr(i, 2), 0, 16)); } catch(...) {}
    }
    return bin;
}

char HelperFunctions::getHexChar(int32_t nibble)
{
    if(nibble < 0 || nibble > 15) return 0;
    return _binaryToASCIITable[nibble];
}

std::string HelperFunctions::getHexString(const uint8_t* buffer, uint32_t size)
{
    if(!buffer) return "";
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0') << std::uppercase;
    for(const uint8_t* i = buffer; i < buffer + size; ++i)
    {
        stringstream << std::setw(2) << (int32_t)(*i);
    }
    stringstream << std::dec;
    return stringstream.str();
}

std::string HelperFunctions::getHexString(const char* buffer, uint32_t size)
{
    if(!buffer) return "";
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0') << std::uppercase;
    for(const char* i = buffer; i < buffer + size; ++i)
    {
        stringstream << std::setw(2) << (int32_t)((uint8_t)(*i));
    }
    stringstream << std::dec;
    return stringstream.str();
}

std::string HelperFunctions::getHexString(const std::vector<uint8_t>& data)
{
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0') << std::uppercase;
    for(std::vector<uint8_t>::const_iterator i = data.begin(); i != data.end(); ++i)
    {
        stringstream << std::setw(2) << (int32_t)(*i);
    }
    stringstream << std::dec;
    return stringstream.str();
}

std::string HelperFunctions::getHexString(const std::vector<char>& data)
{
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0') << std::uppercase;
    for(std::vector<char>::const_iterator i = data.begin(); i != data.end(); ++i)
    {
        stringstream << std::setw(2) << (int32_t)((uint8_t)(*i));
    }
    stringstream << std::dec;
    return stringstream.str();
}

std::string HelperFunctions::getHexString(const std::vector<uint16_t>& data)
{
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0') << std::uppercase;
    for(std::vector<uint16_t>::const_iterator i = data.begin(); i != data.end(); ++i)
    {
        stringstream << std::setw(2) << (int32_t)((*i) >> 8) << std::setw(2) << (int32_t)((*i) & 0xFF);
    }
    stringstream << std::dec;
    return stringstream.str();
}

std::string HelperFunctions::getHexString(const std::string& data)
{
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0') << std::uppercase;
    for(std::string::const_iterator i = data.begin(); i != data.end(); ++i)
    {
        stringstream << std::setw(2) << (int32_t)((uint8_t)(*i));
    }
    stringstream << std::dec;
    return stringstream.str();
}

std::string HelperFunctions::getHexString(int32_t number, int32_t width)
{
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0');
    if(width > -1) stringstream << std::setw(width);
    stringstream << std::uppercase << number << std::dec;
    return stringstream.str();
}

std::string HelperFunctions::getHexString(uint32_t number, int32_t width)
{
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0');
    if(width > -1) stringstream << std::setw(width);
    stringstream << std::uppercase << number << std::dec;
    return stringstream.str();
}

std::string HelperFunctions::getHexString(int64_t number, int32_t width)
{
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0');
    if(width > -1) stringstream << std::setw(width);
    stringstream << std::uppercase << number << std::dec;
    return stringstream.str();
}

std::string HelperFunctions::getHexString(uint64_t number, int32_t width)
{
    std::ostringstream stringstream;
    stringstream << std::hex << std::setfill('0');
    if(width > -1) stringstream << std::setw(width);
    stringstream << std::uppercase << number << std::dec;
    return stringstream.str();
}

std::vector<char> HelperFunctions::getBinary(const std::string& hexString)
{
    std::vector<char> binary;
    if(hexString.empty()) return binary;
    if(hexString.size() % 2 != 0 && !std::isspace(hexString.back()))
    {
        std::string hexStringCopy = hexString.substr(1);
        binary.reserve(hexStringCopy.size() / 2);
        for(int32_t i = 0; i < (signed)hexStringCopy.size(); i += 2)
        {
            uint8_t byte = 0;
            if(i < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i])) byte = (uint8_t)(_asciiToBinaryTable[std::toupper(hexStringCopy[i]) - '0'] << 4);
            else continue;
            if(i + 1 < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexStringCopy[i + 1]) - '0'];
            else continue;
            binary.push_back(byte);
        }
    }
    else
    {
        binary.reserve(hexString.size() / 2);
        for(int32_t i = 0; i < (signed)hexString.size(); i += 2)
        {
            uint8_t byte = 0;
            if(i < (signed)hexString.size() && isxdigit(hexString[i])) byte = (uint8_t)(_asciiToBinaryTable[std::toupper(hexString[i]) - '0'] << 4);
            else continue;
            if(i + 1 < (signed)hexString.size() && isxdigit(hexString[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexString[i + 1]) - '0'];
            else continue;
            binary.push_back(byte);
        }
    }
    return binary;
}

std::string HelperFunctions::getBinaryString(const std::string& hexString)
{
    std::string binary;
    if(hexString.empty()) return binary;
    if(hexString.size() % 2 != 0 && !std::isspace(hexString.back()))
    {
        std::string hexStringCopy = hexString.substr(1);
        binary.reserve(hexStringCopy.size() / 2);
        for(int32_t i = 0; i < (signed)hexStringCopy.size(); i += 2)
        {
            uint8_t byte = 0;
            if(i < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i])) byte = (uint8_t) (_asciiToBinaryTable[std::toupper(hexStringCopy[i]) - '0'] << 4);
            else continue;
            if(i + 1 < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexStringCopy[i + 1]) - '0'];
            else continue;
            binary.push_back(byte);
        }
    }
    else
    {
        binary.reserve(hexString.size() / 2);
        for(int32_t i = 0; i < (signed)hexString.size(); i += 2)
        {
            uint8_t byte = 0;
            if(i < (signed)hexString.size() && isxdigit(hexString[i])) byte = (uint8_t)(_asciiToBinaryTable[std::toupper(hexString[i]) - '0'] << 4);
            else continue;
            if(i + 1 < (signed)hexString.size() && isxdigit(hexString[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexString[i + 1]) - '0'];
            else continue;
            binary.push_back(byte);
        }
    }
    return binary;
}

std::vector<uint8_t> HelperFunctions::getUBinary(const std::string& hexString)
{
    std::vector<uint8_t> binary;
    if(hexString.empty()) return binary;
    if(hexString.size() % 2 != 0 && !std::isspace(hexString.back()))
    {
        std::string hexStringCopy = hexString.substr(1);
        binary.reserve(hexStringCopy.size() / 2);
        for(int32_t i = 0; i < (signed)hexStringCopy.size(); i += 2)
        {
            uint8_t byte = 0;
            if(i < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i])) byte = (uint8_t) (_asciiToBinaryTable[std::toupper(hexStringCopy[i]) - '0'] << 4);
            else continue;
            if(i + 1 < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexStringCopy[i + 1]) - '0'];
            else continue;
            binary.push_back(byte);
        }
    }
    else
    {
        binary.reserve(hexString.size() / 2);
        for(int32_t i = 0; i < (signed)hexString.size(); i += 2)
        {
            uint8_t byte = 0;
            if(i < (signed)hexString.size() && isxdigit(hexString[i])) byte = (uint8_t)(_asciiToBinaryTable[std::toupper(hexString[i]) - '0'] << 4);
            else continue;
            if(i + 1 < (signed)hexString.size() && isxdigit(hexString[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexString[i + 1]) - '0'];
            else continue;
            binary.push_back(byte);
        }
    }
    return binary;
}

std::vector<uint8_t>& HelperFunctions::getUBinary(const std::string& hexString, uint32_t size, std::vector<uint8_t>& binary)
{
    if(hexString.empty()) return binary;
    if(size > hexString.size()) size = (uint32_t)hexString.size();
    if(size % 2 != 0 && !std::isspace(hexString.back()))
    {
        std::string hexStringCopy = hexString.substr(1);
        binary.reserve(size / 2);
        for(int32_t i = 0; i < (signed)size; i += 2)
        {
            uint8_t byte = 0;
            if(i < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i])) byte = (uint8_t) (_asciiToBinaryTable[std::toupper(hexStringCopy[i]) - '0'] << 4);
            else continue;
            if(i + 1 < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexStringCopy[i + 1]) - '0'];
            else continue;
            binary.push_back(byte);
        }
    }
    else
    {
        binary.reserve(size / 2);
        for(int32_t i = 0; i < (signed)size; i += 2)
        {
            uint8_t byte = 0;
            if(i < (signed)hexString.size() && isxdigit(hexString[i])) byte = (uint8_t) (_asciiToBinaryTable[std::toupper(hexString[i]) - '0'] << 4);
            else continue;
            if(i + 1 < (signed)hexString.size() && isxdigit(hexString[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexString[i + 1]) - '0'];
            else continue;
            binary.push_back(byte);
        }
    }
    return binary;
}

std::vector<uint8_t> HelperFunctions::getUBinary(const std::vector<uint8_t>& hexData)
{
    std::vector<uint8_t> binary;
    if(hexData.empty()) return binary;
    binary.reserve(hexData.size() / 2);
    uint8_t byte = 0;
    for(int32_t i = 0; i < (signed)hexData.size(); i += 2)
    {
        byte = 0;
        if(i < (signed)hexData.size() && isxdigit(hexData[i])) byte = (uint8_t) (_asciiToBinaryTable[std::toupper(hexData[i]) - '0'] << 4);
        else continue;
        if(i + 1 < (signed)hexData.size() && isxdigit(hexData[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexData[i + 1]) - '0'];
        else continue;
        binary.push_back(byte);
    }
    return binary;
}

uid_t HelperFunctions::userId(const std::string& username)
{
    if(username.empty()) return -1;
    struct passwd pwd{};
    struct passwd* pwdResult = nullptr;
    int32_t bufferSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if(bufferSize < 0) bufferSize = 16384;
    std::vector<char> buffer(bufferSize);
    getpwnam_r(username.c_str(), &pwd, &buffer.at(0), buffer.size(), &pwdResult);
    if(!pwdResult) return -1;
    return pwd.pw_uid;
}

gid_t HelperFunctions::groupId(const std::string& groupname)
{
    if(groupname.empty()) return -1;
    struct group grp{};
    struct group* grpResult = nullptr;
    int32_t bufferSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if(bufferSize < 0) bufferSize = 16384;
    std::vector<char> buffer(bufferSize);
    getgrnam_r(groupname.c_str(), &grp, &buffer.at(0), buffer.size(), &grpResult);
    if(!grpResult) return -1;
    return grp.gr_gid;
}

bool HelperFunctions::getBigEndian()
{
    static bool bigEndian = isBigEndian();
    return bigEndian;
}

bool HelperFunctions::isBigEndian()
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

std::string HelperFunctions::getGNUTLSCertVerificationError(uint32_t errorCode)
{
    if(errorCode & GNUTLS_CERT_REVOKED) return "Certificate is revoked by its authority.";
    else if(errorCode & GNUTLS_CERT_SIGNER_NOT_FOUND) return "The certificate’s issuer is not known.";
    else if(errorCode & GNUTLS_CERT_SIGNER_NOT_CA) return "The certificate’s signer was not a CA.";
    else if(errorCode & GNUTLS_CERT_INSECURE_ALGORITHM) return "The certificate was signed using an insecure algorithm such as MD2 or MD5. These algorithms have been broken and should not be trusted.";
    else if(errorCode & GNUTLS_CERT_NOT_ACTIVATED) return "The certificate is not yet activated. ";
    else if(errorCode & GNUTLS_CERT_EXPIRED) return "The certificate has expired. ";
    //TODO: Add missing errors, when GNUTLS 3 is in Debian stable
    return "Unknown error code.";
}

bool HelperFunctions::isShortCliCommand(const std::string& command)
{
    int32_t spaceIndex = command.find(' ');
    if(spaceIndex < 0 && command.size() < 4) return true;
    else if(spaceIndex > 0 && spaceIndex < 4) return true;
    return false;
}

bool HelperFunctions::checkCliCommand(const std::string& command, const std::string& longCommand, const std::string& shortCommand1, const std::string& shortCommand2, uint32_t minArgumentCount, std::vector<std::string>& arguments, bool& showHelp)
{
    showHelp = false;

    bool isLongCommand = (command.size() == longCommand.size() || (command.size() > longCommand.size() && command.at(longCommand.size()) == ' ')) && command.compare(0, longCommand.size(), longCommand) == 0;
    bool isShortCommand1 = !shortCommand1.empty() && (command.size() == shortCommand1.size() || (command.size() > shortCommand1.size() && command.at(shortCommand1.size()) == ' ')) && command.compare(0, shortCommand1.size(), shortCommand1) == 0;
    bool isShortCommand2 = !shortCommand2.empty() && (command.size() == shortCommand2.size() || (command.size() > shortCommand2.size() && command.at(shortCommand2.size()) == ' ')) && command.compare(0, shortCommand2.size(), shortCommand2) == 0;
    if(!isLongCommand && !isShortCommand1 && !isShortCommand2) return false;

    std::stringstream stream(command);
    std::string element;
    int32_t offset = isLongCommand ? std::count(longCommand.begin(), longCommand.end(), ' ') : 0;
    int32_t index = 0;
    arguments.reserve(10);
    while(std::getline(stream, element, ' '))
    {
        if(index < 1 + offset)
        {
            index++;
            continue;
        }
        if(element == "help")
        {
            showHelp = true;
            return true;
        }
        arguments.push_back(element);
    }
    if(arguments.size() < minArgumentCount) showHelp = true;
    return true;
}

void* HelperFunctions::memrchr(const void* s, int c, size_t n)
{
    const unsigned char *cp;

    if(n != 0)
    {
        cp = (unsigned char *)s + n;
        do
        {
            if (*(--cp) == (unsigned char)c)
                return((void *)cp);
        } while (--n != 0);
    }
    return((void *)0);
}

}
