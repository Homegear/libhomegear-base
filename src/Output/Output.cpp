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

#include "Output.h"
#include "../BaseLib.h"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace BaseLib
{
std::mutex Output::_outputMutex;

std::string Output::getPrefix()
{
    return _prefix;
}

void Output::setPrefix(const std::string& prefix)
{
    _prefix = prefix;
}

void Output::enableStdOutput()
{
    _stdOutput = true;
}

void Output::disableStdOutput()
{
    _stdOutput = false;
}

void Output::setOutputCallback(std::function<void(int32_t, const std::string&)> value)
{
	_outputCallback.swap(value);
}

Output::Output() = default;

Output::~Output() = default;

void Output::init(SharedObjects* baseLib)
{
	_bl = baseLib;
}

std::string Output::getTimeString(int64_t time)
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
	std::tm localTime{};
	localtime_r(&t, &localTime);
	strftime(&timeString[0], 50, &timeFormat[0], &localTime);
	std::ostringstream timeStream;
	timeStream << timeString << "." << std::setw(3) << std::setfill('0') << milliseconds;
	return timeStream.str();
}

void Output::printEx(const std::string& file, uint32_t line, const std::string& function, const std::string& what)
{
	if(_bl && _bl->debugLevel < 2) return;
	std::string error;
	if(!what.empty())
	{
		error = _prefix + "Error in file " + file + " line " + std::to_string(line) + " in function " + function + ": " + what;
        if(_stdOutput)
        {
            std::lock_guard<std::mutex> outputGuard(_outputMutex);
            std::cout << getTimeString() << " " << error << std::endl;
            std::cerr << getTimeString() << " " << error << std::endl;
        }
	}
	else
	{
		error = _prefix + "Unknown error in file " + file + " line " + std::to_string(line) + " in function " + function + ".";
        if(_stdOutput)
        {
            std::lock_guard<std::mutex> outputGuard(_outputMutex);
            std::cout << getTimeString() << " " << error << std::endl;
            std::cerr << getTimeString() << " " << error << std::endl;
        }
	}
	if(_outputCallback) _outputCallback(2, error);
}

void Output::printCritical(const std::string& message)
{
	if(_bl && _bl->debugLevel < 1) return;
	std::string error = _prefix + message;
    if(_stdOutput)
	{
    	std::lock_guard<std::mutex> outputGuard(_outputMutex);
    	std::cout << getTimeString() << " " << error << std::endl;
    	std::cerr << getTimeString() << " " << error << std::endl;
	}
    if(_outputCallback) _outputCallback(1, error);
}

void Output::printError(const std::string& errorString)
{
	if(_bl && _bl->debugLevel < 2) return;
	std::string error = _prefix + errorString;
	if(_stdOutput)
	{
    	std::lock_guard<std::mutex> outputGuard(_outputMutex);
    	std::cout << getTimeString() << " " << error << std::endl;
    	std::cerr << getTimeString() << " " << error << std::endl;
	}
    if(_outputCallback) _outputCallback(2, error);
}

void Output::printWarning(const std::string& errorString)
{
	if(_bl && _bl->debugLevel < 3) return;
	std::string error = _prefix + errorString;
	if(_stdOutput)
	{
	    std::lock_guard<std::mutex> outputGuard(_outputMutex);
    	std::cout << getTimeString() << " " << error << std::endl;
    	std::cerr << getTimeString() << " " << error << std::endl;
	}
    if(_outputCallback) _outputCallback(3, error);
}

void Output::printInfo(const std::string& message)
{
	if(_bl && _bl->debugLevel < 4) return;
    if(_stdOutput)
    {
        std::lock_guard<std::mutex> outputGuard(_outputMutex);
        std::cout << getTimeString() << " " << _prefix << message << std::endl;
    }
    if(_outputCallback) _outputCallback(4, message);
}

void Output::printDebug(const std::string& message, int32_t minDebugLevel)
{
	if(_bl && _bl->debugLevel < minDebugLevel) return;
    if(_stdOutput)
    {
        std::lock_guard<std::mutex> outputGuard(_outputMutex);
        std::cout << getTimeString() << " " << _prefix << message << std::endl;
    }
    if(_outputCallback) _outputCallback(minDebugLevel, message);
}

void Output::printMessage(const std::string& message, int32_t minDebugLevel, bool errorLog)
{
	if(_bl && _bl->debugLevel < minDebugLevel) return;
	std::string prefixedMessage = _prefix + message;

	if(_stdOutput)
    {
        std::lock_guard<std::mutex> outputGuard(_outputMutex);
        std::cout << getTimeString() << " " << prefixedMessage << std::endl;
        if(minDebugLevel <= 3 && errorLog) std::cerr << getTimeString() << " " << prefixedMessage << std::endl;
    }
    if(_outputCallback) _outputCallback(minDebugLevel, prefixedMessage);
}

}
