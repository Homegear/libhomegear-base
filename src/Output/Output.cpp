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

#include "Output.h"
#include "../BaseLib.h"

namespace BaseLib
{
std::mutex Output::_outputMutex;

std::function<void(int32_t, std::string)>* Output::getErrorCallback()
{
	return _errorCallback;
}

void Output::setErrorCallback(std::function<void(int32_t, std::string)>* errorCallback)
{
	_errorCallback = errorCallback;
}

Output::Output()
{
}

Output::~Output()
{
}

void Output::init(SharedObjects* baseLib)
{
	_bl = baseLib;
	_errorCallback = baseLib->out.getErrorCallback();
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
	std::tm localTime;
	localtime_r(&t, &localTime);
	strftime(&timeString[0], 50, &timeFormat[0], &localTime);
	std::ostringstream timeStream;
	timeStream << timeString << "." << std::setw(3) << std::setfill('0') << milliseconds;
	return timeStream.str();
}

void Output::printThreadPriority()
{
	try
	{
		int32_t policy, error;
		sched_param param;
		if((error = pthread_getschedparam(pthread_self(), &policy, &param)) != 0) printError("Could not get thread priority. Error: " + std::to_string(error));

		printMessage("Thread policy: " + std::to_string(policy) + " Thread priority: " + std::to_string(param.sched_priority));
	}
	catch(const std::exception& ex)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void Output::printBinary(std::vector<unsigned char>& data)
{
	try
	{
		if(data.empty()) return;
		std::ostringstream stringstream;
		stringstream << std::hex << std::setfill('0') << std::uppercase;
		for(std::vector<unsigned char>::iterator i = data.begin(); i != data.end(); ++i)
		{
			stringstream << std::setw(2) << (int32_t)((uint8_t)(*i));
		}
		stringstream << std::dec;
		std::lock_guard<std::mutex> outputGuard(_outputMutex);
		std::cout << stringstream.str() << std::endl;
	}
	catch(const std::exception& ex)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void Output::printBinary(std::shared_ptr<std::vector<char>> data)
{
	try
	{
		if(!data || data->empty()) return;
		std::ostringstream stringstream;
		stringstream << std::hex << std::setfill('0') << std::uppercase;
		for(std::vector<char>::iterator i = data->begin(); i != data->end(); ++i)
		{
			stringstream << std::setw(2) << (int32_t)((uint8_t)(*i));
		}
		stringstream << std::dec;
		std::lock_guard<std::mutex> outputGuard(_outputMutex);
		std::cout << stringstream.str() << std::endl;
	}
	catch(const std::exception& ex)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void Output::printBinary(std::vector<char>& data)
{
	try
	{
		if(data.empty()) return;
		std::ostringstream stringstream;
		stringstream << std::hex << std::setfill('0') << std::uppercase;
		for(std::vector<char>::iterator i = data.begin(); i != data.end(); ++i)
		{
			stringstream << std::setw(2) << (int32_t)((uint8_t)(*i));
		}
		stringstream << std::dec;
		std::lock_guard<std::mutex> outputGuard(_outputMutex);
		std::cout << stringstream.str() << std::endl;
	}
	catch(const std::exception& ex)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void Output::printEx(std::string file, uint32_t line, std::string function, std::string what)
{
	if(_bl && _bl->debugLevel < 2) return;
	std::string error;
	if(!what.empty())
	{
		error = _prefix + "Error in file " + file + " line " + std::to_string(line) + " in function " + function + ": " + what;
		std::lock_guard<std::mutex> outputGuard(_outputMutex);
		std::cout << getTimeString() << " " << error << std::endl;
		std::cerr << getTimeString() << " " << error << std::endl;
	}
	else
	{
		error = _prefix + "Unknown error in file " + file + " line " + std::to_string(line) + " in function " + function + ".";
		std::lock_guard<std::mutex> outputGuard(_outputMutex);
		std::cout << getTimeString() << " " << error << std::endl;
		std::cerr << getTimeString() << " " << error << std::endl;
	}
	if(_errorCallback && *_errorCallback) (*_errorCallback)(2, error);
}

void Output::printCritical(std::string errorString, bool errorCallback)
{
	if(_bl && _bl->debugLevel < 1) return;
	std::string error = _prefix + errorString;
	{
    	std::lock_guard<std::mutex> outputGuard(_outputMutex);
    	std::cout << getTimeString() << " " << error << std::endl;
    	std::cerr << getTimeString() << " " << error << std::endl;
	}
	if(_errorCallback && *_errorCallback && errorCallback) (*_errorCallback)(1, error);
}

void Output::printError(std::string errorString)
{
	if(_bl && _bl->debugLevel < 2) return;
	std::string error = _prefix + errorString;
	{
    	std::lock_guard<std::mutex> outputGuard(_outputMutex);
    	std::cout << getTimeString() << " " << error << std::endl;
    	std::cerr << getTimeString() << " " << error << std::endl;
	}
	if(_errorCallback && *_errorCallback) (*_errorCallback)(2, error);
}

void Output::printWarning(std::string errorString)
{
	if(_bl && _bl->debugLevel < 3) return;
	std::string error = _prefix + errorString;
	{
	    std::lock_guard<std::mutex> outputGuard(_outputMutex);
    	std::cout << getTimeString() << " " << error << std::endl;
    	std::cerr << getTimeString() << " " << error << std::endl;
	}
	if(_errorCallback && *_errorCallback) (*_errorCallback)(3, error);
}

void Output::printInfo(std::string message)
{
	if(_bl && _bl->debugLevel < 4) return;
	std::lock_guard<std::mutex> outputGuard(_outputMutex);
	std::cout << getTimeString() << " " << _prefix << message << std::endl;
}

void Output::printDebug(std::string message, int32_t minDebugLevel)
{
	if(_bl && _bl->debugLevel < minDebugLevel) return;
	std::lock_guard<std::mutex> outputGuard(_outputMutex);
	std::cout << getTimeString() << " " << _prefix << message << std::endl;
}

void Output::printMessage(std::string message, int32_t minDebugLevel, bool errorLog)
{
	if(_bl && _bl->debugLevel < minDebugLevel) return;
	message = _prefix + message;
	std::unique_lock<std::mutex> outputGuard(_outputMutex);
	std::cout << getTimeString() << " " << message << std::endl;
	if(minDebugLevel <= 3 && errorLog)
	{
		std::cerr << getTimeString() << " " << message << std::endl;
		outputGuard.unlock();
		if(_errorCallback && *_errorCallback) (*_errorCallback)(3, message);
	}
}

}
